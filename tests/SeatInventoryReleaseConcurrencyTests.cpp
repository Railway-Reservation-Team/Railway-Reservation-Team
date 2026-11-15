
#include "../src/SeatInventory.h"

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

int main() {
    Train train;
    train.trainNumber = "RELEASE-TEST";
    train.journeyDate = "2026-11-13";
    train.ac3TierSeatsAvailable = 25;

    SeatInventoryManager inventory;

    auto testTime = std::chrono::steady_clock::now();

    // Create one hold for five seats.
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

    // All threads attempt to release the same hold.
    for (int i = 0; i < THREAD_COUNT; ++i) {
        workers.emplace_back([&]() {
            ++ready;

            while (!start.load()) {
                std::this_thread::yield();
            }

            if (inventory.releaseHold(hold.holdId)) {
                ++successful;
            } else {
                ++unsuccessful;
            }
        });
    }

    // Wait until all threads are ready.
    while (ready.load() != THREAD_COUNT) {
        std::this_thread::yield();
    }

    // Start all release attempts.
    start.store(true);

    for (auto& worker : workers) {
        worker.join();
    }

    int remainingSeats =
        inventory.getLiveAvailableSeats(train, "3A", testTime);

    int activeHolds =
        inventory.getActiveHoldCount(testTime);

    bool passed =
        successful == 1 &&
        unsuccessful == THREAD_COUNT - 1 &&
        remainingSeats == 25 &&
        activeHolds == 0;

    std::cout << "\n===== CONCURRENT HOLD RELEASE TEST =====\n";

    std::cout << "Total threads: " << THREAD_COUNT << '\n';
    std::cout << "Successful releases: " << successful << '\n';
    std::cout << "Unsuccessful releases: " << unsuccessful << '\n';
    std::cout << "Remaining seats: " << remainingSeats << '\n';
    std::cout << "Active holds: " << activeHolds << '\n';

    std::cout << (passed
        ? "\n[PASS] Concurrent release test passed\n"
        : "\n[FAIL] Concurrent release test failed\n");

    return passed ? 0 : 1;
}
