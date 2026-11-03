#include "TrainData.h"
#include "TrainSearch.h"
#include "Validation.h"
#include "FareCalculator.h"
#include "PassengerDetails.h"

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

int main() {
    std::string source;
    std::string destination;
    std::string journeyDate;
    std::string errorMessage;
    int seats = 0;

    std::cout << "===== Railway Reservation System =====\n";

    // 1. Get source station
    while (true) {
        std::cout << "Enter source station: ";
        if (!std::getline(std::cin, source)) return 1;

        if (source.find_first_not_of(" \t") != std::string::npos)
            break;

        std::cout << "Error: Source station cannot be empty. Try again.\n";
    }

    // 2. Get destination station
    while (true) {
        std::cout << "Enter destination station: ";
        if (!std::getline(std::cin, destination)) return 1;

        if (destination.find_first_not_of(" \t") != std::string::npos)
            break;

        std::cout << "Error: Destination station cannot be empty. Try again.\n";
    }

    // 3. Validate journey date
    while (true) {
        std::cout << "Enter journey date (YYYY-MM-DD): ";
        if (!std::getline(std::cin, journeyDate)) return 1;

        if (Validation::isValidDate(journeyDate, errorMessage))
            break;

        std::cout << "Error: " << errorMessage << " Try again.\n";
    }

    // 4. Load train records
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

    // 5. Search trains
    const std::vector<Train> results =
        searchTrains(trains, source, destination, journeyDate);

    if (results.empty()) {
        std::cout << "No trains found for the selected route and date.\n";
        return 0;
    }

    // 6. Display matching trains
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

    // 7. Select train
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

    // 8. Validate requested passenger/seat count
    std::string seatInput;

    while (true) {
        std::cout << "\nEnter number of passengers (1-6): ";

        if (!std::getline(std::cin, seatInput)) return 1;

        if (Validation::parseSeatCount(seatInput, seats, errorMessage))
            break;

        std::cout << "Error: " << errorMessage << " Try again.\n";
    }

    // 9. Collect details for every passenger
    std::vector<Passenger> passengers;

    if (!PassengerDetails::collectPassengerDetails(
            seats, passengers, std::cin, std::cout)) {
        std::cout << "Passenger details collection failed.\n";
        return 1;
    }

    // 10. Select travel class
    int classChoice;

    while (true) {
        std::cout << "\n========== SELECT TRAVEL CLASS ==========\n";
        std::cout << "1. Sleeper\n";
        std::cout << "2. AC 3-Tier\n";
        std::cout << "3. AC 2-Tier\n";
        std::cout << "4. Chair Car\n";
        std::cout << "Enter your class choice (1-4): ";

        if (!(std::cin >> classChoice)) {
            std::cout << "Error: Enter a valid number.\n";
            return 1;
        }

        if (classChoice >= 1 && classChoice <= 4)
            break;

        std::cout << "Error: Select a class between 1 and 4.\n";
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

    // 11. Check seat availability
    if (availableSeats < seats) {
        std::cout << "Error: Not enough seats available.\n";
        return 1;
    }

    // 12. Calculate fare and display passenger details
    try {
        double totalFare = FareCalculator::calculateFare(
            selectedTrain, travelClass, seats
        );

        std::cout << "\n========== BOOKING DETAILS ==========\n";
        std::cout << "Train Name: " << selectedTrain.trainName << '\n';
        std::cout << "Train Number: " << selectedTrain.trainNumber << '\n';
        std::cout << "Journey Date: " << selectedTrain.journeyDate << '\n';
        std::cout << "Travel Class: " << travelClass << '\n';
        std::cout << "Number of Passengers: " << passengers.size() << '\n';

        std::cout << "\n========== PASSENGER LIST ==========\n";

        for (size_t i = 0; i < passengers.size(); ++i) {
            std::cout << "\nPassenger " << i + 1
                      << "\nName: " << passengers[i].name
                      << "\nAge: " << passengers[i].age
                      << "\nGender: " << passengers[i].gender << '\n';
        }

        std::cout << "\nFare Per Passenger: Rs. "
                  << totalFare / seats << '\n';

        std::cout << "Total Ticket Fare: Rs. "
                  << totalFare << '\n';

        std::cout << "\nPassenger details collected successfully!\n";
        std::cout << "Fare calculated successfully!\n";
    }
    catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << '\n';
        return 1;
    }

    std::cout << "\nTrain search completed successfully!\n";
    return 0;
}
