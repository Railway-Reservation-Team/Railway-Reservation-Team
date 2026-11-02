#include "../src/SeatInventory.h"

#include <iostream>
#include <string>

int passed = 0;
int failed = 0;

void check(
    const std::string& testName,
    bool condition
) {
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

    train.sleeperSeatsAvailable = 0;
    train.ac3TierSeatsAvailable = 65;
    train.ac2TierSeatsAvailable = 35;
    train.chairCarSeatsAvailable = 40;

    std::cout << "\n===== SEAT INVENTORY UNIT TESTS =====\n\n";

    // 1. Seat availability tests
    std::cout << "--- Availability Tests ---\n";

    check("Sleeper availability is zero",
          getAvailableSeats(train, "Sleeper") == 0);

    check("AC 3-Tier availability",
          getAvailableSeats(train, "AC 3-Tier") == 65);

    check("AC 2-Tier availability",
          getAvailableSeats(train, "AC 2-Tier") == 35);

    check("Chair Car availability",
          getAvailableSeats(train, "Chair Car") == 40);

    // 2. Class normalization tests
    std::cout << "\n--- Class Normalization Tests ---\n";

    check("Lowercase class name",
          getAvailableSeats(train, "sleeper") == 0);

    check("Uppercase class name",
          getAvailableSeats(train, "AC 3-TIER") == 65);

    check("Class name with extra spaces",
          getAvailableSeats(train, "  AC 2-Tier  ") == 35);

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

    // 4. Valid seat request tests
    std::cout << "\n--- Valid Seat Request Tests ---\n";

    check("One seat request accepted",
          validateSeatRequest(65, 1) == SeatRequestResult::VALID);

    check("Six passengers accepted",
          validateSeatRequest(65, 6) == SeatRequestResult::VALID);

    check("Exact available seat count accepted",
          validateSeatRequest(6, 6) == SeatRequestResult::VALID);

    // 5. Invalid seat request tests
    std::cout << "\n--- Invalid Seat Request Tests ---\n";

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

    check("Zero available seats rejects positive request",
          validateSeatRequest(0, 1) ==
              SeatRequestResult::EXCEEDS_AVAILABLE_SEATS);

    check("Negative availability rejected",
          validateSeatRequest(-1, 1) ==
              SeatRequestResult::INVALID_AVAILABILITY);

    // 6. Boundary tests
    std::cout << "\n--- Boundary Tests ---\n";

    check("Exactly six passengers accepted",
          validateSeatRequest(6, MAX_PASSENGERS_PER_BOOKING) ==
              SeatRequestResult::VALID);

    check("Seven passengers rejected",
          validateSeatRequest(7, MAX_PASSENGERS_PER_BOOKING + 1) ==
              SeatRequestResult::EXCEEDS_MAX_PASSENGERS);

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
