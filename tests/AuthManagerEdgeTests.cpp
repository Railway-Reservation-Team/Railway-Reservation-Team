#include "AuthManager.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

int passed = 0;
int failed = 0;

void test(const std::string& name, bool condition) {
    if (condition) {
        std::cout << "[PASS] " << name << '\n';
        ++passed;
    } else {
        std::cout << "[FAIL] " << name << '\n';
        ++failed;
    }
}

int main() {
    const fs::path originalDirectory = fs::current_path();
    const fs::path testDirectory = fs::temp_directory_path() / "railway_auth_edge_tests";

    fs::remove_all(testDirectory);
    fs::create_directories(testDirectory);
    fs::current_path(testDirectory);

    std::string error;

    // Registration validation paths.
    test("Reject empty name",
         !AuthManager::registerPassenger("", "a@b.com", "9876543210", "Pass1234", error));
    test("Reject empty email",
         !AuthManager::registerPassenger("Name", "", "9876543210", "Pass1234", error));
    test("Reject password longer than 128 chars",
         !AuthManager::registerPassenger("Name", "long@b.com", "9876543210",
                                         std::string(129, 'a'), error));
    test("Reject password exactly 128 accepted",
         AuthManager::registerPassenger("Name", "long128@b.com", "9876543210",
                                        std::string(128, 'a'), error));

    // recordActivity with no active session returns false.
    AuthManager::logout();
    test("Record activity on inactive session returns false",
         !AuthManager::recordActivity(100));

    // Missing users.dat is treated as empty (login fails, not corrupt).
    {
        std::error_code ec;
        fs::remove("data/users.dat", ec);
    }
    test("Login with missing users file fails gracefully",
         !AuthManager::login("a@b.com", "Pass1234", error, 100));

    // Register then verify login works.
    test("Register valid passenger",
         AuthManager::registerPassenger("Edge User", "edge@b.com", "9876543210", "Pass1234", error));
    test("Login succeeds after registration",
         AuthManager::login("edge@b.com", "Pass1234", error, 100));
    test("Session active right after login",
         AuthManager::isSessionActive(100));
    AuthManager::logout();

    // Corrupt header (wrong version) is rejected.
    {
        std::ofstream file("data/users.dat");
        file << "BAD_HEADER|1|x\n";
    }
    test("Corrupt account header rejected",
         !AuthManager::login("edge@b.com", "Pass1234", error, 200) &&
         error.find("integrity") != std::string::npos);

    // Restore valid account and verify recordActivity boundary.
    {
        std::error_code ec;
        fs::remove("data/users.dat", ec);
    }
    test("Register again after cleanup",
         AuthManager::registerPassenger("Edge User", "edge@b.com", "9876543210", "Pass1234", error));
    test("Login after re-register",
         AuthManager::login("edge@b.com", "Pass1234", error, 300));

    // recordActivity returns true while session active (refreshes timestamp).
    test("Record activity refreshes session",
         AuthManager::recordActivity(400));
    test("Session still active after refresh",
         AuthManager::isSessionActive(400));

    // Session expires at/after timeout boundary (15 min = 900s).
    test("Session expires at 15-minute boundary",
         !AuthManager::isSessionActive(1300));

    AuthManager::logout();
    test("Session inactive after logout",
         !AuthManager::isSessionActive(1300));

    std::cout << "\nAuthManager Edge Test Results\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "Total: " << passed + failed << '\n';

    fs::current_path(originalDirectory);
    fs::remove_all(testDirectory);

    return failed == 0 ? 0 : 1;
}
