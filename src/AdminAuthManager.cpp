#include "AdminAuthManager.h"
#include "AtomicFileReplace.h"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

namespace {

constexpr int ITERATIONS = 600000;
constexpr int SALT_SIZE = 16;
constexpr int HASH_SIZE = 32;
constexpr int MAX_ATTEMPTS = 5;
constexpr long long LOCKOUT_SECONDS = 15 * 60;

const std::string ADMIN_FILE = "data/admin.dat";

bool authenticated = false;

long long nowSeconds() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

std::string normalizeUsername(const std::string& username) {
    std::string result = username;

    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });

    return result;
}

std::string toHex(const unsigned char* data, int length) {
    std::ostringstream output;
    output << std::hex << std::setfill('0');

    for (int i = 0; i < length; ++i) {
        output << std::setw(2)
               << static_cast<unsigned int>(data[i]);
    }

    return output.str();
}

bool fromHex(const std::string& input,
             std::vector<unsigned char>& output) {
    if (input.empty() || input.size() % 2 != 0) {
        return false;
    }

    output.clear();

    for (std::size_t i = 0; i < input.size(); i += 2) {
        try {
            unsigned long value =
                std::stoul(input.substr(i, 2), nullptr, 16);

            if (value > 255) return false;

            output.push_back(static_cast<unsigned char>(value));
        } catch (...) {
            return false;
        }
    }

    return true;
}

bool createHash(const std::string& password,
                std::string& saltHex,
                std::string& hashHex) {
    unsigned char salt[SALT_SIZE];
    unsigned char hash[HASH_SIZE];

    if (RAND_bytes(salt, SALT_SIZE) != 1) {
        return false;
    }

    if (PKCS5_PBKDF2_HMAC(
            password.data(),
            static_cast<int>(password.size()),
            salt,
            SALT_SIZE,
            ITERATIONS,
            EVP_sha256(),
            HASH_SIZE,
            hash) != 1) {
        OPENSSL_cleanse(hash, sizeof(hash));
        return false;
    }

    saltHex = toHex(salt, SALT_SIZE);
    hashHex = toHex(hash, HASH_SIZE);

    OPENSSL_cleanse(hash, sizeof(hash));
    return true;
}

bool verifyPassword(const std::string& password,
                    const std::string& saltHex,
                    const std::string& expectedHash) {
    std::vector<unsigned char> salt;
    std::vector<unsigned char> expected;

    if (!fromHex(saltHex, salt) ||
        !fromHex(expectedHash, expected) ||
        salt.size() != SALT_SIZE ||
        expected.size() != HASH_SIZE) {
        return false;
    }

    unsigned char calculated[HASH_SIZE];

    bool success = PKCS5_PBKDF2_HMAC(
        password.data(),
        static_cast<int>(password.size()),
        salt.data(),
        SALT_SIZE,
        ITERATIONS,
        EVP_sha256(),
        HASH_SIZE,
        calculated
    ) == 1;

    bool matches = success &&
        CRYPTO_memcmp(calculated, expected.data(), HASH_SIZE) == 0;

    OPENSSL_cleanse(calculated, sizeof(calculated));
    return matches;
}

struct AdminRecord {
    std::string username;
    std::string salt;
    std::string hash;
    int failedAttempts = 0;
    long long lockedUntil = 0;
};

bool loadRecord(AdminRecord& record) {
    std::ifstream file(ADMIN_FILE);

    if (!file.is_open()) return false;

    std::string username;
    std::string salt;
    std::string hash;
    long long lockedUntil;
    int attempts;

    if (!(file >> username >> salt >> hash >> attempts >> lockedUntil)) {
        return false;
    }

    if (attempts < 0 || attempts >= MAX_ATTEMPTS ||
        lockedUntil < 0) {
        return false;
    }

    std::vector<unsigned char> saltBytes;
    std::vector<unsigned char> hashBytes;

    if (!fromHex(salt, saltBytes) ||
        !fromHex(hash, hashBytes) ||
        saltBytes.size() != SALT_SIZE ||
        hashBytes.size() != HASH_SIZE) {
        return false;
    }

    record.username = username;
    record.salt = salt;
    record.hash = hash;
    record.failedAttempts = attempts;
    record.lockedUntil = lockedUntil;

    return true;
}

