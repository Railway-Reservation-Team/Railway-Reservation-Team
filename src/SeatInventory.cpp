#include "SeatInventory.h"

#include <algorithm>
#include <cctype>
#include <string>

static std::string normalizeClass(const std::string& travelClass) {
    std::string result = travelClass;

    result.erase(
        result.begin(),
        std::find_if(result.begin(), result.end(),
            [](unsigned char ch) {
                return !std::isspace(ch);
            })
    );

    result.erase(
        std::find_if(result.rbegin(), result.rend(),
            [](unsigned char ch) {
                return !std::isspace(ch);
            }).base(),
        result.end()
    );

    std::transform(
        result.begin(), result.end(), result.begin(),
        [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        }
    );

    return result;
}

int getAvailableSeats(
    const Train& train,
    const std::string& travelClass
) {
    const std::string selectedClass = normalizeClass(travelClass);

    if (selectedClass == "sleeper") {
        return train.sleeperSeatsAvailable;
    }

    if (selectedClass == "ac 3-tier" ||
        selectedClass == "ac3tier" ||
        selectedClass == "3a") {
        return train.ac3TierSeatsAvailable;
    }

    if (selectedClass == "ac 2-tier" ||
        selectedClass == "ac2tier" ||
        selectedClass == "2a") {
        return train.ac2TierSeatsAvailable;
    }

    if (selectedClass == "chair car" ||
        selectedClass == "chaircar" ||
        selectedClass == "cc") {
        return train.chairCarSeatsAvailable;
    }

    return -1;
}
