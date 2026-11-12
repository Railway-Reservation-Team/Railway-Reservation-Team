
#include "../src/BookingManager.h"
#include "../src/TrainData.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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

    assert(parseTrainRecord(trainRecord, train));
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

    assert(created);
    assert(pnr.size() == 10);
    std::cout << "[PASS] Booking creation and PNR generation\n";

    // Test existing booking retrieval.
    Booking booking;
    bool retrieved = BookingManager::findBooking(pnr, booking);

    assert(retrieved);
    assert(booking.pnr == pnr);
    assert(booking.trainNumber == "99999");
    assert(booking.passengerCount == 2);
    assert(booking.status == "Confirmed");
    std::cout << "[PASS] Booking retrieval using PNR\n";

    // Verify eight-field booking serialization.
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

    assert(fieldCount == 8);
    std::cout << "[PASS] Existing eight-field booking format\n";

    // Verify that two AC 2-Tier seats were deducted.
    std::ifstream updatedTrains("data/trains.dat");
    std::string updatedLine;
    std::getline(updatedTrains, updatedLine);
    updatedTrains.close();

    Train updatedTrain;
    assert(parseTrainRecord(updatedLine, updatedTrain));
    assert(updatedTrain.ac2TierSeatsAvailable == 18);

    std::cout << "[PASS] Seat inventory deduction (20 to 18)\n";


    // Reject booking creation without passengers.
    std::vector<Passenger> emptyPassengers;
    std::string rejectedPnr;

    assert(!BookingManager::createBooking(
        train, "AC 2-Tier", emptyPassengers, 0.0,
        rejectedPnr, error
    ));

    std::cout << "[PASS] Empty passenger booking rejection\n";

    std::cout << "\nALL BOOKING INTEGRATION TESTS PASSED!\n";
    return 0;
}
