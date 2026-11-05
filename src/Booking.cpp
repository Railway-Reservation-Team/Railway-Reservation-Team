
#include "Booking.h"

#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <chrono>

namespace {
    bool pnrExists(const std::string& pnr) {
        std::ifstream file("data/bookings.dat");
        std::string line;

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string existingPNR;

            if (std::getline(ss, existingPNR, '|')) {
                if (existingPNR == pnr) {
                    return true;
                }
            }
        }

        return false;
    }
}

std::string generateUniquePNR() {
    static std::mt19937_64 generator(
        static_cast<unsigned long long>(
            std::chrono::high_resolution_clock::now()
                .time_since_epoch().count()
        )
    );

    static std::uniform_int_distribution<unsigned long long>
        distribution(1000000000ULL, 9999999999ULL);

    std::string pnr;

    do {
        pnr = std::to_string(distribution(generator));
    } while (pnrExists(pnr));

    return pnr;
}

bool saveBooking(const Booking& booking, std::string& errorMessage) {
    errorMessage.clear();

    if (booking.pnr.empty()) {
        errorMessage = "Invalid PNR.";
        return false;
    }

    if (pnrExists(booking.pnr)) {
        errorMessage = "Duplicate PNR. Please try again.";
        return false;
    }

    std::ofstream file("data/bookings.dat", std::ios::app);

    if (!file.is_open()) {
        errorMessage = "Unable to open booking data file.";
        return false;
    }

    file << booking.pnr << '|'
         << booking.passengerName << '|'
         << booking.trainNumber << '|'
         << booking.trainName << '|'
         << booking.source << '|'
         << booking.destination << '|'
         << booking.journeyDate << '|'
         << booking.travelClass << '|'
         << booking.numberOfPassengers << '|'
         << std::fixed << std::setprecision(2)
         << booking.totalFare << '\n';

    if (!file.good()) {
        errorMessage = "Failed to save booking details.";
        return false;
    }

    return true;
}
