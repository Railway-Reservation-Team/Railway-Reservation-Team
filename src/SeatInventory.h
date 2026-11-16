#include <mutex>
#ifndef SEATINVENTORY_H
#define SEATINVENTORY_H

#include "Train.h"

#include <chrono>
#include <string>
#include <vector>

constexpr int MAX_PASSENGERS_PER_BOOKING = 6;
constexpr int SEAT_HOLD_DURATION_MINUTES = 5;

enum class SeatRequestResult {
    VALID,
    INVALID_REQUEST,
    EXCEEDS_AVAILABLE_SEATS,
    EXCEEDS_MAX_PASSENGERS,
    INVALID_AVAILABILITY
};

enum class SeatHoldResult {
    SUCCESS,
    INVALID_REQUEST,
    EXCEEDS_AVAILABLE_SEATS,
    EXCEEDS_MAX_PASSENGERS,
    INVALID_CLASS
};

struct SeatHold {
    std::string holdId;
    std::string trainNumber;
    std::string journeyDate;
    std::string travelClass;
    int seatCount;

    std::chrono::steady_clock::time_point expiryTime;
};

struct SeatHoldResponse {
    SeatHoldResult result;
    std::string holdId;
};

int getAvailableSeats(
    const Train& train,
    const std::string& travelClass
);

SeatRequestResult validateSeatRequest(
    int availableSeats,
    int requestedSeats
);

// Thread-safe manager for temporary seat holds and confirmations.
// Inventory operations are protected by a recursive mutex.
class SeatInventoryManager {
private:
    mutable std::recursive_mutex inventoryMutex;
    std::vector<SeatHold> holds;
    unsigned long nextHoldNumber = 1;

    void removeExpiredHolds(
        std::chrono::steady_clock::time_point currentTime
    );

public:
    // Returns available seats after subtracting active holds.
    int getLiveAvailableSeats(
        const Train& train,
        const std::string& travelClass,
        std::chrono::steady_clock::time_point currentTime =
            std::chrono::steady_clock::now()
    );

    // Temporarily holds seats for five minutes.
    SeatHoldResponse holdSeats(
        const Train& train,
        const std::string& travelClass,
        int requestedSeats,
        std::chrono::steady_clock::time_point currentTime =
            std::chrono::steady_clock::now()
    );

    // Releases a hold manually.
    bool releaseHold(
        const std::string& holdId
    );

    // Confirms a hold and reduces the in-memory seat count.
    bool confirmHold(
        Train& train,
        const std::string& holdId,
        std::chrono::steady_clock::time_point currentTime =
            std::chrono::steady_clock::now()
    );

    // Removes all expired holds.
    void expireHolds(
        std::chrono::steady_clock::time_point currentTime =
            std::chrono::steady_clock::now()
    );

    // Returns the number of active holds.
    int getActiveHoldCount(
        std::chrono::steady_clock::time_point currentTime =
            std::chrono::steady_clock::now()
    );
};

#endif
