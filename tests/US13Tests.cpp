#include "Validation.h"

#include <iostream>
#include <string>

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
    int seats = 0;
    std::string error;

    // Acceptance 1: Positive integer
    test("One passenger accepted",
         Validation::parseSeatCount("1", seats, error) && seats == 1);

    test("Six passengers accepted",
         Validation::parseSeatCount("6", seats, error) && seats == 6);

    // Acceptance 2: Invalid quantities rejected
    test("Zero rejected",
         !Validation::parseSeatCount("0", seats, error));

    test("Negative number rejected",
         !Validation::parseSeatCount("-2", seats, error));

    test("Alphabetic input rejected",
         !Validation::parseSeatCount("abc", seats, error));

    test("Mixed numeric input rejected",
         !Validation::parseSeatCount("2abc", seats, error));

    test("Decimal input rejected",
         !Validation::parseSeatCount("2.5", seats, error));

    test("Empty input rejected",
         !Validation::parseSeatCount("", seats, error));

    // Acceptance 3: Maximum six passengers
    test("Seven passengers rejected",
         !Validation::parseSeatCount("7", seats, error));

    test("Large passenger count rejected",
         !Validation::parseSeatCount("100", seats, error));

    // Acceptance 6: Validation can be called again after an error
    bool invalidRejected =
        !Validation::parseSeatCount("0", seats, error);

    bool validAccepted =
        Validation::parseSeatCount("3", seats, error) && seats == 3;

    test("Valid input accepted after invalid input",
         invalidRejected && validAccepted);

    std::cout << "\n========== US-13 TEST SUMMARY ==========\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "Total:  " << passed + failed << '\n';

    return failed == 0 ? 0 : 1;
}
