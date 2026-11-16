
#include "../src/BookingManager.h"
#include "../src/TrainData.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int passed = 0;
int failed = 0;

void check(const std::string& name, bool condition) {
    if (condition) {
        std::cout << "[PASS] " << name << '\n';
        ++passed;
    } else {
        std::cout << "[FAIL] " << name << '\n';
        ++failed;
    }
}

int main() {
    std::cout << "Starting Booking Integration Tests...\n";

    // Prepare isolated test train data.
    std::ofstream trainFile("data/trains.dat");
    trainFile << "99999|Test Express|Bengaluru|Chennai|10:00|18:00|8h|"
              << "500|800|1000|400|20|20|20|20|2026-11-01\n";
    trainFile.close();

    // Start with an empty test booking file.
    std::ofstream bookingFile("data/bookings.dat");
    bookingFile.close();

    Train train;
    std::string trainRecord =
        "99999|Test Express|Bengaluru|Chennai|10:00|18:00|8h|"
        "500|800|1000|400|20|20|20|20|2026-11-01";

    check("Train record parsing", parseTrainRecord(trainRecord, train));
    std::cout << "[PASS] Train record parsing\n";

    std::vector<Passenger> passengers;
    passengers.push_back({"Test Passenger", 20, "Male"});
    passengers.push_back({"Second Passenger", 21, "Female"});

    std::string pnr;
    std::string error;

    // Test actual booking creation.
    bool created = BookingManager::createBooking(
        train, "AC 2-Tier", passengers, 2000.0, pnr, error
    );

    check("Booking creation", created);
    check("PNR contains 10 digits", pnr.size() == 10);
    std::cout << "[PASS] Booking creation and PNR generation\n";

    // Test existing booking retrieval.
    Booking booking;
    bool retrieved = BookingManager::findBooking(pnr, booking);

    check("Booking retrieval", retrieved);
    check("Retrieved PNR matches", booking.pnr == pnr);
    check("Retrieved train number matches", booking.trainNumber == "99999");
    check("Retrieved passenger count matches", booking.passengerCount == 2);
    check("Booking status is confirmed", booking.status == "Confirmed");
    std::cout << "[PASS] Booking retrieval using PNR\n";

    // Verify the new nine-field format includes a waitlist-position slot.
    std::ifstream savedBookings("data/bookings.dat");
    std::string savedLine;
    std::getline(savedBookings, savedLine);
    savedBookings.close();

    std::stringstream parser(savedLine);
    std::string field;
    int fieldCount = 0;

    while (std::getline(parser, field, '|')) {
        ++fieldCount;
    }

    check(
        "Booking serialization has nine fields including waitlist position",
        fieldCount == 9
    );

    // Verify that two AC 2-Tier seats were deducted.
    std::ifstream updatedTrains("data/trains.dat");
    std::string updatedLine;
    std::getline(updatedTrains, updatedLine);
    updatedTrains.close();

    Train updatedTrain;
    check("Updated train record parses", parseTrainRecord(updatedLine, updatedTrain));
    check("Two seats deducted", updatedTrain.ac2TierSeatsAvailable == 18);



    // Reject booking creation without passengers.
    std::vector<Passenger> emptyPassengers;
    std::string rejectedPnr;

    check("Empty passenger booking rejected", !BookingManager::createBooking(
        train, "AC 2-Tier", emptyPassengers, 1000.0,
        rejectedPnr, error
    ));


    // Reject bookings exceeding the maximum passenger limit.
    std::vector<Passenger> tooManyPassengers(7, {"Test Passenger", 20, "Male"});
    std::string rejectedLimitPnr;

    check("Maximum passenger limit enforced", !BookingManager::createBooking(
        train, "AC 2-Tier", tooManyPassengers, 7000.0,
        rejectedLimitPnr, error
    ));





    // Reject invalid passenger details.
    std::vector<Passenger> invalidName = {{"   ", 20, "Male"}};
    std::string invalidPnr;

    check("Blank passenger name rejected", !BookingManager::createBooking(
        train, "AC 2-Tier", invalidName, 1000.0, invalidPnr, error
    ));

    std::vector<Passenger> invalidAge = {{"Test Passenger", 0, "Male"}};
    check("Invalid passenger age rejected", !BookingManager::createBooking(
        train, "AC 2-Tier", invalidAge, 1000.0, invalidPnr, error
    ));

    std::vector<Passenger> invalidGender = {{"Test Passenger", 20, "Unknown"}};
    check("Invalid passenger gender rejected", !BookingManager::createBooking(
        train, "AC 2-Tier", invalidGender, 1000.0, invalidPnr, error
    ));

    std::cout << "\n===== BOOKING TEST SUMMARY =====\n";
    std::cout << "Total: " << passed + failed << '\n';
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';

    return failed == 0 ? 0 : 1;
}
