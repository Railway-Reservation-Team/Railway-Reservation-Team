
#ifndef BOOKING_H
#define BOOKING_H

#include "Train.h"
#include <string>

struct Booking {
    std::string pnr;
    std::string passengerName;
    std::string trainNumber;
    std::string trainName;
    std::string source;
    std::string destination;
    std::string journeyDate;
    std::string travelClass;

    int numberOfPassengers = 0;
    double totalFare = 0.0;
};

std::string generateUniquePNR();
bool saveBooking(const Booking& booking, std::string& errorMessage);

#endif
