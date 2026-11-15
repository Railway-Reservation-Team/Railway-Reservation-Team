
#include "SeatInventory.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    Train train;
    train.trainNumber = "12627";
    train.journeyDate = "2026-11-13";
    train.ac3TierSeatsAvailable = 10;

    SeatInventoryManager inventory;

    constexpr int THREAD_COUNT = 20;
    std::atomic<int> successful{0};
    std::atomic<int> rejected{0};

    const auto testTime = std::chrono::steady_clock::now();

    std::vector<std::thread> workers;

    for (int i = 0; i < THREAD_COUNT; ++i) {
        workers.emplace_back([&]() {
            SeatHoldResponse response =
                inventory.holdSeats(train, "3A", 1, testTime);

            if (response.result == SeatHoldResult::SUCCESS) {
                ++successful;
            } else if (
                response.result ==
                SeatHoldResult::EXCEEDS_AVAILABLE_SEATS
            ) {
                ++rejected;
            }
        });
    }

    for (auto& worker : workers) {
        worker.join();
    }

    int remaining =
        inventory.getLiveAvailableSeats(train, "3A", testTime);

    int active = inventory.getActiveHoldCount(testTime);

    std::cout << "Concurrent Reservation Test\n";
    std::cout << "Total threads: " << THREAD_COUNT << '\n';
    std::cout << "Successful: " << successful << '\n';
    std::cout << "Rejected: " << rejected << '\n';
    std::cout << "Remaining seats: " << remaining << '\n';
    std::cout << "Active holds: " << active << '\n';

    bool passed =
        successful == 10 &&
        rejected == 10 &&
        remaining == 0 &&
        active == 10;

    std::cout << (passed ? "PASS" : "FAIL") << '\n';

    return passed ? 0 : 1;
}
