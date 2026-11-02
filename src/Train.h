#ifndef TRAIN_H
#define TRAIN_H

#include <string>

struct Train {
    std::string trainNumber;
    std::string trainName;

    std::string source;
    std::string destination;
    std::string journeyDate;

    std::string departureTime;
    std::string arrivalTime;
    std::string journeyDuration;

    double sleeperFare = 0.0;
    double ac3TierFare = 0.0;
    double ac2TierFare = 0.0;
    double chairCarFare = 0.0;

    int sleeperSeatsAvailable = 0;
    int ac3TierSeatsAvailable = 0;
    int ac2TierSeatsAvailable = 0;
    int chairCarSeatsAvailable = 0;

    bool active = true;
};

#endif