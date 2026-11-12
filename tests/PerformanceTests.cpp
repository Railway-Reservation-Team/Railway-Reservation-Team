#include "BookingManager.h"
#include "TrainSearch.h"
#include "TrainData.h"

#include <cassert>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main() {
  using Clock = std::chrono::steady_clock;
    // RRS-56: Train search response-time benchmark.
    std::cout << "\n=== RRS-56 Search Benchmarks ===\n";

    for (int size : {1000, 10000, 50000}) {
        std::vector<Train> benchmarkTrains;
        benchmarkTrains.reserve(size);

        for (int i = 0; i < size; ++i) {
            Train train{};
            train.trainNumber = std::to_string(10000 + i);
            train.source = "Bengaluru";
            train.destination =
                (i == size - 1) ? "Chennai" : "Other City";
            train.journeyDate = "2099-12-31";
            train.active = true;
            benchmarkTrains.push_back(train);
        }

        constexpr int ITERATIONS = 100;
        const auto start = Clock::now();

        for (int i = 0; i < ITERATIONS; ++i) {
            auto result = searchTrains(
                benchmarkTrains,
                "Bengaluru",
                "Chennai",
                "2099-12-31"
            );

            assert(result.size() == 1);
        }

        const auto elapsed =
            std::chrono::duration<double, std::milli>(
                Clock::now() - start
            ).count();

        std::cout << "Trains: " << size
                  << " | Average search: "
                  << elapsed / ITERATIONS << " ms\n";
    }
    using Clock = std::chrono::steady_clock;
    std::vector<Train> trains;
    trains.reserve(200);

    for (int i = 0; i < 200; ++i) {
        Train train{};
        train.trainNumber = std::to_string(10000 + i);
        train.trainName = "Performance Express " + std::to_string(i);
        train.source = "Bengaluru";
        train.destination = (i == 0) ? "Chennai" : "Other City";
        train.departureTime = "10:00";
        train.arrivalTime = "18:00";
        train.journeyDuration = "8h";
        train.sleeperFare = 500;
        train.ac3TierFare = 800;
        train.ac2TierFare = 1000;
        train.chairCarFare = 400;
        train.sleeperSeatsAvailable = 50;
        train.ac3TierSeatsAvailable = 50;
        train.ac2TierSeatsAvailable = 50;
        train.chairCarSeatsAvailable = 50;
        train.journeyDate = "2099-12-31";
        train.active = true;
        trains.push_back(train);
    }

    // NFR-01: train search against a seeded 200-train dataset (<= 2 seconds).
    const auto searchStart = Clock::now();
    const auto results = searchTrains(
        trains, " Bengaluru ", "Chennai", "2099-12-31");
    const auto searchMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - searchStart).count();
    assert(results.size() == 1);
    assert(searchMs <= 2000);

    // Seed the same 200-train dataset for booking and file I/O checks.
    {
        std::ofstream file("data/trains.dat", std::ios::trunc);
        assert(file.is_open());
        for (const Train& train : trains) {
            file << train.trainNumber << '|'
                 << train.trainName << '|'
                 << train.source << '|'
                 << train.destination << '|'
                 << train.departureTime << '|'
                 << train.arrivalTime << '|'
                 << train.journeyDuration << '|'
                 << train.sleeperFare << '|'
                 << train.ac3TierFare << '|'
                 << train.ac2TierFare << '|'
                 << train.chairCarFare << '|'
                 << train.sleeperSeatsAvailable << '|'
                 << train.ac3TierSeatsAvailable << '|'
                 << train.ac2TierSeatsAvailable << '|'
                 << train.chairCarSeatsAvailable << '|'
                 << train.journeyDate << '\n';
        }
    }
    {
        std::ofstream bookings("data/bookings.dat", std::ios::trunc);
        assert(bookings.is_open());
    }

    // NFR-04: read the seeded train file (<= 200 milliseconds).
    const auto ioStart = Clock::now();
    std::ifstream input("data/trains.dat");
    assert(input.is_open());
    std::string line;
    int recordCount = 0;
    while (std::getline(input, line)) {
        if (!line.empty()) ++recordCount;
    }
    const auto ioMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - ioStart).count();
    assert(recordCount == 200);
    assert(ioMs <= 200);

    // NFR-02: booking transaction against the seeded dataset (<= 3 seconds).
    const Passenger passenger{"Performance Test", 21, "Other"};
    std::string pnr;
    std::string error;
    const auto bookingStart = Clock::now();
    const bool created = BookingManager::createBooking(
        trains.front(), "Sleeper", std::vector<Passenger>{passenger},
        500.0, pnr, error);
    const auto bookingMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - bookingStart).count();
    assert(created);
    assert(pnr.size() == 10);
    assert(bookingMs <= 3000);

    {
        std::ofstream report("performance-results.txt", std::ios::trunc);
        assert(report.is_open());
        report << "Seeded train count: 200\n"
               << "NFR-01 search: " << searchMs << " ms (limit 2000 ms)\n"
               << "NFR-02 booking: " << bookingMs << " ms (limit 3000 ms)\n"
               << "NFR-04 train-file read: " << ioMs << " ms (limit 200 ms)\n"
               << "NFR-03 startup: not measured by this test\n"
               << "NFR-05 peak memory: not measured by this test\n";
    }

    std::cout << "Performance checks passed for 200 seeded trains.\n";
    std::cout << "Search time (NFR-01): " << searchMs << " ms (limit 2000 ms)\n";
    std::cout << "Booking time (NFR-02): " << bookingMs << " ms (limit 3000 ms)\n";
    std::cout << "Train file read time (NFR-04): " << ioMs << " ms (limit 200 ms)\n";
    std::cout << "Startup time (NFR-03) and peak memory (NFR-05) require separate process-level measurement.\n";
    return 0;
}
