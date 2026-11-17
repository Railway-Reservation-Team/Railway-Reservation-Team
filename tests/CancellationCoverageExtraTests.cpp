#include "../src/Cancellation.h"
#include "../src/BookingManager.h"
#include "../src/TrainData.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

namespace fs = std::filesystem;

const std::string DATE = "2099-12-15";
const std::string TRAIN_NO = "COV101";
const std::string CONFIRMED_PNR = "1111111111";

void writeTrain(
    const std::string& departure = "08:00",
    int sleeper = 10,
    int ac3 = 10,
    int ac2 = 10,
    int chair = 10
) {
    fs::create_directories("data");

    std::ofstream file("data/trains.dat", std::ios::trunc);
    assert(file.is_open());

    file << TRAIN_NO
         << "|Coverage Test Express|Bengaluru|Chennai|"
         << departure << "|14:00|6h|"
         << "1000|1500|2000|800|"
         << sleeper << "|" << ac3 << "|"
         << ac2 << "|" << chair << "|"
         << DATE << '\n';
}

void writeBooking(
    const std::string& pnr = CONFIRMED_PNR,
    const std::string& travelClass = "Sleeper",
    const std::string& passengerCount = "1",
    const std::string& fare = "1000.00",
    const std::string& status = "Confirmed"
) {
    std::ofstream file("data/bookings.dat", std::ios::trunc);
    assert(file.is_open());

    file << pnr << "|" << TRAIN_NO << "|" << DATE << "|"
         << travelClass << "|Test Passenger,25,Other|"
         << passengerCount << "|" << fare << "|"
         << status << "|0\n";
}

void resetFixture(
    const std::string& departure = "08:00",
    int sleeper = 10,
    int ac3 = 10,
    int ac2 = 10,
    int chair = 10
) {
    fs::create_directories("data");

    writeTrain(departure, sleeper, ac3, ac2, chair);
    writeBooking();

    std::error_code ec;
    fs::remove("data/cancellations.dat", ec);
}

bool cancel(const std::string& pnr = CONFIRMED_PNR) {
    double refund = 0.0;
    double charge = 0.0;
    std::string error;

    const bool result =
        Cancellation::cancelBooking(pnr, refund, charge, error);

    if (!result) {
        std::cout << "Cancellation rejected: " << error << '\n';
    }

    return result;
}

int getAvailableSeats(const std::string& travelClass) {
    std::ifstream file("data/trains.dat");
    std::string line;
    Train train{};

    if (!std::getline(file, line) || !parseTrainRecord(line, train)) {
        return -999;
    }

    if (travelClass == "Sleeper") return train.sleeperSeatsAvailable;
    if (travelClass == "AC 3-Tier") return train.ac3TierSeatsAvailable;
    if (travelClass == "AC 2-Tier") return train.ac2TierSeatsAvailable;
    if (travelClass == "Chair Car") return train.chairCarSeatsAvailable;

    return -999;
}

void testNonConfirmedBooking() {
    resetFixture();
    writeBooking(CONFIRMED_PNR, "Sleeper", "1", "1000.00", "Waitlisted");

    assert(!cancel());
    std::cout << "[PASS] Non-confirmed booking rejected\n";
}

void testInvalidBookingDetails() {
    resetFixture();
    writeBooking(CONFIRMED_PNR, "Sleeper", "0", "1000.00");
    assert(!cancel());
    std::cout << "[PASS] Zero passenger count rejected\n";

    resetFixture();
    writeBooking(CONFIRMED_PNR, "Sleeper", "1", "0");
    assert(!cancel());
    std::cout << "[PASS] Zero fare rejected\n";

    resetFixture();
    writeBooking(CONFIRMED_PNR, "Sleeper", "1", "nan");
    assert(!cancel());
    std::cout << "[PASS] NaN fare rejected\n";

    resetFixture();
    writeBooking(CONFIRMED_PNR, "Sleeper", "1", "inf");
    assert(!cancel());
    std::cout << "[PASS] Infinite fare rejected\n";
}

void testInvalidDepartureRanges() {
    resetFixture("25:00");
    assert(!cancel());
    std::cout << "[PASS] Invalid departure hour rejected\n";

    resetFixture("12:99");
    assert(!cancel());
    std::cout << "[PASS] Invalid departure minute rejected\n";
}

void testSeatRestorationOverflow() {
    resetFixture(
        "08:00",
        std::numeric_limits<int>::max()
    );

    double refund = 0.0;
    double charge = 0.0;
    std::string error;

    assert(!Cancellation::cancelBooking(
        CONFIRMED_PNR, refund, charge, error
    ));

    assert(refund == 0.0);
    assert(charge == 0.0);

    std::cout << "[PASS] Seat restoration overflow rejected\n";
}

void testAllTravelClasses() {
    const std::string classes[] = {
        "Sleeper",
        "AC 3-Tier",
        "AC 2-Tier",
        "Chair Car"
    };

    for (const std::string& travelClass : classes) {
        resetFixture();
        writeBooking(
            CONFIRMED_PNR, travelClass, "1", "1000.00"
        );

        assert(cancel());
        assert(getAvailableSeats(travelClass) == 11);

        std::cout << "[PASS] Seat restoration for "
                  << travelClass << '\n';
    }
}

void testMissingTrainFile() {
    resetFixture();

    fs::remove("data/trains.dat");

    double refund = 0.0;
    double charge = 0.0;
    std::string error;

    assert(!Cancellation::cancelBooking(
        CONFIRMED_PNR, refund, charge, error
    ));

    std::cout << "[PASS] Missing train file handled\n";
}

void testNonPositiveWaitlistPosition(const std::string& position) {
    resetFixture("08:00", 0);

    {
        std::ofstream file("data/bookings.dat", std::ios::app);
        assert(file.is_open());

        file << "2222222222|" << TRAIN_NO << "|" << DATE
             << "|Sleeper|Waiting Passenger,25,Other|1|500.00"
             << "|Waitlisted|" << position << '\n';
    }

    assert(cancel());

    Booking promoted;
    assert(BookingManager::findBooking("2222222222", promoted));
    assert(promoted.status == "Confirmed");
    assert(promoted.waitlistPosition == 0);
    assert(getAvailableSeats("Sleeper") == 0);

    std::cout << "[PASS] Non-positive waitlist position fallback: "
              << position << '\n';
}

int main() {
    fs::create_directories("data");

    testNonConfirmedBooking();
    testInvalidBookingDetails();
    testInvalidDepartureRanges();
    testSeatRestorationOverflow();
    testAllTravelClasses();
    testMissingTrainFile();

    testNonPositiveWaitlistPosition("0");
    testNonPositiveWaitlistPosition("-1");

    std::cout << "\nAll Cancellation coverage-extra tests passed.\n";
    return 0;
}
