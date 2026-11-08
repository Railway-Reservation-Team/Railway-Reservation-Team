#include "AuthManager.h"

#include "AtomicFileReplace.h"
#include "Validation.h"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

namespace {

constexpr int PASSWORD_HASH_ITERATIONS = 600000;
constexpr int SALT_BYTES = 16;
constexpr int HASH_BYTES = 32;
constexpr std::int64_t SESSION_TIMEOUT_SECONDS =
    static_cast<std::int64_t>(AuthManager::SESSION_TIMEOUT_MINUTES) * 60;
constexpr std::int64_t LOCKOUT_SECONDS =
    static_cast<std::int64_t>(AuthManager::ACCOUNT_LOCKOUT_MINUTES) * 60;

const std::string USERS_FILE = "data/users.dat";
const std::string AUDIT_FILE = "data/auth_audit.log";

std::string currentEmail;
std::int64_t lastActivity = 0;

std::int64_t nowSeconds() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

std::int64_t effectiveTime(std::int64_t supplied) {
    return supplied == 0 ? nowSeconds() : supplied;
}

std::string trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string normalizeEmail(const std::string& email) {
    std::string result = trim(email);

    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });

    return result;
}

bool validEmail(const std::string& email) {
    if (email.empty() || email.size() > 254 ||
        email.find_first_of(" \t\r\n|") != std::string::npos) {
        return false;
    }

    const auto at = email.find('@');

    if (at == std::string::npos || at == 0 ||
        at != email.rfind('@') || at + 1 >= email.size()) {
        return false;
    }

    const std::string domain = email.substr(at + 1);
    const auto dot = domain.rfind('.');

    return dot != std::string::npos &&
           dot > 0 &&
           dot + 1 < domain.size() &&
           domain.find("..") == std::string::npos;
}

bool validPhone(const std::string& phone) {
    int digits = 0;

    for (std::size_t i = 0; i < phone.size(); ++i) {
        const unsigned char ch = static_cast<unsigned char>(phone[i]);

        if (std::isdigit(ch)) {
            ++digits;
        } else if (ch == '+' && i == 0) {
            continue;
        } else if (ch == ' ' || ch == '-') {
            continue;
        } else {
            return false;
        }
    }

    return digits >= 7 && digits <= 15;
}

std::string toHex(const unsigned char* data, std::size_t length) {
    std::ostringstream output;
    output << std::hex << std::setfill('0');

    for (std::size_t i = 0; i < length; ++i) {
        output << std::setw(2) << static_cast<unsigned int>(data[i]);
    }

    return output.str();
}

bool fromHex(const std::string& input, std::vector<unsigned char>& output) {
    if (input.empty() || input.size() % 2 != 0) {
        return false;
    }

    output.clear();

    for (std::size_t i = 0; i < input.size(); i += 2) {
        try {
            const unsigned long value =
                std::stoul(input.substr(i, 2), nullptr, 16);

            if (value > 255) {
                return false;
            }

            output.push_back(static_cast<unsigned char>(value));
        } catch (...) {
            return false;
        }
    }

    return true;
}

std::string encodeText(const std::string& value) {
    return toHex(
        reinterpret_cast<const unsigned char*>(value.data()),
        value.size()
    );
}

bool decodeText(const std::string& value, std::string& result) {
    std::vector<unsigned char> bytes;

    if (!fromHex(value, bytes)) {
        return false;
    }

    result.assign(bytes.begin(), bytes.end());
    return true;
}

bool createPasswordHash(
    const std::string& password,
    std::string& saltHex,
    std::string& hashHex) {

    unsigned char salt[SALT_BYTES];
    unsigned char hash[HASH_BYTES];

    if (RAND_bytes(salt, SALT_BYTES) != 1) {
        return false;
    }

    if (PKCS5_PBKDF2_HMAC(
            password.data(),
            static_cast<int>(password.size()),
            salt,
            SALT_BYTES,
            PASSWORD_HASH_ITERATIONS,
            EVP_sha256(),
            HASH_BYTES,
            hash) != 1) {
        return false;
    }

    saltHex = toHex(salt, SALT_BYTES);
    hashHex = toHex(hash, HASH_BYTES);

    OPENSSL_cleanse(hash, sizeof(hash));
    return true;
}

