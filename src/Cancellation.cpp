
#include "Cancellation.h"
#include "BookingManager.h"
#include "TrainData.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cstdio>

namespace Cancellation {

const std::string BOOKING_FILE = "data/bookings.dat";
const std::string TRAIN_FILE = "data/trains.dat";
const std::string CANCELLATION_FILE = "data/cancellations.dat";

bool readLines(
    const std::string& filename,
    std::vector<std::string>& lines,
    std::string& errorMessage
) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        errorMessage = "Cannot open file: " + filename;
        return false;
    }

    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }

    return true;
}

bool writeLines(
    const std::string& filename,
    const std::vector<std::string>& lines,
    std::string& errorMessage
) {
    std::string tempFile = filename + ".tmp";

    {
        std::ofstream file(tempFile, std::ios::trunc);

        if (!file.is_open()) {
            errorMessage = "Cannot create temporary file.";
            return false;
        }

        for (const std::string& line : lines) {
            file << line << '\n';
        }

        if (!file.good()) {
            errorMessage = "Failed to write temporary file.";
            return false;
        }
    }

    // Replace the original file with the updated file.
    std::remove(filename.c_str());

    if (std::rename(tempFile.c_str(), filename.c_str()) != 0) {
        errorMessage = "Could not update file: " + filename;
        return false;
    }

    return true;
}

bool cancelBooking(
    const std::string& pnr,
    double& refundAmount,
    double& cancellationCharge,
    std::string& errorMessage
) {
    refundAmount = 0.0;
    cancellationCharge = 0.0;
    errorMessage.clear();

    // 1. Find booking using PNR.
    Booking booking;

    if (!BookingManager::findBooking(pnr, booking)) {
        errorMessage = "Booking not found for the given PNR.";
        return false;
    }

    // 2. Check booking status.
    if (booking.status != "Confirmed") {
        errorMessage = "Only confirmed bookings can be cancelled.";
        return false;
    }

    if (booking.passengerCount <= 0 ||
        booking.totalFare <= 0.0) {
        errorMessage = "Invalid booking details.";
        return false;
    }

    // 3. Calculate refund: 80% refund, 20% charge.
    refundAmount = booking.totalFare * 0.80;
    cancellationCharge = booking.totalFare * 0.20;

    // 4. Read existing booking records.
    std::vector<std::string> bookingLines;

    if (!readLines(BOOKING_FILE, bookingLines, errorMessage)) {
        return false;
    }

    bool bookingUpdated = false;

    for (std::string& line : bookingLines) {
        std::stringstream stream(line);
        std::vector<std::string> fields;
        std::string field;

        while (std::getline(stream, field, '|')) {
            fields.push_back(field);
        }

        if (fields.size() == 8 && fields[0] == pnr) {
            if (fields[7] != "Confirmed") {
                errorMessage = "Booking is no longer eligible for cancellation.";
                return false;
            }

            fields[7] = "Cancelled";

            std::ostringstream updatedLine;

            for (std::size_t i = 0; i < fields.size(); ++i) {
                if (i > 0) {
                    updatedLine << '|';
                }

                updatedLine << fields[i];
            }

            line = updatedLine.str();
            bookingUpdated = true;
            break;
        }
    }

    if (!bookingUpdated) {
        errorMessage = "Booking record could not be updated.";
        return false;
    }

    // 5. Read train records.
    std::vector<std::string> trainLines;

    if (!readLines(TRAIN_FILE, trainLines, errorMessage)) {
        return false;
    }

    bool trainUpdated = false;

    for (std::string& line : trainLines) {
        Train train;

        if (!parseTrainRecord(line, train)) {
            errorMessage = "Invalid train record.";
            return false;
        }

        if (train.trainNumber != booking.trainNumber ||
            train.journeyDate != booking.journeyDate) {
            continue;
        }

        int* availableSeats = nullptr;

        if (booking.travelClass == "Sleeper") {
            availableSeats = &train.sleeperSeatsAvailable;
        }
        else if (booking.travelClass == "AC 3-Tier") {
            availableSeats = &train.ac3TierSeatsAvailable;
        }
        else if (booking.travelClass == "AC 2-Tier") {
            availableSeats = &train.ac2TierSeatsAvailable;
        }
        else if (booking.travelClass == "Chair Car") {
            availableSeats = &train.chairCarSeatsAvailable;
        }

        if (availableSeats == nullptr) {
            errorMessage = "Invalid travel class in booking.";
            return false;
        }

        *availableSeats += booking.passengerCount;

        std::ostringstream updatedTrain;

        updatedTrain
            << train.trainNumber << '|'
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

        line = updatedTrain.str();
        trainUpdated = true;
        break;
    }

    if (!trainUpdated) {
        errorMessage = "Train record for this booking was not found.";
        return false;
    }

    // 6. Save updated train availability.
    if (!writeLines(TRAIN_FILE, trainLines, errorMessage)) {
        return false;
    }

    // 7. Save cancelled booking status.
    if (!writeLines(BOOKING_FILE, bookingLines, errorMessage)) {
        errorMessage += " Train availability may need recovery.";
        return false;
    }

    // 8. Record cancellation and refund details.
    {
        std::ofstream file(CANCELLATION_FILE, std::ios::app);

        if (!file.is_open()) {
            errorMessage = "Booking cancelled, but cancellation record could not be saved.";
            return false;
        }

        file << std::fixed << std::setprecision(2)
             << pnr << '|'
             << booking.trainNumber << '|'
             << booking.journeyDate << '|'
             << booking.travelClass << '|'
             << booking.passengerCount << '|'
             << booking.totalFare << '|'
             << cancellationCharge << '|'
             << refundAmount << '\n';

        if (!file.good()) {
            errorMessage = "Booking cancelled, but refund record could not be saved.";
            return false;
        }
    }

    return true;
}

}
