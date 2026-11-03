#include "Validation.h"
#include "TrainSearch.h"
#include "FareCalculator.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

int passed = 0;
int failed = 0;

void test(const std::string& name, bool condition) {
    if (condition) {
        std::cout << "[PASS] " << name << '\n';
        ++passed;
    } else {
        std::cout << "[FAIL] " << name << '\n';
        ++failed;
    }
}

int main() {
    std::string error;
    int seats = 0;
    double fare = 0.0;

    // Passenger validation
    test("Valid passenger name",
         Validation::isValidPassengerName("Pooja", error));

    test("Empty passenger name rejected",
         !Validation::isValidPassengerName("", error));

    // Date validation
    test("Valid journey date",
         Validation::isValidDate("2026-11-15", error));

    test("Invalid date rejected",
         !Validation::isValidDate("2026-02-30", error));

    test("Invalid date format rejected",
         !Validation::isValidDate("15-11-2026", error));

    // Passenger count validation
    test("Valid passenger count",
         Validation::parseSeatCount("3", seats, error) && seats == 3);

    test("Zero passengers rejected",
         !Validation::parseSeatCount("0", seats, error));

    test("More than six passengers rejected",
         !Validation::parseSeatCount("7", seats, error));

    // Fare input validation
    test("Valid fare accepted",
         Validation::parseFare("1500", fare, error) && fare == 1500.0);

    test("Negative fare rejected",
         !Validation::parseFare("-100", fare, error));

    // Menu validation
    test("Valid menu option",
         Validation::isValidMenuChoice(2, 1, 4, error));

    test("Invalid menu option rejected",
         !Validation::isValidMenuChoice(5, 1, 4, error));

    // Train search
    Train train{};
    train.trainNumber = "12627";
    train.trainName = "Karnataka Express";
    train.source = "Bengaluru";
    train.destination = "New Delhi";
    train.journeyDate = "2026-11-15";
    train.active = true;

    std::vector<Train> trains = {train};

    auto results = searchTrains(
        trains, "Bengaluru", "New Delhi", "2026-11-15"
    );

    test("Matching train found", results.size() == 1);

    auto noResults = searchTrains(
        trains, "Chennai", "New Delhi", "2026-11-15"
    );

    test("Unmatched train search returns empty",
         noResults.empty());

    // Fare calculation
    train.sleeperFare = 1200;
    train.ac3TierFare = 1800;
    train.ac2TierFare = 2400;
    train.chairCarFare = 900;

    test("Sleeper fare calculated correctly",
         FareCalculator::calculateFare(train, "Sleeper", 2) == 2400);

    test("AC 3-Tier fare calculated correctly",
         FareCalculator::calculateFare(train, "AC 3-Tier", 2) == 3600);

    bool invalidClassRejected = false;
    try {
        FareCalculator::calculateFare(train, "Invalid", 1);
    } catch (const std::invalid_argument&) {
        invalidClassRejected = true;
    }

    test("Invalid travel class rejected", invalidClassRejected);

    bool invalidCountRejected = false;
    try {
        FareCalculator::calculateFare(train, "Sleeper", 7);
    } catch (const std::invalid_argument&) {
        invalidCountRejected = true;
    }

    test("Invalid passenger count rejected",
         invalidCountRejected);

    std::cout << "\n========== TEST SUMMARY ==========\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "Total:  " << passed + failed << '\n';

    return failed == 0 ? 0 : 1;
}
