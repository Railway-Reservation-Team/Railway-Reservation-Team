#include "ETicketGenerator.h"
#include "BookingManager.h"
#include "TrainData.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <filesystem>
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

std::string trim(const std::string& value) {
    std::size_t start = value.find_first_not_of(" \t\r\n");

    if (start == std::string::npos) {
        return "";
    }

    std::size_t end = value.find_last_not_of(" \t\r\n");

    return value.substr(start, end - start + 1);
}

struct PassengerRecord {
    std::string name;
    int age;
    std::string gender;
};

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

    if (booking.passengerCount <= 0 ||
        booking.passengerDetails.empty() ||
        booking.passengerDetails.back() == ';') {
        errorMessage = "Invalid passenger details in booking record.";
        return false;
    }

    std::vector<PassengerRecord> passengers;
    std::stringstream passengerStream(booking.passengerDetails);
    std::string passenger;

    while (std::getline(passengerStream, passenger, ';')) {
        std::stringstream fields(passenger);

        std::string name;
        std::string ageText;
        std::string gender;
        std::string extra;

        if (!std::getline(fields, name, ',') ||
            !std::getline(fields, ageText, ',') ||
            !std::getline(fields, gender)) {
            errorMessage = "Invalid passenger details in booking record.";
            return false;
        }

        if (std::getline(fields, extra, ',')) {
            errorMessage = "Invalid passenger details in booking record.";
            return false;
        }

        name = trim(decodeField(name));
        ageText = trim(ageText);
        gender = trim(decodeField(gender));

        if (name.empty() || ageText.empty() || gender.empty()) {
            errorMessage = "Invalid passenger details in booking record.";
            return false;
        }

        int age = 0;

        try {
            std::size_t position = 0;
            age = std::stoi(ageText, &position);

            if (position != ageText.size() || age < 1 || age > 120) {
                errorMessage = "Invalid passenger age.";
                return false;
            }
        } catch (...) {
            errorMessage = "Invalid passenger age.";
            return false;
        }

        if (gender != "Male" &&
            gender != "Female" &&
            gender != "Other") {
            errorMessage = "Invalid passenger gender.";
            return false;
        }

        passengers.push_back({name, age, gender});
    }

    if (passengers.size() !=
        static_cast<std::size_t>(booking.passengerCount)) {
        errorMessage = "Passenger count does not match booking record.";
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

    int passengerNumber = 1;

    for (const PassengerRecord& record : passengers) {
        ticket << "Passenger " << passengerNumber++ << '\n';
        ticket << "Name             : " << record.name << '\n';
        ticket << "Age              : " << record.age << '\n';
        ticket << "Gender           : " << record.gender << "\n\n";
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

}
