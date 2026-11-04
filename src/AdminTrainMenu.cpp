#include "AdminTrainMenu.h"
#include "AdminTrainManager.h"
#include "Train.h"
#include <cmath>

#include <string>
#include <vector>
#include <limits>

namespace {

const std::string TRAIN_FILE = "data/trains.dat";

bool readNumber(
    std::istream& input,
    std::ostream& output,
    const std::string& prompt,
    double& value) {

    std::string text;

    output << prompt;

    if (!std::getline(input, text)) return false;

    try {
        size_t position = 0;
        value = std::stod(text, &position);

        if (position != text.size() ||
            value < 0 ||
            !std::isfinite(value)) {
            return false;
        }

        return true;
    } catch (...) {
        return false;
    }
}

bool readInteger(
    std::istream& input,
    std::ostream& output,
    const std::string& prompt,
    int& value) {

    std::string text;

    output << prompt;

    if (!std::getline(input, text)) return false;

    try {
        size_t position = 0;
        value = std::stoi(text, &position);

        return position == text.size() && value >= 0;
    } catch (...) {
        return false;
    }
}

bool readTrain(
    std::istream& input,
    std::ostream& output,
    Train& train) {

    output << "Train number: ";
    if (!std::getline(input, train.trainNumber)) return false;

    output << "Train name: ";
    if (!std::getline(input, train.trainName)) return false;

    output << "Source: ";
    if (!std::getline(input, train.source)) return false;

    output << "Destination: ";
    if (!std::getline(input, train.destination)) return false;

    output << "Departure time (HH:MM): ";
    if (!std::getline(input, train.departureTime)) return false;

    output << "Arrival time (HH:MM): ";
    if (!std::getline(input, train.arrivalTime)) return false;

    output << "Journey duration: ";
    if (!std::getline(input, train.journeyDuration)) return false;

    output << "Journey date (YYYY-MM-DD): ";
    if (!std::getline(input, train.journeyDate)) return false;

    if (!readNumber(input, output, "Sleeper fare: ", train.sleeperFare))
        return false;

    if (!readNumber(input, output, "AC 3-Tier fare: ", train.ac3TierFare))
        return false;

    if (!readNumber(input, output, "AC 2-Tier fare: ", train.ac2TierFare))
        return false;

    if (!readNumber(input, output, "Chair Car fare: ", train.chairCarFare))
        return false;

    if (!readInteger(input, output, "Sleeper seats: ", train.sleeperSeatsAvailable))
        return false;

    if (!readInteger(input, output, "AC 3-Tier seats: ", train.ac3TierSeatsAvailable))
        return false;

    if (!readInteger(input, output, "AC 2-Tier seats: ", train.ac2TierSeatsAvailable))
        return false;

    if (!readInteger(input, output, "Chair Car seats: ", train.chairCarSeatsAvailable))
        return false;

    return true;
}

void addTrain(std::istream& input, std::ostream& output) {
    Train train;
    std::string error;

    output << "\n========== ADD TRAIN ==========\n";

    if (!readTrain(input, output, train)) {
        output << "Invalid input. Train was not added.\n";
        return;
    }

    if (AdminTrainManager::addTrain(TRAIN_FILE, train, error))
        output << "Train added successfully.\n";
    else
        output << "Error: " << error << '\n';
}

void viewTrains(std::ostream& output) {
    std::vector<Train> trains;
    std::string error;

    if (!AdminTrainManager::viewTrains(TRAIN_FILE, trains, error)) {
        output << "Error: " << error << '\n';
        return;
    }

    if (trains.empty()) {
        output << "No trains available.\n";
        return;
    }

    output << "\n========== ALL TRAINS ==========\n";

    for (const Train& train : trains) {
        output << "\nTrain Number: " << train.trainNumber
               << "\nTrain Name: " << train.trainName
               << "\nRoute: " << train.source << " -> " << train.destination
               << "\nDate: " << train.journeyDate
               << "\nDeparture: " << train.departureTime
               << "\nArrival: " << train.arrivalTime
               << "\nSleeper Fare: Rs. " << train.sleeperFare
               << "\nAC 3-Tier Fare: Rs. " << train.ac3TierFare
               << "\nAC 2-Tier Fare: Rs. " << train.ac2TierFare
               << "\nChair Car Fare: Rs. " << train.chairCarFare
               << "\n";
    }
}

void updateTrain(std::istream& input, std::ostream& output) {
    std::string trainNumber;
    std::string error;
    Train train;

    output << "Enter train number to update: ";
    if (!std::getline(input, trainNumber)) return;

    output << "Enter updated train details:\n";

    if (!readTrain(input, output, train)) {
        output << "Invalid input. Train was not updated.\n";
        return;
    }

    if (AdminTrainManager::updateTrain(
            TRAIN_FILE, trainNumber, train, error))
        output << "Train updated successfully.\n";
    else
        output << "Error: " << error << '\n';
}

void deleteTrain(std::istream& input, std::ostream& output) {
    std::string trainNumber;
    std::string error;

    output << "Enter train number to delete: ";
    if (!std::getline(input, trainNumber)) return;

    if (AdminTrainManager::deleteTrain(TRAIN_FILE, trainNumber, error))
        output << "Train deleted successfully.\n";
    else
        output << "Error: " << error << '\n';
}

}

namespace AdminTrainMenu {

void run(std::istream& input, std::ostream& output) {
    std::string choice;

    while (true) {
        output << "\n========== ADMIN TRAIN MANAGEMENT ==========\n"
               << "1. Add Train\n"
               << "2. View All Trains\n"
               << "3. Update Train\n"
               << "4. Delete Train\n"
               << "5. Back to Main Menu\n"
               << "Enter choice: ";

        if (!std::getline(input, choice)) return;

        if (choice == "1")
            addTrain(input, output);
        else if (choice == "2")
            viewTrains(output);
        else if (choice == "3")
            updateTrain(input, output);
        else if (choice == "4")
            deleteTrain(input, output);
        else if (choice == "5")
            return;
        else
            output << "Invalid choice. Please try again.\n";
    }
}

}
