
#include "SeatInventory.h"

#include <algorithm>
#include <cctype>
#include <string>

// Normalize class names by removing spaces and hyphens
// and converting characters to lowercase.
static std::string normalizeClass(const std::string& travelClass) {
    std::string result;

    for (unsigned char ch : travelClass) {
        if (!std::isspace(ch) && ch != '-') {
            result += static_cast<char>(std::tolower(ch));
        }
    }

    return result;
}

// Convert class aliases into standard class names.
static std::string canonicalClass(const std::string& travelClass) {
    const std::string selectedClass = normalizeClass(travelClass);

    if (selectedClass == "sleeper") {
        return "sleeper";
    }

    if (selectedClass == "ac3tier" || selectedClass == "3a") {
        return "ac 3-tier";
    }

    if (selectedClass == "ac2tier" || selectedClass == "2a") {
        return "ac 2-tier";
    }

    if (selectedClass == "chaircar" || selectedClass == "cc") {
        return "chair car";
    }

    return "";
}

// Returns available seats for the selected class.
// Returns -1 if the class is invalid.
int getAvailableSeats(
    const Train& train,
    const std::string& travelClass
) {
    const std::string selectedClass = canonicalClass(travelClass);

    if (selectedClass == "sleeper") {
        return train.sleeperSeatsAvailable;
    }

    if (selectedClass == "ac 3-tier") {
        return train.ac3TierSeatsAvailable;
    }

    if (selectedClass == "ac 2-tier") {
        return train.ac2TierSeatsAvailable;
    }

    if (selectedClass == "chair car") {
        return train.chairCarSeatsAvailable;
    }

    return -1;
}

// Validate the requested number of seats.
SeatRequestResult validateSeatRequest(
    int availableSeats,
    int requestedSeats
) {
    if (availableSeats < 0) {
        return SeatRequestResult::INVALID_AVAILABILITY;
    }

    if (requestedSeats <= 0) {
        return SeatRequestResult::INVALID_REQUEST;
    }

    if (requestedSeats > MAX_PASSENGERS_PER_BOOKING) {
        return SeatRequestResult::EXCEEDS_MAX_PASSENGERS;
    }

    if (requestedSeats > availableSeats) {
        return SeatRequestResult::EXCEEDS_AVAILABLE_SEATS;
    }

    return SeatRequestResult::VALID;
}

// Remove expired holds from the inventory.
void SeatInventoryManager::removeExpiredHolds(
    std::chrono::steady_clock::time_point currentTime
) {
    holds.erase(
        std::remove_if(
            holds.begin(),
            holds.end(),
            [currentTime](const SeatHold& hold) {
                return currentTime >= hold.expiryTime;
            }
        ),
        holds.end()
    );
}

// Public function to remove expired holds.
void SeatInventoryManager::expireHolds(
    std::chrono::steady_clock::time_point currentTime
) {
    removeExpiredHolds(currentTime);
}

// Return available seats after subtracting active holds.
int SeatInventoryManager::getLiveAvailableSeats(
    const Train& train,
    const std::string& travelClass,
    std::chrono::steady_clock::time_point currentTime
) {
    removeExpiredHolds(currentTime);

    const std::string selectedClass = canonicalClass(travelClass);

    if (selectedClass.empty()) {
        return -1;
    }

    const int totalAvailable =
        getAvailableSeats(train, selectedClass);

    if (totalAvailable < 0) {
        return -1;
    }

    int heldSeats = 0;

    for (const SeatHold& hold : holds) {
        if (hold.trainNumber == train.trainNumber &&
            hold.journeyDate == train.journeyDate &&
            hold.travelClass == selectedClass) {
            heldSeats += hold.seatCount;
        }
    }

    return totalAvailable - heldSeats;
}

// Temporarily reserve seats for five minutes.
SeatHoldResponse SeatInventoryManager::holdSeats(
    const Train& train,
    const std::string& travelClass,
    int requestedSeats,
    std::chrono::steady_clock::time_point currentTime
) {
    removeExpiredHolds(currentTime);

    const std::string selectedClass = canonicalClass(travelClass);

    if (selectedClass.empty()) {
        return {SeatHoldResult::INVALID_CLASS, ""};
    }

    const int availableSeats = getLiveAvailableSeats(
        train,
        selectedClass,
        currentTime
    );

    const SeatRequestResult validation =
        validateSeatRequest(availableSeats, requestedSeats);

    if (validation == SeatRequestResult::INVALID_REQUEST ||
        validation == SeatRequestResult::INVALID_AVAILABILITY) {
        return {SeatHoldResult::INVALID_REQUEST, ""};
    }

    if (validation == SeatRequestResult::EXCEEDS_MAX_PASSENGERS) {
        return {SeatHoldResult::EXCEEDS_MAX_PASSENGERS, ""};
    }

    if (validation == SeatRequestResult::EXCEEDS_AVAILABLE_SEATS) {
        return {SeatHoldResult::EXCEEDS_AVAILABLE_SEATS, ""};
    }

    const std::string holdId =
        "HOLD-" + std::to_string(nextHoldNumber++);

    SeatHold newHold;

    newHold.holdId = holdId;
    newHold.trainNumber = train.trainNumber;
    newHold.journeyDate = train.journeyDate;
    newHold.travelClass = selectedClass;
    newHold.seatCount = requestedSeats;

    newHold.expiryTime =
        currentTime +
        std::chrono::minutes(SEAT_HOLD_DURATION_MINUTES);

    holds.push_back(newHold);

    return {SeatHoldResult::SUCCESS, holdId};
}

// Manually release a temporary hold.
bool SeatInventoryManager::releaseHold(
    const std::string& holdId
) {
    const auto it = std::find_if(
        holds.begin(),
        holds.end(),
        [&holdId](const SeatHold& hold) {
            return hold.holdId == holdId;
        }
    );

    if (it == holds.end()) {
        return false;
    }

    holds.erase(it);

    return true;
}

// Confirm a valid hold and update the in-memory Train seat count.
bool SeatInventoryManager::confirmHold(
    Train& train,
    const std::string& holdId,
    std::chrono::steady_clock::time_point currentTime
) {
    removeExpiredHolds(currentTime);

    const auto it = std::find_if(
        holds.begin(),
        holds.end(),
        [&holdId, &train](const SeatHold& hold) {
            return hold.holdId == holdId &&
                   hold.trainNumber == train.trainNumber &&
                   hold.journeyDate == train.journeyDate;
        }
    );

    if (it == holds.end()) {
        return false;
    }

    int* seatCount = nullptr;

    if (it->travelClass == "sleeper") {
        seatCount = &train.sleeperSeatsAvailable;
    }
    else if (it->travelClass == "ac 3-tier") {
        seatCount = &train.ac3TierSeatsAvailable;
    }
    else if (it->travelClass == "ac 2-tier") {
        seatCount = &train.ac2TierSeatsAvailable;
    }
    else if (it->travelClass == "chair car") {
        seatCount = &train.chairCarSeatsAvailable;
    }

    if (seatCount == nullptr || *seatCount < it->seatCount) {
        return false;
    }

    *seatCount -= it->seatCount;

    holds.erase(it);

    return true;
}

// Return the number of active holds.
int SeatInventoryManager::getActiveHoldCount(
    std::chrono::steady_clock::time_point currentTime
) {
    removeExpiredHolds(currentTime);

    return static_cast<int>(holds.size());
}