bool verifyPassword(
    const std::string& password,
    const std::string& saltHex,
    const std::string& expectedHash) {

    std::vector<unsigned char> salt;
    std::vector<unsigned char> expected;

    if (!fromHex(saltHex, salt) ||
        !fromHex(expectedHash, expected) ||
        salt.size() != SALT_BYTES ||
        expected.size() != HASH_BYTES) {
        return false;
    }

    unsigned char calculated[HASH_BYTES];

    const bool success = PKCS5_PBKDF2_HMAC(
        password.data(),
        static_cast<int>(password.size()),
        salt.data(),
        static_cast<int>(salt.size()),
        PASSWORD_HASH_ITERATIONS,
        EVP_sha256(),
        HASH_BYTES,
        calculated
    ) == 1;

    const bool matches = success &&
        CRYPTO_memcmp(calculated, expected.data(), HASH_BYTES) == 0;

    OPENSSL_cleanse(calculated, sizeof(calculated));
    return matches;
}

std::string serializeAccount(const AuthManager::UserAccount& account) {
    std::ostringstream output;

    output << encodeText(account.name) << '|'
           << encodeText(account.email) << '|'
           << encodeText(account.phone) << '|'
           << account.passwordSalt << '|'
           << account.passwordHash << '|'
           << account.failedLoginAttempts << '|'
           << account.lockedUntil;

    return output.str();
}

bool parseInteger(const std::string& value, std::int64_t& result) {
    try {
        std::size_t used = 0;
        result = std::stoll(value, &used);

        return used == value.size();
    } catch (...) {
        return false;
    }
}

bool parseAccount(
    const std::string& record,
    AuthManager::UserAccount& account) {

    std::vector<std::string> fields;
    std::stringstream input(record);
    std::string field;

    while (std::getline(input, field, '|')) {
        fields.push_back(field);
    }

    if (fields.size() != 7) {
        return false;
    }

    std::int64_t attempts = 0;
    std::int64_t lockedUntil = 0;

    if (!decodeText(fields[0], account.name) ||
        !decodeText(fields[1], account.email) ||
        !decodeText(fields[2], account.phone) ||
        !parseInteger(fields[5], attempts) ||
        !parseInteger(fields[6], lockedUntil) ||
        attempts < 0 ||
        attempts >= AuthManager::MAX_LOGIN_ATTEMPTS ||
        lockedUntil < 0 ||
        attempts > std::numeric_limits<int>::max()) {
        return false;
    }

    std::vector<unsigned char> salt;
    std::vector<unsigned char> hash;

    if (!fromHex(fields[3], salt) ||
        !fromHex(fields[4], hash) ||
        salt.size() != SALT_BYTES ||
        hash.size() != HASH_BYTES) {
        return false;
    }

    account.passwordSalt = fields[3];
    account.passwordHash = fields[4];
    account.failedLoginAttempts = static_cast<int>(attempts);
    account.lockedUntil = lockedUntil;

    return !account.name.empty() &&
           validEmail(account.email) &&
           validPhone(account.phone);
}

