
#include "../src/SeatInventory.h"

#include <iostream>
#include <string>
#include <chrono>

int passed = 0;
int failed = 0;

void check(const std::string& testName, bool condition) {
    if (condition) {
        std::cout << "[PASS] " << testName << '\n';
        ++passed;
    } else {
        std::cout << "[FAIL] " << testName << '\n';
        ++failed;
    }
}

int main() {
    Train train;

    train.trainNumber = "12627";
    train.journeyDate = "2026-11-01";

    train.sleeperSeatsAvailable = 0;
    train.ac3TierSeatsAvailable = 65;
    train.ac2TierSeatsAvailable = 35;
    train.chairCarSeatsAvailable = 40;

    std::cout << "\n===== SEAT INVENTORY UNIT TESTS =====\n\n";

    // 1. Availability tests
    std::cout << "--- Availability Tests ---\n";

    check("Sleeper availability is zero",
          getAvailableSeats(train, "Sleeper") == 0);

    check("AC 3-Tier availability",
          getAvailableSeats(train, "AC 3-Tier") == 65);

    check("AC 2-Tier availability",
          getAvailableSeats(train, "AC 2-Tier") == 35);

    check("Chair Car availability",
          getAvailableSeats(train, "Chair Car") == 40);

    // 2. Normalization tests
    std::cout << "\n--- Class Normalization Tests ---\n";

    check("Lowercase class name",
          getAvailableSeats(train, "sleeper") == 0);

    check("Uppercase class name",
          getAvailableSeats(train, "AC 3-TIER") == 65);

    check("Class name with extra spaces",
          getAvailableSeats(train, "  AC 2-Tier  ") == 35);

    check("AC 3-Tier with hyphen and no spaces",
          getAvailableSeats(train, "AC-3-Tier") == 65);

    check("AC 3-Tier without spaces",
          getAvailableSeats(train, "AC3Tier") == 65);

    check("AC 2-Tier with hyphen and no spaces",
          getAvailableSeats(train, "AC-2-Tier") == 35);

    check("AC 2-Tier without spaces",
          getAvailableSeats(train, "AC2Tier") == 35);

    check("AC 3-Tier alias",
          getAvailableSeats(train, "3A") == 65);

    check("AC 2-Tier alias",
          getAvailableSeats(train, "2A") == 35);

    check("Chair Car alias",
          getAvailableSeats(train, "CC") == 40);

    // 3. Invalid class tests
    std::cout << "\n--- Invalid Class Tests ---\n";

    check("Invalid class returns -1",
          getAvailableSeats(train, "First Class") == -1);

    check("Empty class returns -1",
          getAvailableSeats(train, "") == -1);

    check("Whitespace-only class returns -1",
          getAvailableSeats(train, "   ") == -1);

    // 4. Seat request tests
    std::cout << "\n--- Seat Request Tests ---\n";

    check("One seat request accepted",
          validateSeatRequest(65, 1) == SeatRequestResult::VALID);

    check("Six passengers accepted",
          validateSeatRequest(65, 6) == SeatRequestResult::VALID);

    check("Exact available seat count accepted",
          validateSeatRequest(6, 6) == SeatRequestResult::VALID);

    check("Zero seats rejected",
          validateSeatRequest(65, 0) == SeatRequestResult::INVALID_REQUEST);

    check("Negative seats rejected",
          validateSeatRequest(65, -1) == SeatRequestResult::INVALID_REQUEST);

    check("Request exceeding availability rejected",
          validateSeatRequest(3, 4) ==
              SeatRequestResult::EXCEEDS_AVAILABLE_SEATS);

    check("Request exceeding six passengers rejected",
          validateSeatRequest(65, 7) ==
              SeatRequestResult::EXCEEDS_MAX_PASSENGERS);

    check("Zero availability rejects positive request",
          validateSeatRequest(0, 1) ==
              SeatRequestResult::EXCEEDS_AVAILABLE_SEATS);

    check("Negative availability rejected",
          validateSeatRequest(-1, 1) ==
              SeatRequestResult::INVALID_AVAILABILITY);

    // 5. Soft-lock tests
    std::cout << "\n--- Soft Lock Tests ---\n";

    SeatInventoryManager inventory;

    auto startTime = std::chrono::steady_clock::now();

    SeatHoldResponse firstHold =
        inventory.holdSeats(train, "3A", 3, startTime);

    check("Valid seat hold created",
          firstHold.result == SeatHoldResult::SUCCESS);

    check("Hold has an ID",
          !firstHold.holdId.empty());

    check("Availability decreases after hold",
          inventory.getLiveAvailableSeats(
              train, "AC 3-Tier", startTime
          ) == 62);

    check("Active hold count is one",
          inventory.getActiveHoldCount(startTime) == 1);

    Train limitedTrain = train;
    limitedTrain.trainNumber = "LIMITED";
    limitedTrain.ac3TierSeatsAvailable = 5;

    SeatHoldResponse limitedHold =
        inventory.holdSeats(limitedTrain, "3A", 3, startTime);

    check("First limited hold succeeds",
          limitedHold.result == SeatHoldResult::SUCCESS);

    check("Second hold cannot exceed remaining availability",
          inventory.holdSeats(
              limitedTrain, "3A", 3, startTime
          ).result == SeatHoldResult::EXCEEDS_AVAILABLE_SEATS);

    check("Invalid class cannot be held",
          inventory.holdSeats(
              train, "First Class", 1, startTime
          ).result == SeatHoldResult::INVALID_CLASS);

    check("Seven passengers cannot be held",
          inventory.holdSeats(
              train, "3A", 7, startTime
          ).result == SeatHoldResult::EXCEEDS_MAX_PASSENGERS);

    // 6. Hold release tests
    std::cout << "\n--- Hold Release Tests ---\n";

    check("Manual hold release succeeds",
          inventory.releaseHold(firstHold.holdId));

    check("Availability restored after release",
          inventory.getLiveAvailableSeats(
              train, "3A", startTime
          ) == 65);

    check("Released hold cannot be released twice",
          !inventory.releaseHold(firstHold.holdId));

    // 7. Expiry tests
    std::cout << "\n--- Hold Expiry Tests ---\n";

    SeatHoldResponse expiryHold =
        inventory.holdSeats(train, "3A", 4, startTime);

    auto fourMinutesLater =
        startTime + std::chrono::minutes(4);

    check("Hold remains active before five minutes",
          inventory.getLiveAvailableSeats(
              train, "3A", fourMinutesLater
          ) == 61);

    auto fiveMinutesLater =
        startTime + std::chrono::minutes(5);

    check("Hold expires after five minutes",
          inventory.getLiveAvailableSeats(
              train, "3A", fiveMinutesLater
          ) == 65);

    check("Expired hold is removed",
          inventory.getActiveHoldCount(fiveMinutesLater) == 0);

    check("Expired hold cannot be released",
          !inventory.releaseHold(expiryHold.holdId));

    // 8. Confirmation tests
    std::cout << "\n--- Hold Confirmation Tests ---\n";

    SeatHoldResponse confirmationHold =
        inventory.holdSeats(train, "3A", 5, startTime);

    check("Valid hold can be confirmed",
          inventory.confirmHold(
              train, confirmationHold.holdId, startTime
          ));

    check("Confirmed seats reduce train availability",
          train.ac3TierSeatsAvailable == 60);

    check("Confirmed hold is removed",
          inventory.getActiveHoldCount(startTime) == 0);

    check("Confirmation cannot be repeated",
          !inventory.confirmHold(
              train, confirmationHold.holdId, startTime
          ));

    // 9. Separate train/date inventory tests
    std::cout << "\n--- Train and Date Isolation Tests ---\n";

    Train anotherTrain = train;
    anotherTrain.trainNumber = "12027";

    SeatHoldResponse anotherHold =
        inventory.holdSeats(anotherTrain, "3A", 2, startTime);

    check("Hold on another train does not affect first train",
          inventory.getLiveAvailableSeats(
              train, "3A", startTime
          ) == 60);

    check("Hold affects its own train",
          inventory.getLiveAvailableSeats(
              anotherTrain, "3A", startTime
          ) == 58);

    check("Hold release on another train succeeds",
          inventory.releaseHold(anotherHold.holdId));

    // Final summary
    std::cout << "\n===== TEST SUMMARY =====\n";

    std::cout << "Total tests: " << passed + failed << '\n';
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';

    if (failed == 0) {
        std::cout << "\nAll tests passed successfully!\n";
        return 0;
    }

    std::cout << "\nSome tests failed. Review the implementation.\n";
    return 1;
}
