#ifndef PASSENGER_DETAILS_H
#define PASSENGER_DETAILS_H

#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

struct Passenger {
    std::string name;
    int age;
    std::string gender;
};

namespace PassengerDetails {
    bool collectPassengerDetails(
        int requestedSeatCount,
        std::vector<Passenger>& passengers,
        std::istream& input,
        std::ostream& output,
        const std::function<bool()>& onActivity = {}
    );
}

#endif
