#include "BookingManager.h"
#include "TrainSearch.h"
#include "TrainData.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

int main() {
    using Clock = std::chrono::steady_clock;

    int passed = 0;
    int failed = 0;

    // Unlike assert(), these checks remain active in Release builds.
    auto check = [&](bool condition, const std::string& description) {
        if (condition) {
            ++passed;
            std::cout << "[PASS] " << description << '\n';
        } else {
            ++failed;
            std::cerr << "[FAIL] " << description << '\n';
        }
    };

    auto elapsedMs = [](Clock::time_point start) {
        return std::chrono::duration<double, std::milli>(
            Clock::now() - start
        ).count();
    };

    const fs::path originalDirectory = fs::current_path();
    const fs::path testDirectory =
        fs::temp_directory_path() / "railway_performance_tests";

    try {
        fs::remove_all(testDirectory);
        fs::create_directories(testDirectory / "data");
        fs::current_path(testDirectory);

        // RRS-56: Search benchmarks for different dataset sizes.
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

            bool correctResults = true;

            for (int i = 0; i < ITERATIONS; ++i) {
                auto result = searchTrains(
                    benchmarkTrains,
                    "Bengaluru",
                    "Chennai",
                    "2099-12-31"
                );

                if (result.size() != 1) {
                    correctResults = false;
                }
            }

            const double averageMs = elapsedMs(start) / ITERATIONS;

            check(
                correctResults,
                "Search returns the correct result for " +
                    std::to_string(size) + " trains"
            );

            std::cout << "Trains: " << size
                      << " | Average search: "
                      << std::fixed << std::setprecision(4)
                      << averageMs << " ms\n";
        }

        // Prepare the typical SRS dataset of 200 trains.
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

        // RRS-56: Repeatable response-time and p90 measurement.
        constexpr int SAMPLE_COUNT = 100;
        constexpr double JIRA_LIMIT_MS = 3000.0;

        std::vector<double> searchSamples;
        searchSamples.reserve(SAMPLE_COUNT);

        for (int i = 0; i < SAMPLE_COUNT; ++i) {
            const auto sampleStart = Clock::now();

            const auto sampleResult = searchTrains(
                trains, " Bengaluru ", "Chennai", "2099-12-31"
            );

            searchSamples.push_back(elapsedMs(sampleStart));

            if (sampleResult.size() != 1) {
                check(false, "Search returns the expected result");
                break;
            }
        }

        check(
            searchSamples.size() == SAMPLE_COUNT,
            "Completed all 100 search measurements"
        );

        double searchMs = 0.0;
        double p90Ms = 0.0;
        int requestsWithinLimit = 0;

        if (searchSamples.size() == SAMPLE_COUNT) {
            for (double sample : searchSamples) {
                searchMs += sample;

                if (sample <= JIRA_LIMIT_MS) {
                    ++requestsWithinLimit;
                }
            }

            searchMs /= SAMPLE_COUNT;

            std::sort(searchSamples.begin(), searchSamples.end());

            // Nearest-rank p90: the 90th observation in 100 samples.
            p90Ms = searchSamples[89];

            const double compliance =
                (requestsWithinLimit * 100.0) / SAMPLE_COUNT;

            std::cout << "\n=== RRS-56 Response-Time Results ===\n"
                      << "Environment: Ubuntu WSL, local test execution\n"
                      << "Dataset: 200 trains\n"
                      << "Sample count: " << SAMPLE_COUNT << '\n'
                      << "Average search latency: " << searchMs << " ms\n"
                      << "P90 search latency: " << p90Ms << " ms\n"
                      << "Requests within 3000 ms: "
                      << requestsWithinLimit << "/" << SAMPLE_COUNT << '\n'
                      << "Compliance: " << compliance << "%\n";

            check(
                p90Ms <= JIRA_LIMIT_MS,
                "Jira RRS-56: p90 latency is within 3000 ms"
            );

            check(
                compliance >= 90.0,
                "Jira RRS-56: At least 90% of requests meet 3000 ms"
            );

            check(
                searchMs <= 2000.0,
                "SRS NFR-01: Average search time is within 2000 ms"
            );
        } else {
            check(false, "Response-time measurements completed");
        }

        const auto results = searchTrains(
            trains, " Bengaluru ", "Chennai", "2099-12-31"
        );

        check(
            results.size() == 1,
            "Train search returns the expected result for 200 trains"
        );

        check(
            searchMs <= 2000.0,
            "NFR-01: Search response time is within 2000 ms"
        );

        // Seed train data in the isolated temporary directory.
        {
            std::ofstream file("data/trains.dat", std::ios::trunc);

            check(file.is_open(), "Train data file can be opened for writing");

            if (file.is_open()) {
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

                check(file.good(), "Train records written successfully");
            }
        }

        {
            std::ofstream bookings("data/bookings.dat", std::ios::trunc);
            check(bookings.is_open(), "Booking data file can be created");
        }

        // NFR-04: Measure train-file read time.
        const auto ioStart = Clock::now();

        std::ifstream input("data/trains.dat");
        check(input.is_open(), "Train data file can be opened for reading");

        std::string line;
        int recordCount = 0;

        if (input.is_open()) {
            while (std::getline(input, line)) {
                if (!line.empty()) {
                    ++recordCount;
                }
            }
        }

        input.close();

        const double ioMs = elapsedMs(ioStart);

        check(
            recordCount == 200,
            "Train file contains all 200 expected records"
        );

        check(
            ioMs <= 200.0,
            "NFR-04: Train-file read time is within 200 ms"
        );

        // NFR-02: Measure booking creation.
        const Passenger passenger{"Performance Test", 21, "Other"};

        std::string pnr;
        std::string error;

        const auto bookingStart = Clock::now();

        const bool created = BookingManager::createBooking(
            trains.front(),
            "Sleeper",
            std::vector<Passenger>{passenger},
            500.0,
            pnr,
            error
        );

        const double bookingMs = elapsedMs(bookingStart);

        if (!created) {
            std::cerr << "Booking creation error: " << error << '\n';
        }

        check(created, "Booking creation succeeds");
        check(pnr.size() == 10, "Generated PNR has 10 characters");

        check(
            bookingMs <= 3000.0,
            "NFR-02: Booking operation is within 3000 ms"
        );

        // Write performance results in the original CTest directory.
        fs::current_path(originalDirectory);

        {
            std::ofstream report(
                "performance-results.txt",
                std::ios::trunc
            );

            check(report.is_open(), "Performance report can be created");

            if (report.is_open()) {
                report << std::fixed << std::setprecision(4);

                report << "Performance Test Results\n"
                       << "Typical dataset: 200 trains\n"
                       << "NFR-01 search: " << searchMs
                       << " ms (limit 2000 ms)\n"
                       << "NFR-02 booking creation: " << bookingMs
                       << " ms (limit 3000 ms)\n"
                       << "NFR-04 train-file read: " << ioMs
                       << " ms (limit 200 ms)\n"
                       << "NFR-03 startup: not measured\n"
                       << "NFR-04 booking/seat-inventory/user file I/O: "
                       << "not fully measured\n"
                       << "NFR-05 peak memory: not measured\n";

                report << "Checks passed: " << passed << '\n'
                       << "Checks failed before report: " << failed << '\n';

                check(
                    report.good(),
                    "Performance results written successfully"
                );
            }
        }

        fs::remove_all(testDirectory);

    } catch (const std::exception& exception) {
        std::cerr << "[FAIL] Performance test exception: "
                  << exception.what() << '\n';
        ++failed;

        fs::current_path(originalDirectory);
        fs::remove_all(testDirectory);
    }


    if (failed > 0) {
        std::cerr << "Performance tests FAILED.\n";
        return 1;
    }

    std::cout << "All performance checks passed.\n";
    return 0;
}
