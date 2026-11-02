#ifndef SEATINVENTORY_H
#define SEATINVENTORY_H

#include "Train.h"
#include <string>

// Returns available seats for the selected class.
// Returns -1 if the class name is invalid.
int getAvailableSeats(
    const Train& train,
    const std::string& travelClass
);

#endif
