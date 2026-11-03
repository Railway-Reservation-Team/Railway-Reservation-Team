#include "FareCalculator.h"

#include <stdexcept>

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

    if (farePerPassenger <= 0) {
        throw std::invalid_argument(
            "Selected class is not available."
        );
    }

    return farePerPassenger * passengerCount;
}

}