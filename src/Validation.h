
#ifndef VALIDATION_H
#define VALIDATION_H

#include <string>

namespace Validation {

    // Checks whether a string contains non-whitespace characters.


    bool isNonEmpty(const std::string& input, std::string& error);

    // Validates a passenger name, including supported punctuation.\n    bool isValidPassengerName(
        const std::string& name,
        std::string& error
    );

    // Validates a date in YYYY-MM-DD format.\n    bool isValidDate(
        const std::string& date,
        std::string& error
    );

    // Parses and validates the requested passenger seat count.\n    bool parseSeatCount(
        const std::string& input,
        int& seats,
        std::string& error
    );

    // Parses a positive, finite fare value.\n    bool parseFare(
        const std::string& input,
        double& fare,
        std::string& error
    );

    // Checks whether a menu choice is within the allowed range.\n    bool isValidMenuChoice(
        int choice,
        int minimum,
        int maximum,
        std::string& error
    );

    // Parses a menu choice from user input and checks its range.\n    bool parseMenuChoice(
        const std::string& input,
        int minimum,
        int maximum,
        int& choice,
        std::string& error
    );
}

#endif
