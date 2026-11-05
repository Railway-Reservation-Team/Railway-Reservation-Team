#include "TrainData.h"
#include "TrainSearch.h"
#include "Validation.h"
#include "FareCalculator.h"
#include "PassengerDetails.h"
#include "BookingManager.h"
#include "Cancellation.h"
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
    int menuChoice = 0;

    std::cout << "===== Railway Reservation System =====\n";

    // 1. Display main menu
    std::cout << "\n========== MAIN MENU ==========\n";
    std::cout << "1. Book Ticket\n";
    std::cout << "2. Cancel Ticket\n";

    std::cout << "Enter your choice (1-2): ";

    if (!std::getline(std::cin, input)) {
        return 1;
    }

    if (!Validation::parseMenuChoice(
            input, 1, 2, menuChoice, errorMessage)) {

        std::cout << "Error: " << errorMessage << '\n';
        return 1;
    }

    // 2. Ticket cancellation flow
    if (menuChoice == 2) {

        std::string pnr;
        double refundAmount = 0.0;
        double cancellationCharge = 0.0;

        std::cout << "\n========== TICKET CANCELLATION ==========\n";
        std::cout << "Enter your booking PNR: ";

        if (!std::getline(std::cin, pnr)) {
            return 1;
        }

        if (pnr.find_first_not_of(" \t") == std::string::npos) {
            std::cout << "Error: PNR cannot be empty.\n";
            return 1;
        }

        // Confirm cancellation
        std::string confirmation;

        std::cout << "Are you sure you want to cancel this ticket? (Y/N): ";

        if (!std::getline(std::cin, confirmation)) {
            return 1;
        }

        if (confirmation != "Y" && confirmation != "y") {
            std::cout << "Cancellation request stopped.\n";
            return 0;
        }

        // Process cancellation
        if (!Cancellation::cancelBooking(
                pnr,
                refundAmount,
                cancellationCharge,
                errorMessage)) {

            std::cout << "\nCancellation failed: "
                      << errorMessage << '\n';

            return 1;
        }

        // Display cancellation details
        std::cout << std::fixed << std::setprecision(2);

        std::cout << "\n========================================\n";
        std::cout << "       TICKET CANCELLED SUCCESSFULLY\n";
        std::cout << "========================================\n";

        std::cout << "PNR: " << pnr << '\n';
        std::cout << "Cancellation Charge: Rs. "
                  << cancellationCharge << '\n';
        std::cout << "Refund Amount: Rs. "
                  << refundAmount << '\n';
        std::cout << "Booking Status: Cancelled\n";

        std::cout << "Seats restored successfully.\n";
        std::cout << "Cancellation details saved.\n";

        std::cout << "========================================\n";

        return 0;
    }

    // 3. Existing ticket booking flow

    // Validate source station
    while (true) {

        std::cout << "\nEnter source station: ";

        if (!std::getline(std::cin, source)) {
            return 1;
        }

        if (Validation::isNonEmpty(source, errorMessage)) {
            break;
        }

        std::cout << "Error: " << errorMessage << " Try again.\n";
    }

    // Validate destination station
    while (true) {

        std::cout << "Enter destination station: ";

        if (!std::getline(std::cin, destination)) {
            return 1;
        }

        if (Validation::isNonEmpty(destination, errorMessage)) {
            break;
        }

        std::cout << "Error: " << errorMessage << " Try again.\n";
    }

    // Validate journey date
    while (true) {

        std::cout << "Enter journey date (YYYY-MM-DD): ";

        if (!std::getline(std::cin, journeyDate)) {
            return 1;
        }

        if (Validation::isValidDate(journeyDate, errorMessage)) {
            break;
        }

        std::cout << "Error: " << errorMessage << " Try again.\n";
    }

    // Load trains
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

    // Search trains
    const std::vector<Train> results =
        searchTrains(trains, source, destination, journeyDate);

    if (results.empty()) {
        std::cout << "No trains found for this route and date.\n";
        return 0;
    }

    // Display matching trains
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

    // Validate train selection
    while (true) {

        std::cout << "\nSelect train option (1-"
                  << results.size() << "): ";

        if (!std::getline(std::cin, input)) {
            return 1;
        }

        if (Validation::parseMenuChoice(
                input, 1, static_cast<int>(results.size()),
                trainChoice, errorMessage)) {
            break;
        }

        std::cout << "Error: " << errorMessage << " Try again.\n";
    }

    const Train selectedTrain = results[trainChoice - 1];

    // Validate passenger count
    while (true) {

        std::cout << "\nEnter number of passengers (1-6): ";

        if (!std::getline(std::cin, input)) {
            return 1;
        }

        if (Validation::parseSeatCount(input, seats, errorMessage)) {
            break;
        }

        std::cout << "Error: " << errorMessage << " Try again.\n";
    }

    // Collect individual passenger details
    std::vector<Passenger> passengers;

    if (!PassengerDetails::collectPassengerDetails(
            seats, passengers, std::cin, std::cout)) {

        std::cout << "Passenger details collection failed.\n";
        return 1;
    }

    // Validate travel class selection
    std::cout << "\n========== SELECT TRAVEL CLASS ==========\n"
              << "1. Sleeper\n"
              << "2. AC 3-Tier\n"
              << "3. AC 2-Tier\n"
              << "4. Chair Car\n";

    while (true) {

        std::cout << "Select travel class (1-4): ";

        if (!std::getline(std::cin, input)) {
            return 1;
        }

        if (Validation::parseMenuChoice(
                input, 1, 4, classChoice, errorMessage)) {
            break;
        }

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

    // Check seat availability
    if (availableSeats < seats) {

        std::cout << "Error: Not enough seats available. Only "
                  << availableSeats << " seats remain.\n";

        return 1;
    }

    // Calculate fare and create booking
    try {

        const double totalFare =
            FareCalculator::calculateFare(
                selectedTrain,
                travelClass,
                seats
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

        // Confirm booking
        std::string confirmation;

        std::cout << "\nConfirm booking? (Y/N): ";

        if (!std::getline(std::cin, confirmation)) {
            return 1;
        }

        if (confirmation != "Y" && confirmation != "y") {

            std::cout << "Booking cancelled. No seats allocated.\n";
            return 0;
        }

        // Create booking and generate unique PNR
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

        // Display booking confirmation
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