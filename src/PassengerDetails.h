#ifndef PASSENGER_DETAILS_H
#define PASSENGER_DETAILS_H

#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

// Stores the personal details of one passenger.
struct Passenger {
    std::string name;
    int age;
    std::string gender;
};

// Collects and validates passenger information from an input stream.
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
