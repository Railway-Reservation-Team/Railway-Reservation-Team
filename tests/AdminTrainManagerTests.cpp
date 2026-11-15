
#include "../src/AdminTrainManager.h"
#include "../src/TrainData.h"

#include <cassert>
#include <filesystem>
#include <iterator>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const std::string TEST_FILE = "admin_train_test_data.dat";

Train createTrain() {
    Train train;

    train.trainNumber = "12345";
    train.trainName = "Test Express";
    train.source = "Bengaluru";
    train.destination = "Chennai";
    train.departureTime = "08:00";
    train.arrivalTime = "13:00";
    train.journeyDuration = "5h";
    train.sleeperFare = 500;
    train.ac3TierFare = 900;
    train.ac2TierFare = 1200;
    train.chairCarFare = 400;
    train.sleeperSeatsAvailable = 50;
    train.ac3TierSeatsAvailable = 30;
    train.ac2TierSeatsAvailable = 20;
    train.chairCarSeatsAvailable = 40;
    train.journeyDate = "2026-12-01";

    return train;
}

void prepareFile() {
    std::ofstream file(TEST_FILE);
    file.close();
}

void writeRawRecord(const std::string& filename,
                    const std::string& content) {
    std::ofstream file(filename, std::ios::trunc);
    file << content;
}

void expectInvalidTrain(const Train& train, const std::string& description) {
    std::string error;

    assert(!AdminTrainManager::addTrain(TEST_FILE, train, error));
    assert(error == "Invalid train details.");

    std::cout << "PASS: Reject " << description << '\n';
}

