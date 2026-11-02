#ifndef VALIDATION_H
#define VALIDATION_H

#include <string>

namespace Validation {

    bool isValidPassengerName(
        const std::string& name,
        std::string& error
    );

    bool isValidDate(
        const std::string& date,
        std::string& error
    );

    bool parseSeatCount(
        const std::string& input,
        int& seats,
        std::string& error
    );

    bool parseFare(
        const std::string& input,
        double& fare,
        std::string& error
    );

    bool isValidMenuChoice(
        int choice,
        int minimum,
        int maximum,
        std::string& error
    );

}

#endif