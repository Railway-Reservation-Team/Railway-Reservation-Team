#include "AuthManager.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

namespace {

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

}

int main() {
    namespace fs = std::filesystem;

    const fs::path originalDirectory = fs::current_path();
    const fs::path temporaryRoot = fs::temp_directory_path();

    std::error_code filesystemError;
    fs::path testDirectory;
    std::random_device randomDevice;

    for (int attempt = 0; attempt < 100; ++attempt) {
        const auto timestamp =
            std::chrono::steady_clock::now().time_since_epoch().count();

        const std::string directoryName =
            "RRS_AuthManager_Test_" + std::to_string(timestamp) + "_" +
            std::to_string(randomDevice());

        const fs::path candidate = temporaryRoot / directoryName;

        filesystemError.clear();

        if (fs::create_directory(candidate, filesystemError)) {
            testDirectory = candidate;
            break;
        }

        if (filesystemError) {
            std::cerr << "Unable to create authentication test directory.\n";
            return 1;
        }
    }

    if (testDirectory.empty()) {
        std::cerr << "Unable to create a unique authentication test directory.\n";
        return 1;
    }

    fs::current_path(testDirectory);

    const std::string password = "RailwayPass123";
    const std::string email = "pooja@example.com";
    std::string error;

    // 1. Register a valid passenger.
    test("Valid passenger registration",
         AuthManager::registerPassenger(
             "Pooja Koppad", email, "9876543210", password, error));

    // 2. Reject duplicate email addresses, ignoring letter case.
    test("Duplicate email registration rejected",
         !AuthManager::registerPassenger(
             "Another Passenger", "POOJA@EXAMPLE.COM",
             "9876543211", "AnotherPass123", error));

    // 3. Confirm that plaintext passwords are not stored.
    {
        std::ifstream file("data/users.dat");
        std::stringstream content;
        content << file.rdbuf();

        test("Plaintext password is not stored",
             content.str().find(password) == std::string::npos);
    }

    // 4. Confirm that the account file has a record-count/checksum header.
    {
        std::ifstream file("data/users.dat");
        std::string header;
        std::getline(file, header);

        test("Account storage contains integrity header",
             header.rfind("RRS_USERS_V1|1|", 0) == 0);
    }

    // 5. Reject an invalid email address.
    test("Invalid email rejected",
         !AuthManager::registerPassenger(
             "Test Passenger", "not-an-email",
             "9876543212", "AnotherPass123", error));

    // 6. Reject a short password.
    test("Password shorter than 8 characters rejected",
         !AuthManager::registerPassenger(
             "Test Passenger", "short@example.com",
             "9876543212", "short", error));

    // 7. Reject an invalid phone number.
    test("Invalid phone number rejected",
         !AuthManager::registerPassenger(
             "Test Passenger", "phone@example.com",
             "123", "AnotherPass123", error));

    // 8. Reject a wrong password.
    test("Incorrect password rejected",
         !AuthManager::login(email, "WrongPassword123", error, 1000));

    // 9. Correct credentials should authenticate.
    test("Correct email and password accepted",
         AuthManager::login(email, password, error, 1000));

    // 10. Session should remain active before 15 minutes.
    test("Session active before timeout",
         AuthManager::isSessionActive(1899));

    // 11. Activity should refresh the session timeout.
    test("Session activity successfully refreshed",
         AuthManager::recordActivity(1899));

    test("Session remains active after activity refresh",
         AuthManager::isSessionActive(2798));

    // 12. Session should expire after 15 minutes of inactivity.
    test("Session expires after 15 minutes of inactivity",
         !AuthManager::isSessionActive(2799));

    test("Expired session is no longer authenticated",
         AuthManager::getCurrentUserEmail().empty());

    AuthManager::logout();

    // 13. Five consecutive incorrect passwords should trigger lockout.
    bool fiveFailuresRejected = true;

    for (int attempt = 0;
         attempt < AuthManager::MAX_LOGIN_ATTEMPTS;
         ++attempt) {
        if (AuthManager::login(
                email, "WrongPassword123", error, 2000)) {
            fiveFailuresRejected = false;
        }
    }

    test("Five incorrect login attempts rejected",
         fiveFailuresRejected);

    // 14. Correct password must not bypass an active lockout.
    test("Account locked after five failed attempts",
         !AuthManager::login(email, password, error, 2001));

    // 15. Login should succeed once the lockout expires.
    test("Login accepted after 15-minute lockout expires",
         AuthManager::login(email, password, error, 2900));

    // 16. Lockout should be recorded in the audit log.
    {
        std::ifstream file("data/auth_audit.log");
        std::stringstream content;
        content << file.rdbuf();

        test("Account lockout recorded in audit log",
             content.str().find(
                 "ACCOUNT_LOCKED_AFTER_FAILED_LOGINS"
             ) != std::string::npos);
    }

    AuthManager::logout();

    // 17. Logout should terminate the session.
    test("Logout terminates passenger session",
         !AuthManager::isSessionActive(2901));

    // 18. Corrupted storage must be rejected.
    {
        std::ofstream file("data/users.dat", std::ios::app);
        file << "CORRUPTED_RECORD\n";
    }

    test("Corrupted account storage rejected",
         !AuthManager::login(email, password, error, 3000) &&
         error.find("integrity") != std::string::npos);

    AuthManager::logout();

    std::cout << "\n==============================\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "==============================\n";

    fs::current_path(originalDirectory);
    fs::remove_all(testDirectory, filesystemError);

    return failed == 0 ? 0 : 1;
}
