
#include "../src/Cancellation.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

const std::string TRAIN_NUMBER = "WAIT101";
const std::string JOURNEY_DATE = "2027-12-15";
const std::string CONFIRMED_PNR = "1111111111";

int passed = 0;
int failed = 0;

void check(bool condition, const std::string& message) {
    if (condition) {
        std::cout << "[PASS] " << message << '\n';
        ++passed;
    } else {
        std::cout << "[FAIL] " << message << '\n';
        ++failed;
    }
}

void resetFixture() {
    fs::create_directories("data");

    {
        std::ofstream train("data/trains.dat", std::ios::trunc);
        // 17-field train record; zero Sleeper seats before cancellation.
        train << TRAIN_NUMBER
              << "|Waitlist Test Express|Bengaluru|Chennai|23:59|06:00|6h"
              << "|1000|1500|2000|800|0|10|10|10|"
              << JOURNEY_DATE << "|1\n";
    }

    {
        std::ofstream bookings("data/bookings.dat", std::ios::trunc);
        bookings << CONFIRMED_PNR
                 << '|' << TRAIN_NUMBER
                 << '|' << JOURNEY_DATE
                 << "|Sleeper|Confirmed Passenger,25,Other"
                 << "|1|1000.00|Confirmed|0\n";
    }

    std::error_code ec;
    fs::remove("data/cancellations.dat", ec);
    fs::remove("data/admin_audit.log", ec);
}

void addWaitlistedBooking(
    const std::string& pnr,
    const std::string& passengerCount,
    const std::string& position,
    bool legacyRecord = false) {

    std::ofstream bookings("data/bookings.dat", std::ios::app);

    bookings << pnr
             << '|' << TRAIN_NUMBER
             << '|' << JOURNEY_DATE
             << "|Sleeper|Waitlisted Passenger,25,Other"
             << '|' << passengerCount
             << "|1000.00|Waitlisted";

    if (!legacyRecord) {
        bookings << '|' << position;
    }

    bookings << '\n';
}

std::string readBookingLine(const std::string& pnr) {
    std::ifstream bookings("data/bookings.dat");
    std::string line;

    while (std::getline(bookings, line)) {
        if (line.rfind(pnr + "|", 0) == 0) {
            return line;
        }
    }

    return "";
}

bool cancelConfirmedBooking() {
    double refund = 0.0;
    double charge = 0.0;
    std::string error;

    const bool result = Cancellation::cancelBooking(
        CONFIRMED_PNR, refund, charge, error
    );

    if (!result) {
        std::cout << "Cancellation error: " << error << '\n';
    }

    return result;
}

// 1. A non-numeric waitlist position must be rejected.
void testInvalidWaitlistPosition() {
    resetFixture();
    addWaitlistedBooking("2222222222", "1", "invalid");

    check(
        !cancelConfirmedBooking(),
        "Reject a non-numeric waitlist position"
    );
}

// 2. A non-numeric passenger count must be rejected.
void testNonNumericPassengerCount() {
    resetFixture();
    addWaitlistedBooking("2222222222", "abc", "1");

    check(
        !cancelConfirmedBooking(),
        "Reject a non-numeric waitlisted passenger count"
    );
}

// 3. A zero passenger count must be rejected.
void testZeroPassengerCount() {
    resetFixture();
    addWaitlistedBooking("2222222222", "0", "1");

    check(
        !cancelConfirmedBooking(),
        "Reject a zero waitlisted passenger count"
    );
}

// 4. A negative passenger count must be rejected.
void testNegativePassengerCount() {
    resetFixture();
    addWaitlistedBooking("2222222222", "-2", "1");

    check(
        !cancelConfirmedBooking(),
        "Reject a negative waitlisted passenger count"
    );
}

// 5. A later smaller group must not jump ahead of an earlier group.
void testFIFOBlocking() {
    resetFixture();

    addWaitlistedBooking("2222222222", "2", "1");
    addWaitlistedBooking("3333333333", "1", "2");

    const bool cancelled = cancelConfirmedBooking();

    const std::string first = readBookingLine("2222222222");
    const std::string second = readBookingLine("3333333333");

    check(cancelled, "Cancel confirmed booking with waitlisted groups");

    check(
        first.find("|Waitlisted|1") != std::string::npos,
        "First group remains waitlisted when it cannot fit"
    );

    check(
        second.find("|Waitlisted|2") != std::string::npos,
        "Later smaller group does not jump ahead in the queue"
    );
}

// 6. Legacy 8-field records must be supported and queue positions refreshed.
void testLegacyWaitlistAndQueueRefresh() {
    resetFixture();

    addWaitlistedBooking("2222222222", "1", "", true);
    addWaitlistedBooking("3333333333", "2", "", true);

    const bool cancelled = cancelConfirmedBooking();

    const std::string first = readBookingLine("2222222222");
    const std::string second = readBookingLine("3333333333");

    check(cancelled, "Cancel booking with legacy waitlist records");

    check(
        first.find("|Confirmed|0") != std::string::npos,
        "Promote a fitting legacy waitlist record"
    );

    check(
        second.find("|Waitlisted|1") != std::string::npos,
        "Refresh remaining legacy waitlist position"
    );
}


// 7. Temporary-file creation failure must not complete the cancellation.
void testTemporaryFileFailure() {
    resetFixture();

    // Force creation of the third temporary file to fail.
    fs::create_directory("data/bookings.dat.tmp");

    double refund = 123.0;
    double charge = 456.0;
    std::string error;

    const bool result = Cancellation::cancelBooking(
        CONFIRMED_PNR, refund, charge, error
    );

    check(!result, "Reject cancellation when a temporary file cannot be created");
    check(refund == 0.0 && charge == 0.0,
          "Reset refund and charge after temporary-file failure");
    check(readBookingLine(CONFIRMED_PNR).find("|Confirmed|") != std::string::npos,
          "Keep booking confirmed after temporary-file failure");

    fs::remove_all("data/bookings.dat.tmp");
}

} // namespace

int main() {
    std::cout << "===== Cancellation Waitlist Tests =====\n";

    testInvalidWaitlistPosition();
    testNonNumericPassengerCount();
    testZeroPassengerCount();
    testNegativePassengerCount();
    testFIFOBlocking();
    testLegacyWaitlistAndQueueRefresh();
    testTemporaryFileFailure();

    std::cout << "\nPassed: " << passed
              << "\nFailed: " << failed
              << "\nTotal: " << passed + failed << '\n';

    return failed == 0 ? 0 : 1;
}
