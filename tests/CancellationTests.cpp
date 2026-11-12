
#include "../src/Cancellation.h"
#include "../src/BookingManager.h"
#include "../src/TrainData.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cmath>
#include <ctime>

int passed = 0;
int failed = 0;

void check(bool condition, const std::string& description) {
    if (condition) {
        std::cout << "[PASS] " << description << '\n';
        ++passed;
    } else {
        std::cout << "[FAIL] " << description << '\n';
        ++failed;
    }
}

int main() {
    namespace fs = std::filesystem;

    fs::create_directories("data");

    // Create an isolated train fixture.
    {
        std::ofstream trainFile("data/trains.dat");
        trainFile
            << "TEST101|Test Express|Bengaluru|Chennai|08:00|14:00|6h|"
            << "1000|1500|2000|800|10|10|10|10|2026-12-15\n";
    }

    Train train{};
    train.trainNumber = "TEST101";
    train.journeyDate = "2026-12-15";

    std::vector<Passenger> passengers = {
        {"Alice Smith", 25, "Female"},
        {"Bob Jones", 30, "Male"}
    };

    std::string pnr;
    std::string error;

    // Test 1: Create a confirmed booking.
    bool created = BookingManager::createBooking(
        train, "Sleeper", passengers, 2000.0, pnr, error
    );

    check(created, "Create a booking before cancellation");

    if (!created) {
        std::cout << "Booking error: " << error << '\n';
        return 1;
    }

    // Test 2: Verify that two seats were deducted.
    std::string loadError;
    std::vector<Train> trains = loadTrains(
        "data/trains.dat", "2026-12-15", loadError
    );

    check(
        trains.size() == 1 &&
        trains[0].sleeperSeatsAvailable == 8,
        "Two seats deducted after booking"
    );

    // Test 3: Cancel the valid booking.
    double refund = 0.0;
    double charge = 0.0;

    bool cancelled = Cancellation::cancelBooking(
        pnr, refund, charge, error
    );

    check(cancelled, "Cancel a valid confirmed booking");

    if (!cancelled) {
        std::cout << "Cancellation error: " << error << '\n';
    }

    // Test 4: Verify refund calculation.
    check(
        std::abs(refund - 1500.0) < 0.01,
        "Refund is 75% of the original fare"
    );

    check(
        std::abs(charge - 500.0) < 0.01,
        "Cancellation fee is 25% of the original fare"
    );

    check(
        std::abs((refund + charge) - 2000.0) < 0.01,
        "Refund plus cancellation fee equals original fare"
    );

    // Test 5: Verify booking status.
    Booking booking;
    bool found = BookingManager::findBooking(pnr, booking);

    check(
        found && booking.status == "Cancelled",
        "Booking status changes to Cancelled"
    );

    // Test 6: Verify that seats are restored.
    trains = loadTrains(
        "data/trains.dat", "2026-12-15", loadError
    );

    check(
        trains.size() == 1 &&
        trains[0].sleeperSeatsAvailable == 10,
        "Two seats restored after cancellation"
    );

    // Test 7: Reject duplicate cancellation.
    double secondRefund = 0.0;
    double secondCharge = 0.0;
    std::string secondError;

    check(
        !Cancellation::cancelBooking(
            pnr, secondRefund, secondCharge, secondError
        ),
        "Reject cancellation of an already cancelled booking"
    );

    // Test 8: Verify duplicate cancellation does not restore seats again.
    trains = loadTrains(
        "data/trains.dat", "2026-12-15", loadError
    );

    check(
        trains.size() == 1 &&
        trains[0].sleeperSeatsAvailable == 10,
        "Duplicate cancellation does not restore seats again"
    );

    // Test 9: Reject an invalid PNR.
    double invalidRefund = 0.0;
    double invalidCharge = 0.0;
    std::string invalidError;

    check(
        !Cancellation::cancelBooking(
            "0000000000",
            invalidRefund,
            invalidCharge,
            invalidError
        ),
        "Reject cancellation using an invalid PNR"
    );

    // Test 10: Verify cancellation record.
    std::ifstream cancellationFile("data/cancellations.dat");

    check(
        cancellationFile.good() &&
        cancellationFile.peek() != std::ifstream::traits_type::eof(),
        "Persist cancellation record"
    );

    cancellationFile.close();

    // Test 11: Verify cancellation within 24 hours.
    const std::time_t nearDeparture =
        std::time(nullptr) + 2 * 60 * 60;

    std::tm* localDeparture = std::localtime(&nearDeparture);

    // Fill the remaining inventory, queue two passengers, then cancel a
    // confirmed group. Both waitlisted bookings should be promoted FIFO.
    std::vector<Passenger> sixPassengers(
        6, {"Group Passenger", 25, "Other"}
    );
    std::vector<Passenger> fourPassengers(
        4, {"Another Passenger", 26, "Other"}
    );
    std::string sixPnr;
    std::string fourPnr;
    std::string firstWaitlistPnr;
    std::string secondWaitlistPnr;

    const bool sixCreated = BookingManager::createBooking(
        train, "Sleeper", sixPassengers, 6000.0, sixPnr, error
    );
    check(sixCreated, "Create first confirmed group before waitlist promotion");

    const bool fourCreated = sixCreated && BookingManager::createBooking(
        train, "Sleeper", fourPassengers, 4000.0, fourPnr, error
    );
    check(fourCreated, "Fill remaining seats with second confirmed group");

    const bool firstWaitlistCreated = fourCreated &&
        BookingManager::createBooking(
            train, "Sleeper", passengers, 1000.0, firstWaitlistPnr, error
        );
    check(firstWaitlistCreated, "Queue first passenger group");

    const bool secondWaitlistCreated = firstWaitlistCreated &&
        BookingManager::createBooking(
            train, "Sleeper", passengers, 1000.0, secondWaitlistPnr, error
        );
    check(secondWaitlistCreated, "Queue second passenger group");

    if (secondWaitlistCreated) {
        Booking firstWaiting;
        Booking secondWaiting;
        const bool firstFound =
            BookingManager::findBooking(firstWaitlistPnr, firstWaiting);
        const bool secondFound =
            BookingManager::findBooking(secondWaitlistPnr, secondWaiting);

        check(
            firstFound && firstWaiting.status == "Waitlisted" &&
                firstWaiting.waitlistPosition == 1,
            "First waitlisted booking receives queue position 1"
        );
        check(
            secondFound && secondWaiting.status == "Waitlisted" &&
                secondWaiting.waitlistPosition == 2,
            "Second waitlisted booking receives queue position 2"
        );

        const bool waitlistCancellation =
            Cancellation::cancelBooking(sixPnr, refund, charge, error);
        check(
            waitlistCancellation,
            "Cancellation releases seats and triggers waitlist promotion"
        );

        if (waitlistCancellation) {
            Booking firstPromoted;
            Booking secondPromoted;
            const bool firstPromotedFound =
                BookingManager::findBooking(firstWaitlistPnr, firstPromoted);
            const bool secondPromotedFound =
                BookingManager::findBooking(secondWaitlistPnr, secondPromoted);

            check(
                firstPromotedFound && firstPromoted.status == "Confirmed" &&
                    firstPromoted.waitlistPosition == 0,
                "Promote the first waitlisted booking"
            );
            check(
                secondPromotedFound && secondPromoted.status == "Confirmed" &&
                    secondPromoted.waitlistPosition == 0,
                "Promote the second waitlisted booking in FIFO order"
            );

            Train afterPromotion;
            bool trainParsed = false;
            {
                std::ifstream trains("data/trains.dat");
                std::string line;
                if (std::getline(trains, line)) {
                    trainParsed = parseTrainRecord(line, afterPromotion);
                }
            }
            check(
                trainParsed && afterPromotion.sleeperSeatsAvailable == 2,
                "Seat inventory reflects both FIFO promotions"
            );
        }
    }

    char nearDate[11]{};
    char nearTime[6]{};

    bool validTime = localDeparture != nullptr &&
        std::strftime(
            nearDate, sizeof(nearDate), "%Y-%m-%d", localDeparture
        ) > 0 &&
        std::strftime(
            nearTime, sizeof(nearTime), "%H:%M", localDeparture
        ) > 0;

    check(validTime, "Prepare a train departing within 24 hours");

    if (validTime) {
        Train nearTrain{};
        nearTrain.trainNumber = "TEST102";
        nearTrain.trainName = "Near Departure Express";
        nearTrain.source = "Bengaluru";
        nearTrain.destination = "Chennai";
        nearTrain.departureTime = nearTime;
        nearTrain.arrivalTime = "07:00";
        nearTrain.journeyDuration = "7h";
        nearTrain.sleeperFare = 500;
        nearTrain.ac3TierFare = 800;
        nearTrain.ac2TierFare = 1000;
        nearTrain.chairCarFare = 400;
        nearTrain.sleeperSeatsAvailable = 10;
        nearTrain.ac3TierSeatsAvailable = 10;
        nearTrain.ac2TierSeatsAvailable = 10;
        nearTrain.chairCarSeatsAvailable = 10;
        nearTrain.journeyDate = nearDate;

        {
            std::ofstream trainFile("data/trains.dat", std::ios::trunc);
            trainFile
                << "TEST102|Near Departure Express|Bengaluru|Chennai|"
                << nearTime << "|07:00|7h|500|800|1000|400|"
                << "10|10|10|10|" << nearDate << '\n';
        }

        {
            std::ofstream bookingsFile(
                "data/bookings.dat", std::ios::trunc
            );
        }

        std::string nearPnr;
        std::string nearError;

        bool nearCreated = BookingManager::createBooking(
            nearTrain, "Sleeper", passengers, 1000.0,
            nearPnr, nearError
        );

        check(
            nearCreated,
            "Create a booking within 24 hours of departure"
        );

        if (nearCreated) {
            double nearRefund = 0.0;
            double nearCharge = 0.0;

            bool nearCancelled = Cancellation::cancelBooking(
                nearPnr, nearRefund, nearCharge, nearError
            );

            check(
                nearCancelled,
                "Cancel a booking within 24 hours"
            );

            if (nearCancelled) {
                check(
                    std::abs(nearCharge - 500.0) < 0.01,
                    "Within-24-hour cancellation fee is 50%"
                );

                check(
                    std::abs(nearRefund - 500.0) < 0.01,
                    "Within-24-hour refund is 50%"
                );
            }
        }
    }

    // Test 12: Reject cancellation after departure.
    {
        {
            std::ofstream trainFile("data/trains.dat", std::ios::trunc);
            trainFile
                << "TEST103|Departed Express|Bengaluru|Chennai|"
                << "00:00|07:00|7h|500|800|1000|400|"
                << "10|10|10|10|2000-01-01\n";
        }

        const std::string departedPnr = "1234567891";

        {
            std::ofstream bookingsFile(
                "data/bookings.dat", std::ios::trunc
            );

            bookingsFile
                << departedPnr
                << "|TEST103|2000-01-01|Sleeper|"
                << "Test Passenger,21,Other|1|1000.00|Confirmed\n";
        }

        double departedRefund = 0.0;
        double departedCharge = 0.0;
        std::string departedError;

        check(
            !Cancellation::cancelBooking(
                departedPnr,
                departedRefund,
                departedCharge,
                departedError
            ),
            "Reject cancellation after train departure"
        );
    }

    // Additional invalid PNR validation tests.
    check(
        !Cancellation::cancelBooking(
            "not-a-pnr", refund, charge, error
        ),
        "Reject malformed PNR"
    );

    check(
        !Cancellation::cancelBooking(
            "", refund, charge, error
        ),
        "Reject empty PNR"
    );

    check(
        !Cancellation::cancelBooking(
            "12345", refund, charge, error
        ),
        "Reject PNR with incorrect numeric length"
    );

    check(
        !Cancellation::cancelBooking(
            "12345!@#$%", refund, charge, error
        ),
        "Reject PNR containing special characters"
    );

    std::cout << "\nCancellation Test Results\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "Total: " << passed + failed << '\n';

    return failed == 0 ? 0 : 1;
}
