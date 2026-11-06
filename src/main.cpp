
#include "TrainData.h"
#include "TrainSearch.h"
#include "Validation.h"
#include "FareCalculator.h"
#include "BookingManager.h"
#include "PassengerDetails.h"

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <iomanip>

int main() {
    std::string source, destination, journeyDate, errorMessage;
    int seats = 0;

    std::cout << "===== Railway Reservation System =====\n";

    std::cout << "Enter source station: ";
    std::getline(std::cin, source);

    if (source.find_first_not_of(" \t") == std::string::npos) {
        std::cout << "Error: Source station cannot be empty.\n";
        return 1;
    }

    std::cout << "Enter destination station: ";
    std::getline(std::cin, destination);

    if (destination.find_first_not_of(" \t") == std::string::npos) {
        std::cout << "Error: Destination station cannot be empty.\n";
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

    if (trains.empty()) {
        std::cout << "No train records found for the selected date.\n";
        return 0;
    }

    const std::vector<Train> results =
        searchTrains(trains, source, destination, journeyDate);

    if (results.empty()) {
        std::cout << "No trains found for the selected route and date.\n";
        return 0;
    }

    std::cout << "\n========== MATCHING TRAINS ==========\n";

    for (size_t i = 0; i < results.size(); ++i) {
        const Train& train = results[i];

        std::cout << "\nTrain Option: " << i + 1
                  << "\nTrain Number: " << train.trainNumber
                  << "\nTrain Name: " << train.trainName
                  << "\nSource: " << train.source
                  << "\nDestination: " << train.destination
                  << "\nJourney Date: " << train.journeyDate
                  << "\nDeparture: " << train.departureTime
                  << "\nArrival: " << train.arrivalTime
                  << "\nDuration: " << train.journeyDuration
                  << "\nSleeper: Rs. " << train.sleeperFare
                  << " | Seats: " << train.sleeperSeatsAvailable
                  << "\nAC 3-Tier: Rs. " << train.ac3TierFare
                  << " | Seats: " << train.ac3TierSeatsAvailable
                  << "\nAC 2-Tier: Rs. " << train.ac2TierFare
                  << " | Seats: " << train.ac2TierSeatsAvailable
                  << "\nChair Car: Rs. " << train.chairCarFare
                  << " | Seats: " << train.chairCarSeatsAvailable << '\n';
    }

    int trainChoice;
    std::cout << "\nSelect train option (1-" << results.size() << "): ";

    if (!(std::cin >> trainChoice) ||
        trainChoice < 1 ||
        trainChoice > static_cast<int>(results.size())) {
        std::cout << "Error: Invalid train selection.\n";
        return 1;
    }

    const Train& selectedTrain = results[trainChoice - 1];
    std::cin.ignore(10000, '\n');

    std::string seatInput;
    std::cout << "Enter number of passengers (1-6): ";
    std::getline(std::cin, seatInput);

    if (!Validation::parseSeatCount(seatInput, seats, errorMessage)) {
        std::cout << "Error: " << errorMessage << '\n';
        return 1;
    }

    int classChoice;
    std::cout << "\n1. Sleeper\n2. AC 3-Tier\n"
              << "3. AC 2-Tier\n4. Chair Car\n";
    std::cout << "Enter your class choice (1-4): ";

    if (!(std::cin >> classChoice) || classChoice < 1 || classChoice > 4) {
        std::cout << "Error: Invalid travel class selection.\n";
        return 1;
    }

    std::string travelClass;

    switch (classChoice) {
        case 1: travelClass = "Sleeper"; break;
        case 2: travelClass = "AC 3-Tier"; break;
        case 3: travelClass = "AC 2-Tier"; break;
        case 4: travelClass = "Chair Car"; break;
    }

    std::vector<Passenger> passengers;

    std::cin.ignore(10000, '\n');

    if (!PassengerDetails::collectPassengerDetails(
            seats, passengers, std::cin, std::cout)) {
        std::cout << "Could not collect passenger details.\n";
        return 1;
    }

    try {
        double totalFare = FareCalculator::calculateFare(
            selectedTrain, travelClass, seats
        );

        std::string pnr;

        if (!BookingManager::createBooking(
                selectedTrain, travelClass, passengers,
                totalFare, pnr, errorMessage)) {
            std::cout << "\nBooking failed: " << errorMessage << '\n';
            return 1;
        }

        Booking booking;

        if (!BookingManager::findBooking(pnr, booking)) {
            std::cout << "Booking saved, but PNR retrieval failed.\n";
            return 1;
        }

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "\n========================================\n";
        std::cout << "    BOOKING CONFIRMED SUCCESSFULLY\n";
        std::cout << "========================================\n";
        std::cout << "PNR Number: " << booking.pnr << '\n';
        std::cout << "Train Number: " << booking.trainNumber << '\n';
        std::cout << "Journey Date: " << booking.journeyDate << '\n';
        std::cout << "Travel Class: " << booking.travelClass << '\n';
        std::cout << "Passenger Count: " << booking.passengerCount << '\n';
        std::cout << "Total Fare: Rs. " << booking.totalFare << '\n';
        std::cout << "Status: " << booking.status << '\n';
        std::cout << "========================================\n";

    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
