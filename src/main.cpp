
#include "TrainData.h"
#include "TrainSearch.h"
#include "Validation.h"

#include <iostream>
#include <string>
#include <vector>

int main() {

    std::string source;
    std::string destination;
    std::string journeyDate;
    std::string passengerName;
    std::string errorMessage;

    int seats = 0;

    std::cout << "===== Railway Reservation System =====\n";

    // 1. Get source station
    std::cout << "Enter source station: ";
    std::getline(std::cin, source);

    if (source.find_first_not_of(" \t") == std::string::npos) {
        std::cout << "\nError: Source station cannot be empty.\n";
        return 1;
    }

    // 2. Get destination station
    std::cout << "Enter destination station: ";
    std::getline(std::cin, destination);

    if (destination.find_first_not_of(" \t") == std::string::npos) {
        std::cout << "\nError: Destination station cannot be empty.\n";
        return 1;
    }

    // 3. Get and validate journey date
    std::cout << "Enter journey date (YYYY-MM-DD): ";
    std::getline(std::cin, journeyDate);

    if (!Validation::isValidDate(journeyDate, errorMessage)) {
        std::cout << "\nError: " << errorMessage << '\n';
        return 1;
    }

    // 4. Load train records
    const std::vector<Train> trains =
        loadTrains("data/trains.dat", journeyDate, errorMessage);

    if (!errorMessage.empty()) {
        std::cout << "\nError: " << errorMessage << '\n';
        return 1;
    }

    if (trains.empty()) {
        std::cout << "\nNo train records found for the selected date.\n";
        return 0;
    }

    // 5. Search trains
    const std::vector<Train> results =
        searchTrains(trains, source, destination, journeyDate);

    if (results.empty()) {
        std::cout << "\nNo trains found for the selected route and date.\n";
        return 0;
    }

    // 6. Display matching trains
    std::cout << "\n========== MATCHING TRAINS ==========\n";

    for (const Train& train : results) {

        std::cout << "\nTrain Number: " << train.trainNumber
                  << "\nTrain Name: " << train.trainName
                  << "\nSource: " << train.source
                  << "\nDestination: " << train.destination
                  << "\nJourney Date: " << train.journeyDate
                  << "\nDeparture: " << train.departureTime
                  << "\nArrival: " << train.arrivalTime
                  << "\nDuration: " << train.journeyDuration
                  << "\n\nClass-wise fares and available seats:\n";

        std::cout << "Sleeper: Rs. " << train.sleeperFare
                  << " | Seats: " << train.sleeperSeatsAvailable << '\n';

        std::cout << "AC 3-Tier: Rs. " << train.ac3TierFare
                  << " | Seats: " << train.ac3TierSeatsAvailable << '\n';

        std::cout << "AC 2-Tier: Rs. " << train.ac2TierFare
                  << " | Seats: " << train.ac2TierSeatsAvailable << '\n';

        std::cout << "Chair Car: Rs. " << train.chairCarFare
                  << " | Seats: " << train.chairCarSeatsAvailable << '\n';
    }

    // 7. Validate passenger name
    std::cout << "\n========== PASSENGER DETAILS ==========\n";

    std::cout << "Enter passenger name: ";
    std::getline(std::cin, passengerName);

    if (!Validation::isValidPassengerName(passengerName, errorMessage)) {
        std::cout << "\nError: " << errorMessage << '\n';
        return 1;
    }

    // 8. Validate passenger count
    std::string seatInput;

    std::cout << "Enter number of passengers (1-6): ";
    std::getline(std::cin, seatInput);

    if (!Validation::parseSeatCount(seatInput, seats, errorMessage)) {
        std::cout << "\nError: " << errorMessage << '\n';
        return 1;
    }

    // 9. Display validated details
    std::cout << "\n========== VALIDATION SUCCESSFUL ==========\n";

    std::cout << "Passenger Name: " << passengerName << '\n';
    std::cout << "Number of Passengers: " << seats << '\n';

    std::cout << "\nInput validation completed successfully!\n";
    std::cout << "Train search completed successfully!\n";

    return 0;
}
