
#include "TrainData.h"
#include "TrainSearch.h"
#include "Validation.h"
#include "FareCalculator.h"
#include "PassengerDetails.h"
#include "BookingManager.h"

#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <sstream>
#include <exception>

int main() {
    std::string source, destination, journeyDate;
    std::string errorMessage;
    int seats = 0;

    std::cout << "===== Railway Reservation System =====\n";

    // 1. Collect and validate source
    while (true) {
        std::cout << "Enter source station: ";
        if (!std::getline(std::cin, source)) return 1;

        if (source.find_first_not_of(" \t") != std::string::npos)
            break;

        std::cout << "Error: Source station cannot be empty.\n";
    }

    // 2. Collect and validate destination
    while (true) {
        std::cout << "Enter destination station: ";
        if (!std::getline(std::cin, destination)) return 1;

        if (destination.find_first_not_of(" \t") != std::string::npos)
            break;

        std::cout << "Error: Destination station cannot be empty.\n";
    }

    // 3. Validate journey date
    while (true) {
        std::cout << "Enter journey date (YYYY-MM-DD): ";
        if (!std::getline(std::cin, journeyDate)) return 1;

        if (Validation::isValidDate(journeyDate, errorMessage))
            break;

        std::cout << "Error: " << errorMessage << " Try again.\n";
    }

    // 4. Load trains
    const std::vector<Train> trains =
        loadTrains("data/trains.dat", journeyDate, errorMessage);

    if (!errorMessage.empty()) {
        std::cout << "Error: " << errorMessage << '\n';
        return 1;
    }

    if (trains.empty()) {
        std::cout << "No train records found for this date.\n";
        return 0;
    }

    // 5. Search trains
    const std::vector<Train> results =
        searchTrains(trains, source, destination, journeyDate);

    if (results.empty()) {
        std::cout << "No trains found for this route and date.\n";
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
                  << "\nSleeper: Rs. " << train.sleeperFare
                  << " | Seats: " << train.sleeperSeatsAvailable
                  << "\nAC 3-Tier: Rs. " << train.ac3TierFare
                  << " | Seats: " << train.ac3TierSeatsAvailable
                  << "\nAC 2-Tier: Rs. " << train.ac2TierFare
                  << " | Seats: " << train.ac2TierSeatsAvailable
                  << "\nChair Car: Rs. " << train.chairCarFare
                  << " | Seats: " << train.chairCarSeatsAvailable << '\n';
    }

    // 7. Select train
    int choice;

    std::cout << "\nSelect train option (1-"
              << results.size() << "): ";

    if (!(std::cin >> choice) ||
        choice < 1 ||
        choice > static_cast<int>(results.size())) {
        std::cout << "Error: Invalid train selection.\n";
        return 1;
    }

    const Train selectedTrain = results[choice - 1];

    std::cin.ignore(
        std::numeric_limits<std::streamsize>::max(), '\n'
    );

    // 8. Collect passenger count
    std::string seatInput;

    while (true) {
        std::cout << "\nEnter number of passengers (1-6): ";

        if (!std::getline(std::cin, seatInput)) return 1;

        if (Validation::parseSeatCount(
                seatInput, seats, errorMessage))
            break;

        std::cout << "Error: " << errorMessage << " Try again.\n";
    }

    // 9. Collect individual passenger details
    std::vector<Passenger> passengers;

    if (!PassengerDetails::collectPassengerDetails(
            seats, passengers, std::cin, std::cout)) {
        std::cout << "Passenger details collection failed.\n";
        return 1;
    }

    // 10. Select travel class
    int classChoice;

    std::cout << "\n========== SELECT TRAVEL CLASS ==========\n"
              << "1. Sleeper\n"
              << "2. AC 3-Tier\n"
              << "3. AC 2-Tier\n"
              << "4. Chair Car\n"
              << "Select travel class: ";

    if (!(std::cin >> classChoice) ||
        classChoice < 1 || classChoice > 4) {
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

    // 11. Check seat availability
    if (availableSeats < seats) {
        std::cout << "Error: Not enough seats available. Booking rejected.\n";
        return 1;
    }

    // 12. Calculate fare and display booking details
    try {
        const double totalFare =
            FareCalculator::calculateFare(
                selectedTrain, travelClass, seats
            );

        std::cout << "\n========== BOOKING DETAILS ==========\n"
                  << "Train: " << selectedTrain.trainName
                  << "\nTrain Number: " << selectedTrain.trainNumber
                  << "\nJourney Date: " << journeyDate
                  << "\nTravel Class: " << travelClass
                  << "\nNumber of Passengers: " << passengers.size()
                  << "\nFare Per Passenger: Rs. "
                  << totalFare / seats
                  << "\nTotal Fare: Rs. " << totalFare << '\n';

        std::cout << "\n========== PASSENGER LIST ==========\n";

        std::ostringstream passengerNames;

        for (size_t i = 0; i < passengers.size(); ++i) {
            std::cout << "\nPassenger " << i + 1
                      << "\nName: " << passengers[i].name
                      << "\nAge: " << passengers[i].age
                      << "\nGender: " << passengers[i].gender << '\n';

            if (i > 0)
                passengerNames << "; ";

            passengerNames << passengers[i].name;
        }

        // 13. Confirm booking
        char confirm;

        std::cout << "\nConfirm booking? (Y/N): ";
        std::cin >> confirm;

        if (confirm != 'Y' && confirm != 'y') {
            std::cout << "Booking cancelled. No seats allocated.\n";
            return 0;
        }

        // 14. Create booking and generate unique PNR
        std::string pnr;

        if (!BookingManager::createBooking(
                selectedTrain,
                travelClass,
                passengers,
                totalFare,
                pnr,
                errorMessage)) {
            std::cout << "Booking failed: "
                      << errorMessage << '\n';
            return 1;
        }

        // 15. Display confirmation
        std::cout << "\n========== BOOKING CONFIRMED ==========\n"
                  << "PNR: " << pnr
                  << "\nTrain: " << selectedTrain.trainName
                  << "\nTrain Number: " << selectedTrain.trainNumber
                  << "\nJourney Date: " << journeyDate
                  << "\nPassengers: " << passengers.size()
                  << "\nTravel Class: " << travelClass
                  << "\nSeats Allocated: " << seats
                  << "\nTotal Fare: Rs. " << totalFare
                  << "\nBooking Status: Confirmed\n";

        std::cout << "\nYour seats have been successfully allocated.\n";
    }
    catch (const std::exception& e) {
        std::cout << "Booking error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
