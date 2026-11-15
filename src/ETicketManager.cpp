
#include "ETicketManager.h"
#include "TrainData.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cstdio>
#include <cctype>
#include <vector>

namespace ETicketManager {

static const std::string BOOKING_FILE = "data/bookings.dat";
static const std::string TRAIN_FILE = "data/trains.dat";
static const std::string TICKET_DIRECTORY = "generated_tickets";

// Decode one percent-encoded passenger field.
static bool decodeField(
    const std::string& encoded,
    std::string& decoded) {

    decoded.clear();

    const auto hexValue = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return -1;
    };

    for (std::size_t i = 0; i < encoded.size(); ++i) {
        if (encoded[i] == '%') {
            if (i + 2 >= encoded.size()) {
                return false;
            }

            const int high = hexValue(encoded[i + 1]);
            const int low = hexValue(encoded[i + 2]);

            if (high < 0 || low < 0) {
                return false;
            }

            decoded += static_cast<char>(high * 16 + low);
            i += 2;
        } else {
            decoded += encoded[i];
        }
    }

    return true;
}

// Split a string using a delimiter.
static std::vector<std::string> split(
    const std::string& value,
    char delimiter) {

    std::vector<std::string> parts;
    std::stringstream stream(value);
    std::string part;

    while (std::getline(stream, part, delimiter)) {
        parts.push_back(part);
    }

    return parts;
}

// Decode passenger information.
bool decodePassengers(
    const std::string& encodedDetails,
    std::vector<Passenger>& passengers,
    std::string& errorMessage) {

    passengers.clear();
    errorMessage.clear();

    if (encodedDetails.empty()) {
        errorMessage = "Passenger details are empty.";
        return false;
    }

    const std::vector<std::string> passengerRecords =
        split(encodedDetails, ';');

    if (passengerRecords.empty() || passengerRecords.size() > 6) {
        errorMessage = "Invalid passenger count.";
        return false;
    }

    for (const std::string& record : passengerRecords) {
        const std::vector<std::string> fields = split(record, ',');

        if (fields.size() != 3) {
            errorMessage = "Invalid passenger record.";
            return false;
        }

        Passenger passenger;

        if (!decodeField(fields[0], passenger.name) ||
            !decodeField(fields[2], passenger.gender)) {

            errorMessage = "Invalid encoded passenger details.";
            passengers.clear();
            return false;
        }

        try {
            std::size_t position = 0;
            passenger.age = std::stoi(fields[1], &position);

            if (position != fields[1].size()) {
                errorMessage = "Invalid passenger age.";
                passengers.clear();
                return false;
            }
        } catch (...) {
            errorMessage = "Invalid passenger age.";
            passengers.clear();
            return false;
        }

        if (passenger.name.empty() ||
            passenger.age < 1 ||
            passenger.age > 120 ||
            (passenger.gender != "Male" &&
             passenger.gender != "Female" &&
             passenger.gender != "Other")) {

            errorMessage = "Invalid passenger information.";
            passengers.clear();
            return false;
        }

        passengers.push_back(passenger);
    }

    return true;
}

// Generate an e-ticket from an existing confirmed booking.
bool generateTicket(
    const std::string& pnr,
    ETicket& ticket,
    std::string& errorMessage) {

    errorMessage.clear();

    ticket = ETicket{};

    if (pnr.size() != 10) {
        errorMessage = "Invalid PNR. PNR must contain 10 digits.";
        return false;
    }

    for (unsigned char c : pnr) {
        if (!std::isdigit(c)) {
            errorMessage = "Invalid PNR. PNR must contain only digits.";
            return false;
        }
    }

    Booking booking;

    if (!BookingManager::findBooking(pnr, booking)) {
        errorMessage = "Booking not found for the supplied PNR.";
        return false;
    }

    if (booking.status != "Confirmed") {
        errorMessage =
            "E-ticket cannot be generated for a non-confirmed booking.";
        return false;
    }

    if (booking.passengerCount < 1 ||
        booking.passengerCount > 6 ||
        booking.totalFare <= 0.0) {

        errorMessage = "Invalid booking information.";
        return false;
    }

    std::vector<Passenger> passengers;

    if (!decodePassengers(
            booking.passengerDetails, passengers, errorMessage)) {
        return false;
    }

    if (passengers.size() !=
        static_cast<std::size_t>(booking.passengerCount)) {

        errorMessage = "Passenger count does not match booking.";
        return false;
    }

    // Retrieve train information using train number and journey date.
    std::ifstream trainFile(TRAIN_FILE);

    if (!trainFile.is_open()) {
        errorMessage = "Unable to open train data file.";
        return false;
    }

    Train selectedTrain;
    bool foundTrain = false;
    std::string line;

    while (std::getline(trainFile, line)) {
        if (line.empty()) continue;

        Train train;

        if (!parseTrainRecord(line, train)) {
            errorMessage = "Invalid train record found.";
            return false;
        }

        if (train.trainNumber == booking.trainNumber &&
            train.journeyDate == booking.journeyDate) {

            selectedTrain = train;
            foundTrain = true;
            break;
        }
    }

    trainFile.close();

    if (!foundTrain) {
        errorMessage = "Train details for this booking were not found.";
        return false;
    }

    ticket.pnr = booking.pnr;
    ticket.trainNumber = selectedTrain.trainNumber;
    ticket.trainName = selectedTrain.trainName;
    ticket.source = selectedTrain.source;
    ticket.destination = selectedTrain.destination;
    ticket.journeyDate = selectedTrain.journeyDate;
    ticket.departureTime = selectedTrain.departureTime;
    ticket.arrivalTime = selectedTrain.arrivalTime;
    ticket.travelClass = booking.travelClass;
    ticket.passengers = passengers;
    ticket.totalFare = booking.totalFare;
    ticket.bookingStatus = booking.status;

    return true;
}

