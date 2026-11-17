#include "../src/AdminFareReservationManager.h"

#include <cassert>
#include <filesystem>
#include <iterator>
#include "../src/TrainData.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>

const std::string TRAIN_FILE = "admin_test_trains.dat";
const std::string BOOKING_FILE = "admin_test_bookings.dat";

void prepareFiles() {
    std::ofstream trains(TRAIN_FILE);
    trains << "12345|Test Express|Bengaluru|Chennai|08:00|13:00|5h|500|900|1200|400|50|30|20|40|2026-12-01\n";
    trains.close();

    std::ofstream bookings(BOOKING_FILE);
    bookings << "1234567890|12345|2026-12-01|Sleeper|Test User,20,Male|2|1000.00|Confirmed\n";
    bookings.close();
}

void testViewReservations() {
    std::vector<Booking> bookings;
    std::string error;

    assert(AdminFareReservationManager::viewReservations(
        BOOKING_FILE, bookings, error));

    assert(bookings.size() == 1);
    assert(bookings[0].pnr == "1234567890");

    std::cout << "PASS: View reservations\n";
}

void testSearchReservation() {
    Booking booking;
    std::string error;

    assert(AdminFareReservationManager::searchReservation(
        BOOKING_FILE, "1234567890", booking, error));

    assert(booking.trainNumber == "12345");
    assert(booking.totalFare == 1000.0);

    assert(!AdminFareReservationManager::searchReservation(
        BOOKING_FILE, "9999999999", booking, error));

    std::cout << "PASS: Search reservation\n";
}

void testUpdateFares() {
    std::string error;

    assert(AdminFareReservationManager::updateFares(
        TRAIN_FILE, "12345", "2026-12-01",
        600, 1000, 1400, 500, error));

    std::ifstream file(TRAIN_FILE);
    std::string line;
    std::getline(file, line);

    Train train;
    assert(parseTrainRecord(line, train));

    assert(train.sleeperFare == 600);
    assert(train.ac3TierFare == 1000);
    assert(train.ac2TierFare == 1400);
    assert(train.chairCarFare == 500);

    assert(!AdminFareReservationManager::updateFares(
        TRAIN_FILE, "12345", "2026-12-01",
        -100, 1000, 1400, 500, error));

    assert(!AdminFareReservationManager::updateFares(
        TRAIN_FILE, "99999", "2026-12-01",
        600, 1000, 1400, 500, error));

    std::cout << "PASS: Update fares and validate fare values\n";
}

void testCancelReservation() {
    std::string error;

    assert(AdminFareReservationManager::updateReservationStatus(
        BOOKING_FILE, TRAIN_FILE,
        "1234567890", "Cancelled", error));

    Booking booking;

    assert(AdminFareReservationManager::searchReservation(
        BOOKING_FILE, "1234567890", booking, error));

    assert(booking.status == "Cancelled");

    std::ifstream file(TRAIN_FILE);
    std::string line;
    std::getline(file, line);

    Train train;
    assert(parseTrainRecord(line, train));

    assert(train.sleeperSeatsAvailable == 52);

    std::cout << "PASS: Cancel reservation and restore seats\n";
}

void testRejectRepeatedCancellation() {
    std::string error;

    assert(!AdminFareReservationManager::updateReservationStatus(
        BOOKING_FILE, TRAIN_FILE,
        "1234567890", "Cancelled", error));

    assert(!AdminFareReservationManager::updateReservationStatus(
        BOOKING_FILE, TRAIN_FILE,
        "1234567890", "Confirmed", error));

    std::cout << "PASS: Reject repeated cancellation and reactivation\n";
}

void testRejectInvalidStatus() {
    std::string error;

    assert(!AdminFareReservationManager::updateReservationStatus(
        BOOKING_FILE, TRAIN_FILE,
        "1234567890", "Pending", error));

    std::cout << "PASS: Reject invalid reservation status\n";
}


