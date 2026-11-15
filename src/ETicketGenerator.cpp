#include "ETicketGenerator.h"
#include "BookingManager.h"
#include "TrainData.h"
#include "Validation.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <filesystem>
#include <iostream>
#include <cctype>

namespace {

int hexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

std::string decodeField(const std::string& value) {
    std::string result;

    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '%' && i + 2 < value.size()) {
            int high = hexValue(value[i + 1]);
            int low = hexValue(value[i + 2]);

            if (high >= 0 && low >= 0) {
                result += static_cast<char>(high * 16 + low);
                i += 2;
                continue;
            }
        }

        result += value[i];
    }

    return result;
}

}

namespace ETicketGenerator {

bool generateETicket(
    const std::string& pnr,
    std::string& ticketPath,
    std::string& errorMessage) {

    ticketPath.clear();
    errorMessage.clear();

    Booking booking;

    if (!BookingManager::findBooking(pnr, booking)) {
        errorMessage = "Booking not found for the supplied PNR.";
        return false;
    }

    std::string trainError;

    std::vector<Train> trains =
        loadTrains("data/trains.dat", booking.journeyDate, trainError);

    if (!trainError.empty()) {
        errorMessage = trainError;
        return false;
    }

    const Train* selectedTrain = nullptr;

    for (const Train& train : trains) {
        if (train.trainNumber == booking.trainNumber) {
            selectedTrain = &train;
            break;
        }
    }

    if (selectedTrain == nullptr) {
        errorMessage = "Train details could not be found.";
        return false;
    }

    // Validate all records before creating or truncating the ticket file.
    struct TicketPassenger {
        std::string name;
        std::string age;
        std::string gender;
    };
    std::vector<TicketPassenger> passengers;
    std::stringstream passengerStream(booking.passengerDetails);
    std::string passenger;

    while (std::getline(passengerStream, passenger, ';')) {
        const auto firstComma = passenger.find(',');
        const auto secondComma = firstComma == std::string::npos
            ? std::string::npos : passenger.find(',', firstComma + 1);

        if (firstComma == std::string::npos || secondComma == std::string::npos ||
            passenger.find(',', secondComma + 1) != std::string::npos) {
            errorMessage = "Invalid passenger details in booking record.";
            return false;
        }

        TicketPassenger parsed{
            decodeField(passenger.substr(0, firstComma)),
            passenger.substr(firstComma + 1, secondComma - firstComma - 1),
            decodeField(passenger.substr(secondComma + 1))
        };
        std::string nameError;
        const bool validName =
            Validation::isValidPassengerName(parsed.name, nameError);
        int age = 0;
        try {
            std::size_t position = 0;
            age = std::stoi(parsed.age, &position);
            if (position != parsed.age.size()) age = 0;
        } catch (const std::exception&) {
            age = 0;
        }

        // Use the same passenger constraints as the booking input flow.
        if (!validName ||
            age < 1 || age > 120 ||
            (parsed.gender != "Male" && parsed.gender != "Female" &&
             parsed.gender != "Other")) {
            errorMessage = "Invalid passenger details in booking record.";
            return false;
        }
        passengers.push_back(parsed);
    }

    if (!booking.passengerDetails.empty() && booking.passengerDetails.back() == ';') {
        errorMessage = "Invalid passenger details in booking record.";
        return false;
    }

    if (passengers.empty() || booking.passengerCount < 1 ||
        passengers.size() != static_cast<std::size_t>(booking.passengerCount)) {
        errorMessage = "Passenger count mismatch: booking specifies " +
            std::to_string(booking.passengerCount) + " but parsed " +
            std::to_string(passengers.size()) + " valid passenger records.";
        return false;
    }

    std::error_code filesystemError;
    std::filesystem::create_directories("tickets", filesystemError);

    if (filesystemError) {
        errorMessage = "Could not create tickets directory.";
        return false;
    }

    ticketPath = "tickets/ETicket_" + pnr + ".txt";

    std::ofstream ticket(ticketPath);

    if (!ticket.is_open()) {
        errorMessage = "Could not create e-ticket file.";
        ticketPath.clear();
        return false;
    }

    ticket << "============================================\n";
    ticket << "          RAILWAY RESERVATION SYSTEM         \n";
    ticket << "                 E-TICKET                    \n";
    ticket << "============================================\n\n";

    ticket << "PNR Number       : " << booking.pnr << '\n';
    ticket << "Booking Status   : " << booking.status << "\n\n";

    ticket << "TRAIN DETAILS\n";
    ticket << "--------------------------------------------\n";
    ticket << "Train Number     : " << selectedTrain->trainNumber << '\n';
    ticket << "Train Name       : " << selectedTrain->trainName << '\n';
    ticket << "Source           : " << selectedTrain->source << '\n';
    ticket << "Destination      : " << selectedTrain->destination << '\n';
    ticket << "Journey Date     : " << booking.journeyDate << '\n';
    ticket << "Departure Time   : " << selectedTrain->departureTime << '\n';
    ticket << "Arrival Time     : " << selectedTrain->arrivalTime << '\n';
    ticket << "Journey Duration : " << selectedTrain->journeyDuration << "\n\n";

    ticket << "PASSENGER DETAILS\n";
    ticket << "--------------------------------------------\n";

    for (std::size_t i = 0; i < passengers.size(); ++i) {
        ticket << "Passenger " << i + 1 << '\n';
        ticket << "Name             : " << passengers[i].name << '\n';
        ticket << "Age              : " << passengers[i].age << '\n';
        ticket << "Gender           : " << passengers[i].gender << "\n\n";
    }

    ticket << "BOOKING DETAILS\n";
    ticket << "--------------------------------------------\n";
    ticket << "Travel Class     : " << booking.travelClass << '\n';
    ticket << "Passenger Count  : " << booking.passengerCount << '\n';
    ticket << "Total Fare       : Rs. "
           << std::fixed << std::setprecision(2)
           << booking.totalFare << "\n\n";

    ticket << "============================================\n";
    ticket << "       Thank you for choosing our service!  \n";
    ticket << "============================================\n";

    if (!ticket) {
        ticket.close();
        std::filesystem::remove(ticketPath);
        ticketPath.clear();
        errorMessage = "Failed while writing e-ticket.";
        return false;
    }

    ticket.close();
    return true;
}

bool displayETicket(
    const std::string& pnr,
    std::string& errorMessage) {

    errorMessage.clear();

    if (pnr.size() != 10) {
        errorMessage = "Invalid PNR format.";
        return false;
    }

    for (unsigned char c : pnr) {
        if (!std::isdigit(c)) {
            errorMessage = "Invalid PNR format.";
            return false;
        }
    }

    std::ifstream ticket("tickets/ETicket_" + pnr + ".txt");

    if (!ticket.is_open()) {
        errorMessage = "E-ticket not found. Generate the ticket first.";
        return false;
    }

    std::cout << ticket.rdbuf();

    if (ticket.bad()) {
        errorMessage = "Failed to read e-ticket.";
        return false;
    }

    return true;
}

}
