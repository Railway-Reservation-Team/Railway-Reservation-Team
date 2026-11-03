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

    file << "1234567890|1001|2026-11-10|Sleeper|Dinesh,20,Male|1|500.00|Confirmed\n";
    file << "1234567891|1002|2026-11-12|AC 3-Tier|Dinesh,20,Male|1|900.00|Confirmed\n";
    file << "1234567892|1003|2026-11-15|Sleeper|Test%20Passenger,22,Other|1|400.00|Confirmed\n";
    file << "1234567893|1004|2026-11-20|Sleeper|Dinesh%2C%20Kumar,21,Male|1|600.00|Confirmed\n";

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

    auto history = BookingManager::getBookingHistory("Dinesh");

    check(history.size() == 2, "Retrieve multiple bookings");

    auto lowercaseHistory =
        BookingManager::getBookingHistory("dinesh");

    check(lowercaseHistory.size() == 2,
          "Passenger name matching ignores case");

    check(
        history.size() >= 1 &&
        history[0].pnr == "1234567890" &&
        history[0].totalFare == 500.00 &&
        history[0].status == "Confirmed",
        "Verify booking details"
    );

    auto commaHistory =
        BookingManager::getBookingHistory("Dinesh, Kumar");

    check(commaHistory.size() == 1,
          "Handle passenger names containing commas");

    auto unknown =
        BookingManager::getBookingHistory("Unknown Person");

    check(unknown.empty(), "Reject unknown passenger");

    auto empty =
        BookingManager::getBookingHistory("");

    check(empty.empty(), "Handle empty passenger name");

    std::cout << "\nPassed: " << passed
              << "\nFailed: " << failed << '\n';

    fs::current_path(originalPath);
    fs::remove_all(testPath);

    return failed == 0 ? 0 : 1;
}
