#include "BookingManager.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <cctype>
#include <thread>
#include <set>

namespace fs = std::filesystem;

int passed = 0;
int failed = 0;

void test(const std::string& name, bool condition) {
    if (condition) {
        std::cout << "[PASS] " << name << '\n';
        ++passed;
    } else {
        std::cout << "[FAIL] " << name << '\n';
        ++failed;
    }
}

int main() {
    fs::path originalPath = fs::current_path();
    fs::path testPath = fs::temp_directory_path() / "railway_pnr_tests";

    fs::remove_all(testPath);
    fs::create_directories(testPath / "data");
    fs::current_path(testPath);

    {
        std::ofstream trainFile("data/trains.dat");
        trainFile << "12627|Karnataka Express|Bengaluru|New Delhi|20:00|10:30|14h 30m|1200|1800|2400|900|100|80|60|50|2026-11-15\n";
    }

    Train train;
    train.trainNumber = "12627";
    train.trainName = "Karnataka Express";
    train.source = "Bengaluru";
    train.destination = "New Delhi";
    train.journeyDate = "2026-11-15";

    std::vector<Passenger> passengers = {
        {"Dinesh, Kumar", 20, "Male"},
        {"Test Passenger", 22, "Other"}
    };

    std::string pnr;
    std::string error;

    bool created = BookingManager::createBooking(
        train, "Sleeper", passengers, 2400, pnr, error
    );

    test("Valid booking created", created);

    bool validPNR = pnr.size() == 10;

    for (char c : pnr) {
        validPNR = validPNR && std::isdigit(
            static_cast<unsigned char>(c)
        );
    }

    test("Generated PNR contains exactly 10 digits", validPNR);

    Booking booking;

    bool retrieved = created &&
        BookingManager::findBooking(pnr, booking);

    test("Booking retrieved using PNR", retrieved);

    test("Retrieved train number is correct",
         retrieved && booking.trainNumber == "12627");

    test("Retrieved passenger count is correct",
         retrieved && booking.passengerCount == 2);

    test("Passenger details are decoded correctly",
         retrieved &&
         booking.passengerDetails.find("Dinesh, Kumar") != std::string::npos);

    test("Retrieved fare is correct",
         retrieved && booking.totalFare == 2400.0);

    test("Retrieved booking status is Confirmed",
         retrieved && booking.status == "Confirmed");

    test("Unknown PNR is rejected",
         !BookingManager::findBooking("1234567890", booking));

    std::vector<Passenger> invalidPassengers = {
        {"", 20, "Male"}
    };

    std::string invalidPNR;

    test("Invalid passenger booking rejected",
         !BookingManager::createBooking(
             train, "Sleeper", invalidPassengers,
             1200, invalidPNR, error
         ));

    std::string secondPNR;
    bool secondCreated = BookingManager::createBooking(
        train, "Sleeper", passengers, 2400, secondPNR, error
    );

    test("Second booking receives a different PNR",
         secondCreated && secondPNR != pnr);

    // Concurrent booking and PNR collision test
    const int threadCount = 12;

    std::vector<std::string> concurrentPNRs(threadCount);
    std::vector<int> concurrentResults(threadCount, 0);
    std::vector<std::thread> workers;

    for (int i = 0; i < threadCount; ++i) {
        workers.emplace_back([&, i]() {
            std::string threadError;

            concurrentResults[i] =
                BookingManager::createBooking(
                    train,
                    "Sleeper",
                    passengers,
                    2400,
                    concurrentPNRs[i],
                    threadError
                ) ? 1 : 0;
        });
    }

    for (auto& worker : workers) {
        worker.join();
    }

    int successfulBookings = 0;
    std::set<std::string> uniquePNRs;

    for (int i = 0; i < threadCount; ++i) {
        successfulBookings += concurrentResults[i];
        if (concurrentResults[i]) {
            uniquePNRs.insert(concurrentPNRs[i]);
        }
    }

    test("All concurrent bookings succeed",
         successfulBookings == threadCount);

    test("Concurrent bookings receive unique PNRs",
         uniquePNRs.size() == static_cast<std::size_t>(threadCount));

    std::cout << "\n========== PNR TEST SUMMARY ==========\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "Total: " << passed + failed << '\n';

    fs::current_path(originalPath);
    fs::remove_all(testPath);

    return failed == 0 ? 0 : 1;
}
