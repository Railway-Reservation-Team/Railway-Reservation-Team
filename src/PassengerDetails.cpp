#include "PassengerDetails.h"
#include "Validation.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

namespace PassengerDetails {

bool collectPassengerDetails(
    int requestedSeatCount,
    std::vector<Passenger>& passengers,
    std::istream& input,
    std::ostream& output) {

    if (requestedSeatCount < 1 || requestedSeatCount > 6) {
        output << "Passenger count must be between 1 and 6.\n";
        return false;
    }

    passengers.clear();

    output << "\n========== PASSENGER DETAILS ==========\n";
    output << "Enter details for " << requestedSeatCount
           << " passenger(s).\n";

    for (int i = 0; i < requestedSeatCount; ++i) {
        Passenger passenger{ "", 0, "" };
        std::string error;

        output << "\nPassenger " << i + 1 << ":\n";

        // Validate passenger name
        while (true) {
            output << "Enter name: ";
            if (!std::getline(input, passenger.name)) {
                return false;
            }

            if (Validation::isValidPassengerName(passenger.name, error)) {
                break;
            }

            output << "Error: " << error << " Please try again.\n";
        }

        // Validate passenger age
        while (true) {
            std::string ageInput;
            output << "Enter age: ";

            if (!std::getline(input, ageInput)) {
                return false;
            }

            try {
                std::size_t position = 0;
                int age = std::stoi(ageInput, &position);

                if (position != ageInput.size() || age < 1 || age > 120) {
                    output << "Error: Enter a valid age between 1 and 120.\n";
                    continue;
                }

                passenger.age = age;
                break;
            } catch (...) {
                output << "Error: Enter a valid whole-number age.\n";
            }
        }

        // Validate passenger gender
        while (true) {
            output << "Enter gender (Male/Female/Other): ";

            if (!std::getline(input, passenger.gender)) {
                return false;
            }

            std::string normalized = passenger.gender;

            std::transform(
                normalized.begin(),
                normalized.end(),
                normalized.begin(),
                [](unsigned char ch) {
                    return static_cast<char>(std::tolower(ch));
                }
            );

            if (normalized == "male") {
                passenger.gender = "Male";
                break;
            }

            if (normalized == "female") {
                passenger.gender = "Female";
                break;
            }

            if (normalized == "other") {
                passenger.gender = "Other";
                break;
            }

            output << "Error: Enter Male, Female or Other. Please try again.\n";
        }

        passengers.push_back(passenger);
    }

    if (static_cast<int>(passengers.size()) != requestedSeatCount) {
        output << "Error: Passenger count does not match requested seats.\n";
        return false;
    }

    output << "\nAll passenger details collected successfully.\n";
    return true;
}

}
