#include "TrainData.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {
int passed = 0;
int failed = 0;

void check(const std::string& name, bool condition) {
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
    const fs::path root = fs::temp_directory_path() / "rrs_train_data_file_tests";
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root, ec);
    if (ec) {
        std::cerr << "Could not create test directory: " << ec.message() << '\n';
        return 1;
    }

    const fs::path missing = root / "missing.dat";
    std::string error;
    auto trains = loadTrains(missing.string(), "2026-12-15", error);
    check("Missing train file returns no records and an error", trains.empty() && !error.empty());

    const fs::path empty = root / "empty.dat";
    { std::ofstream file(empty); }
    error.clear();
    trains = loadTrains(empty.string(), "2026-12-15", error);
    check("Empty train file returns no records and an error", trains.empty() && error.find("empty") != std::string::npos);

    const fs::path malformed = root / "malformed.dat";
    {
        std::ofstream file(malformed);
        file << "not|a|valid|train|record\n";
    }
    error.clear();
    trains = loadTrains(malformed.string(), "2026-12-15", error);
    check("Malformed train records are rejected", trains.empty());

    const fs::path mixed = root / "mixed.dat";
    {
        std::ofstream file(mixed);
        file << "not|a|valid|train|record\n";
        file << "TEST101|Test Express|Bengaluru|Chennai|08:00|14:00|6h|1000|1500|2000|800|8|10|10|10|2026-12-15\n";
        file << "TEST102|Inactive Express|Bengaluru|Chennai|09:00|15:00|6h|1000|1500|2000|800|8|10|10|10|2026-12-15|0\n";
    }
    error.clear();
    trains = loadTrains(mixed.string(), "2026-12-15", error);
    check("Legacy 16-field records default to active", trains.size() == 1 && trains[0].trainNumber == "TEST101" && trains[0].active);
    Train inactive;
    check("17-field deactivated record parses with inactive state",
          parseTrainRecord("TEST102|Inactive Express|Bengaluru|Chennai|09:00|15:00|6h|1000|1500|2000|800|8|10|10|10|2026-12-15|0", inactive) && !inactive.active);
    check("Journey date filter excludes other dates", loadTrains(mixed.string(), "2026-12-16", error).empty());

    fs::remove_all(root, ec);
    std::cout << "\nPassed: " << passed << "\nFailed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}
