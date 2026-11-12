#include "../src/AdminFareReservationManager.h"

#include <cassert>
#include <filesystem>
#include <iterator>
#include "../src/TrainData.h"
#include <cstdio>
#include <fstream>
#include <iostream>
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
