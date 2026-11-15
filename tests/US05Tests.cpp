#include "PassengerDetails.h"
#include "FareCalculator.h"

#include <cmath>
#include <iostream>
#include <sstream>
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
    std::vector<Passenger> passengers;
    std::string output;

    // 1. Collect valid passenger details
    {
        std::istringstream input("Pooja Koppad\n20\nFemale\n");
        std::ostringstream result;

        bool success = PassengerDetails::collectPassengerDetails(
            1, passengers, input, result
        );

        test("Valid passenger details accepted",
             success && passengers.size() == 1 &&
             passengers[0].name == "Pooja Koppad" &&
             passengers[0].age == 20 &&
             passengers[0].gender == "Female");
    }

    // 2. Reject invalid name and accept a corrected name
    {
        std::istringstream input("Pooja123\nPooja\n20\nFemale\n");
        std::ostringstream result;

        bool success = PassengerDetails::collectPassengerDetails(
            1, passengers, input, result
        );

        test("Invalid passenger name rejected and corrected",
             success && passengers.size() == 1 &&
             passengers[0].name == "Pooja" &&
             result.str().find("Error:") != std::string::npos);
    }

    // 3. Reject whitespace-only name
    {
        std::istringstream input("   \nPooja\n20\nFemale\n");
        std::ostringstream result;

        bool success = PassengerDetails::collectPassengerDetails(
            1, passengers, input, result
        );

        test("Whitespace-only passenger name rejected",
             success && passengers.size() == 1 &&
             passengers[0].name == "Pooja");
    }

    // 4. Reject invalid ages and accept a valid age
    {
        std::istringstream input("Pooja\nabc\n0\n121\n20\nFemale\n");
        std::ostringstream result;

        bool success = PassengerDetails::collectPassengerDetails(
            1, passengers, input, result
        );

        test("Invalid ages rejected and valid age accepted",
             success && passengers.size() == 1 &&
             passengers[0].age == 20);
    }

    // 5. Reject invalid gender and accept lowercase input
    {
        std::istringstream input("Pooja\n20\nUnknown\nfemale\n");
        std::ostringstream result;

        bool success = PassengerDetails::collectPassengerDetails(
            1, passengers, input, result
        );

        test("Invalid gender rejected and valid gender accepted",
             success && passengers.size() == 1 &&
             passengers[0].gender == "Female");
    }

    // 6. Collect details for multiple passengers
    {
        std::istringstream input(
            "Pooja\n20\nFemale\n"
            "Rahul\n22\nMale\n"
        );
        std::ostringstream result;

        bool success = PassengerDetails::collectPassengerDetails(
            2, passengers, input, result
        );

        test("Multiple passenger details collected",
             success && passengers.size() == 2 &&
             passengers[0].name == "Pooja" &&
             passengers[1].name == "Rahul");
    }

    // 7. Reject invalid requested passenger counts
    {
        std::istringstream input("");
        std::ostringstream result;

        bool zeroRejected = !PassengerDetails::collectPassengerDetails(
            0, passengers, input, result
        );

        std::istringstream input2("");
        std::ostringstream result2;

        bool sevenRejected = !PassengerDetails::collectPassengerDetails(
            7, passengers, input2, result2
        );

        test("Invalid passenger counts rejected",
             zeroRejected && sevenRejected);
    }

    // 8. Reject incomplete passenger input
    {
        std::istringstream input("Pooja\n");
        std::ostringstream result;

        bool success = PassengerDetails::collectPassengerDetails(
            1, passengers, input, result
        );

        test("Incomplete passenger details rejected", !success);
    }

    // 9. Fare calculation for all four travel classes
    {
        Train train{};
        train.sleeperFare = 1200;
        train.ac3TierFare = 1800;
        train.ac2TierFare = 2400;
        train.chairCarFare = 900;

        test("Sleeper fare for two passengers",
             FareCalculator::calculateFare(train, "Sleeper", 2) == 2400);

        test("AC 3-Tier fare for two passengers",
             FareCalculator::calculateFare(train, "AC 3-Tier", 2) == 3600);

        test("AC 2-Tier fare for two passengers",
             FareCalculator::calculateFare(train, "AC 2-Tier", 2) == 4800);

        test("Chair Car fare for two passengers",
             FareCalculator::calculateFare(train, "Chair Car", 2) == 1800);
    }

    // 10. Reject invalid fare calculations
    {
        Train train{};
        train.sleeperFare = 1200;

        bool zeroCountRejected = false;
        bool invalidClassRejected = false;
        bool zeroFareRejected = false;

        try {
            FareCalculator::calculateFare(train, "Sleeper", 0);
        } catch (const std::invalid_argument&) {
            zeroCountRejected = true;
        }

        try {
            FareCalculator::calculateFare(train, "General", 1);
        } catch (const std::invalid_argument&) {
            invalidClassRejected = true;
        }

        train.sleeperFare = 0;

        try {
            FareCalculator::calculateFare(train, "Sleeper", 1);
        } catch (const std::invalid_argument&) {
            zeroFareRejected = true;
        }

        test("Invalid fare calculations rejected",
             zeroCountRejected && invalidClassRejected && zeroFareRejected);
    }

    std::cout << "\n========== US-05 TEST SUMMARY ==========\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "Total:  " << passed + failed << '\n';

    return failed == 0 ? 0 : 1;
}
