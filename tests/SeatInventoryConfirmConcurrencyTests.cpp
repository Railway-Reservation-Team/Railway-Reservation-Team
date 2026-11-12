
#include "../src/SeatInventory.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    Train train;
    train.trainNumber = "CONFIRM-TEST";
    train.journeyDate = "2026-11-13";
    train.ac3TierSeatsAvailable = 25;

    SeatInventoryManager inventory;

    auto testTime = std::chrono::steady_clock::now();

    SeatHoldResponse hold =
        inventory.holdSeats(train, "3A", 5, testTime);

    if (hold.result != SeatHoldResult::SUCCESS) {
        std::cout << "[FAIL] Could not create initial hold\n";
        return 1;
    }

    constexpr int THREAD_COUNT = 20;

    std::atomic<int> ready{0};
    std::atomic<int> successful{0};
    std::atomic<int> unsuccessful{0};
    std::atomic<bool> start{false};

    std::vector<std::thread> workers;

    for (int i = 0; i < THREAD_COUNT; ++i) {
        workers.emplace_back([&]() {
            ready++;

            while (!start.load()) {
                std::this_thread::yield();
            }

            if (inventory.confirmHold(
                    train, hold.holdId, testTime)) {
                successful++;
            } else {
                unsuccessful++;
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

    int expectedRemainingSeats = 20;

    int activeHolds =
        inventory.getActiveHoldCount(testTime);

    bool passed =
        successful == 1 &&
        unsuccessful == THREAD_COUNT - 1 &&
        train.ac3TierSeatsAvailable == expectedRemainingSeats &&
        activeHolds == 0;

    std::cout << "\n===== CONCURRENT HOLD CONFIRMATION TEST =====\n";
    std::cout << "Total threads: " << THREAD_COUNT << '\n';
    std::cout << "Successful confirmations: " << successful << '\n';
    std::cout << "Unsuccessful confirmations: " << unsuccessful << '\n';
    std::cout << "Remaining seats: "
              << train.ac3TierSeatsAvailable << '\n';
    std::cout << "Active holds: " << activeHolds << '\n';

    std::cout << (passed
        ? "\n[PASS] Concurrent confirmation test passed\n"
        : "\n[FAIL] Concurrent confirmation test failed\n");

    return passed ? 0 : 1;
}
