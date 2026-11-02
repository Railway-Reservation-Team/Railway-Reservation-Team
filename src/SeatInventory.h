#ifndef SEATINVENTORY_H
#define SEATINVENTORY_H

#include "Train.h"
#include <string>

// Maximum number of passengers permitted per booking.
constexpr int MAX_PASSENGERS_PER_BOOKING = 6;

// Results returned when validating a seat request.
enum class SeatRequestResult {
    VALID,
    INVALID_REQUEST,
    EXCEEDS_AVAILABLE_SEATS,
    EXCEEDS_MAX_PASSENGERS,
    INVALID_AVAILABILITY
};

// Returns available seats for the selected class.
// Returns -1 if the class name is invalid.
int getAvailableSeats(
    const Train& train,
    const std::string& travelClass
);

// Validates the requested number of seats/passengers.
SeatRequestResult validateSeatRequest(
    int availableSeats,
    int requestedSeats
);

#endif
