#ifndef BOOKING_MANAGER_H
#define BOOKING_MANAGER_H

#include "Train.h"
#include "PassengerDetails.h"

#include <string>
#include <vector>

struct Booking {
    std::string pnr;
    std::string trainNumber;
    std::string journeyDate;
    std::string travelClass;
    std::string passengerDetails;
    int passengerCount = 0;
    double totalFare = 0.0;
    std::string status;
};

namespace BookingManager {

    bool createBooking(
        const Train& train,
        const std::string& travelClass,
        const std::vector<Passenger>& passengers,
        double totalFare,
        std::string& pnr,
        std::string& errorMessage
    );

    bool findBooking(
        const std::string& pnr,
        Booking& booking
    );

}

#endif