int main() {
    std::error_code auditFsError;
    std::filesystem::create_directories("data", auditFsError);
    if (auditFsError) return 1;

    std::filesystem::remove("data/admin_audit.log", auditFsError);

    std::string error;
    std::vector<Train> trains;

    prepareFile();

    Train train = createTrain();

    // Basic add operation
    assert(AdminTrainManager::addTrain(TEST_FILE, train, error));
    std::cout << "PASS: Add train\n";

    // Duplicate train
    assert(!AdminTrainManager::addTrain(TEST_FILE, train, error));
    std::cout << "PASS: Reject duplicate train\n";

    // View trains
    assert(AdminTrainManager::viewTrains(TEST_FILE, trains, error));
    assert(trains.size() == 1);
    assert(trains[0].trainNumber == "12345");
    std::cout << "PASS: View trains\n";

    // Update train
    Train updatedTrain = train;
    updatedTrain.trainName = "Updated Express";
    updatedTrain.sleeperFare = 600;

    assert(AdminTrainManager::updateTrain(
        TEST_FILE, "12345", updatedTrain, error
    ));

    trains.clear();
    assert(AdminTrainManager::viewTrains(TEST_FILE, trains, error));
    assert(trains[0].trainName == "Updated Express");
    assert(trains[0].sleeperFare == 600);
    std::cout << "PASS: Update train\n";

    // Update missing train
    assert(!AdminTrainManager::updateTrain(
        TEST_FILE, "99999", updatedTrain, error
    ));
    std::cout << "PASS: Reject update for missing train\n";

    // Update with changed train number
    Train changedNumberTrain = updatedTrain;
    changedNumberTrain.trainNumber = "99999";

    assert(!AdminTrainManager::updateTrain(
        TEST_FILE, "12345", changedNumberTrain, error
    ));
    assert(error == "Train number cannot be changed.");
    std::cout << "PASS: Reject changing train number\n";

    // Invalid update details
    Train invalidUpdate = updatedTrain;
    invalidUpdate.destination = invalidUpdate.source;

    assert(!AdminTrainManager::updateTrain(
        TEST_FILE, "12345", invalidUpdate, error
    ));
    assert(error == "Invalid updated train details.");
    std::cout << "PASS: Reject invalid updated train details\n";

    // Deactivate train
    assert(AdminTrainManager::deactivateTrain(TEST_FILE, "12345", error));
    assert(!AdminTrainManager::deactivateTrain(TEST_FILE, "12345", error));

    trains.clear();
    assert(AdminTrainManager::viewTrains(TEST_FILE, trains, error));
    assert(trains.size() == 1 && !trains[0].active);
    assert(loadTrains(TEST_FILE, "2026-12-01", error).empty());

    std::cout << "PASS: Deactivate train and exclude it from passenger search\n";

    // Updating a deactivated train must preserve its inactive state.
    updatedTrain.trainName = "Updated Deactivated Express";

    assert(AdminTrainManager::updateTrain(
        TEST_FILE, "12345", updatedTrain, error
    ));

    trains.clear();
    assert(AdminTrainManager::viewTrains(TEST_FILE, trains, error));
    assert(trains.size() == 1 && !trains[0].active);

    std::cout << "PASS: Train updates preserve deactivated state\n";

    // Delete train
    assert(AdminTrainManager::deleteTrain(TEST_FILE, "12345", error));

    trains.clear();
    assert(AdminTrainManager::viewTrains(TEST_FILE, trains, error));
    assert(trains.empty());

    std::cout << "PASS: Delete train\n";

    // Delete missing train
    assert(!AdminTrainManager::deleteTrain(TEST_FILE, "99999", error));
    std::cout << "PASS: Reject deletion of missing train\n";

    // Invalid train details: date
    Train invalidTrain = createTrain();
    invalidTrain.journeyDate = "invalid-date";
    expectInvalidTrain(invalidTrain, "invalid journey date");

    // Invalid train details: empty fields
    invalidTrain = createTrain();
    invalidTrain.trainNumber.clear();
    expectInvalidTrain(invalidTrain, "empty train number");

    invalidTrain = createTrain();
    invalidTrain.trainName.clear();
    expectInvalidTrain(invalidTrain, "empty train name");

    invalidTrain = createTrain();
    invalidTrain.source.clear();
    expectInvalidTrain(invalidTrain, "empty source");

    invalidTrain = createTrain();
    invalidTrain.destination.clear();
    expectInvalidTrain(invalidTrain, "empty destination");

    // Same source and destination
    invalidTrain = createTrain();
    invalidTrain.destination = invalidTrain.source;
    expectInvalidTrain(invalidTrain, "same source and destination");

    // Negative fares
    invalidTrain = createTrain();
    invalidTrain.sleeperFare = -1;
    expectInvalidTrain(invalidTrain, "negative sleeper fare");

    invalidTrain = createTrain();
    invalidTrain.ac3TierFare = -1;
    expectInvalidTrain(invalidTrain, "negative AC3 fare");

    invalidTrain = createTrain();
    invalidTrain.ac2TierFare = -1;
    expectInvalidTrain(invalidTrain, "negative AC2 fare");

    invalidTrain = createTrain();
    invalidTrain.chairCarFare = -1;
    expectInvalidTrain(invalidTrain, "negative chair car fare");

    // Negative seat counts
    invalidTrain = createTrain();
    invalidTrain.sleeperSeatsAvailable = -1;
    expectInvalidTrain(invalidTrain, "negative sleeper seats");

    invalidTrain = createTrain();
    invalidTrain.ac3TierSeatsAvailable = -1;
    expectInvalidTrain(invalidTrain, "negative AC3 seats");

    invalidTrain = createTrain();
    invalidTrain.ac2TierSeatsAvailable = -1;
    expectInvalidTrain(invalidTrain, "negative AC2 seats");

    invalidTrain = createTrain();
    invalidTrain.chairCarSeatsAvailable = -1;
    expectInvalidTrain(invalidTrain, "negative chair car seats");

    // Missing file tests
    const std::string missingFile =
        "missing_admin_train_directory/train.dat";

    std::filesystem::remove_all("missing_admin_train_directory");

    assert(!AdminTrainManager::addTrain(
        missingFile, createTrain(), error
    ));
    std::cout << "PASS: Reject add when data file cannot be opened\n";

    trains.push_back(createTrain());

    assert(!AdminTrainManager::viewTrains(
        missingFile, trains, error
    ));
    assert(trains.empty());
    std::cout << "PASS: Reject view when data file cannot be opened\n";

    assert(!AdminTrainManager::updateTrain(
        missingFile, "12345", createTrain(), error
    ));
    std::cout << "PASS: Reject update when data file cannot be opened\n";

    assert(!AdminTrainManager::deleteTrain(
        missingFile, "12345", error
    ));
    std::cout << "PASS: Reject delete when data file cannot be opened\n";

    assert(!AdminTrainManager::deactivateTrain(
        missingFile, "12345", error
    ));
    std::cout << "PASS: Reject deactivation when data file cannot be opened\n";

    // Malformed record test
    writeRawRecord(TEST_FILE, "this-is-not-a-valid-train-record\n");

    trains.push_back(createTrain());

    assert(!AdminTrainManager::viewTrains(TEST_FILE, trains, error));
    assert(trains.empty());
    assert(error == "Invalid train record found.");

    std::cout << "PASS: Reject malformed train record\n";

    // Malformed records must not be treated as valid trains.
    assert(!AdminTrainManager::deleteTrain(
        TEST_FILE, "12345", error
    ));
    std::cout << "PASS: Reject deletion when train record is malformed\n";

    assert(!AdminTrainManager::deactivateTrain(
        TEST_FILE, "12345", error
    ));
    std::cout << "PASS: Reject deactivation when train record is malformed\n";

    // Empty lines are ignored when reading records.
    writeRawRecord(TEST_FILE, "\n\n");

    trains.clear();
    assert(AdminTrainManager::viewTrains(TEST_FILE, trains, error));
    assert(trains.empty());

    std::cout << "PASS: Ignore empty train records\n";

    // Audit log verification
    {
        std::ifstream auditFile("data/admin_audit.log");

        const std::string audit(
            (std::istreambuf_iterator<char>(auditFile)),
            std::istreambuf_iterator<char>()
        );

        assert(audit.find("ADMIN_TRAIN_ADDED") != std::string::npos);
        assert(audit.find("ADMIN_TRAIN_UPDATED") != std::string::npos);
        assert(audit.find("ADMIN_TRAIN_DEACTIVATED") != std::string::npos);
        assert(audit.find("ADMIN_TRAIN_DELETED") != std::string::npos);

        std::cout << "PASS: Train administration actions are audited\n";
    }

    std::filesystem::remove_all("missing_admin_train_directory");
    std::remove(TEST_FILE.c_str());

    std::cout << "\nAll admin train management tests passed!\n";

    return 0;
}
