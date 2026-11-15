#include "Validation.h"

#include <iostream>
#include <string>

namespace {
int passed = 0;
int failed = 0;

void check(const std::string& name, bool condition) {
    if (condition) {
        ++passed;
        std::cout << "[PASS] " << name << '\n';
    } else {
        ++failed;
        std::cout << "[FAIL] " << name << '\n';
    }
}
}

int main() {
    std::string error;
    int number = -1;
    double fare = -1.0;

    check("Whitespace-only field rejected",
          !Validation::isNonEmpty(" \t\n", error));
    check("Non-empty field accepted",
          Validation::isNonEmpty("Bengaluru", error));

    check("Hyphenated and apostrophe passenger name accepted",
          Validation::isValidPassengerName("Anne-Marie O'Neil", error));
    check("Whitespace-only passenger name rejected",
          !Validation::isValidPassengerName("   ", error));
    check("Digits in passenger name rejected",
          !Validation::isValidPassengerName("Passenger123", error));
    check("Name without letters rejected",
          !Validation::isValidPassengerName("---", error));

    check("Malformed date length rejected",
          !Validation::isValidDate("2026-1-01", error));
    check("Malformed date separators rejected",
          !Validation::isValidDate("2026/11/01", error));
    check("Non-numeric date component rejected",
          !Validation::isValidDate("2026-AB-01", error));
    check("Year zero rejected",
          !Validation::isValidDate("0000-01-01", error));
    check("Month outside range rejected",
          !Validation::isValidDate("2026-13-01", error));
    check("Invalid day rejected",
          !Validation::isValidDate("2026-04-31", error));
    check("Leap day accepted in leap year",
          Validation::isValidDate("2024-02-29", error));
    check("Leap day rejected in non-leap year",
          !Validation::isValidDate("2023-02-29", error));

    check("Empty passenger count rejected",
          !Validation::parseSeatCount("", number, error));
    check("Alphabetic passenger count rejected",
          !Validation::parseSeatCount("abc", number, error));
    check("Zero passenger count rejected",
          !Validation::parseSeatCount("0", number, error));
    check("Passenger count above maximum rejected",
          !Validation::parseSeatCount("7", number, error));
    check("Decimal passenger count rejected",
          !Validation::parseSeatCount("2.5", number, error));
    check("Maximum valid passenger count accepted",
          Validation::parseSeatCount("6", number, error) && number == 6);

    check("Empty fare rejected",
          !Validation::parseFare("", fare, error));
    check("Non-numeric fare rejected",
          !Validation::parseFare("abc", fare, error));
    check("Fare with trailing characters rejected",
          !Validation::parseFare("10abc", fare, error));
    check("NaN fare rejected",
          !Validation::parseFare("nan", fare, error));
    check("Infinite fare rejected",
          !Validation::parseFare("inf", fare, error));
    check("Negative fare rejected",
          !Validation::parseFare("-1", fare, error));
    check("Zero fare rejected",
          !Validation::parseFare("0", fare, error));
    check("Positive decimal fare accepted",
          Validation::parseFare("1250.50", fare, error) && fare == 1250.50);

    check("Invalid menu bounds rejected",
          !Validation::isValidMenuChoice(1, 5, 2, error));
    check("Menu choice below range rejected",
          !Validation::isValidMenuChoice(0, 1, 4, error));
    check("Menu choice above range rejected",
          !Validation::isValidMenuChoice(5, 1, 4, error));
    check("Menu choice within range accepted",
          Validation::isValidMenuChoice(3, 1, 4, error));

    check("Empty menu choice rejected",
          !Validation::parseMenuChoice("", 1, 4, number, error));
    check("Non-numeric menu choice rejected",
          !Validation::parseMenuChoice("x", 1, 4, number, error));
    check("Menu choice above range rejected",
          !Validation::parseMenuChoice("5", 1, 4, number, error));
    check("Menu choice below range rejected",
          !Validation::parseMenuChoice("0", 1, 4, number, error));
    check("Valid menu choice parsed",
          Validation::parseMenuChoice("4", 1, 4, number, error) && number == 4);

    std::cout << "Validation tests: " << passed << " passed, "
              << failed << " failed.\n";
    return failed == 0 ? 0 : 1;
}
