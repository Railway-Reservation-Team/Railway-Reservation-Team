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


    // BATCH: BookingManager edge cases

    std::vector<Passenger> emptyPassengers;
    std::vector<Passenger> sevenPassengers(
        7, {"Extra Passenger", 25, "Other"}
    );

    test("Reject booking with no passengers",
         !BookingManager::createBooking(
             train, "Sleeper", emptyPassengers, 1000, invalidPNR, error));

    test("Reject booking with more than six passengers",
         !BookingManager::createBooking(
             train, "Sleeper", sevenPassengers, 7000, invalidPNR, error));

    test("Reject zero booking fare",
         !BookingManager::createBooking(
             train, "Sleeper", passengers, 0, invalidPNR, error));

    test("Reject negative booking fare",
         !BookingManager::createBooking(
             train, "Sleeper", passengers, -100, invalidPNR, error));

    test("Reject NaN booking fare",
         !BookingManager::createBooking(
             train, "Sleeper", passengers,
             std::stod("nan"), invalidPNR, error));

    test("Reject infinite booking fare",
         !BookingManager::createBooking(
             train, "Sleeper", passengers,
             std::stod("inf"), invalidPNR, error));

    test("Reject whitespace-only passenger name",
         !BookingManager::createBooking(
             train, "Sleeper",
             {{"   ", 25, "Male"}}, 1000, invalidPNR, error));

    test("Reject passenger age zero",
         !BookingManager::createBooking(
             train, "Sleeper",
             {{"Invalid Age", 0, "Male"}}, 1000, invalidPNR, error));

    test("Reject passenger age above 120",
         !BookingManager::createBooking(
             train, "Sleeper",
             {{"Invalid Age", 121, "Male"}}, 1000, invalidPNR, error));

    test("Reject invalid passenger gender",
         !BookingManager::createBooking(
             train, "Sleeper",
             {{"Invalid Gender", 25, "Unknown"}}, 1000,
             invalidPNR, error));

    test("Reject invalid travel class",
         !BookingManager::createBooking(
             train, "General", passengers, 1000, invalidPNR, error));

    Train missingTrain = train;
    missingTrain.trainNumber = "NOTFOUND";

    test("Reject booking for a train that does not exist",
         !BookingManager::createBooking(
             missingTrain, "Sleeper", passengers, 1000,
             invalidPNR, error));

    // Booking history supports case-insensitive passenger matching.
    const auto history =
        BookingManager::getBookingHistory("dInEsH, kUmAr");

    test("Find passenger booking history without case sensitivity",
         !history.empty());

    test("Empty passenger name returns empty booking history",
         BookingManager::getBookingHistory("").empty());

    // Check compatibility with legacy eight-field booking records.
    const std::string legacyPNR = "1234567892";
    {
        std::ofstream file("data/bookings.dat", std::ios::app);
        file << legacyPNR
             << "|12627|2026-11-15|Sleeper|Legacy Passenger,30,Male"
             << "|1|1200.00|Confirmed\n";
    }

    Booking legacyBooking;
    test("Read legacy eight-field booking record",
         BookingManager::findBooking(legacyPNR, legacyBooking) &&
         legacyBooking.waitlistPosition == 0);

    // A malformed record should not prevent searching later valid records.
    const std::string malformedPNR = "1234567893";
    {
        std::ofstream file("data/bookings.dat", std::ios::app);
        file << malformedPNR
             << "|12627|2026-11-15|Sleeper|Bad Passenger,XX,Male"
             << "|bad|1200.00|Confirmed|0\n";
    }

    Booking malformedBooking;
    test("Reject booking record with invalid passenger count",
         !BookingManager::findBooking(malformedPNR, malformedBooking));

    // A zero-seat train causes a valid booking to enter the waitlist.
    {
        std::ofstream file("data/trains.dat", std::ios::trunc);
        file << "12627|Karnataka Express|Bengaluru|New Delhi|20:00|10:30|14h 30m"
             << "|1200|1800|2400|900|0|0|0|0|2026-11-15\n";
    }

    std::string waitlistedPNR;
    const bool waitlistedCreated = BookingManager::createBooking(
        train, "Sleeper", passengers, 2400, waitlistedPNR, error);

    Booking waitlistedBooking;
    test("Create booking when seats are unavailable",
         waitlistedCreated &&
         BookingManager::findBooking(waitlistedPNR, waitlistedBooking) &&
         waitlistedBooking.status == "Waitlisted");

    test("First waitlisted booking receives position one",
         waitlistedCreated && waitlistedBooking.waitlistPosition == 1);

    // Invalid stored queue positions must be rejected.
    {
        std::ofstream file("data/bookings.dat", std::ios::app);
        file << "1234567894|12627|2026-11-15|Sleeper|Queue Passenger,25,Other"
             << "|1|1000.00|Waitlisted|invalid\n";
    }

    test("Reject malformed existing waitlist position",
         !BookingManager::createBooking(
             train, "Sleeper", passengers, 2400, invalidPNR, error));


    // BATCH 2: BookingManager uncovered branches

    // A missing train data file must reject booking creation.
    fs::remove("data/trains.dat");
    test("Reject booking when train data file is missing",
         !BookingManager::createBooking(
             train, "Sleeper", passengers, 2400, invalidPNR, error));

    // Malformed train records must be rejected.
    {
        std::ofstream file("data/trains.dat", std::ios::trunc);
        file << "BROKEN|TRAIN|RECORD\n";
    }
    test("Reject booking when train data contains malformed record",
         !BookingManager::createBooking(
             train, "Sleeper", passengers, 2400, invalidPNR, error));

    // A deactivated train must not accept bookings.
    {
        std::ofstream file("data/trains.dat", std::ios::trunc);
        file << "12627|Karnataka Express|Bengaluru|New Delhi|"
             << "20:00|10:30|14h 30m|1200|1800|2400|900|"
             << "100|80|60|50|2026-11-15|0\n";
    }
    test("Reject booking on a deactivated train",
         !BookingManager::createBooking(
             train, "Sleeper", passengers, 2400, invalidPNR, error));

    // Duplicate matching train records must be rejected.
    {
        std::ofstream file("data/trains.dat", std::ios::trunc);
        const std::string record =
            "12627|Karnataka Express|Bengaluru|New Delhi|"
            "20:00|10:30|14h 30m|1200|1800|2400|900|"
            "100|80|60|50|2026-11-15\n";
        file << record << record;
    }
    test("Reject duplicate matching train records",
         !BookingManager::createBooking(
             train, "Sleeper", passengers, 2400, invalidPNR, error));

    // Restore a valid train record for the remaining tests.
    {
        std::ofstream file("data/trains.dat", std::ios::trunc);
        file << "12627|Karnataka Express|Bengaluru|New Delhi|"
             << "20:00|10:30|14h 30m|1200|1800|2400|900|"
             << "100|80|60|50|2026-11-15\n";
    }

    // findBooking rejects a negative stored waitlist position.
    {
        std::ofstream file("data/bookings.dat", std::ios::app);
        file << "1234567895|12627|2026-11-15|Sleeper|"
             << "Negative Queue,25,Other|1|1000.00|Waitlisted|-1\n";
    }
    Booking negativePositionBooking;
    test("Reject booking with negative stored waitlist position",
         !BookingManager::findBooking(
             "1234567895", negativePositionBooking));

    // Booking history should skip malformed records and continue parsing.
    {
        std::ofstream file("data/bookings.dat", std::ios::trunc);
        file << "MALFORMED|RECORD\n";
        file << "1234567896|12627|2026-11-15|Sleeper|"
             << "Dinesh%2C Kumar,20,Male|bad|1200.00|Confirmed|0\n";
        file << "1234567897|12627|2026-11-15|Sleeper|"
             << "Dinesh%2C Kumar,20,Male|1|1200.00|Confirmed|-2\n";
        file << "1234567898|12627|2026-11-15|Sleeper|"
             << "Dinesh%2C Kumar,20,Male|1|1200.00|Confirmed|0\n";
    }

    const auto cleanedHistory =
        BookingManager::getBookingHistory("Dinesh, Kumar");
    test("Booking history skips malformed and invalid records",
         cleanedHistory.size() == 1 &&
         cleanedHistory[0].pnr == "1234567898");

    // Empty or missing booking data returns no history.
    fs::remove("data/bookings.dat");
    test("Missing booking file returns empty history",
         BookingManager::getBookingHistory("Dinesh, Kumar").empty());

    // Invalid PNR format must be rejected before opening the file.
    Booking invalidLookup;
    test("Reject alphabetic PNR in booking lookup",
         !BookingManager::findBooking("ABCDEFGHIJ", invalidLookup));
    test("Reject short PNR in booking lookup",
         !BookingManager::findBooking("12345", invalidLookup));

    // Check each supported travel class can allocate seats.
    const std::vector<std::string> classes = {
        "AC 3-Tier", "AC 2-Tier", "Chair Car"
    };

    for (const std::string& travelClass : classes) {
        {
            std::ofstream file("data/trains.dat", std::ios::trunc);
            file << "12627|Karnataka Express|Bengaluru|New Delhi|"
                 << "20:00|10:30|14h 30m|1200|1800|2400|900|"
                 << "100|80|60|50|2026-11-15\n";
        }
        {
            std::ofstream file("data/bookings.dat", std::ios::trunc);
        }

        std::string classPNR;
        const bool classCreated = BookingManager::createBooking(
            train, travelClass, passengers, 2400, classPNR, error);

        test("Create booking for class " + travelClass, classCreated);
    }

    std::cout << "\n========== PNR TEST SUMMARY ==========\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "Total: " << passed + failed << '\n';

    fs::current_path(originalPath);
    fs::remove_all(testPath);

    return failed == 0 ? 0 : 1;
}
