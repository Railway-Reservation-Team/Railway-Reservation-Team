
#include "Validation.h"

#include <cctype>
#include <cmath>
#include <limits>
#include <string>

namespace Validation {

bool isNonEmpty(const std::string& input, std::string& error) {
    for (unsigned char ch : input) {
        if (!std::isspace(ch)) {
            error.clear();
            return true;
        }
    }

    error = "This field cannot be empty.";
    return false;
}

bool isValidPassengerName(
    const std::string& name,
    std::string& error) {

    if (!isNonEmpty(name, error)) {
        error = "Passenger name cannot be empty.";
        return false;
    }

    bool hasLetter = false;

    for (unsigned char ch : name) {
        if (std::isalpha(ch)) {
            hasLetter = true;
        } else if (ch != ' ' && ch != '-' && ch != '\'') {
            error = "Passenger name contains invalid characters.";
            return false;
        }
    }

    if (!hasLetter) {
        error = "Passenger name must contain letters.";
        return false;
    }

    error.clear();
    return true;
}

bool isValidDate(
    const std::string& date,
    std::string& error) {

    if (date.size() != 10 ||
        date[4] != '-' || date[7] != '-') {
        error = "Date must be in YYYY-MM-DD format.";
        return false;
    }

    for (size_t i = 0; i < date.size(); ++i) {
        if (i != 4 && i != 7 &&
            !std::isdigit(static_cast<unsigned char>(date[i]))) {
            error = "Date must contain only numeric digits and hyphens.";
            return false;
        }
    }

    int year = std::stoi(date.substr(0, 4));
    int month = std::stoi(date.substr(5, 2));
    int day = std::stoi(date.substr(8, 2));

    if (year < 1 || month < 1 || month > 12) {
        error = "Invalid year or month.";
        return false;
    }

    const int days[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    int maxDays = days[month - 1];

    bool leapYear =
        (year % 400 == 0) ||
        (year % 4 == 0 && year % 100 != 0);

    if (month == 2 && leapYear) {
        maxDays = 29;
    }

    if (day < 1 || day > maxDays) {
        error = "Invalid day for the selected month.";
        return false;
    }

    error.clear();
    return true;
}

bool parseSeatCount(
    const std::string& input,
    int& seats,
    std::string& error) {

    if (input.empty()) {
        error = "Seat count cannot be empty.";
        return false;
    }

    int value = 0;

    for (unsigned char ch : input) {
        if (!std::isdigit(ch)) {
            error = "Enter a valid whole number of passengers.";
            return false;
        }

        const int digit = ch - '0';

        // Check the permitted range before multiplying to prevent overflow
        // even when the input contains an extremely long digit sequence.
        if (digit > 6 || value > (6 - digit) / 10) {
            error = "Passenger count must be between 1 and 6.";
            return false;
        }

        value = value * 10 + digit;
    }

    if (value < 1) {
        error = "Passenger count must be between 1 and 6.";
        return false;
    }

    seats = value;
    error.clear();
    return true;
}

bool parseFare(
    const std::string& input,
    double& fare,
    std::string& error) {

    if (input.empty()) {
        error = "Fare cannot be empty.";
        return false;
    }

    try {
        size_t position = 0;
        double value = std::stod(input, &position);

        if (position != input.size() ||
            !std::isfinite(value) ||
            value <= 0) {
            error = "Fare must be a valid positive number.";
            return false;
        }

        fare = value;
        error.clear();
        return true;

    } catch (...) {
        error = "Invalid fare amount.";
        return false;
    }
}

bool isValidMenuChoice(
    int choice,
    int minimum,
    int maximum,
    std::string& error) {

    if (minimum > maximum ||
        choice < minimum || choice > maximum) {
        error = "Invalid menu choice. Please select an available option.";
        return false;
    }

    error.clear();
    return true;
}

bool parseMenuChoice(
    const std::string& input,
    int minimum,
    int maximum,
    int& choice,
    std::string& error) {

    if (input.empty()) {
        error = "Menu choice cannot be empty.";
        return false;
    }

    if (minimum > maximum || maximum < 0) {
        error = "Invalid menu choice range.";
        return false;
    }

    int value = 0;

    for (unsigned char ch : input) {
        if (!std::isdigit(ch)) {
            error = "Menu choice must be a whole number.";
            return false;
        }

        const int digit = ch - '0';

        // Guard integer conversion before multiplication and addition.
        if (value > (std::numeric_limits<int>::max() - digit) / 10) {
            error = "Menu choice is outside the allowed range.";
            return false;
        }

        value = value * 10 + digit;

        if (value > maximum) {
            error = "Menu choice is outside the allowed range.";
            return false;
        }
    }

    if (!isValidMenuChoice(value, minimum, maximum, error)) {
        return false;
    }

    choice = value;
    error.clear();
    return true;
}

}
