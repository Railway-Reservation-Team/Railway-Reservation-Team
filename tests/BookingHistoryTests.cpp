
#include "../src/BookingManager.h"

#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    fs::path originalPath = fs::current_path();
    fs::path testPath =
        fs::temp_directory_path() / "railway_history_tests";

    fs::remove_all(testPath);
    fs::create_directories(testPath / "data");
    fs::current_path(testPath);

    std::ofstream file("data/bookings.dat");

    if (!file) {
        std::cout << "FAIL: Cannot create test booking file\n";
        fs::current_path(originalPath);
        fs::remove_all(testPath);
        return 1;
    }

    // Legacy 8-field booking records.
    file << "1234567890|1001|2026-11-10|Sleeper|Dinesh,20,Male|1|500.00|Confirmed\n";
    file << "1234567891|1002|2026-11-12|AC 3-Tier|Dinesh,20,Male|1|900.00|Confirmed\n";
    file << "1234567892|1003|2026-11-15|Sleeper|Test%20Passenger,22,Other|1|400.00|Confirmed\n";
    file << "1234567893|1004|2026-11-20|Sleeper|Dinesh%2C%20Kumar,21,Male|1|600.00|Confirmed\n";
    file << "1234567894|1005|2026-11-22|Sleeper|Dinesh%3B Kumar,22,Male|1|650.00|Confirmed\n";

    // New 9-field waitlisted booking record.
    file << "1234567895|1006|2026-11-25|Sleeper|Dinesh,25,Male|1|700.00|Waitlisted|3\n";

    // Invalid record with an unsupported number of fields.
    file << "1234567896|1007|2026-11-26|Sleeper|Dinesh,25,Male|1|700.00|Confirmed|2|EXTRA\n";

    // Invalid negative waitlist position.
    file << "1234567897|1008|2026-11-27|Sleeper|Dinesh,25,Male|1|700.00|Waitlisted|-1\n";

    file.close();

    int passed = 0;
    int failed = 0;

    auto check = [&](bool result, const char* name) {
        if (result) {
            std::cout << "PASS: " << name << '\n';
            passed++;
        } else {
            std::cout << "FAIL: " << name << '\n';
            failed++;
        }
    };

    // Retrieve legacy and new bookings.
    auto history = BookingManager::getBookingHistory("Dinesh");

    check(history.size() == 3,
          "Retrieve legacy and new 9-field bookings");

    auto lowercaseHistory =
        BookingManager::getBookingHistory("dinesh");

    check(lowercaseHistory.size() == 3,
          "Passenger name matching ignores case");

    check(
        history.size() >= 1 &&
        history[0].pnr == "1234567890" &&
        history[0].totalFare == 500.00 &&
        history[0].status == "Confirmed",
        "Verify legacy booking details"
    );

    // Check the new 9-field waitlisted booking.
    bool foundWaitlisted = false;

    for (const Booking& booking : history) {
        if (booking.pnr == "1234567895") {
            foundWaitlisted =
                booking.status == "Waitlisted" &&
                booking.waitlistPosition == 3;
        }
    }

    check(foundWaitlisted,
          "Retrieve waitlisted booking and its position");

    // Legacy records should have a default waitlist position of zero.
    check(
        history.size() >= 1 &&
        history[0].waitlistPosition == 0,
        "Legacy booking has default waitlist position"
    );

    auto commaHistory =
        BookingManager::getBookingHistory("Dinesh, Kumar");

    check(commaHistory.size() == 1,
          "Handle passenger names containing commas");

    auto semicolonHistory =
        BookingManager::getBookingHistory("Dinesh; Kumar");

    check(
        semicolonHistory.size() == 1 &&
        semicolonHistory[0].passengerDetails ==
            "Dinesh%3B Kumar,22,Male",
        "Preserve encoded semicolons in passenger details"
    );

    auto unknown =
        BookingManager::getBookingHistory("Unknown Person");

    check(unknown.empty(),
          "Reject unknown passenger");

    auto empty =
        BookingManager::getBookingHistory("");

    check(empty.empty(),
          "Handle empty passenger name");

    std::cout << "\n========== BOOKING HISTORY TEST SUMMARY ==========\n";
    std::cout << "Passed: " << passed
              << "\nFailed: " << failed
              << "\nTotal: " << passed + failed << '\n';

    fs::current_path(originalPath);
    fs::remove_all(testPath);

    return failed == 0 ? 0 : 1;
}
