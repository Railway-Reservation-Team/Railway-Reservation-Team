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

    test("Valid existing PNR accepted",
         BookingManager::validatePNR(pnr));

    test("Short PNR rejected",
         !BookingManager::validatePNR("12345"));

    test("Long PNR rejected",
         !BookingManager::validatePNR("12345678901"));

    test("Alphabetic PNR rejected",
         !BookingManager::validatePNR("12345ABCDE"));

    test("Special characters in PNR rejected",
         !BookingManager::validatePNR("12345@7890"));

    test("Unknown 10-digit PNR rejected",
         !BookingManager::validatePNR("9999999999"));

    test("Empty PNR rejected",
         !BookingManager::validatePNR(""));

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

    // Passenger booking events must be timestamped without exposing passenger data.
    {
        std::ifstream auditFile("data/admin_audit.log");
        std::string audit(
            (std::istreambuf_iterator<char>(auditFile)),
            std::istreambuf_iterator<char>()
        );
        test("Confirmed passenger booking is recorded in the audit log",
             audit.find("PASSENGER_BOOKING_CONFIRMED") != std::string::npos);
        test("Passenger names are not written to the transaction audit log",
             audit.find("Dinesh, Kumar") == std::string::npos &&
             audit.find("Test Passenger") == std::string::npos);
    }

    // Waitlisted booking and deactivated-train rejection.
    {
        // Deactivated train (active flag "0") must be rejected.
        {
            std::ofstream trainFile("data/trains.dat", std::ios::trunc);
            trainFile
                << "WAIT100|Waitlist Express|Bengaluru|Chennai|08:00|14:00|6h|"
                << "1000|1500|2000|800|2|2|2|2|2026-12-20|0\n";
        }
        Train deactivatedTrain;
        deactivatedTrain.trainNumber = "WAIT100";
        deactivatedTrain.trainName = "Waitlist Express";
        deactivatedTrain.source = "Bengaluru";
        deactivatedTrain.destination = "Chennai";
        deactivatedTrain.journeyDate = "2026-12-20";

        std::vector<Passenger> twoPassengers = {
            {"Wait One", 25, "Male"},
            {"Wait Two", 26, "Female"}
        };
        std::string deactivatedPNR;
        std::string deactivatedError;

        test("Deactivated train booking rejected",
             !BookingManager::createBooking(
                 deactivatedTrain, "Sleeper", twoPassengers,
                 2000.0, deactivatedPNR, deactivatedError));

        // Reactivate the train with only 2 sleeper seats.
        {
            std::ofstream trainFile("data/trains.dat", std::ios::trunc);
            trainFile
                << "WAIT100|Waitlist Express|Bengaluru|Chennai|08:00|14:00|6h|"
                << "1000|1500|2000|800|2|2|2|2|2026-12-20|1\n";
        }
        Train activeTrain = deactivatedTrain;
        activeTrain.active = true;

        std::string firstPnr;
        std::string firstError;
        bool firstCreated = BookingManager::createBooking(
            activeTrain, "Sleeper", twoPassengers, 2000.0, firstPnr, firstError);
        test("First booking on 2-seat train confirmed", firstCreated);

        Booking firstBooking;
        test("First booking retrieved",
             firstCreated && BookingManager::findBooking(firstPnr, firstBooking));
        test("First booking status Confirmed",
             firstCreated && firstBooking.status == "Confirmed");

        // Second booking of 2 passengers exhausts seats -> waitlisted.
        std::string secondPnr;
        std::string secondError;
        bool secondCreated = BookingManager::createBooking(
            activeTrain, "Sleeper", twoPassengers, 2000.0, secondPnr, secondError);
        test("Second booking waitlisted when seats exhausted",
             secondCreated && !secondPnr.empty());

        Booking secondBooking;
        test("Waitlisted booking retrieved",
             secondCreated && BookingManager::findBooking(secondPnr, secondBooking));
        test("Waitlisted booking status is Waitlisted",
             secondCreated && secondBooking.status == "Waitlisted");
        test("Waitlisted booking has queue position 1",
             secondCreated && secondBooking.waitlistPosition == 1);

        test("Waitlisted booking recorded in audit log",
             []{
                 std::ifstream auditFile("data/admin_audit.log");
                 std::string audit(
                     (std::istreambuf_iterator<char>(auditFile)),
                     std::istreambuf_iterator<char>()
                 );
                 return audit.find("PASSENGER_BOOKING_WAITLISTED") != std::string::npos;
             }());
    }

    std::cout << "\n========== PNR TEST SUMMARY ==========\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "Total: " << passed + failed << '\n';

    fs::current_path(originalPath);
    fs::remove_all(testPath);

    return failed == 0 ? 0 : 1;
}
