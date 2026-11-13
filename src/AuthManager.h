#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <cstdint>
#include <string>

namespace AuthManager {

constexpr int MAX_LOGIN_ATTEMPTS = 5;
constexpr int ACCOUNT_LOCKOUT_MINUTES = 15;
constexpr int SESSION_TIMEOUT_MINUTES = 15;

/**
 * Represents a registered passenger account.
 * Passwords are stored only as salted password hashes.
 */
struct UserAccount {
    std::string name;
    std::string email;
    std::string phone;
    std::string passwordSalt;
    std::string passwordHash;
    int failedLoginAttempts = 0;
    std::int64_t lockedUntil = 0;
};

/**
 * Registers a new passenger account.
 * Rejects invalid details and duplicate email addresses.
 */
bool registerPassenger(
    const std::string& name,
    const std::string& email,
    const std::string& phone,
    const std::string& password,
    std::string& errorMessage
);

/**
 * Authenticates a passenger using registered email and password.
 * The optional timestamp supports deterministic lockout testing.
 */
bool login(
    const std::string& email,
    const std::string& password,
    std::string& errorMessage,
    std::int64_t currentTime = 0
);

/**
 * Checks whether the current passenger session is still active.
 */
bool isSessionActive(std::int64_t currentTime = 0);

/**
 * Updates the current session's last-activity timestamp.
 */
bool recordActivity(std::int64_t currentTime = 0);

/**
 * Ends the current passenger session.
 */
void logout();

/**
 * Returns the email address of the currently authenticated passenger.
 * Returns an empty string when no session is active.
 */
std::string getCurrentUserEmail();

/**
 * Clears in-memory authentication state.
 * Intended for isolated unit tests.
 */
void resetForTesting();

}

#endif