std::string checksum(const std::vector<std::string>& records) {
    // FNV-1a detects accidental corruption; it is not a security signature.
    std::uint64_t hash = 14695981039346656037ULL;

    for (const std::string& record : records) {
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

bool loadAccounts(
    std::vector<AuthManager::UserAccount>& accounts,
    std::string& error) {

    accounts.clear();

    std::error_code filesystemError;
    const bool exists = std::filesystem::exists(USERS_FILE, filesystemError);

    if (filesystemError) {
        error = "Unable to inspect the account storage file.";
        return false;
    }

    if (!exists) {
        return true;
    }

    std::ifstream file(USERS_FILE);

    if (!file.is_open()) {
        error = "Unable to open account storage.";
        return false;
    }

    std::string header;

    if (!std::getline(file, header)) {
        error = "Account storage is empty or corrupted.";
        return false;
    }

    std::vector<std::string> records;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) {
            error = "Account storage contains an empty record.";
            return false;
        }

        records.push_back(line);
    }

    if (file.bad()) {
        error = "Unable to read account storage.";
        return false;
    }

    std::vector<std::string> headerFields;
    std::stringstream headerStream(header);

    while (std::getline(headerStream, line, '|')) {
        headerFields.push_back(line);
    }

    std::int64_t count = 0;

    if (headerFields.size() != 3 ||
        headerFields[0] != "RRS_USERS_V1" ||
        !parseInteger(headerFields[1], count) ||
        count < 0 ||
        static_cast<std::uint64_t>(count) != records.size() ||
        headerFields[2] != checksum(records)) {
        error = "Account storage integrity check failed.";
        return false;
    }

    for (const std::string& record : records) {
        AuthManager::UserAccount account;

        if (!parseAccount(record, account)) {
            error = "An invalid account record was found.";
            accounts.clear();
            return false;
        }

        for (const auto& existing : accounts) {
            if (existing.email == account.email) {
                error = "Duplicate email found in account storage.";
                accounts.clear();
                return false;
            }
        }

        accounts.push_back(account);
    }

    return true;
}

bool saveAccounts(
    const std::vector<AuthManager::UserAccount>& accounts,
    std::string& error) {

    std::error_code filesystemError;
    std::filesystem::create_directories("data", filesystemError);

    if (filesystemError) {
        error = "Unable to create the application data directory.";
        return false;
    }

    std::vector<std::string> records;

    for (const auto& account : accounts) {
        records.push_back(serializeAccount(account));
    }

    const std::string temporaryFile = USERS_FILE + ".tmp";
    std::ofstream file(temporaryFile, std::ios::trunc);

    if (!file.is_open()) {
        error = "Unable to create temporary account storage.";
        return false;
    }

    file << "RRS_USERS_V1|" << records.size()
         << '|' << checksum(records) << '\n';

    for (const std::string& record : records) {
        file << record << '\n';
    }

    file.close();

    if (!file) {
        std::remove(temporaryFile.c_str());
        error = "Unable to write account storage.";
        return false;
    }

    if (!replaceFileAtomically(temporaryFile, USERS_FILE)) {
        std::remove(temporaryFile.c_str());
        error = "Unable to replace account storage safely.";
        return false;
    }

    return true;
}

bool appendAuditEvent(const std::string& email, const std::string& event) {
    std::error_code filesystemError;
    std::filesystem::create_directories("data", filesystemError);

    if (filesystemError) {
        return false;
    }

    std::ofstream file(AUDIT_FILE, std::ios::app);

    if (!file.is_open()) {
        return false;
    }

    file << nowSeconds() << '|' << email << '|' << event << '\n';
    return static_cast<bool>(file);
}

} // namespace

