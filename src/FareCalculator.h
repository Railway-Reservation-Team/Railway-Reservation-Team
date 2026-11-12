#ifndef FARE_CALCULATOR_H
#define FARE_CALCULATOR_H

#include "Train.h"
#include <string>

// Provides fare calculation for a selected train class and passenger count.
namespace FareCalculator {

    double calculateFare(
        const Train& train,
        const std::string& travelClass,
        int passengerCount
    );

}

#endif