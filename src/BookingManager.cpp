
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

namespace BookingManager {

// Check whether a PNR already exists in the booking records.
bool isPNRUsed(const std::string& candidate) {
    std::ifstream bookings("data/bookings.dat");

    if (!bookings)
        return false;

    std::string line;

    while (std::getline(bookings, line)) {
        std::size_t separator = line.find('|');

        if (separator != std::string::npos &&
            line.substr(0, separator) == candidate) {
            return true;
        }
    }

    return false;
}

// Generate a unique 10-digit PNR.
bool generateUniquePNR(std::string& pnr) {
    std::random_device rd;
    std::mt19937_64 generator(rd());

    std::uniform_int_distribution<unsigned long long> distribution(
        1000000000ULL, 9999999999ULL
    );

    // Retry if a generated PNR already exists.
    for (int attempt = 0; attempt < 1000; ++attempt) {
        std::string candidate = std::to_string(distribution(generator));

        if (!isPNRUsed(candidate)) {
            pnr = candidate;
            return true;
        }
    }

    return false;
}

bool createBooking(
    const Train& selectedTrain,
    const std::string& travelClass,
    const std::string& passengerName,
    int passengerCount,
    double totalFare,
    std::string& pnr,
    std::string& errorMessage) {

    errorMessage.clear();

    if (passengerName.find_first_not_of(" \t") == std::string::npos ||
        passengerCount < 1 || passengerCount > 6 ||
        !std::isfinite(totalFare) || totalFare <= 0) {
        errorMessage = "Invalid booking details.";
        return false;
    }

    // Generate a unique PNR before changing seat inventory.
    if (!generateUniquePNR(pnr)) {
        errorMessage = "Could not generate a unique PNR. Booking rejected.";
        return false;
    }

    std::ifstream input("data/trains.dat");

    if (!input) {
        errorMessage = "Cannot open train data.";
        return false;
    }

    std::vector<Train> trains;
    std::string line;
    bool found = false;

    while (std::getline(input, line)) {
        if (line.empty())
            continue;

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

            if (!available || *available < passengerCount) {
                errorMessage =
                    "Invalid class or insufficient seats. Booking rejected.";
                return false;
            }

            *available -= passengerCount;
            found = true;
        }

        trains.push_back(train);
    }

    input.close();

    if (!found) {
        errorMessage = "Selected train was not found.";
        return false;
    }

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

    std::ifstream originalFile("data/trains.dat");
    std::stringstream originalContent;
    originalContent << originalFile.rdbuf();
    originalFile.close();

    {
        std::ofstream temp("data/trains.tmp");

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

    if (std::rename("data/trains.tmp", "data/trains.dat") != 0) {
        std::remove("data/trains.tmp");
        errorMessage = "Could not update seat inventory.";
        return false;
    }

    std::ofstream bookings("data/bookings.dat", std::ios::app);

    if (!bookings) {
        std::ofstream restore("data/trains.dat", std::ios::trunc);
        restore << originalContent.str();

        errorMessage =
            "Booking file unavailable. Seat allocation rolled back.";
        return false;
    }

    bookings << pnr << '|'
             << selectedTrain.trainNumber << '|'
             << selectedTrain.journeyDate << '|'
             << travelClass << '|'
             << passengerName << '|'
             << passengerCount << '|'
             << std::fixed << std::setprecision(2) << totalFare
             << "|Confirmed\n";

    bookings.close();

    if (!bookings) {
        std::ofstream restore("data/trains.dat", std::ios::trunc);
        restore << originalContent.str();

        errorMessage =
            "Could not save booking. Seat allocation rolled back.";
        return false;
    }

    return true;
}

}
