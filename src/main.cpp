
#include "TrainData.h"
#include "TrainSearch.h"
#include "Validation.h"
#include "FareCalculator.h"
#include "Booking.h"
#include "Payment.h"

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <iomanip>

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

    // 3. Validate journey date
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

        std::cout << "\nError: Invalid train selection.\n";
        return 1;
    }

    const Train& selectedTrain = results[trainChoice - 1];

    std::cin.ignore(10000, '\n');

    // 8. Passenger details
    std::cout << "\n========== PASSENGER DETAILS ==========\n";

    std::cout << "Enter passenger name: ";
    std::getline(std::cin, passengerName);

    if (!Validation::isValidPassengerName(passengerName, errorMessage)) {
        std::cout << "\nError: " << errorMessage << '\n';
        return 1;
    }

    // 9. Validate passenger count
    std::string seatInput;

    std::cout << "Enter number of passengers (1-6): ";
    std::getline(std::cin, seatInput);

    if (!Validation::parseSeatCount(seatInput, seats, errorMessage)) {
        std::cout << "\nError: " << errorMessage << '\n';
        return 1;
    }

    // 10. Select travel class
    int classChoice;

    std::cout << "\n========== SELECT TRAVEL CLASS ==========\n";
    std::cout << "1. Sleeper\n";
    std::cout << "2. AC 3-Tier\n";
    std::cout << "3. AC 2-Tier\n";
    std::cout << "4. Chair Car\n";

    std::cout << "Enter your class choice (1-4): ";

    if (!(std::cin >> classChoice) ||
        classChoice < 1 || classChoice > 4) {

        std::cout << "\nError: Invalid travel class selection.\n";
        return 1;
    }

    std::string travelClass;

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

    // 11. Check seat availability
    int availableSeats = 0;

    switch (classChoice) {
        case 1:
            availableSeats = selectedTrain.sleeperSeatsAvailable;
            break;
        case 2:
            availableSeats = selectedTrain.ac3TierSeatsAvailable;
            break;
        case 3:
            availableSeats = selectedTrain.ac2TierSeatsAvailable;
            break;
        case 4:
            availableSeats = selectedTrain.chairCarSeatsAvailable;
            break;
    }

    if (availableSeats < seats) {
        std::cout << "\nError: Not enough seats available.\n";
        return 1;
    }

    // 12. Calculate fare
    try {

        double totalFare = FareCalculator::calculateFare(
            selectedTrain,
            travelClass,
            seats
        );

        // 13. Process payment BEFORE saving the booking.
        std::string paymentMethod;
        std::string paymentStatus;

        std::cout << "\n========== PAYMENT PROCESSING ==========\n";

        bool paymentSuccessful = processPayment(
            totalFare,
            paymentMethod,
            paymentStatus
        );

        if (!paymentSuccessful) {
            std::cout << "\n========================================\n";
            std::cout << "          BOOKING NOT CONFIRMED\n";
            std::cout << "========================================\n";
            std::cout << "Payment Status: " << paymentStatus << '\n';
            std::cout << "Payment failed. No booking was saved.\n";

            return 1;
        }

        std::cout << "\nPayment Status: SUCCESS\n";
        std::cout << "Payment Method: " << paymentMethod << '\n';

        // 14. Create booking only after successful payment.
        Booking booking;

        booking.pnr = generateUniquePNR();
        booking.passengerName = passengerName;
        booking.trainNumber = selectedTrain.trainNumber;
        booking.trainName = selectedTrain.trainName;
        booking.source = selectedTrain.source;
        booking.destination = selectedTrain.destination;
        booking.journeyDate = selectedTrain.journeyDate;
        booking.travelClass = travelClass;
        booking.numberOfPassengers = seats;
        booking.totalFare = totalFare;

        // 15. Save booking
        if (!saveBooking(booking, errorMessage)) {
            std::cout << "\nBooking failed: " << errorMessage << '\n';
            std::cout << "Payment was successful, but booking was not saved.\n";
            std::cout << "Manual payment reversal may be required.\n";
            return 1;
        }

        // 16. Display booking confirmation
        std::cout << std::fixed << std::setprecision(2);

        std::cout << "\n========================================\n";
        std::cout << "       BOOKING CONFIRMED SUCCESSFULLY\n";
        std::cout << "========================================\n";

        std::cout << "PNR Number: " << booking.pnr << '\n';
        std::cout << "Passenger Name: " << booking.passengerName << '\n';
        std::cout << "Train Number: " << booking.trainNumber << '\n';
        std::cout << "Train Name: " << booking.trainName << '\n';
        std::cout << "Source: " << booking.source << '\n';
        std::cout << "Destination: " << booking.destination << '\n';
        std::cout << "Journey Date: " << booking.journeyDate << '\n';
        std::cout << "Travel Class: " << booking.travelClass << '\n';

        std::cout << "Number of Passengers: "
                  << booking.numberOfPassengers << '\n';

        std::cout << "Fare Per Passenger: Rs. "
                  << (booking.totalFare / booking.numberOfPassengers) << '\n';

        std::cout << "Total Fare: Rs. " << booking.totalFare << '\n';
        std::cout << "Payment Method: " << paymentMethod << '\n';
        std::cout << "Payment Status: " << paymentStatus << '\n';

        std::cout << "========================================\n";
        std::cout << "Your booking details are saved successfully.\n";

    }
    catch (const std::exception& e) {
        std::cout << "\nError: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
