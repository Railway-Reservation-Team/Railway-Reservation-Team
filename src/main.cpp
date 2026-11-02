
#include "TrainData.h"
#include "TrainSearch.h"

#include <iostream>
#include <string>
#include <vector>

int main() {
    std::string source;
    std::string destination;
    std::string journeyDate;
    std::string errorMessage;

    std::cout << "===== Railway Reservation System =====\n";

    std::cout << "Enter source station: ";
    std::getline(std::cin, source);

    std::cout << "Enter destination station: ";
    std::getline(std::cin, destination);

    std::cout << "Enter journey date (YYYY-MM-DD): ";
    std::getline(std::cin, journeyDate);

    if (source.find_first_not_of(" \t") == std::string::npos ||
        destination.find_first_not_of(" \t") == std::string::npos) {
        std::cout << "\nError: Source and destination cannot be empty.\n";
        return 1;
    }

    if (!isValidDate(journeyDate)) {
        std::cout << "\nError: Invalid journey date. "
                     "Please use YYYY-MM-DD format and enter a valid date.\n";
        return 1;
    }

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

    const std::vector<Train> results =
        searchTrains(trains, source, destination, journeyDate);

    if (results.empty()) {
        std::cout << "\nNo trains found for the selected route and date.\n";
        return 0;
    }

    std::cout << "\nMatching trains:\n";

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

        std::cout << "-----------------------------------\n";
    }

    return 0;
}
