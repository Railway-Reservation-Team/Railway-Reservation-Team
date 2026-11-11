#include "../src/AdminTrainManager.h"

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

int main() {
    std::error_code auditFsError;
    std::filesystem::create_directories("data", auditFsError);
    if (auditFsError) return 1;
    std::filesystem::remove("data/admin_audit.log", auditFsError);

    std::string error;
    std::vector<Train> trains;

    prepareFile();

    Train train = createTrain();

    assert(AdminTrainManager::addTrain(TEST_FILE, train, error));
    std::cout << "PASS: Add train\n";

    assert(!AdminTrainManager::addTrain(TEST_FILE, train, error));
    std::cout << "PASS: Reject duplicate train\n";

    assert(AdminTrainManager::viewTrains(TEST_FILE, trains, error));
    assert(trains.size() == 1);
    assert(trains[0].trainNumber == "12345");
    std::cout << "PASS: View trains\n";

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

    assert(!AdminTrainManager::updateTrain(
        TEST_FILE, "99999", updatedTrain, error
    ));
    std::cout << "PASS: Reject update for missing train\n";

    assert(AdminTrainManager::deleteTrain(TEST_FILE, "12345", error));

    trains.clear();
    assert(AdminTrainManager::viewTrains(TEST_FILE, trains, error));
    assert(trains.empty());
    std::cout << "PASS: Delete train\n";

    assert(!AdminTrainManager::deleteTrain(TEST_FILE, "99999", error));
    std::cout << "PASS: Reject deletion of missing train\n";

    Train invalidTrain = createTrain();
    invalidTrain.journeyDate = "invalid-date";

    assert(!AdminTrainManager::addTrain(TEST_FILE, invalidTrain, error));
    std::cout << "PASS: Reject invalid train details\n";

    {
        std::ifstream auditFile("data/admin_audit.log");
        const std::string audit(
            (std::istreambuf_iterator<char>(auditFile)),
            std::istreambuf_iterator<char>());
        assert(audit.find("ADMIN_TRAIN_ADDED") != std::string::npos);
        assert(audit.find("ADMIN_TRAIN_UPDATED") != std::string::npos);
        assert(audit.find("ADMIN_TRAIN_DELETED") != std::string::npos);
        std::cout << "PASS: Train administration actions are audited\n";
    }

    std::remove(TEST_FILE.c_str());

    std::cout << "\nAll 8 admin train management tests passed!\n";

    return 0;
}
