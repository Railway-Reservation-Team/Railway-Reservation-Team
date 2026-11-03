#ifndef BOOKING_MANAGER_H
#define BOOKING_MANAGER_H

#include "Train.h"
#include <string>

namespace BookingManager {
    bool createBooking(
        const Train& train,
        const std::string& travelClass,
        const std::string& passengerName,
        int passengerCount,
        double totalFare,
        std::string& pnr,
        std::string& errorMessage
    );
}

#endif
