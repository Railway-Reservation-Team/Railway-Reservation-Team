#include "Validation.h"

#include <cctype>
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace Validation {

bool isValidPassengerName(
    const std::string& name,
    std::string& error) {

    if (name.empty()) {
        error = "Passenger name cannot be empty.";
        return false;
    }

    for (unsigned char ch : name) {
        if (!std::isalpha(ch) &&
            ch != ' ' && ch != '-' && ch != '\'') {
            error = "Passenger name contains invalid characters.";
            return false;
        }
    }

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

    try {
        int year = std::stoi(date.substr(0, 4));
        int month = std::stoi(date.substr(5, 2));
        int day = std::stoi(date.substr(8, 2));

        if (year < 1 || month < 1 || month > 12) {
            error = "Invalid year or month.";
            return false;
        }

        int days[] = {
            31, 28, 31, 30, 31, 30,
            31, 31, 30, 31, 30, 31
        };

        bool leapYear =
            (year % 400 == 0) ||
            (year % 4 == 0 && year % 100 != 0);

        if (leapYear) {
            days[1] = 29;
        }

        if (day < 1 || day > days[month - 1]) {
            error = "Invalid day for the selected month.";
            return false;
        }

    } catch (...) {
        error = "Date contains invalid characters.";
        return false;
    }

    return true;
}

bool parseSeatCount(
    const std::string& input,
    int& seats,
    std::string& error) {

    try {
        size_t position;
        int value = std::stoi(input, &position);

        if (position != input.size()) {
            error = "Enter a valid whole number.";
            return false;
        }

        if (value < 1 || value > 6) {
            error = "Seat count must be between 1 and 6.";
            return false;
        }

        seats = value;
        return true;

    } catch (...) {
        error = "Invalid seat count.";
        return false;
    }
}

bool parseFare(
    const std::string& input,
    double& fare,
    std::string& error) {

    try {
        size_t position;
        double value = std::stod(input, &position);

        if (position != input.size() ||
            !std::isfinite(value) || value < 0) {
            error = "Fare must be a valid non-negative number.";
            return false;
        }

        fare = value;
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

    if (choice < minimum || choice > maximum) {
        error = "Invalid menu choice. Try again.";
        return false;
    }

    return true;
}

}