// Format the ticket as readable text.
bool formatTicket(
    const ETicket& ticket,
    std::string& ticketText,
    std::string& errorMessage) {

    ticketText.clear();
    errorMessage.clear();

    if (ticket.bookingStatus != "Confirmed") {
        errorMessage = "Only confirmed bookings can have an e-ticket.";
        return false;
    }

    if (ticket.pnr.size() != 10 ||
        ticket.trainNumber.empty() ||
        ticket.trainName.empty() ||
        ticket.source.empty() ||
        ticket.destination.empty() ||
        ticket.travelClass.empty() ||
        ticket.passengers.empty() ||
        ticket.passengers.size() > 6 ||
        ticket.totalFare <= 0.0) {

        errorMessage = "Incomplete or invalid ticket information.";
        return false;
    }

    std::ostringstream output;

    output << "============================================\n"
           << "           RAILWAY E-TICKET\n"
           << "============================================\n\n";

    output << "PNR Number       : " << ticket.pnr << '\n'
           << "Booking Status   : " << ticket.bookingStatus << "\n\n";

    output << "TRAIN DETAILS\n"
           << "--------------------------------------------\n"
           << "Train Name       : " << ticket.trainName << '\n'
           << "Train Number     : " << ticket.trainNumber << '\n'
           << "Source           : " << ticket.source << '\n'
           << "Destination      : " << ticket.destination << '\n'
           << "Journey Date     : " << ticket.journeyDate << '\n'
           << "Departure Time   : " << ticket.departureTime << '\n'
           << "Arrival Time     : " << ticket.arrivalTime << '\n'
           << "Travel Class     : " << ticket.travelClass << "\n\n";

    output << "PASSENGER DETAILS\n"
           << "--------------------------------------------\n";

    for (std::size_t i = 0; i < ticket.passengers.size(); ++i) {
        const Passenger& passenger = ticket.passengers[i];

        output << "Passenger " << (i + 1) << '\n'
               << "Name             : " << passenger.name << '\n'
               << "Age              : " << passenger.age << '\n'
               << "Gender           : " << passenger.gender << "\n\n";
    }

    output << "PAYMENT DETAILS\n"
           << "--------------------------------------------\n"
           << std::fixed << std::setprecision(2)
           << "Total Fare       : Rs. " << ticket.totalFare << "\n\n";

    output << "============================================\n"
           << "Please carry a valid identity document.\n"
           << "Keep your PNR available for enquiry.\n"
           << "============================================\n";

    ticketText = output.str();

    return true;
}

// Export a ticket into generated_tickets.
bool exportTicket(
    const ETicket& ticket,
    std::string& filePath,
    std::string& errorMessage) {

    filePath.clear();

    std::string ticketText;

    if (!formatTicket(ticket, ticketText, errorMessage)) {
        return false;
    }

    // Create the output directory if it does not exist.
    const int directoryResult = std::system(
        "mkdir -p generated_tickets"
    );

    if (directoryResult != 0) {
        errorMessage = "Unable to create ticket output directory.";
        return false;
    }

    filePath = TICKET_DIRECTORY + "/" + ticket.pnr + ".txt";

    std::ofstream file(filePath, std::ios::trunc);

    if (!file.is_open()) {
        errorMessage = "Unable to create e-ticket file.";
        filePath.clear();
        return false;
    }

    file << ticketText;
    file.close();

    if (!file) {
        std::remove(filePath.c_str());
        errorMessage = "Failed to save e-ticket.";
        filePath.clear();
        return false;
    }

    return true;
}

// Display ticket on console.
bool displayTicket(
    const ETicket& ticket,
    std::string& errorMessage) {

    std::string ticketText;

    if (!formatTicket(ticket, ticketText, errorMessage)) {
        return false;
    }

    std::cout << ticketText;

    return true;
}

}
