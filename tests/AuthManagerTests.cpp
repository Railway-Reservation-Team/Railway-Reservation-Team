#include "AuthManager.h"

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

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


std::string testChecksum(const std::vector<std::string>& records) {
    std::uint64_t hash = 14695981039346656037ULL;

    for (const auto& record : records) {
        for (unsigned char ch : record) {
            hash ^= ch;
            hash *= 1099511628211ULL;
        }
        hash ^= static_cast<unsigned char>('\n');
        hash *= 1099511628211ULL;
    }

    std::ostringstream output;
    output << std::hex << std::setfill('0') << std::setw(16) << hash;
    return output.str();
}

void writeAccountFile(const std::vector<std::string>& records) {
    std::ofstream file("data/users.dat", std::ios::trunc);
    file << "RRS_USERS_V1|" << records.size()
         << "|" << testChecksum(records) << '\n';

    for (const auto& record : records) {
        file << record << '\n';
    }
}

std::vector<std::string> splitAccountRecord(const std::string& record) {
    std::vector<std::string> fields;
    std::stringstream stream(record);
    std::string field;

    while (std::getline(stream, field, '|')) {
        fields.push_back(field);
    }

    return fields;
}

std::string joinAccountFields(const std::vector<std::string>& fields) {
    std::ostringstream output;

    for (std::size_t i = 0; i < fields.size(); ++i) {
        if (i != 0) output << '|';
        output << fields[i];
    }

    return output.str();
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

    // Verify the persisted credential record contains a salt and a derived hash.
    {
        std::ifstream file("data/users.dat");
        std::string header;
        std::string record;
        std::getline(file, header);
        std::getline(file, record);

        std::vector<std::string> fields;
        std::stringstream stream(record);
        std::string field;
        while (std::getline(stream, field, '|')) {
            fields.push_back(field);
        }

        const auto isHex = [](const std::string& value) {
            return !value.empty() && value.find_first_not_of(
                "0123456789abcdefABCDEF") == std::string::npos;
        };

        test("Passenger credential record has seven fields",
             fields.size() == 7);
        test("Passenger password uses a 16-byte hexadecimal salt",
             fields.size() == 7 && fields[3].size() == 32 && isHex(fields[3]));
        test("Passenger password uses a 32-byte hexadecimal PBKDF2 hash",
             fields.size() == 7 && fields[4].size() == 64 && isHex(fields[4]));
        test("Passenger salt and derived hash are not identical",
             fields.size() == 7 && fields[3] != fields[4]);
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

    // Authentication logs must never contain the plaintext password.
    {
        std::ifstream file("data/auth_audit.log");
        std::stringstream content;
        content << file.rdbuf();
        test("Passenger authentication log excludes plaintext password",
             content.str().find(password) == std::string::npos);
    }

    AuthManager::logout();

    // 17. Logout should terminate the session.
    test("Logout terminates passenger session",
         !AuthManager::isSessionActive(2901));


    // Additional registration validation cases.
    test("Empty passenger name rejected",
         !AuthManager::registerPassenger(
             "", "emptyname@example.com",
             "9876543212", "AnotherPass123", error));

    test("Password longer than 128 characters rejected",
         !AuthManager::registerPassenger(
             "Test Passenger", "longpass@example.com",
             "9876543212", std::string(129, 'A'), error));

    test("Email without at sign rejected",
         !AuthManager::registerPassenger(
             "Test Passenger", "invalidemail.example.com",
             "9876543212", "AnotherPass123", error));

    test("Email with multiple at signs rejected",
         !AuthManager::registerPassenger(
             "Test Passenger", "bad@@example.com",
             "9876543212", "AnotherPass123", error));

    test("Email with missing domain rejected",
         !AuthManager::registerPassenger(
             "Test Passenger", "user@",
             "9876543212", "AnotherPass123", error));

    test("Email with missing domain extension rejected",
         !AuthManager::registerPassenger(
             "Test Passenger", "user@domain",
             "9876543212", "AnotherPass123", error));

    test("Email with consecutive domain dots rejected",
         !AuthManager::registerPassenger(
             "Test Passenger", "user@domain..com",
             "9876543212", "AnotherPass123", error));

    test("Email containing whitespace rejected",
         !AuthManager::registerPassenger(
             "Test Passenger", "bad email@example.com",
             "9876543212", "AnotherPass123", error));

    test("Email longer than 254 characters rejected",
         !AuthManager::registerPassenger(
             "Test Passenger", std::string(245, 'a') + "@example.com",
             "9876543212", "AnotherPass123", error));

    test("Phone containing alphabetic characters rejected",
         !AuthManager::registerPassenger(
             "Test Passenger", "badphone1@example.com",
             "98765abc10", "AnotherPass123", error));

    test("Phone with fewer than seven digits rejected",
         !AuthManager::registerPassenger(
             "Test Passenger", "badphone2@example.com",
             "123456", "AnotherPass123", error));

    test("Phone with more than fifteen digits rejected",
         !AuthManager::registerPassenger(
             "Test Passenger", "badphone3@example.com",
             "1234567890123456", "AnotherPass123", error));

    test("Phone with plus sign in the middle rejected",
         !AuthManager::registerPassenger(
             "Test Passenger", "badphone4@example.com",
             "98765+43210", "AnotherPass123", error));

    // Unknown account and inactive session paths.
    test("Login with unknown email rejected",
         !AuthManager::login(
             "unknown@example.com", password, error, 3000));

    AuthManager::logout();

    test("Activity update rejected without an active session",
         !AuthManager::recordActivity(3001));

    test("Current user email empty after logout",
         AuthManager::getCurrentUserEmail().empty());


    // Additional account-storage parsing and validation tests.
    {
        std::ifstream input("data/users.dat");
        std::string header;
        std::string validRecord;
        std::getline(input, header);
        std::getline(input, validRecord);

        const auto originalFields = splitAccountRecord(validRecord);

        if (originalFields.size() != 7) {
            std::cerr << "Could not read the original valid account record.\n";
            return 1;
        }

        const auto rejectModifiedRecord =
            [&](const std::string& label,
                const std::function<void(std::vector<std::string>&)>& modify) {
                auto fields = originalFields;
                modify(fields);

                writeAccountFile({joinAccountFields(fields)});
                const bool rejected =
                    !AuthManager::login(email, password, error, 3000);

                test(label, rejected);
                writeAccountFile({validRecord});
            };

        rejectModifiedRecord("Account record with missing field rejected",
            [](auto& fields) { fields.pop_back(); });

        rejectModifiedRecord("Non-numeric failed login count rejected",
            [](auto& fields) { fields[5] = "abc"; });

        rejectModifiedRecord("Negative failed login count rejected",
            [](auto& fields) { fields[5] = "-1"; });

        rejectModifiedRecord("Failed login count at lockout limit rejected",
            [](auto& fields) {
                fields[5] = std::to_string(AuthManager::MAX_LOGIN_ATTEMPTS);
            });

        rejectModifiedRecord("Negative lockout timestamp rejected",
            [](auto& fields) { fields[6] = "-1"; });

        rejectModifiedRecord("Invalid hexadecimal password salt rejected",
            [](auto& fields) { fields[3] = std::string(32, 'Z'); });

        rejectModifiedRecord("Incorrect password salt length rejected",
            [](auto& fields) { fields[3] = "abcd"; });

        rejectModifiedRecord("Invalid hexadecimal password hash rejected",
            [](auto& fields) { fields[4] = std::string(64, 'Z'); });

        rejectModifiedRecord("Incorrect password hash length rejected",
            [](auto& fields) { fields[4] = "abcd"; });

        // Two records with the same email must be rejected.
        writeAccountFile({validRecord, validRecord});
        test("Duplicate email records in storage rejected",
             !AuthManager::login(email, password, error, 3000));

        // Restore valid storage for the remaining tests.
        writeAccountFile({validRecord});

        // A file without a header is invalid.
        {
            std::ofstream file("data/users.dat", std::ios::trunc);
        }
        test("Empty account storage file rejected",
             !AuthManager::login(email, password, error, 3000));

        // An empty record is rejected before checksum validation.
        {
            std::ofstream file("data/users.dat", std::ios::trunc);
            file << "RRS_USERS_V1|1|invalid-checksum\n\n";
        }
        test("Empty account record rejected",
             !AuthManager::login(email, password, error, 3000));

        // Restore valid storage before continuing.
        writeAccountFile({validRecord});
    }

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
