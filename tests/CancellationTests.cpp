
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

    // Successful cancellation must be auditable without passenger details.
    {
        std::ifstream auditFile("data/admin_audit.log");
        std::string audit(
            (std::istreambuf_iterator<char>(auditFile)),
            std::istreambuf_iterator<char>()
        );
        check(
            audit.find("PASSENGER_CANCELLATION_SUCCESS") != std::string::npos,
            "Successful passenger cancellation is recorded in the audit log"
        );
        check(
            audit.find("Alice Smith") == std::string::npos &&
            audit.find("Bob Jones") == std::string::npos,
            "Passenger names are not written to the cancellation audit log"
        );
    }


    // Partial waitlist promotion: release enough seats for only the first
    // waitlisted group; the second group must remain waitlisted.
    {
        {
            std::ofstream trainFile("data/trains.dat", std::ios::trunc);
            trainFile
                << "PART101|Partial Express|Bengaluru|Chennai|08:00|14:00|6h|"
                << "1000|1500|2000|800|10|10|10|10|2026-12-25\n";
        }

        std::vector<Passenger> sixPassengers(
            6, {"Group Passenger", 25, "Other"});
        std::vector<Passenger> fourPassengers(
            4, {"Another Passenger", 26, "Other"});
        std::vector<Passenger> twoPassengers(
            2, {"Waitlist One", 27, "Male"});
        std::vector<Passenger> threePassengers(
            3, {"Waitlist Two", 28, "Female"});

        std::string sixPnr, fourPnr, wl1Pnr, wl2Pnr;
        std::string partError;

        Train partTrain{};
        partTrain.trainNumber = "PART101";
        partTrain.trainName = "Partial Express";
        partTrain.source = "Bengaluru";
        partTrain.destination = "Chennai";
        partTrain.departureTime = "08:00";
        partTrain.arrivalTime = "14:00";
        partTrain.journeyDuration = "6h";
        partTrain.sleeperFare = 1000;
        partTrain.ac3TierFare = 1500;
        partTrain.ac2TierFare = 2000;
        partTrain.chairCarFare = 800;
        partTrain.sleeperSeatsAvailable = 10;
        partTrain.ac3TierSeatsAvailable = 10;
        partTrain.ac2TierSeatsAvailable = 10;
        partTrain.chairCarSeatsAvailable = 10;
        partTrain.journeyDate = "2026-12-25";

        bool sixCreated = BookingManager::createBooking(
            partTrain, "Sleeper", sixPassengers, 6000.0, sixPnr, partError);
        check(sixCreated, "Create confirmed group of 6 for partial promotion");

        bool fourCreated = sixCreated && BookingManager::createBooking(
            partTrain, "Sleeper", fourPassengers, 4000.0, fourPnr, partError);
        check(fourCreated, "Fill remaining seats with confirmed group of 4");

        bool wl1Created = fourCreated && BookingManager::createBooking(
            partTrain, "Sleeper", twoPassengers, 1000.0, wl1Pnr, partError);
        check(wl1Created, "Queue first waitlisted group of 2");

        bool wl2Created = wl1Created && BookingManager::createBooking(
            partTrain, "Sleeper", threePassengers, 1000.0, wl2Pnr, partError);
        check(wl2Created, "Queue second waitlisted group of 3");

        if (wl2Created) {
            Booking wl1, wl2;
            bool wl1Found = BookingManager::findBooking(wl1Pnr, wl1);
            bool wl2Found = BookingManager::findBooking(wl2Pnr, wl2);
            check(wl1Found && wl1.status == "Waitlisted" &&
                      wl1.waitlistPosition == 1,
                  "First waitlisted group at position 1");
            check(wl2Found && wl2.status == "Waitlisted" &&
                      wl2.waitlistPosition == 2,
                  "Second waitlisted group at position 2");

            // Cancel the group of 4: releases 4 seats. Enough for group of 2
            // (promoted, 2 left) but not for group of 3 (stays waitlisted).
            double partRefund = 0.0;
            double partCharge = 0.0;
            bool partCancelled = Cancellation::cancelBooking(
                fourPnr, partRefund, partCharge, partError);
            check(partCancelled, "Cancel group of 4 to trigger partial promotion");

            if (partCancelled) {
                Booking promoted, remaining;
                bool promFound = BookingManager::findBooking(wl1Pnr, promoted);
                bool remFound = BookingManager::findBooking(wl2Pnr, remaining);
                check(promFound && promoted.status == "Confirmed",
                      "First waitlisted group promoted");
                check(remFound && remaining.status == "Waitlisted",
                      "Second waitlisted group stays waitlisted (partial promotion)");

                std::ifstream trains("data/trains.dat");
                Train after;
                std::string trainLine;
                bool parsed = false;
                if (std::getline(trains, trainLine)) {
                    parsed = parseTrainRecord(trainLine, after);
                }
                check(parsed && after.sleeperSeatsAvailable == 2,
                      "Seat inventory after partial promotion is 2");
            }
        }
    }

    // Additional cancellation failure-path tests.

    // Test: Reject cancellation when the train does not exist.
    {
        {
            std::ofstream trainFile("data/trains.dat", std::ios::trunc);
            trainFile
                << "OTHER101|Other Express|Bengaluru|Chennai|08:00|14:00|6h|"
                << "1000|1500|2000|800|10|10|10|10|2026-12-15\n";
        }

        {
            std::ofstream bookingsFile(
                "data/bookings.dat", std::ios::trunc
            );
            bookingsFile
                << "1234567892|MISSING101|2026-12-15|Sleeper|"
                << "Test Passenger,21,Other|1|500.00|Confirmed\n";
        }

        double testRefund = 0.0;
        double testCharge = 0.0;
        std::string testError;

        check(
            !Cancellation::cancelBooking(
                "1234567892", testRefund, testCharge, testError
            ),
            "Reject cancellation when the train record is missing"
        );
    }

    // Test: Reject cancellation when the booking has an invalid class.
    {
        {
            std::ofstream trainFile("data/trains.dat", std::ios::trunc);
            trainFile
                << "TEST104|Class Test Express|Bengaluru|Chennai|"
                << "08:00|14:00|6h|1000|1500|2000|800|"
                << "10|10|10|10|2026-12-15\n";
        }

        {
            std::ofstream bookingsFile(
                "data/bookings.dat", std::ios::trunc
            );
            bookingsFile
                << "1234567893|TEST104|2026-12-15|InvalidClass|"
                << "Test Passenger,21,Other|1|500.00|Confirmed\n";
        }

        double testRefund = 0.0;
        double testCharge = 0.0;
        std::string testError;

        check(
            !Cancellation::cancelBooking(
                "1234567893", testRefund, testCharge, testError
            ),
            "Reject cancellation for an invalid travel class"
        );
    }


    // Additional train-record and departure-time validation tests.
    auto runInvalidTrainCase = [&](const std::string& trainRecord,
                                   const std::string& description) {
        {
            std::ofstream trainFile("data/trains.dat", std::ios::trunc);
            trainFile << trainRecord << '\n';
        }

        {
            std::ofstream bookingsFile("data/bookings.dat", std::ios::trunc);
            bookingsFile
                << "1234567894|TEST105|2026-12-15|Sleeper|"
                << "Test Passenger,21,Other|1|500.00|Confirmed\n";
        }

        double testRefund = 100.0;
        double testCharge = 100.0;
        std::string testError;

        const bool result = Cancellation::cancelBooking(
            "1234567894", testRefund, testCharge, testError
        );

        check(!result, description);
        check(testRefund == 0.0 && testCharge == 0.0,
              "Reset refund and charge after invalid train data");
    };

    // Malformed train record: fewer than the required fields.
    runInvalidTrainCase(
        "TEST105|Malformed Train",
        "Reject cancellation when the train record is malformed"
    );

    // Correct number of fields, but departure time has the wrong length.
    runInvalidTrainCase(
        "TEST105|Time Test|Bengaluru|Chennai|8:00|14:00|6h|"
        "1000|1500|2000|800|10|10|10|10|2026-12-15",
        "Reject cancellation when departure time has the wrong format"
    );

    // Correct length, but departure time contains non-numeric characters.
    runInvalidTrainCase(
        "TEST105|Time Test|Bengaluru|Chennai|ab:cd|14:00|6h|"
        "1000|1500|2000|800|10|10|10|10|2026-12-15",
        "Reject cancellation when departure time contains invalid characters"
    );

    std::cout << "\nCancellation Test Results\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "Total: " << passed + failed << '\n';

    return failed == 0 ? 0 : 1;
}
