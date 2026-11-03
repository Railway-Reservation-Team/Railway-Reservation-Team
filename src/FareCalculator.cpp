#include "FareCalculator.h"
#include <cmath>
#include <stdexcept>

namespace FareCalculator {

double calculateFare(
    const Train& train,
    const std::string& travelClass,
    int passengerCount) {

    if (passengerCount < 1 || passengerCount > 6)
        throw std::invalid_argument("Passenger count must be between 1 and 6.");

    double fare = 0.0;

    if (travelClass == "Sleeper")
        fare = train.sleeperFare;
    else if (travelClass == "AC 3-Tier")
        fare = train.ac3TierFare;
    else if (travelClass == "AC 2-Tier")
        fare = train.ac2TierFare;
    else if (travelClass == "Chair Car")
        fare = train.chairCarFare;
    else
        throw std::invalid_argument("Invalid travel class.");

    if (!std::isfinite(fare) || fare <= 0)
        throw std::invalid_argument("Selected class has an invalid or unavailable fare.");

    double total = fare * passengerCount;

    if (!std::isfinite(total))
        throw std::overflow_error("Calculated fare is invalid.");

    return total;
}

}