namespace AuthManager {

bool registerPassenger(
    const std::string& name,
    const std::string& email,
    const std::string& phone,
    const std::string& password,
    std::string& errorMessage) {

    errorMessage.clear();

    const std::string cleanName = trim(name);
    const std::string cleanEmail = normalizeEmail(email);
    const std::string cleanPhone = trim(phone);

    std::string nameError;

    if (!Validation::isValidPassengerName(cleanName, nameError)) {
        errorMessage = "Invalid passenger name: " + nameError;
        return false;
    }

    if (!validEmail(cleanEmail)) {
        errorMessage = "Please enter a valid email address.";
        return false;
    }

    if (!validPhone(cleanPhone)) {
        errorMessage = "Phone number must contain 7 to 15 digits.";
        return false;
    }

    if (password.size() < 8 || password.size() > 128) {
        errorMessage = "Password must contain between 8 and 128 characters.";
        return false;
    }

    std::vector<UserAccount> accounts;

    if (!loadAccounts(accounts, errorMessage)) {
        return false;
    }

    for (const auto& account : accounts) {
        if (account.email == cleanEmail) {
            errorMessage = "An account with this email already exists.";
            return false;
        }
    }

    UserAccount account;
    account.name = cleanName;
    account.email = cleanEmail;
    account.phone = cleanPhone;

    if (!createPasswordHash(password, account.passwordSalt,
                            account.passwordHash)) {
        errorMessage = "Secure password hashing failed.";
        return false;
    }

    accounts.push_back(account);

    if (!saveAccounts(accounts, errorMessage)) {
        return false;
    }

    appendAuditEvent(cleanEmail, "REGISTRATION_SUCCESS");
    return true;
}

bool login(
    const std::string& email,
    const std::string& password,
    std::string& errorMessage,
    std::int64_t currentTime) {

    errorMessage.clear();
    logout();

    const std::string cleanEmail = normalizeEmail(email);
    const std::int64_t now = effectiveTime(currentTime);

    std::vector<UserAccount> accounts;

    if (!loadAccounts(accounts, errorMessage)) {
        return false;
    }

    auto account = std::find_if(
        accounts.begin(), accounts.end(),
        [&cleanEmail](const UserAccount& item) {
            return item.email == cleanEmail;
        }
    );

    if (account == accounts.end()) {
        errorMessage = "Invalid email or password.";
        return false;
    }

    if (account->lockedUntil > now) {
        errorMessage = "Account is locked. Please try again after the lockout period.";
        return false;
    }

    if (account->lockedUntil != 0 && account->lockedUntil <= now) {
        account->lockedUntil = 0;
        account->failedLoginAttempts = 0;
    }

    if (!verifyPassword(password, account->passwordSalt,
                        account->passwordHash)) {

        ++account->failedLoginAttempts;

        const bool accountLocked =
            account->failedLoginAttempts >= MAX_LOGIN_ATTEMPTS;

        if (accountLocked) {
            account->failedLoginAttempts = 0;
            account->lockedUntil = now + LOCKOUT_SECONDS;
        }

        if (!saveAccounts(accounts, errorMessage)) {
            return false;
        }

        if (accountLocked &&
            !appendAuditEvent(cleanEmail, "ACCOUNT_LOCKED_AFTER_FAILED_LOGINS")) {
            errorMessage =
                "Account lockout was saved, but its audit event could not be recorded.";
            return false;
        }

        errorMessage = accountLocked
            ? "Account locked for 15 minutes after repeated failed attempts."
            : "Invalid email or password.";

        return false;
    }

    account->failedLoginAttempts = 0;
    account->lockedUntil = 0;

    if (!saveAccounts(accounts, errorMessage)) {
        return false;
    }

    currentEmail = cleanEmail;
    lastActivity = now;

    appendAuditEvent(cleanEmail, "LOGIN_SUCCESS");
    return true;
}

bool isSessionActive(std::int64_t currentTime) {
    if (currentEmail.empty() || lastActivity == 0) {
        return false;
    }

    const std::int64_t now = effectiveTime(currentTime);

    if (now < lastActivity ||
        now - lastActivity >= SESSION_TIMEOUT_SECONDS) {
        logout();
        return false;
    }

    return true;
}

bool recordActivity(std::int64_t currentTime) {
    if (!isSessionActive(currentTime)) {
        return false;
    }

    lastActivity = effectiveTime(currentTime);
    return true;
}

void logout() {
    currentEmail.clear();
    lastActivity = 0;
}

std::string getCurrentUserEmail() {
    return isSessionActive() ? currentEmail : "";
}

void resetForTesting() {
    logout();
}

} // namespace AuthManager
