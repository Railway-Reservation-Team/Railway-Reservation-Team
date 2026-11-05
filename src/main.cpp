
#include "TrainData.h"
#include "TrainSearch.h"
#include "Validation.h"
#include "FareCalculator.h"
#include "PassengerDetails.h"
#include "BookingManager.h"
#include "Payment.h"
#include "AdminTrainMenu.h"

#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <iomanip>

int runBooking() {
    std::string source;
    std::string destination;
    std::string journeyDate;
    std::string errorMessage;
    std::string input;

    int seats = 0;
    int trainChoice = 0;
    int classChoice = 0;

    std::cout << "===== Railway Reservation System =====\n";

    while (true) {
        std::cout << "Enter source station: ";
        if (!std::getline(std::cin, source)) return 1;

        if (Validation::isNonEmpty(source, errorMessage))
            break;

        std::cout << "Error: " << errorMessage << " Try again.\n";
    }

    while (true) {
        std::cout << "Enter destination station: ";
        if (!std::getline(std::cin, destination)) return 1;

        if (Validation::isNonEmpty(destination, errorMessage))
            break;

        std::cout << "Error: " << errorMessage << " Try again.\n";
    }

    while (true) {
        std::cout << "Enter journey date (YYYY-MM-DD): ";
        if (!std::getline(std::cin, journeyDate)) return 1;

        if (Validation::isValidDate(journeyDate, errorMessage))
            break;

        std::cout << "Error: " << errorMessage << " Try again.\n";
    }

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

    const std::vector<Train> results =
        searchTrains(trains, source, destination, journeyDate);

    if (results.empty()) {
        std::cout << "No trains found for this route and date.\n";
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
                  << "\nSleeper: Rs. " << train.sleeperFare
                  << " | Seats: " << train.sleeperSeatsAvailable
                  << "\nAC 3-Tier: Rs. " << train.ac3TierFare
                  << " | Seats: " << train.ac3TierSeatsAvailable
                  << "\nAC 2-Tier: Rs. " << train.ac2TierFare
                  << " | Seats: " << train.ac2TierSeatsAvailable
                  << "\nChair Car: Rs. " << train.chairCarFare
                  << " | Seats: " << train.chairCarSeatsAvailable << '\n';
    }

    while (true) {
        std::cout << "\nSelect train option (1-"
                  << results.size() << "): ";

        if (!std::getline(std::cin, input)) return 1;

        if (Validation::parseMenuChoice(
                input, 1, static_cast<int>(results.size()),
                trainChoice, errorMessage))
            break;

        std::cout << "Error: " << errorMessage << " Try again.\n";
    }

    const Train selectedTrain = results[trainChoice - 1];

    while (true) {
        std::cout << "\nEnter number of passengers (1-6): ";

        if (!std::getline(std::cin, input)) return 1;

        if (Validation::parseSeatCount(input, seats, errorMessage))
            break;

        std::cout << "Error: " << errorMessage << " Try again.\n";
    }

    std::vector<Passenger> passengers;

    if (!PassengerDetails::collectPassengerDetails(
            seats, passengers, std::cin, std::cout)) {
        std::cout << "Passenger details collection failed.\n";
        return 1;
    }

    std::cout << "\n========== SELECT TRAVEL CLASS ==========\n"
              << "1. Sleeper\n"
              << "2. AC 3-Tier\n"
              << "3. AC 2-Tier\n"
              << "4. Chair Car\n";

    while (true) {
        std::cout << "Select travel class (1-4): ";

        if (!std::getline(std::cin, input)) return 1;

        if (Validation::parseMenuChoice(
                input, 1, 4, classChoice, errorMessage))
            break;

        std::cout << "Error: " << errorMessage << " Try again.\n";
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
        std::cout << "Error: Not enough seats available. Only "
                  << availableSeats << " seats remain.\n";
        return 1;
    }

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

        for (size_t i = 0; i < passengers.size(); ++i) {
            std::cout << "\nPassenger " << i + 1
                      << "\nName: " << passengers[i].name
                      << "\nAge: " << passengers[i].age
                      << "\nGender: " << passengers[i].gender << '\n';
        }

        std::string confirmation;

        std::cout << "\nConfirm booking? (Y/N): ";
        if (!std::getline(std::cin, confirmation)) return 1;

        if (confirmation != "Y" && confirmation != "y") {
            std::cout << "Booking cancelled. No seats allocated.\n";
            return 0;
        }

        // Process payment before creating the booking.
        std::string paymentMethod;
        std::string paymentStatus;

        const bool paymentSuccessful = processPayment(
            totalFare,
            paymentMethod,
            paymentStatus
        );

        if (!paymentSuccessful) {
            std::cout << "\n========================================\n"
                      << "          BOOKING NOT CONFIRMED\n"
                      << "========================================\n"
                      << "Payment Status: " << paymentStatus << '\n'
                      << "Payment failed. No booking was saved.\n";

            return 1;
        }

        std::cout << "\nPayment Status: SUCCESS\n"
                  << "Payment Method: " << paymentMethod << '\n';

        // Create booking only after successful payment.
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

            std::cout << "Payment was successful, but booking was not saved.\n";
            std::cout << "Manual payment reversal may be required.\n";

            return 1;
        }

        std::cout << std::fixed << std::setprecision(2);

        std::cout << "\n========== BOOKING CONFIRMED ==========\n"
                  << "PNR: " << pnr
                  << "\nTrain: " << selectedTrain.trainName
                  << "\nTrain Number: " << selectedTrain.trainNumber
                  << "\nJourney Date: " << journeyDate
                  << "\nPassengers: " << passengers.size()
                  << "\nTravel Class: " << travelClass
                  << "\nSeats Allocated: " << seats
                  << "\nTotal Fare: Rs. " << totalFare
                  << "\nPayment Method: " << paymentMethod
                  << "\nPayment Status: " << paymentStatus
                  << "\nBooking Status: Confirmed\n";

        std::cout << "\nYour seats have been successfully allocated.\n";
    }
    catch (const std::exception& e) {
        std::cout << "Booking error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}

int main() {
    std::string choice;

    while (true) {
        std::cout << "\n===== RAILWAY RESERVATION SYSTEM =====\n"
                  << "1. Passenger Booking\n"
                  << "2. Admin Train Management\n"
                  << "3. Exit\n"
                  << "Enter choice: ";

        if (!std::getline(std::cin, choice))
            break;

        if (choice == "1") {
            runBooking();
        }
        else if (choice == "2") {
            AdminTrainMenu::run(std::cin, std::cout);
        }
        else if (choice == "3") {
            std::cout << "Exiting system. Goodbye!\n";
            break;
        }
        else {
            std::cout << "Invalid choice. Try again.\n";
        }
    }

    return 0;
}
