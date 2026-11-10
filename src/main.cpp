
#include "TrainData.h"
#include "TrainSearch.h"
#include "Validation.h"
#include "FareCalculator.h"
#include "PassengerDetails.h"
#include "BookingManager.h"
#include "Cancellation.h"
#include "Payment.h"
#include "SeatInventory.h"
#include "AdminTrainMenu.h"
#include "AuthManager.h"
#include "AdminAuthManager.h"

#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include <iomanip>

int runBooking() {
    if (!AuthManager::isSessionActive()) {
        std::cout << "Your passenger session has expired. Please log in again.\n";
        return 1;
    }

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

    // Ticket cancellation
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

        std::string confirmation;
        std::cout << "Are you sure you want to cancel this ticket? (Y/N): ";

        if (!std::getline(std::cin, confirmation)) {
            return 1;
        }

        if (confirmation != "Y" && confirmation != "y") {
            std::cout << "Cancellation request stopped.\n";
            return 0;
        }

        if (!Cancellation::cancelBooking(
                pnr, refundAmount, cancellationCharge, errorMessage)) {
            std::cout << "\nCancellation failed: "
                      << errorMessage << '\n';
            return 1;
        }

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "\n========================================\n";
        std::cout << "       TICKET CANCELLED SUCCESSFULLY\n";
        std::cout << "========================================\n";
        std::cout << "PNR: " << pnr << '\n';
        std::cout << "Cancellation Charge: Rs. "
                  << cancellationCharge << '\n';
        std::cout << "Refund Amount: Rs. " << refundAmount << '\n';
        std::cout << "Booking Status: Cancelled\n";
        std::cout << "Seats restored successfully.\n";
        std::cout << "Cancellation details saved.\n";
        std::cout << "========================================\n";

        return 0;
    }

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

    std::string travelClass;
    int availableSeats = 0;
    bool bookingReady = false;

    // Validate passenger count and class availability
    while (!bookingReady) {
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

        // Select travel class
        while (true) {
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

            switch (classChoice) {
                case 1:
                    travelClass = "Sleeper";
                    break;
                case 2:
                    travelClass = "AC 3-Tier";
                    break;
                case 3:
                    travelClass = "AC 2-Tier";
                    break;
                case 4:
                    travelClass = "Chair Car";
                    break;
            }

            availableSeats = getAvailableSeats(selectedTrain, travelClass);

            const SeatRequestResult seatResult =
                validateSeatRequest(availableSeats, seats);

            if (seatResult == SeatRequestResult::VALID) {
                bookingReady = true;
                break;
            }

            if (seatResult == SeatRequestResult::INVALID_AVAILABILITY) {
                std::cout << "\nError: Invalid seat availability.\n";
            }
            else if (seatResult == SeatRequestResult::INVALID_REQUEST) {
                std::cout << "\nError: Invalid passenger count.\n";
            }
            else if (seatResult == SeatRequestResult::EXCEEDS_MAX_PASSENGERS) {
                std::cout << "\nError: Maximum six passengers are allowed per booking.\n";
            }
            else if (availableSeats == 0) {
                std::cout << "\nSorry, no seats are available in "
                          << travelClass << ".\n";
            }
            else {
                std::cout << "\nError: Only " << availableSeats
                          << " seats are available in " << travelClass
                          << ". You requested " << seats << " seats.\n";
            }

            std::cout << "1. Enter passenger count again\n"
                      << "2. Select travel class again\n"
                      << "3. Cancel booking\n";

            int retryChoice = 0;

            while (true) {
                std::cout << "Choose an option (1-3): ";

                if (!std::getline(std::cin, input)) {
                    return 1;
                }

                if (Validation::parseMenuChoice(
                        input, 1, 3, retryChoice, errorMessage)) {
                    break;
                }

                std::cout << "Error: " << errorMessage << " Try again.\n";
            }

            if (retryChoice == 1) {
                break;
            }

            if (retryChoice == 2) {
                continue;
            }

            std::cout << "Booking cancelled.\n";
            return 0;
        }
    }

    // Collect passenger details
    std::vector<Passenger> passengers;

    if (!PassengerDetails::collectPassengerDetails(
            seats, passengers, std::cin, std::cout)) {
        std::cout << "Passenger details collection failed.\n";
        return 1;
    }

    try {
        // Calculate fare
        const double totalFare =
            FareCalculator::calculateFare(
                selectedTrain, travelClass, seats);

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

        // Verify passenger session
        if (!AuthManager::isSessionActive()) {
            std::cout << "Session expired. Please log in again. Booking was not saved.\n";
            return 1;
        }

        // Process payment
        std::string paymentMethod;
        std::string paymentStatus;

        const bool paymentSuccessful = processPayment(
            totalFare, paymentMethod, paymentStatus);

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

        // Create booking and generate PNR
        std::string pnr;

        if (!BookingManager::createBooking(
                selectedTrain,
                travelClass,
                passengers,
                totalFare,
                pnr,
                errorMessage)) {
            std::cout << "Booking failed: " << errorMessage << '\n';
            std::cout << "Payment was successful, but booking was not saved.\n";
            std::cout << "Manual payment reversal may be required.\n";
            return 1;
        }

        // Display booking confirmation
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
    std::string name;
    std::string email;
    std::string phone;
    std::string password;
    std::string errorMessage;

    while (true) {
        const bool loggedIn = AuthManager::isSessionActive();

        std::cout << "\n===== RAILWAY RESERVATION SYSTEM =====\n"
                  << "1. Passenger Registration\n"
                  << "2. Passenger Login\n"
                  << "3. Passenger Booking\n"
                  << "4. Administrator Login\n"
                  << "5. Administrator Logout\n"
                  << "6. Passenger Logout\n"
                  << "7. Exit\n";

        if (loggedIn) {
            std::cout << "Logged-in passenger: "
                      << AuthManager::getCurrentUserEmail() << '\n';
        } else {
            std::cout << "Passenger status: Not logged in\n";
        }

        std::cout << "Administrator status: "
                  << (AdminAuthManager::isAuthenticated()
                          ? "Logged in" : "Not logged in") << '\n';

        std::cout << "Enter choice: ";

        if (!std::getline(std::cin, choice)) {
            break;
        }

        if (choice == "1") {
            std::cout << "\n========== PASSENGER REGISTRATION ==========\n";

            std::cout << "Enter full name: ";
            if (!std::getline(std::cin, name)) break;

            std::cout << "Enter email: ";
            if (!std::getline(std::cin, email)) break;

            std::cout << "Enter phone number: ";
            if (!std::getline(std::cin, phone)) break;

            std::cout << "Enter password: ";
            if (!std::getline(std::cin, password)) break;

            if (AuthManager::registerPassenger(
                    name, email, phone, password, errorMessage)) {
                std::cout << "Registration successful! Please log in.\n";
            } else {
                std::cout << "Registration failed: "
                          << errorMessage << '\n';
            }

            password.clear();
        }
        else if (choice == "2") {
            std::cout << "\n========== PASSENGER LOGIN ==========\n";

            std::cout << "Enter registered email: ";
            if (!std::getline(std::cin, email)) break;

            std::cout << "Enter password: ";
            if (!std::getline(std::cin, password)) break;

            if (AuthManager::login(email, password, errorMessage)) {
                std::cout << "Login successful. Welcome!\n";
            } else {
                std::cout << "Login failed: " << errorMessage << '\n';
            }

            password.clear();
        }
        else if (choice == "3") {
            if (!AuthManager::isSessionActive()) {
                std::cout << "Please register and log in before booking.\n";
                continue;
            }

            AuthManager::recordActivity();
            runBooking();
            AuthManager::recordActivity();
        }
        else if (choice == "4") {
            std::string adminUsername;
            errorMessage.clear();

            bool accountExists =
                AdminAuthManager::hasAdminAccount(errorMessage);

            if (!accountExists && !errorMessage.empty()) {
                std::cout << "Administrator account error: "
                          << errorMessage << '\n';
                continue;
            }

            if (!accountExists) {
                std::cout << "\n========== FIRST-TIME ADMIN SETUP ==========\n";
                std::cout << "Create administrator username: ";
                if (!std::getline(std::cin, adminUsername)) break;

                std::cout << "Create administrator password: ";
                if (!std::getline(std::cin, password)) break;

                if (!AdminAuthManager::setupAdmin(
                        adminUsername, password, errorMessage)) {
                    std::cout << "Setup failed: " << errorMessage << '\n';
                    password.clear();
                    continue;
                }

                std::cout << "Administrator account created.\n";
            } else {
                std::cout << "\n========== ADMINISTRATOR LOGIN ==========\n";
                std::cout << "Username: ";
                if (!std::getline(std::cin, adminUsername)) break;

                std::cout << "Password: ";
                if (!std::getline(std::cin, password)) break;
            }

            if (AdminAuthManager::login(
                    adminUsername, password, errorMessage)) {
                std::cout << "Administrator login successful.\n";
                AdminTrainMenu::run(std::cin, std::cout);
                AdminAuthManager::logout();
                std::cout << "Administrator session ended.\n";
            } else {
                std::cout << "Administrator login failed: "
                          << errorMessage << '\n';
            }

            password.clear();
        }
        else if (choice == "5") {
            if (AdminAuthManager::isAuthenticated()) {
                AdminAuthManager::logout();
                std::cout << "Administrator logged out successfully.\n";
            } else {
                std::cout << "No active administrator session.\n";
            }
        }
        else if (choice == "6") {
            if (AuthManager::isSessionActive()) {
                AuthManager::logout();
                std::cout << "Passenger logged out successfully.\n";
            } else {
                std::cout << "No active passenger session.\n";
            }
        }
        else if (choice == "7") {
            AuthManager::logout();
            AdminAuthManager::logout();
            std::cout << "Exiting system. Goodbye!\n";
            break;
        }
        else {
            std::cout << "Invalid choice. Please enter 1-7.\n";
        }
    }

    AuthManager::logout();
    return 0;
}
