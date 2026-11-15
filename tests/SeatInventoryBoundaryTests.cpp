
#include "../src/SeatInventory.h"

#include <chrono>
#include <iostream>

void check(const char* name, bool condition, int& passed, int& failed) {
    if (condition) {
        std::cout << "[PASS] " << name << '\n';
        ++passed;
    } else {
        std::cout << "[FAIL] " << name << '\n';
        ++failed;
    }
}

int main() {
    Train train;
    train.trainNumber = "BOUNDARY-TEST";
    train.journeyDate = "2026-11-13";
    train.ac3TierSeatsAvailable = 6;

    SeatInventoryManager inventory;
    auto testTime = std::chrono::steady_clock::now();

    int passed = 0;
    int failed = 0;

    std::cout << "\n===== SEAT INVENTORY BOUNDARY TESTS =====\n";

    // Test 1: Hold exactly six available seats.
    SeatHoldResponse fullHold =
        inventory.holdSeats(train, "3A", 6, testTime);

    check("Exactly six available seats can be held",
          fullHold.result == SeatHoldResult::SUCCESS,
          passed, failed);

    check("No seats remain after full hold",
          inventory.getLiveAvailableSeats(train, "3A", testTime) == 0,
          passed, failed);

    // Test 2: One additional seat must be rejected.
    check("Additional seat request is rejected",
          inventory.holdSeats(train, "3A", 1, testTime).result ==
              SeatHoldResult::EXCEEDS_AVAILABLE_SEATS,
          passed, failed);

    // Test 3: Zero seats must be rejected.
    check("Zero-seat request is rejected",
          inventory.holdSeats(train, "3A", 0, testTime).result ==
              SeatHoldResult::INVALID_REQUEST,
          passed, failed);

    // Test 4: Negative seats must be rejected.
    check("Negative seat request is rejected",
          inventory.holdSeats(train, "3A", -2, testTime).result ==
              SeatHoldResult::INVALID_REQUEST,
          passed, failed);

    // Test 5: Releasing the full hold restores availability.
    check("Full hold can be released",
          inventory.releaseHold(fullHold.holdId),
          passed, failed);

    check("All six seats are restored after release",
          inventory.getLiveAvailableSeats(train, "3A", testTime) == 6,
          passed, failed);

    std::cout << "\nTotal: " << passed + failed
              << " | Passed: " << passed
              << " | Failed: " << failed << '\n';

    return failed == 0 ? 0 : 1;
}
