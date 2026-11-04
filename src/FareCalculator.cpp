
#include "FareCalculator.h"
#include "Validation.h"

#include <cmath>
#include <stdexcept>
#include <string>

namespace FareCalculator {

double calculateFare(
    const Train& train,
    const std::string& travelClass,
    int passengerCount) {

    if (passengerCount < 1 || passengerCount > 6) {
        throw std::invalid_argument(
            "Passenger count must be between 1 and 6."
        );
    }

    double farePerPassenger = 0.0;

    if (travelClass == "Sleeper") {
        farePerPassenger = train.sleeperFare;
    }
    else if (travelClass == "AC 3-Tier") {
        farePerPassenger = train.ac3TierFare;
    }
    else if (travelClass == "AC 2-Tier") {
        farePerPassenger = train.ac2TierFare;
    }
    else if (travelClass == "Chair Car") {
        farePerPassenger = train.chairCarFare;
    }
    else {
        throw std::invalid_argument("Invalid travel class.");
    }

    if (!std::isfinite(farePerPassenger) ||
        farePerPassenger <= 0) {
        throw std::invalid_argument(
            "Selected class has an invalid or unavailable fare."
        );
    }

    double totalFare = farePerPassenger * passengerCount;

    if (!std::isfinite(totalFare)) {
        throw std::overflow_error("Calculated fare is too large.");
    }

    return totalFare;
}

}
