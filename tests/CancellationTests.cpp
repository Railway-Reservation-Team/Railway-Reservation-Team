#include "BookingManager.h"
#include "Cancellation.h"
#include "TrainData.h"

#include <cassert>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main() {
    std::string error;
    double refund = -1.0;
    double charge = -1.0;

    // Unknown PNR must fail without changing monetary outputs.
    assert(!Cancellation::cancelBooking(
        "0000000000", refund, charge, error));
    assert(refund == 0.0);
    assert(charge == 0.0);
    assert(error.find("Booking not found") != std::string::npos);

    Train train{};
    train.trainNumber = "77777";
    train.trainName = "Cancellation Test Express";
    train.source = "Bengaluru";
    train.destination = "Chennai";
    train.departureTime = "23:59";
    train.arrivalTime = "07:00";
    train.journeyDuration = "7h";
    train.sleeperFare = 500;
    train.ac3TierFare = 800;
    train.ac2TierFare = 1000;
    train.chairCarFare = 400;
    train.sleeperSeatsAvailable = 10;
    train.ac3TierSeatsAvailable = 10;
    train.ac2TierSeatsAvailable = 10;
    train.chairCarSeatsAvailable = 10;
    train.journeyDate = "2099-12-31";

    {
        std::ofstream trains("data/trains.dat");
        trains << "77777|Cancellation Test Express|Bengaluru|Chennai|"
               << "23:59|07:00|7h|500|800|1000|400|10|10|10|10|"
               << "2099-12-31\n";
    }
    {
        std::ofstream bookings("data/bookings.dat");
    }

    std::vector<Passenger> passengers{
        {"Test Passenger", 21, "Other"}
    };
    std::string pnr;

    assert(BookingManager::createBooking(
        train, "Sleeper", passengers, 1000.0, pnr, error));
    assert(pnr.size() == 10);

    Train afterBooking;
    {
        std::ifstream trains("data/trains.dat");
        std::string line;
        std::getline(trains, line);
        assert(parseTrainRecord(line, afterBooking));
    }
    assert(afterBooking.sleeperSeatsAvailable == 9);

    // A valid future booking is cancelled with the >24-hour fee tier.
    assert(Cancellation::cancelBooking(pnr, refund, charge, error));
    assert(std::fabs(charge - 250.0) < 0.001);
    assert(std::fabs(refund - 750.0) < 0.001);

    Booking cancelled;
    assert(BookingManager::findBooking(pnr, cancelled));
    assert(cancelled.status == "Cancelled");

    Train afterCancellation;
    {
        std::ifstream trains("data/trains.dat");
        std::string line;
        std::getline(trains, line);
        assert(parseTrainRecord(line, afterCancellation));
    }
    assert(afterCancellation.sleeperSeatsAvailable == 10);

    // Repeated cancellation is rejected because the booking is no longer confirmed.
    assert(!Cancellation::cancelBooking(pnr, refund, charge, error));
    assert(error.find("Only confirmed bookings") != std::string::npos);

    // The within-24-hour fee tier charges 50%.
    const std::time_t nearDeparture = std::time(nullptr) + 2 * 60 * 60;
    std::tm localDeparture = *std::localtime(&nearDeparture);
    char nearDate[11]{};
    char nearTime[6]{};
    assert(std::strftime(nearDate, sizeof(nearDate), "%Y-%m-%d", &localDeparture) > 0);
    assert(std::strftime(nearTime, sizeof(nearTime), "%H:%M", &localDeparture) > 0);

    Train nearTrain = train;
    nearTrain.trainNumber = "77776";
    nearTrain.journeyDate = nearDate;
    nearTrain.departureTime = nearTime;
    {
        std::ofstream trains("data/trains.dat", std::ios::trunc);
        trains << nearTrain.trainNumber << "|Near Departure Express|Bengaluru|Chennai|"
               << nearTrain.departureTime << "|07:00|7h|500|800|1000|400|10|10|10|10|"
               << nearTrain.journeyDate << "\\n";
    }
    {
        std::ofstream bookings("data/bookings.dat", std::ios::trunc);
    }

    std::string nearPnr;
    assert(BookingManager::createBooking(
        nearTrain, "Sleeper", passengers, 1000.0, nearPnr, error));
    assert(Cancellation::cancelBooking(nearPnr, refund, charge, error));
    assert(std::fabs(charge - 500.0) < 0.001);
    assert(std::fabs(refund - 500.0) < 0.001);

    // A booking whose train has already departed cannot be cancelled.
    Train departedTrain = train;
    departedTrain.trainNumber = "77775";
    departedTrain.journeyDate = "2000-01-01";
    departedTrain.departureTime = "00:00";
    {
        std::ofstream trains("data/trains.dat", std::ios::trunc);
        trains << departedTrain.trainNumber << "|Departed Express|Bengaluru|Chennai|"
               << "00:00|07:00|7h|500|800|1000|400|10|10|10|10|"
               << "2000-01-01\\n";
    }
    {
        std::ofstream bookings("data/bookings.dat", std::ios::trunc);
    }

    std::string departedPnr;
    assert(BookingManager::createBooking(
        departedTrain, "Sleeper", passengers, 1000.0, departedPnr, error));
    assert(!Cancellation::cancelBooking(departedPnr, refund, charge, error));
    assert(error.find("Cannot cancel after train departure") != std::string::npos);

    // A malformed PNR does not identify a valid booking.
    assert(!Cancellation::cancelBooking(
        "not-a-pnr", refund, charge, error));

    std::cout << "Cancellation tests passed.\n";
    std::cout << "Unknown PNR rejection: PASS\n";
    std::cout << "Successful cancellation and 25% fee tier: PASS\n";
    std::cout << "Refund calculation and seat restoration: PASS\n";
    std::cout << "Booking status update and repeat-cancel rejection: PASS\n";
    return 0;
}