void testAdditionalFailurePaths() {
    std::string error;

    // Reset the normal fixtures.
    prepareFiles();

    // Invalid fare values.
    assert(!AdminFareReservationManager::updateFares(
        TRAIN_FILE, "12345", "2026-12-01",
        std::numeric_limits<double>::quiet_NaN(),
        900, 1200, 400, error));

    assert(!AdminFareReservationManager::updateFares(
        TRAIN_FILE, "12345", "2026-12-01",
        500, std::numeric_limits<double>::infinity(),
        1200, 400, error));

    assert(!AdminFareReservationManager::updateFares(
        TRAIN_FILE, "12345", "2026-12-01",
        500, 900, -1, 400, error));

    assert(!AdminFareReservationManager::updateFares(
        TRAIN_FILE, "12345", "2026-12-01",
        500, 900, 1200, -1, error));

    // Zero fare is permitted by the implementation.
    assert(AdminFareReservationManager::updateFares(
        TRAIN_FILE, "12345", "2026-12-01",
        0, 0, 0, 0, error));

    // Missing train file.
    std::remove(TRAIN_FILE.c_str());

    assert(!AdminFareReservationManager::updateFares(
        TRAIN_FILE, "12345", "2026-12-01",
        500, 900, 1200, 400, error));

    // Malformed train record.
    {
        std::ofstream file(TRAIN_FILE);
        file << "invalid train record\n";
    }

    assert(!AdminFareReservationManager::updateFares(
        TRAIN_FILE, "12345", "2026-12-01",
        500, 900, 1200, 400, error));

    // Missing booking file.
    std::remove(BOOKING_FILE.c_str());

    std::vector<Booking> bookings;

    assert(!AdminFareReservationManager::viewReservations(
        BOOKING_FILE, bookings, error));

    Booking missingFileBooking;
    assert(!AdminFareReservationManager::searchReservation(
        BOOKING_FILE, "1234567890", missingFileBooking, error));

    // Wrong number of fields.
    {
        std::ofstream file(BOOKING_FILE);
        file << "1234567890|12345|2026-12-01|Sleeper|Passenger|2|1000\n";
    }

    assert(!AdminFareReservationManager::viewReservations(
        BOOKING_FILE, bookings, error));

    // Non-numeric passenger count.
    {
        std::ofstream file(BOOKING_FILE);
        file << "1234567890|12345|2026-12-01|Sleeper|Passenger|abc|1000|Confirmed\n";
    }

    assert(!AdminFareReservationManager::viewReservations(
        BOOKING_FILE, bookings, error));

    // Invalid booking details.
    {
        std::ofstream file(BOOKING_FILE);
        file << "1234567890|12345|2026-12-01|Sleeper|Passenger|0|1000|Confirmed\n";
    }

    assert(!AdminFareReservationManager::viewReservations(
        BOOKING_FILE, bookings, error));

    {
        std::ofstream file(BOOKING_FILE);
        file << "1234567890|12345|2026-12-01|Sleeper|Passenger|2|0|Confirmed\n";
    }

    assert(!AdminFareReservationManager::viewReservations(
        BOOKING_FILE, bookings, error));

    // Reset valid fixtures for reservation status tests.
    prepareFiles();

    // Unknown reservation.
    assert(!AdminFareReservationManager::updateReservationStatus(
        BOOKING_FILE, TRAIN_FILE,
        "9999999999", "Cancelled", error));

    // Already confirmed reservation cannot be confirmed again.
    assert(!AdminFareReservationManager::updateReservationStatus(
        BOOKING_FILE, TRAIN_FILE,
        "1234567890", "Confirmed", error));

    // Waitlisted reservation cannot be cancelled through this operation.
    {
        std::ofstream file(BOOKING_FILE, std::ios::trunc);
        file << "1234567890|12345|2026-12-01|Sleeper|Passenger|2|1000|Waitlisted\n";
    }

    assert(!AdminFareReservationManager::updateReservationStatus(
        BOOKING_FILE, TRAIN_FILE,
        "1234567890", "Cancelled", error));

    // Invalid travel class.
    prepareFiles();

    {
        std::ofstream file(BOOKING_FILE, std::ios::trunc);
        file << "1234567890|12345|2026-12-01|InvalidClass|Passenger|2|1000|Confirmed\n";
    }

    assert(!AdminFareReservationManager::updateReservationStatus(
        BOOKING_FILE, TRAIN_FILE,
        "1234567890", "Cancelled", error));

    // Train file missing.
    prepareFiles();
    std::remove(TRAIN_FILE.c_str());

    assert(!AdminFareReservationManager::updateReservationStatus(
        BOOKING_FILE, TRAIN_FILE,
        "1234567890", "Cancelled", error));

    // Malformed train record.
    prepareFiles();

    {
        std::ofstream file(TRAIN_FILE, std::ios::trunc);
        file << "malformed train record\n";
    }

    assert(!AdminFareReservationManager::updateReservationStatus(
        BOOKING_FILE, TRAIN_FILE,
        "1234567890", "Cancelled", error));

    // Train exists, but does not match the reservation.
    prepareFiles();

    {
        std::ofstream file(TRAIN_FILE, std::ios::trunc);
        file << "99999|Other Express|Bengaluru|Chennai|08:00|13:00|5h|"
                "500|900|1200|400|50|30|20|40|2026-12-01\n";
    }

    assert(!AdminFareReservationManager::updateReservationStatus(
        BOOKING_FILE, TRAIN_FILE,
        "1234567890", "Cancelled", error));

    // Seat restoration would exceed the allowed inventory limit.
    prepareFiles();

    {
        std::ofstream file(TRAIN_FILE, std::ios::trunc);
        file << "12345|Test Express|Bengaluru|Chennai|08:00|13:00|5h|"
                "500|900|1200|400|1000000|30|20|40|2026-12-01\n";
    }

    assert(!AdminFareReservationManager::updateReservationStatus(
        BOOKING_FILE, TRAIN_FILE,
        "1234567890", "Cancelled", error));

    // Fare update for a valid train number but wrong date.
    prepareFiles();

    assert(!AdminFareReservationManager::updateFares(
        TRAIN_FILE, "12345", "2027-01-01",
        500, 900, 1200, 400, error));

    std::cout << "PASS: Additional admin fare and reservation failure paths\n";
}

int main() {
    std::error_code auditFsError;
    std::filesystem::create_directories("data", auditFsError);
    if (auditFsError) return 1;
    std::filesystem::remove("data/admin_audit.log", auditFsError);

    prepareFiles();

    testViewReservations();
    testSearchReservation();
    testUpdateFares();
    testCancelReservation();
    testRejectRepeatedCancellation();
    testRejectInvalidStatus();
    testAdditionalFailurePaths();

    {
        std::ifstream auditFile("data/admin_audit.log");
        const std::string audit(
            (std::istreambuf_iterator<char>(auditFile)),
            std::istreambuf_iterator<char>());
        assert(audit.find("ADMIN_FARES_UPDATED") != std::string::npos);
        assert(audit.find("ADMIN_RESERVATION_CANCELLED") != std::string::npos);
        std::cout << "PASS: Fare and reservation administration actions are audited\n";
    }

    std::remove(TRAIN_FILE.c_str());
    std::remove(BOOKING_FILE.c_str());

    std::cout << "\nAll 6 admin fare and reservation tests passed!\n";

    return 0;
}
