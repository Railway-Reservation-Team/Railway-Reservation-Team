#include "AdminFareReservationManager.h"
#include "TrainData.h"
#include "AtomicFileReplace.h"

#include <fstream>
#include <sstream>
#include <cstdio>
#include <cmath>
#include <iomanip>

namespace {

bool readBookings(
    const std::string& filename,
    std::vector<Booking>& bookings,
    std::string& errorMessage) {

    std::ifstream file(filename);

    if (!file.is_open()) {
        errorMessage = "Unable to open booking file.";
        return false;
    }

    bookings.clear();
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream stream(line);
        std::vector<std::string> fields;
        std::string field;

        while (std::getline(stream, field, '|'))
            fields.push_back(field);

        if (fields.size() != 8) {
            errorMessage = "Invalid booking record found.";
            return false;
        }

        try {
            Booking booking;
            booking.pnr = fields[0];
            booking.trainNumber = fields[1];
            booking.journeyDate = fields[2];
            booking.travelClass = fields[3];
            booking.passengerDetails = fields[4];
            booking.passengerCount = std::stoi(fields[5]);
            booking.totalFare = std::stod(fields[6]);
            booking.status = fields[7];

            if (booking.pnr.empty() ||
                booking.trainNumber.empty() ||
                booking.passengerCount < 1 ||
                !std::isfinite(booking.totalFare) ||
                booking.totalFare <= 0) {
                errorMessage = "Invalid booking details found.";
                return false;
            }

            bookings.push_back(booking);
        } catch (...) {
            errorMessage = "Unable to parse booking record.";
            return false;
        }
    }

    if (file.bad()) {
        errorMessage = "Failed to read booking file.";
        return false;
    }

    return true;
}

bool writeBookings(
    const std::string& filename,
    const std::vector<Booking>& bookings,
    std::string& errorMessage) {

    const std::string temporaryFile = filename + ".tmp";
    std::ofstream file(temporaryFile, std::ios::trunc);

    if (!file.is_open()) {
        errorMessage = "Unable to create temporary booking file.";
        return false;
    }

    for (const Booking& booking : bookings) {
        file << booking.pnr << '|'
             << booking.trainNumber << '|'
             << booking.journeyDate << '|'
             << booking.travelClass << '|'
             << booking.passengerDetails << '|'
             << booking.passengerCount << '|'
             << std::fixed << std::setprecision(2)
             << booking.totalFare << '|'
             << booking.status << '\n';
    }

    file.close();

    if (!file) {
        std::remove(temporaryFile.c_str());
        errorMessage = "Failed to save bookings.";
        return false;
    }

    if (!replaceFileAtomically(temporaryFile, filename)) {
        std::remove(temporaryFile.c_str());
        errorMessage = "Unable to replace booking file.";
        return false;
    }

    return true;
}

bool updateTrainSeats(
    const std::string& filename,
    const Booking& booking,
    std::string& errorMessage) {

    std::ifstream file(filename);

    if (!file.is_open()) {
        errorMessage = "Unable to open train data file.";
        return false;
    }

    std::vector<std::string> records;
    std::string line;
    bool found = false;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        Train train;

        if (!parseTrainRecord(line, train)) {
            errorMessage = "Invalid train record found.";
            return false;
        }

        if (train.trainNumber == booking.trainNumber &&
            train.journeyDate == booking.journeyDate) {

            int* seats = nullptr;

            if (booking.travelClass == "Sleeper")
                seats = &train.sleeperSeatsAvailable;
            else if (booking.travelClass == "AC 3-Tier")
                seats = &train.ac3TierSeatsAvailable;
            else if (booking.travelClass == "AC 2-Tier")
                seats = &train.ac2TierSeatsAvailable;
            else if (booking.travelClass == "Chair Car")
                seats = &train.chairCarSeatsAvailable;

            if (!seats) {
                errorMessage = "Invalid travel class.";
                return false;
            }

            if (*seats > 1000000 - booking.passengerCount) {
                errorMessage = "Seat count would exceed the allowed limit.";
                return false;
            }

            *seats += booking.passengerCount;
            found = true;
        }

        std::ostringstream record;

        record << train.trainNumber << '|'
               << train.trainName << '|'
               << train.source << '|'
               << train.destination << '|'
               << train.departureTime << '|'
               << train.arrivalTime << '|'
               << train.journeyDuration << '|'
               << train.sleeperFare << '|'
               << train.ac3TierFare << '|'
               << train.ac2TierFare << '|'
               << train.chairCarFare << '|'
               << train.sleeperSeatsAvailable << '|'
               << train.ac3TierSeatsAvailable << '|'
               << train.ac2TierSeatsAvailable << '|'
               << train.chairCarSeatsAvailable << '|'
               << train.journeyDate;

        records.push_back(record.str());
    }

    file.close();

    if (!found) {
        errorMessage = "Train for this reservation was not found.";
        return false;
    }

    const std::string temporaryFile = filename + ".tmp";
    std::ofstream output(temporaryFile, std::ios::trunc);

    if (!output.is_open()) {
        errorMessage = "Unable to create temporary train file.";
        return false;
    }

    for (const std::string& record : records)
        output << record << '\n';

    output.close();

    if (!output) {
        std::remove(temporaryFile.c_str());
        errorMessage = "Failed to save train inventory.";
        return false;
    }

    if (!replaceFileAtomically(temporaryFile, filename)) {
        std::remove(temporaryFile.c_str());
        errorMessage = "Unable to replace train data file.";
        return false;
    }

    return true;
}

}

