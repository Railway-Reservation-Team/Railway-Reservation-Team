
#ifndef TRAINDATA_H
#define TRAINDATA_H

#include "Train.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>

// Validate YYYY-MM-DD, including leap years and month lengths.
inline bool isValidDate(const std::string& date) {
    if (date.size() != 10 ||
        date[4] != '-' || date[7] != '-') {
        return false;
    }

    for (int i = 0; i < 10; ++i) {
        if (i != 4 && i != 7 &&
            !std::isdigit(static_cast<unsigned char>(date[i]))) {
            return false;
        }
    }

    int year = std::stoi(date.substr(0, 4));
    int month = std::stoi(date.substr(5, 2));
    int day = std::stoi(date.substr(8, 2));

    if (year < 1 || month < 1 || month > 12) {
        return false;
    }

    const int daysInMonth[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    int maxDays = daysInMonth[month - 1];

    bool leapYear = (year % 400 == 0) ||
                    (year % 4 == 0 && year % 100 != 0);

    if (month == 2 && leapYear) {
        maxDays = 29;
    }

    return day >= 1 && day <= maxDays;
}

// Convert one line from trains.dat into a Train object.
inline bool parseTrainRecord(const std::string& line, Train& train) {
    std::stringstream ss(line);
    std::vector<std::string> fields;
    std::string field;

    while (std::getline(ss, field, '|')) {
        fields.push_back(field);
    }

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

    if (train.trainNumber.empty() ||
        train.trainName.empty() ||
        train.source.empty() ||
        train.destination.empty() ||
        !isValidDate(train.journeyDate)) {
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

// Load trains and report the file status separately.
inline std::vector<Train> loadTrains(
    const std::string& filename,
    const std::string& journeyDate,
    std::string& errorMessage) {

    std::vector<Train> trains;
    std::ifstream file(filename);
    std::string line;

    errorMessage.clear();

    if (!file.is_open()) {
        errorMessage = "Unable to open train data file: " + filename;
        return trains;
    }

    bool hasRecords = false;

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        hasRecords = true;
        Train train;

        if (parseTrainRecord(line, train) &&
            train.journeyDate == journeyDate &&
            train.active) {
            trains.push_back(train);
        }
    }

    if (!hasRecords) {
        errorMessage = "Train data file is empty.";
    }

    return trains;
}

#endif
