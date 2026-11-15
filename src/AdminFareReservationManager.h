#ifndef ADMIN_FARE_RESERVATION_MANAGER_H
#define ADMIN_FARE_RESERVATION_MANAGER_H

#include "BookingManager.h"
#include "Train.h"

#include <string>
#include <vector>

namespace AdminFareReservationManager {

    bool updateFares(
        const std::string& filename,
        const std::string& trainNumber,
        const std::string& journeyDate,
        double sleeperFare,
        double ac3TierFare,
        double ac2TierFare,
        double chairCarFare,
        std::string& errorMessage
    );

    bool viewReservations(
        const std::string& filename,
        std::vector<Booking>& bookings,
        std::string& errorMessage
    );

    bool searchReservation(
        const std::string& filename,
        const std::string& pnr,
        Booking& booking,
        std::string& errorMessage
    );

    bool updateReservationStatus(
        const std::string& bookingsFile,
        const std::string& trainsFile,
        const std::string& pnr,
        const std::string& newStatus,
        std::string& errorMessage
    );
}

#endif
