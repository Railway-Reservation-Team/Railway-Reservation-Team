#include "BookingManager.h"
#include "TrainData.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <iomanip>
#include <cmath>
#include <cstdio>
#include <string>
#include <cctype>

namespace BookingManager {

static const std::string BOOKING_FILE = "data/bookings.dat";
static const std::string TRAIN_FILE = "data/trains.dat";

// Encode special characters so passenger details remain safe in pipe-separated records.
std::string encodeField(const std::string& value) {
    std::ostringstream result;
    const char* hex = "0123456789ABCDEF";

    for (unsigned char c : value) {
        if (std::isalnum(c) || c == ' ' || c == '-' || c == '_' || c == '.') {
            result << static_cast<char>(c);
        } else {
            result << '%' << hex[c >> 4] << hex[c & 15];
        }
    }

    return result.str();
}

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

bool isPNRUsed(const std::string& candidate) {
    std::ifstream file(BOOKING_FILE);
    std::string line;

    while (std::getline(file, line)) {
        std::size_t separator = line.find('|');

        if (separator != std::string::npos &&
            line.substr(0, separator) == candidate) {
            return true;
        }
    }

    return false;
}

bool generateUniquePNR(std::string& pnr) {
    std::random_device rd;
    std::mt19937_64 generator(rd());

    std::uniform_int_distribution<unsigned long long> distribution(
        1000000000ULL, 9999999999ULL
    );

    for (int attempt = 0; attempt < 1000; ++attempt) {
        std::string candidate = std::to_string(distribution(generator));

        if (!isPNRUsed(candidate)) {
            pnr = candidate;
            return true;
        }
    }

    return false;
}

bool findBooking(const std::string& requestedPNR, Booking& booking) {
    std::ifstream file(BOOKING_FILE);

    if (!file.is_open()) {
        return false;
    }

    std::string line;

    while (std::getline(file, line)) {
        std::stringstream stream(line);
        std::vector<std::string> fields;
        std::string field;

        while (std::getline(stream, field, '|')) {
            fields.push_back(field);
        }

        // Current booking format:
        // PNR|Train|Date|Class|PassengerDetails|Count|Fare|Status
        if (fields.size() != 8 || fields[0] != requestedPNR) {
            continue;
        }

        try {
            booking.pnr = fields[0];
            booking.trainNumber = fields[1];
            booking.journeyDate = fields[2];
            booking.travelClass = fields[3];
            booking.passengerDetails = fields[4];
            booking.passengerCount = std::stoi(fields[5]);
            booking.totalFare = std::stod(fields[6]);
            booking.status = fields[7];
            return true;
        } catch (...) {
            return false;
        }
    }

    return false;
}

bool createBooking(
    const Train& selectedTrain,
    const std::string& travelClass,
    const std::vector<Passenger>& passengers,
    double totalFare,
    std::string& pnr,
    std::string& errorMessage) {

    errorMessage.clear();

    if (passengers.empty() || passengers.size() > 6 ||
        !std::isfinite(totalFare) || totalFare <= 0) {
        errorMessage = "Invalid booking details.";
        return false;
    }

    for (const Passenger& passenger : passengers) {
        if (passenger.name.find_first_not_of(" \t") == std::string::npos ||
            passenger.age < 1 || passenger.age > 120 ||
            (passenger.gender != "Male" &&
             passenger.gender != "Female" &&
             passenger.gender != "Other")) {
            errorMessage = "Invalid passenger name, age or gender.";
            return false;
        }
    }

    if (!generateUniquePNR(pnr)) {
        errorMessage = "Could not generate a unique PNR.";
        return false;
    }

    std::ifstream input(TRAIN_FILE);

    if (!input.is_open()) {
        errorMessage = "Cannot open train data.";
        return false;
    }

    std::vector<Train> trains;
    std::string line;
    bool found = false;

    while (std::getline(input, line)) {
        if (line.empty()) continue;

        Train train;

        if (!parseTrainRecord(line, train)) {
            errorMessage = "Invalid train record found.";
            return false;
        }

        if (train.trainNumber == selectedTrain.trainNumber &&
            train.journeyDate == selectedTrain.journeyDate) {

            int* available = nullptr;

            if (travelClass == "Sleeper")
                available = &train.sleeperSeatsAvailable;
            else if (travelClass == "AC 3-Tier")
                available = &train.ac3TierSeatsAvailable;
            else if (travelClass == "AC 2-Tier")
                available = &train.ac2TierSeatsAvailable;
            else if (travelClass == "Chair Car")
                available = &train.chairCarSeatsAvailable;

            if (!available || *available < static_cast<int>(passengers.size())) {
                errorMessage = "Invalid class or insufficient seats.";
                return false;
            }

            *available -= static_cast<int>(passengers.size());
            found = true;
        }

        trains.push_back(train);
    }

    input.close();

    if (!found) {
        errorMessage = "Selected train was not found.";
        return false;
    }

    std::ifstream originalFile(TRAIN_FILE);

    if (!originalFile.is_open()) {
        errorMessage = "Cannot read original train data for rollback.";
        return false;
    }

    std::stringstream originalContent;
    originalContent << originalFile.rdbuf();
    originalFile.close();

    std::ostringstream updatedData;

    for (const Train& train : trains) {
        updatedData << train.trainNumber << '|'
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
                    << train.journeyDate << '\n';
    }

    {
        std::ofstream temp("data/trains.tmp", std::ios::trunc);

        if (!temp) {
            errorMessage = "Cannot create temporary train data.";
            return false;
        }

        temp << updatedData.str();

        if (!temp) {
            errorMessage = "Failed to write updated train data.";
            return false;
        }
    }

    if (std::rename("data/trains.tmp", TRAIN_FILE.c_str()) != 0) {
        std::remove("data/trains.tmp");
        errorMessage = "Could not update seat inventory.";
        return false;
    }

    std::ostringstream passengerData;

    for (std::size_t i = 0; i < passengers.size(); ++i) {
        if (i > 0) passengerData << ';';

        passengerData << encodeField(passengers[i].name) << ','
                      << passengers[i].age << ','
                      << encodeField(passengers[i].gender);
    }

    bool saved = false;

    {
        std::ofstream bookings(BOOKING_FILE, std::ios::app);

        if (bookings.is_open()) {
            bookings << pnr << '|'
                     << selectedTrain.trainNumber << '|'
                     << selectedTrain.journeyDate << '|'
                     << travelClass << '|'
                     << passengerData.str() << '|'
                     << passengers.size() << '|'
                     << std::fixed << std::setprecision(2) << totalFare
                     << "|Confirmed\n";

            saved = static_cast<bool>(bookings);
        }
    }

    if (!saved) {
        std::ofstream restore(TRAIN_FILE, std::ios::trunc);
        restore << originalContent.str();
        restore.close();

        errorMessage = "Could not save booking. Seat allocation rolled back.";
        return false;
    }

    return true;
}

}
