
#include "SeatInventory.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

bool runStressTest(int iteration) {
    Train train;
    train.trainNumber = "STRESS-" + std::to_string(iteration);
    train.journeyDate = "2026-11-13";
    train.ac3TierSeatsAvailable = 25;

    SeatInventoryManager inventory;

    constexpr int THREAD_COUNT = 100;

    std::atomic<int> ready{0};
    std::atomic<int> successful{0};
    std::atomic<int> rejected{0};
    std::atomic<bool> start{false};

    const auto testTime = std::chrono::steady_clock::now();

    std::vector<std::thread> workers;

    for (int i = 0; i < THREAD_COUNT; ++i) {
        workers.emplace_back([&]() {
            ready++;

            while (!start.load()) {
                std::this_thread::yield();
            }

            SeatHoldResponse response =
                inventory.holdSeats(train, "3A", 1, testTime);

            if (response.result == SeatHoldResult::SUCCESS) {
                successful++;
            } else if (
                response.result ==
                SeatHoldResult::EXCEEDS_AVAILABLE_SEATS
            ) {
                rejected++;
            }
        });
    }

    while (ready.load() != THREAD_COUNT) {
        std::this_thread::yield();
    }

    start.store(true);

    for (auto& worker : workers) {
        worker.join();
    }

    int remaining =
        inventory.getLiveAvailableSeats(train, "3A", testTime);

    int active = inventory.getActiveHoldCount(testTime);

    bool passed =
        successful == 25 &&
        rejected == 75 &&
        remaining == 0 &&
        active == 25;

    std::cout << "Iteration " << iteration
              << ": " << (passed ? "PASS" : "FAIL")
              << " | Threads: " << THREAD_COUNT
              << " | Successful: " << successful
              << " | Rejected: " << rejected
              << " | Remaining: " << remaining
              << " | Active holds: " << active << '\n';

    return passed;
}

int main() {
    constexpr int ITERATIONS = 20;

    int passed = 0;

    for (int i = 1; i <= ITERATIONS; ++i) {
        if (runStressTest(i)) {
            ++passed;
        }
    }

    std::cout << "\n100-Thread Stress Test Summary\n";
    std::cout << "Total iterations: " << ITERATIONS << '\n';
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << ITERATIONS - passed << '\n';

    return passed == ITERATIONS ? 0 : 1;
}
