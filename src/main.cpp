#include "TrainData.h"
#include "TrainSearch.h"
#include "Validation.h"
#include "FareCalculator.h"
#include "BookingManager.h"

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>

int main() {
    std::string source, destination, journeyDate;
    std::string passengerName, errorMessage;
    int seats = 0;

    std::cout << "===== Railway Reservation System =====\n";

    std::cout << "Enter source station: ";
    std::getline(std::cin, source);

    if (source.find_first_not_of(" \t") == std::string::npos) {
        std::cout << "Error: Source cannot be empty.\n";
        return 1;
    }

    std::cout << "Enter destination station: ";
    std::getline(std::cin, destination);

    if (destination.find_first_not_of(" \t") == std::string::npos) {
        std::cout << "Error: Destination cannot be empty.\n";
        return 1;
    }

    std::cout << "Enter journey date (YYYY-MM-DD): ";
    std::getline(std::cin, journeyDate);

    if (!Validation::isValidDate(journeyDate, errorMessage)) {
        std::cout << "Error: " << errorMessage << '\n';
        return 1;
    }

    const std::vector<Train> trains =
        loadTrains("data/trains.dat", journeyDate, errorMessage);

    if (!errorMessage.empty()) {
        std::cout << "Error: " << errorMessage << '\n';
        return 1;
    }

    const std::vector<Train> results =
        searchTrains(trains, source, destination, journeyDate);

    if (results.empty()) {
        std::cout << "No trains found for this route and date.\n";
        return 0;
    }

    std::cout << "\n========== MATCHING TRAINS ==========\n";

    for (size_t i = 0; i < results.size(); ++i) {
        const Train& t = results[i];

        std::cout << "\nTrain Option: " << i + 1
                  << "\nTrain Number: " << t.trainNumber
                  << "\nTrain Name: " << t.trainName
                  << "\nSource: " << t.source
                  << "\nDestination: " << t.destination
                  << "\nJourney Date: " << t.journeyDate
                  << "\nSleeper: Rs. " << t.sleeperFare
                  << " | Seats: " << t.sleeperSeatsAvailable
                  << "\nAC 3-Tier: Rs. " << t.ac3TierFare
                  << " | Seats: " << t.ac3TierSeatsAvailable
                  << "\nAC 2-Tier: Rs. " << t.ac2TierFare
                  << " | Seats: " << t.ac2TierSeatsAvailable
                  << "\nChair Car: Rs. " << t.chairCarFare
                  << " | Seats: " << t.chairCarSeatsAvailable << '\n';
    }

    int choice;
    std::cout << "\nSelect train option (1-" << results.size() << "): ";

    if (!(std::cin >> choice) ||
        choice < 1 || choice > static_cast<int>(results.size())) {
        std::cout << "Error: Invalid train selection.\n";
        return 1;
    }

    const Train selectedTrain = results[choice - 1];
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "\nEnter passenger name: ";
    std::getline(std::cin, passengerName);

    if (!Validation::isValidPassengerName(passengerName, errorMessage)) {
        std::cout << "Error: " << errorMessage << '\n';
        return 1;
    }

    std::string seatInput;
    std::cout << "Enter number of passengers (1-6): ";
    std::getline(std::cin, seatInput);

    if (!Validation::parseSeatCount(seatInput, seats, errorMessage)) {
        std::cout << "Error: " << errorMessage << '\n';
        return 1;
    }

    int classChoice;
    std::cout << "\n1. Sleeper\n2. AC 3-Tier\n3. AC 2-Tier\n4. Chair Car\n";
    std::cout << "Select travel class: ";

    if (!(std::cin >> classChoice) || classChoice < 1 || classChoice > 4) {
        std::cout << "Error: Invalid class selection.\n";
        return 1;
    }

    std::string travelClass;
    int availableSeats = 0;

    switch (classChoice) {
        case 1:
            travelClass = "Sleeper";
            availableSeats = selectedTrain.sleeperSeatsAvailable;
            break;
        case 2:
            travelClass = "AC 3-Tier";
            availableSeats = selectedTrain.ac3TierSeatsAvailable;
            break;
        case 3:
            travelClass = "AC 2-Tier";
            availableSeats = selectedTrain.ac2TierSeatsAvailable;
            break;
        case 4:
            travelClass = "Chair Car";
            availableSeats = selectedTrain.chairCarSeatsAvailable;
            break;
    }

    if (availableSeats < seats) {
        std::cout << "Error: Not enough seats available. Booking rejected.\n";
        return 1;
    }

    try {
        const double totalFare =
            FareCalculator::calculateFare(selectedTrain, travelClass, seats);

        std::cout << "\n========== FARE DETAILS ==========\n"
                  << "Passenger: " << passengerName
                  << "\nTravel Class: " << travelClass
                  << "\nPassengers: " << seats
                  << "\nFare Per Passenger: Rs. " << totalFare / seats
                  << "\nTotal Fare: Rs. " << totalFare << '\n';

        char confirm;
        std::cout << "\nConfirm booking? (Y/N): ";
        std::cin >> confirm;

        if (confirm != 'Y' && confirm != 'y') {
            std::cout << "Booking cancelled by user. No seats allocated.\n";
            return 0;
        }

        std::string pnr;

        if (!BookingManager::createBooking(
                selectedTrain, travelClass, passengerName,
                seats, totalFare, pnr, errorMessage)) {
            std::cout << "Booking failed: " << errorMessage << '\n';
            return 1;
        }

        std::cout << "\n========== BOOKING CONFIRMED ==========\n"
                  << "PNR: " << pnr
                  << "\nTrain: " << selectedTrain.trainName
                  << "\nTrain Number: " << selectedTrain.trainNumber
                  << "\nJourney Date: " << journeyDate
                  << "\nPassenger: " << passengerName
                  << "\nTravel Class: " << travelClass
                  << "\nSeats Allocated: " << seats
                  << "\nTotal Fare: Rs. " << totalFare
                  << "\nBooking Status: Confirmed\n"
                  << "\nYour seats have been successfully allocated.\n";
    }
    catch (const std::exception& e) {
        std::cout << "Booking error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