bool saveRecord(const AdminRecord& record) {
    std::error_code filesystemError;
    std::filesystem::create_directories("data", filesystemError);

    if (filesystemError) return false;

    const std::string temporaryFile = ADMIN_FILE + ".tmp";

    std::ofstream file(temporaryFile, std::ios::trunc);

    if (!file.is_open()) return false;

    file << record.username << '\n'
         << record.salt << '\n'
         << record.hash << '\n'
         << record.failedAttempts << '\n'
         << record.lockedUntil << '\n';

    file.close();

    if (!file) {
        std::remove(temporaryFile.c_str());
        return false;
    }

    if (!replaceFileAtomically(temporaryFile, ADMIN_FILE)) {
        std::remove(temporaryFile.c_str());
        return false;
    }

    return true;
}

} // namespace

namespace AdminAuthManager {

bool hasAdminAccount(std::string& errorMessage) {
    errorMessage.clear();

    std::error_code filesystemError;
    bool exists = std::filesystem::exists(ADMIN_FILE, filesystemError);

    if (filesystemError) {
        errorMessage = "Unable to inspect administrator storage.";
        return false;
    }

    if (!exists) return false;

    AdminRecord record;

    if (!loadRecord(record)) {
        errorMessage = "Administrator storage is invalid or corrupted.";
        return false;
    }

    return true;
}

bool setupAdmin(const std::string& username,
                const std::string& password,
                std::string& errorMessage) {
    errorMessage.clear();

    if (username.size() < 3 || username.size() > 32 ||
        username.find_first_of(" \t\r\n") != std::string::npos) {
        errorMessage =
            "Username must be 3-32 characters without whitespace.";
        return false;
    }

    if (password.size() < 12 || password.size() > 128) {
        errorMessage =
            "Administrator password must contain 12-128 characters.";
        return false;
    }

    std::error_code filesystemError;

    if (std::filesystem::exists(ADMIN_FILE, filesystemError) ||
        filesystemError) {
        errorMessage =
            "Administrator account already exists or storage cannot be checked.";
        return false;
    }

    AdminRecord record;
    record.username = normalizeUsername(username);

    if (!createHash(password, record.salt, record.hash)) {
        errorMessage = "Secure password hashing failed.";
        return false;
    }

    if (!saveRecord(record)) {
        errorMessage = "Unable to save administrator credentials.";
        return false;
    }

    return true;
}

bool login(const std::string& username,
           const std::string& password,
           std::string& errorMessage) {
    errorMessage.clear();
    logout();

    AdminRecord record;

    if (!loadRecord(record)) {
        errorMessage = "Administrator account not found or storage is invalid.";
        return false;
    }

    const long long now = nowSeconds();

    if (record.lockedUntil > now) {
        errorMessage = "Administrator account is locked for 15 minutes.";
        return false;
    }

    if (record.lockedUntil != 0 && record.lockedUntil <= now) {
        record.lockedUntil = 0;
        record.failedAttempts = 0;
    }

    const bool validUsername =
        normalizeUsername(username) == record.username;

    const bool validPassword =
        verifyPassword(password, record.salt, record.hash);

    if (!validUsername || !validPassword) {
        ++record.failedAttempts;

        if (record.failedAttempts >= MAX_ATTEMPTS) {
            record.failedAttempts = 0;
            record.lockedUntil = now + LOCKOUT_SECONDS;
        }

        if (!saveRecord(record)) {
            errorMessage = "Unable to save administrator login state.";
            return false;
        }

        errorMessage = record.lockedUntil > now
            ? "Administrator account locked after repeated failed attempts."
            : "Invalid administrator username or password.";

        return false;
    }

    record.failedAttempts = 0;
    record.lockedUntil = 0;

    if (!saveRecord(record)) {
        errorMessage = "Unable to update administrator account.";
        return false;
    }

    authenticated = true;
    return true;
}

bool isAuthenticated() {
    return authenticated;
}

void logout() {
    authenticated = false;
}

void resetForTesting() {
    logout();
}

} // namespace AdminAuthManager
