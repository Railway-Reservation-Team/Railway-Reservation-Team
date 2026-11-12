
#include "../src/Cancellation.h"
#include "../src/BookingManager.h"
#include "../src/TrainData.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cmath>

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

    // Create an isolated train fixture.
    {
        std::ofstream trainFile("data/trains.dat");
        trainFile
            << "TEST101|Test Express|Bengaluru|Chennai|08:00|14:00|6h|"
            << "1000|1500|2000|800|10|10|10|10|2026-12-15\n";
    }

    Train train{};
    train.trainNumber = "TEST101";
    train.journeyDate = "2026-12-15";

    std::vector<Passenger> passengers = {
        {"Alice Smith", 25, "Female"},
        {"Bob Jones", 30, "Male"}
    };

    std::string pnr;
    std::string error;

    // Test 1: Create a confirmed booking.
    bool created = BookingManager::createBooking(
        train, "Sleeper", passengers, 2000.0, pnr, error
    );

    check(created, "Create a booking before cancellation");

    if (!created) {
        std::cout << "Booking error: " << error << '\n';
        return 1;
    }

    // Test 2: Verify that two seats were deducted.
    std::string loadError;
    std::vector<Train> trains = loadTrains(
        "data/trains.dat", "2026-12-15", loadError
    );

    check(
        trains.size() == 1 &&
        trains[0].sleeperSeatsAvailable == 8,
        "Two seats deducted after booking"
    );

    // Test 3: Cancel the valid booking.
    double refund = 0.0;
    double charge = 0.0;

    bool cancelled = Cancellation::cancelBooking(
        pnr, refund, charge, error
    );

    check(cancelled, "Cancel a valid confirmed booking");

    if (!cancelled) {
        std::cout << "Cancellation error: " << error << '\n';
    }

    // Test 4: Verify refund calculation.
    check(
        std::abs(refund - 1500.0) < 0.01,
        "Refund is 75% of the original fare"
    );

    check(
        std::abs(charge - 500.0) < 0.01,
        "Cancellation fee is 25% of the original fare"
    );

    check(
        std::abs((refund + charge) - 2000.0) < 0.01,
        "Refund plus cancellation fee equals original fare"
    );

    // Test 5: Verify booking status.
    Booking booking;
    bool found = BookingManager::findBooking(pnr, booking);

    check(
        found && booking.status == "Cancelled",
        "Booking status changes to Cancelled"
    );

    // Test 6: Verify that seats are restored.
    trains = loadTrains(
        "data/trains.dat", "2026-12-15", loadError
    );

    check(
        trains.size() == 1 &&
        trains[0].sleeperSeatsAvailable == 10,
        "Two seats restored after cancellation"
    );

    // Test 7: Reject duplicate cancellation.
    double secondRefund = 0.0;
    double secondCharge = 0.0;
    std::string secondError;

    check(
        !Cancellation::cancelBooking(
            pnr, secondRefund, secondCharge, secondError
        ),
        "Reject cancellation of an already cancelled booking"
    );

    // Test 8: Verify duplicate cancellation does not restore seats again.
    trains = loadTrains(
        "data/trains.dat", "2026-12-15", loadError
    );

    check(
        trains.size() == 1 &&
        trains[0].sleeperSeatsAvailable == 10,
        "Duplicate cancellation does not restore seats again"
    );

    // Test 9: Reject an invalid PNR.
    double invalidRefund = 0.0;
    double invalidCharge = 0.0;
    std::string invalidError;

    check(
        !Cancellation::cancelBooking(
            "0000000000",
            invalidRefund,
            invalidCharge,
            invalidError
        ),
        "Reject cancellation using an invalid PNR"
    );

    // Test 10: Verify cancellation record.
    std::ifstream cancellationFile("data/cancellations.dat");

    check(
        cancellationFile.good() &&
        cancellationFile.peek() != std::ifstream::traits_type::eof(),
        "Persist cancellation record"
    );

    std::cout << "\nCancellation Test Results\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "Total: " << passed + failed << '\n';

    return failed == 0 ? 0 : 1;
}
