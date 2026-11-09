#include "AdminTrainManager.h"
#include "TrainData.h"
#include "AtomicFileReplace.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdio>

namespace {

bool validTrain(const Train& train) {
    return !train.trainNumber.empty() &&
           !train.trainName.empty() &&
           !train.source.empty() &&
           !train.destination.empty() &&
           train.source != train.destination &&
           isValidDate(train.journeyDate) &&
           train.sleeperFare >= 0 &&
           train.ac3TierFare >= 0 &&
           train.ac2TierFare >= 0 &&
           train.chairCarFare >= 0 &&
           train.sleeperSeatsAvailable >= 0 &&
           train.ac3TierSeatsAvailable >= 0 &&
           train.ac2TierSeatsAvailable >= 0 &&
           train.chairCarSeatsAvailable >= 0;
}

std::string serializeTrain(const Train& train) {
    std::ostringstream record;

    record << train.trainNumber << '|'
           << train.trainName << '|'
           << train.source << '|'
           << train.destination << '|'
           << train.departureTime << '|'
           << train.arrivalTime << '|'
           << train.journeyDuration << '|'
           << train.sleeperFare << '|'
           << train.ac3TierFare << '|'
           << train.ac2TierFare << '|'
           << train.chairCarFare << '|'
           << train.sleeperSeatsAvailable << '|'
           << train.ac3TierSeatsAvailable << '|'
           << train.ac2TierSeatsAvailable << '|'
           << train.chairCarSeatsAvailable << '|'
           << train.journeyDate;

    return record.str();
}

bool readRecords(
    const std::string& filename,
    std::vector<std::string>& records,
    std::string& errorMessage) {

    std::ifstream file(filename);

    if (!file.is_open()) {
        errorMessage = "Unable to open train data file.";
        return false;
    }

    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty()) {
            records.push_back(line);
        }
    }

    if (file.bad()) {
        errorMessage = "Failed to read train data file.";
        return false;
    }

    return true;
}

bool writeRecords(
    const std::string& filename,
    const std::vector<std::string>& records,
    std::string& errorMessage) {

    const std::string temporaryFile = filename + ".tmp";
    std::ofstream file(temporaryFile, std::ios::trunc);

    if (!file.is_open()) {
        errorMessage = "Unable to write temporary train data file.";
        return false;
    }

    for (const std::string& record : records) {
        file << record << '\n';
    }

    file.close();

    if (!file) {
        std::remove(temporaryFile.c_str());
        errorMessage = "Failed to save train records.";
        return false;
    }

    if (!replaceFileAtomically(temporaryFile, filename)) {
        std::remove(temporaryFile.c_str());
        errorMessage = "Unable to replace train data file.";
        return false;
    }

    return true;
}

bool findTrain(
    const std::vector<std::string>& records,
    const std::string& trainNumber,
    std::size_t& index) {

    for (std::size_t i = 0; i < records.size(); ++i) {
        Train train;

        if (parseTrainRecord(records[i], train) &&
            train.trainNumber == trainNumber) {
            index = i;
            return true;
        }
    }

    return false;
}

}

namespace AdminTrainManager {

bool addTrain(
    const std::string& filename,
    const Train& train,
    std::string& errorMessage) {

    errorMessage.clear();

    if (!validTrain(train)) {
        errorMessage = "Invalid train details.";
        return false;
    }

    std::vector<std::string> records;

    if (!readRecords(filename, records, errorMessage)) {
        return false;
    }

    std::size_t index;

    if (findTrain(records, train.trainNumber, index)) {
        errorMessage = "Train number already exists.";
        return false;
    }

    records.push_back(serializeTrain(train));

    return writeRecords(filename, records, errorMessage);
}

bool viewTrains(
    const std::string& filename,
    std::vector<Train>& trains,
    std::string& errorMessage) {

    trains.clear();
    errorMessage.clear();

    std::vector<std::string> records;

    if (!readRecords(filename, records, errorMessage)) {
        return false;
    }

    for (const std::string& record : records) {
        Train train;

        if (!parseTrainRecord(record, train)) {
            errorMessage = "Invalid train record found.";
            trains.clear();
            return false;
        }

        trains.push_back(train);
    }

    return true;
}

bool updateTrain(
    const std::string& filename,
    const std::string& trainNumber,
    const Train& updatedTrain,
    std::string& errorMessage) {

    errorMessage.clear();

    if (!validTrain(updatedTrain)) {
        errorMessage = "Invalid updated train details.";
        return false;
    }

    if (trainNumber != updatedTrain.trainNumber) {
        errorMessage = "Train number cannot be changed.";
        return false;
    }

    std::vector<std::string> records;

    if (!readRecords(filename, records, errorMessage)) {
        return false;
    }

    std::size_t index;

    if (!findTrain(records, trainNumber, index)) {
        errorMessage = "Train not found.";
        return false;
    }

    records[index] = serializeTrain(updatedTrain);

    return writeRecords(filename, records, errorMessage);
}

bool deleteTrain(
    const std::string& filename,
    const std::string& trainNumber,
    std::string& errorMessage) {

    errorMessage.clear();

    std::vector<std::string> records;

    if (!readRecords(filename, records, errorMessage)) {
        return false;
    }

    std::size_t index;

    if (!findTrain(records, trainNumber, index)) {
        errorMessage = "Train not found.";
        return false;
    }

    records.erase(records.begin() + index);

    return writeRecords(filename, records, errorMessage);
}

}
