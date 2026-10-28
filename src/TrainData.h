#ifndef TRAINDATA_H
#define TRAINDATA_H

#include "Train.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Convert one line from trains.dat into a Train object.
inline bool parseTrainRecord(const std::string& line, Train& train) {
    std::stringstream ss(line);
    std::vector<std::string> fields;
    std::string field;

    while (std::getline(ss, field, '|')) {
        fields.push_back(field);
    }

    // Every record must contain exactly 16 fields.
    if (fields.size() != 16) {
        return false;
    }

    try {
        train.trainNumber = fields[0];
        train.trainName = fields[1];
        train.source = fields[2];
        train.destination = fields[3];

        train.departureTime = fields[4];
        train.arrivalTime = fields[5];
        train.journeyDuration = fields[6];

        train.sleeperFare = std::stod(fields[7]);
        train.ac3TierFare = std::stod(fields[8]);
        train.ac2TierFare = std::stod(fields[9]);
        train.chairCarFare = std::stod(fields[10]);

        train.sleeperSeatsAvailable = std::stoi(fields[11]);
        train.ac3TierSeatsAvailable = std::stoi(fields[12]);
        train.ac2TierSeatsAvailable = std::stoi(fields[13]);
        train.chairCarSeatsAvailable = std::stoi(fields[14]);

        train.journeyDate = fields[15];
        train.active = true;
    } catch (...) {
        return false;
    }

    // Basic validation of required fields and values.
    if (train.trainNumber.empty() ||
        train.trainName.empty() ||
        train.source.empty() ||
        train.destination.empty() ||
        train.journeyDate.empty()) {
        return false;
    }

    if (train.sleeperFare < 0 ||
        train.ac3TierFare < 0 ||
        train.ac2TierFare < 0 ||
        train.chairCarFare < 0 ||
        train.sleeperSeatsAvailable < 0 ||
        train.ac3TierSeatsAvailable < 0 ||
        train.ac2TierSeatsAvailable < 0 ||
        train.chairCarSeatsAvailable < 0) {
        return false;
    }

    return true;
}

// Load trains that operate on the requested journey date.
inline std::vector<Train> loadTrains(
    const std::string& filename,
    const std::string& journeyDate) {

    std::vector<Train> trains;
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        return trains;
    }

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        Train train;

        if (parseTrainRecord(line, train) &&
            train.journeyDate == journeyDate &&
            train.active) {
            trains.push_back(train);
        }
    }

    return trains;
}

#endif