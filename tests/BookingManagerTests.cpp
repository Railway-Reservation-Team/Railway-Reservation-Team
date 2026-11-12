#include "../src/BookingManager.h"
#include "../src/TrainData.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cctype>
#include <limits>

int passed = 0;
int failed = 0;

void check(bool condition, const std::string& description) {
    if (condition) {
        std::cout << "[PASS] " << description << '\n';
        ++passed;
    } else {
        std::cout << "[FAIL] " << description << '\n';
        ++failed;
    }
}

int main() {
    namespace fs = std::filesystem;

    fs::create_directories("data");

    // Clear previous test data to ensure an isolated test run.
    {
        std::ofstream bookingFile("data/bookings.dat", std::ios::trunc);
    }

    // Isolated test fixture. Run this executable from a temporary directory.
    {
        std::ofstream trainFile("data/trains.dat");
        trainFile
            << "TEST101|Test Express|Bengaluru|Chennai|08:00|14:00|6h|"
            << "1000|1500|2000|800|3|10|10|10|2026-12-15\n";
    }

    Train train;
    train.trainNumber = "TEST101";
    train.journeyDate = "2026-12-15";

    std::vector<Passenger> passengers = {
        {"Alice Smith", 25, "Female"},
        {"Bob Jones", 30, "Male"}
    };

    std::string pnr;
    std::string error;


    // Test 0: Invalid fare validation.
    std::string invalidFarePNR;
    std::string invalidFareError;

    check(
        !BookingManager::createBooking(
            train, "Sleeper", passengers, 0.0,
            invalidFarePNR, invalidFareError
        ),
        "Reject zero fare"
    );

    check(
        !BookingManager::createBooking(
            train, "Sleeper", passengers, -100.0,
            invalidFarePNR, invalidFareError
        ),
        "Reject negative fare"
    );

    check(
        !BookingManager::createBooking(
            train, "Sleeper", passengers,
            std::numeric_limits<double>::quiet_NaN(),
            invalidFarePNR, invalidFareError
        ),
        "Reject NaN fare"
    );

    check(
        !BookingManager::createBooking(
            train, "Sleeper", passengers,
            std::numeric_limits<double>::infinity(),
            invalidFarePNR, invalidFareError
        ),
        "Reject infinite fare"
    );

    // Test 0b: Invalid travel class validation.
    std::string invalidClassPNR;
    std::string invalidClassError;

    check(
        !BookingManager::createBooking(
            train, "First Class", passengers, 2000.0,
            invalidClassPNR, invalidClassError
        ),
        "Reject unsupported travel class"
    );

    check(
        !BookingManager::createBooking(
            train, "", passengers, 2000.0,
            invalidClassPNR, invalidClassError
        ),
        "Reject empty travel class"
    );


    // Test 0c: Passenger name whitespace validation.
    std::string nameTestPNR;
    std::string nameTestError;

    std::vector<Passenger> tabOnlyName = {
        {"\t\t", 25, "Female"}
    };

    check(
        !BookingManager::createBooking(
            train, "Sleeper", tabOnlyName, 1000.0,
            nameTestPNR, nameTestError
        ),
        "Reject passenger name containing only tabs"
    );

    std::vector<Passenger> whitespaceOnlyName = {
        {" \t  \t ", 25, "Female"}
    };

    check(
        !BookingManager::createBooking(
            train, "Sleeper", whitespaceOnlyName, 1000.0,
            nameTestPNR, nameTestError
        ),
        "Reject passenger name containing only spaces and tabs"
    );

    std::vector<Passenger> paddedValidName = {
        {" Alice Smith ", 25, "Female"}
    };

    check(
        BookingManager::createBooking(
            train, "Sleeper", paddedValidName, 1000.0,
            nameTestPNR, nameTestError
        ),
        "Accept a non-empty passenger name with surrounding spaces"
    );

    // Test 1: Multi-passenger booking.
    bool bookingCreated = BookingManager::createBooking(
        train, "Sleeper", passengers, 2000.0, pnr, error
    );

    check(bookingCreated, "Create booking with two passengers");

    // Test 2: PNR format.
    bool validPNR = pnr.size() == 10;

    for (char c : pnr) {
        validPNR = validPNR &&
            std::isdigit(static_cast<unsigned char>(c));
    }

    check(bookingCreated && validPNR, "Generate a unique 10-digit PNR");

    // Test 2b: A second persisted booking must receive a different PNR.
    std::vector<Passenger> secondPassengers = {
        {"Charlie Brown", 28, "Other"}
    };
    std::string secondPNR;
    std::string secondError;
    bool secondBookingCreated = BookingManager::createBooking(
        train, "Sleeper", secondPassengers, 1000.0, secondPNR, secondError
    );
    check(
        secondBookingCreated && secondPNR.size() == 10 && secondPNR != pnr,
        "Generate a different PNR for a second booking"
    );

    Booking secondBooking;
    const bool secondFound = secondBookingCreated &&
        BookingManager::findBooking(secondPNR, secondBooking);
    check(
        secondFound &&
        secondBooking.status == "Waitlisted" &&
        secondBooking.waitlistPosition == 1,
        "Persist an insufficient-seat booking with waitlist position 1"
    );

    std::string thirdPNR;
    std::string thirdError;
    const bool thirdBookingCreated = BookingManager::createBooking(
        train, "Sleeper", secondPassengers, 1000.0, thirdPNR, thirdError
    );
    Booking thirdBooking;
    const bool thirdFound = thirdBookingCreated &&
        BookingManager::findBooking(thirdPNR, thirdBooking);
    check(
        thirdFound &&
        thirdBooking.status == "Waitlisted" &&
        thirdBooking.waitlistPosition == 2,
        "Assign increasing positions to later waitlisted bookings"
    );


    // Test 3: Booking lookup.
    Booking booking;
    bool found = bookingCreated &&
        BookingManager::findBooking(pnr, booking);

    check(found, "Find booking using generated PNR");

    // Test 4: Passenger information persistence.
    check(
        found &&
        booking.passengerCount == 2 &&
        booking.passengerDetails.find("Alice Smith") != std::string::npos &&
        booking.passengerDetails.find("Bob Jones") != std::string::npos,
        "Persist both passengers' details"
    );

    // Test 5: Booking fields.
    check(
        found &&
        booking.trainNumber == "TEST101" &&
        booking.journeyDate == "2026-12-15" &&
        booking.travelClass == "Sleeper" &&
        booking.status == "Confirmed" &&
        booking.totalFare == 2000.0,
        "Persist booking information correctly"
    );

    // Test 6: Invalid PNR lookup.
    Booking missingBooking;
    check(
        !BookingManager::findBooking("0000000000", missingBooking),
        "Reject lookup for an invalid PNR"
    );

    // Test 7: Seat deduction.
    std::string loadError;
    std::vector<Train> updatedTrains = loadTrains(
        "data/trains.dat", "2026-12-15", loadError
    );

    check(
        updatedTrains.size() == 1 &&
        updatedTrains[0].sleeperSeatsAvailable == 0,
        "Deduct seats for confirmed bookings only; waitlisted bookings do not consume inventory"
    );

    // Test 8: Deactivated trains must not accept new bookings.
    {
        std::ofstream trainFile("data/trains.dat", std::ios::trunc);
        trainFile
            << "TEST101|Test Express|Bengaluru|Chennai|08:00|14:00|6h|"
            << "1000|1500|2000|800|7|10|10|10|2026-12-15|0\n";
    }
    std::string inactivePNR;
    std::string inactiveError;
    check(
        !BookingManager::createBooking(
            train, "Sleeper", secondPassengers, 1000.0,
            inactivePNR, inactiveError
        ) && inactiveError.find("deactivated") != std::string::npos,
        "Reject new bookings for deactivated trains"
    );

    // Test 9: Reject more than six passengers.
    std::vector<Passenger> tooManyPassengers(7);
    for (auto& passenger : tooManyPassengers) {
        passenger = {"Test Passenger", 20, "Other"};
    }

    std::string rejectedPNR;
    std::string rejectionMessage;

    check(
        !BookingManager::createBooking(
            train, "Sleeper", tooManyPassengers, 7000.0,
            rejectedPNR, rejectionMessage
        ),
        "Reject bookings with more than six passengers"
    );

    std::cout << "\nBooking Manager Test Results\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';

    return failed == 0 ? 0 : 1;
}