namespace AdminFareReservationManager {

bool updateFares(
    const std::string& filename,
    const std::string& trainNumber,
    const std::string& journeyDate,
    double sleeperFare,
    double ac3TierFare,
    double ac2TierFare,
    double chairCarFare,
    std::string& errorMessage) {

    errorMessage.clear();

    if (!std::isfinite(sleeperFare) ||
        !std::isfinite(ac3TierFare) ||
        !std::isfinite(ac2TierFare) ||
        !std::isfinite(chairCarFare) ||
        sleeperFare < 0 ||
        ac3TierFare < 0 ||
        ac2TierFare < 0 ||
        chairCarFare < 0) {
        errorMessage = "Invalid fare values.";
        return false;
    }

    std::ifstream file(filename);

    if (!file.is_open()) {
        errorMessage = "Unable to open train data file.";
        return false;
    }

    std::vector<std::string> records;
    std::string line;
    bool found = false;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        Train train;

        if (!parseTrainRecord(line, train)) {
            errorMessage = "Invalid train record found.";
            return false;
        }

        if (train.trainNumber == trainNumber &&
            train.journeyDate == journeyDate) {

            train.sleeperFare = sleeperFare;
            train.ac3TierFare = ac3TierFare;
            train.ac2TierFare = ac2TierFare;
            train.chairCarFare = chairCarFare;

            found = true;
        }

        std::ostringstream record;

        record << train.trainNumber << '|'
               << train.trainName << '|'
               << train.source << '|'
               << train.destination << '|'
               << train.departureTime << '|'
               << train.arrivalTime << '|'
               << train.journeyDuration << '|'
               << train.sleeperFare << '|'
               << train.ac3TierFare << '|'
               << train.ac2TierFare << '|'
               << train.chairCarFare << '|'
               << train.sleeperSeatsAvailable << '|'
               << train.ac3TierSeatsAvailable << '|'
               << train.ac2TierSeatsAvailable << '|'
               << train.chairCarSeatsAvailable << '|'
               << train.journeyDate;

        records.push_back(record.str());
    }

    file.close();

    if (!found) {
        errorMessage = "Train not found.";
        return false;
    }

    const std::string temporaryFile = filename + ".tmp";
    std::ofstream output(temporaryFile, std::ios::trunc);

    if (!output.is_open()) {
        errorMessage = "Unable to create temporary train file.";
        return false;
    }

    for (const std::string& record : records)
        output << record << '\n';

    output.close();

    if (!output) {
        std::remove(temporaryFile.c_str());
        errorMessage = "Failed to save updated fares.";
        return false;
    }

    if (!replaceFileAtomically(temporaryFile, filename)) {
        std::remove(temporaryFile.c_str());
        errorMessage = "Unable to replace train data file.";
        return false;
    }

    return true;
}

bool viewReservations(
    const std::string& filename,
    std::vector<Booking>& bookings,
    std::string& errorMessage) {

    bookings.clear();
    errorMessage.clear();

    return readBookings(filename, bookings, errorMessage);
}

bool searchReservation(
    const std::string& filename,
    const std::string& pnr,
    Booking& booking,
    std::string& errorMessage) {

    errorMessage.clear();

    std::vector<Booking> bookings;

    if (!readBookings(filename, bookings, errorMessage))
        return false;

    for (const Booking& current : bookings) {
        if (current.pnr == pnr) {
            booking = current;
            return true;
        }
    }

    errorMessage = "Reservation not found.";
    return false;
}

bool updateReservationStatus(
    const std::string& bookingsFile,
    const std::string& trainsFile,
    const std::string& pnr,
    const std::string& newStatus,
    std::string& errorMessage) {

    errorMessage.clear();

    if (newStatus != "Confirmed" && newStatus != "Cancelled") {
        errorMessage = "Invalid reservation status.";
        return false;
    }

    std::vector<Booking> bookings;

    if (!readBookings(bookingsFile, bookings, errorMessage))
        return false;

    std::size_t index = bookings.size();

    for (std::size_t i = 0; i < bookings.size(); ++i) {
        if (bookings[i].pnr == pnr) {
            index = i;
            break;
        }
    }

    if (index == bookings.size()) {
        errorMessage = "Reservation not found.";
        return false;
    }

    Booking& booking = bookings[index];

    if (booking.status == newStatus) {
        errorMessage = "Reservation already has this status.";
        return false;
    }

    if (booking.status == "Cancelled" && newStatus == "Confirmed") {
        errorMessage = "Cancelled reservations cannot be reactivated.";
        return false;
    }

    if (booking.status != "Confirmed" || newStatus != "Cancelled") {
        errorMessage = "Unsupported reservation status change.";
        return false;
    }

    if (!updateTrainSeats(trainsFile, booking, errorMessage))
        return false;

    booking.status = "Cancelled";

    if (!writeBookings(bookingsFile, bookings, errorMessage)) {
        errorMessage = "Seats were restored, but booking status could not be saved. Manual reconciliation is required.";
        return false;
    }

    return true;
}

}
