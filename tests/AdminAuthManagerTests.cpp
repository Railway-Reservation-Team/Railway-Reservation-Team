#include "AdminAuthManager.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <iostream>
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
    const fs::path testDirectory =
        fs::temp_directory_path() / "RRS_AdminAuthManager_Test";

    std::error_code filesystemError;
    fs::remove_all(testDirectory, filesystemError);
    fs::create_directories(testDirectory, filesystemError);

    if (filesystemError) {
        std::cerr << "Unable to create administrator test directory.\n";
        return 1;
    }

    fs::current_path(testDirectory);

    std::string error;
    const std::string username = "railadmin";
    const std::string password = "SecureRailwayPass123";

    // 1. No administrator account should exist initially.
    test("No administrator account initially exists",
         !AdminAuthManager::hasAdminAccount(error) && error.empty());

    // 2. Create the first administrator account.
    test("Valid administrator account setup",
         AdminAuthManager::setupAdmin(username, password, error));

    // 3. Confirm the account now exists.
    error.clear();
    test("Administrator account exists after setup",
         AdminAuthManager::hasAdminAccount(error));

    // 4. Reject a duplicate administrator account.
    test("Duplicate administrator setup rejected",
         !AdminAuthManager::setupAdmin(
             "anotheradmin", "AnotherSecurePass123", error));

    // 5. Reject a short username.
    test("Short administrator username rejected",
         !AdminAuthManager::setupAdmin("ab", password, error));

    // 6. Reject a short password.
    test("Short administrator password rejected",
         !AdminAuthManager::setupAdmin(username, "short", error));

    // 7. Confirm plaintext password is not stored.
    {
        std::ifstream file("data/admin.dat");
        std::string content(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());

        test("Plaintext administrator password is not stored",
             content.find(password) == std::string::npos);
    }

    // 8. Reject incorrect credentials.
    test("Incorrect administrator password rejected",
         !AdminAuthManager::login(username, "WrongPassword123", error));

    // 9. Confirm failed login does not create an authenticated session.
    test("Failed login does not authenticate administrator",
         !AdminAuthManager::isAuthenticated());

    // 10. Accept correct credentials.
    test("Correct administrator credentials accepted",
         AdminAuthManager::login(username, password, error));

    // 11. Confirm authenticated session.
    test("Administrator session becomes active",
         AdminAuthManager::isAuthenticated());

    // 12. Logout should terminate the session.
    AdminAuthManager::logout();
    test("Administrator logout terminates session",
         !AdminAuthManager::isAuthenticated());

    // 13. Five consecutive failed attempts should trigger lockout.
    bool failuresRejected = true;

    for (int attempt = 0;
         attempt < 5;
         ++attempt) {
        if (AdminAuthManager::login(
                username, "WrongPassword123", error)) {
            failuresRejected = false;
        }
    }

    test("Five incorrect login attempts rejected",
         failuresRejected);

    // 14. Correct password must not bypass an active lockout.
    test("Administrator account locked after five failures",
         !AdminAuthManager::login(username, password, error));

    test("Locked administrator cannot authenticate",
         !AdminAuthManager::isAuthenticated());

    // 15. Administrator security-relevant events must be timestamped.
    {
        std::ifstream file("data/admin_audit.log");
        std::string audit(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());

        test("Administrator account creation is audited",
             audit.find("ADMIN_ACCOUNT_CREATED") != std::string::npos);
        test("Successful administrator login is audited",
             audit.find("ADMIN_LOGIN_SUCCESS") != std::string::npos);
        test("Failed administrator login is audited",
             audit.find("ADMIN_LOGIN_FAILED") != std::string::npos);
        test("Administrator lockout is audited",
             audit.find("ADMIN_ACCOUNT_LOCKED") != std::string::npos);
        const std::size_t firstSeparator = audit.find('|');
        bool timestampValid = false;
        if (firstSeparator != std::string::npos && firstSeparator > 0) {
            try {
                (void)std::stoll(audit.substr(0, firstSeparator));
                timestampValid = true;
            } catch (...) {
                timestampValid = false;
            }
        }
        test("Audit records contain Unix timestamps", timestampValid);
        test("Audit log does not contain plaintext password",
             audit.find(password) == std::string::npos);
    }

    AdminAuthManager::resetForTesting();

    std::cout << "\n==============================\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "==============================\n";

    fs::current_path(originalDirectory);
    fs::remove_all(testDirectory, filesystemError);

    return failed == 0 ? 0 : 1;
}
