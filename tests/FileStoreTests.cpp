#include "FileStore.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

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

bool validRecord(const std::string& record) {
    return !record.empty() && record.find('|') != std::string::npos;
}
}

int main() {
    const std::filesystem::path testDirectory =
        std::filesystem::temp_directory_path() / "rrs_filestore_tests";
    std::error_code filesystemError;
    std::filesystem::remove_all(testDirectory, filesystemError);
    std::filesystem::create_directories(testDirectory, filesystemError);

    const std::string dataFile = (testDirectory / "records.dat").string();
    const std::string missingFile = (testDirectory / "missing.dat").string();
    const std::string invalidPath = (testDirectory / "absent" / "records.dat").string();

    std::string error;
    std::vector<std::string> records;

    check("Missing file loads as an empty store",
          FileStore::loadLines(missingFile, records, error) && records.empty());

    check("Empty file loads successfully",
          FileStore::saveLines(dataFile, {}, validRecord, error) &&
          FileStore::loadLines(dataFile, records, error) && records.empty());

    check("First validated record can be appended",
          FileStore::appendLine(dataFile, "12627|Karnataka Express",
                                validRecord, error));

    check("Appending preserves the existing record",
          FileStore::appendLine(dataFile, "12007|Shatabdi Express",
                                validRecord, error) &&
          FileStore::loadLines(dataFile, records, error) &&
          records.size() == 2 &&
          records[0] == "12627|Karnataka Express" &&
          records[1] == "12007|Shatabdi Express");

    check("Invalid record is rejected",
          !FileStore::appendLine(dataFile, "invalid", validRecord, error));

    check("Rejected record does not change saved records",
          FileStore::loadLines(dataFile, records, error) &&
          records.size() == 2 &&
          records[0] == "12627|Karnataka Express");

    check("Updating one record retains the others",
          FileStore::updateLine(dataFile, 0, "12627|Karnataka Express Updated",
                                validRecord, error) &&
          FileStore::loadLines(dataFile, records, error) &&
          records.size() == 2 &&
          records[0] == "12627|Karnataka Express Updated" &&
          records[1] == "12007|Shatabdi Express");

    check("Out-of-range update is rejected",
          !FileStore::updateLine(dataFile, 10, "12627|Replacement",
                                 validRecord, error));

    check("Invalid record set is not written",
          !FileStore::saveLines(dataFile,
                                {"12627|Valid", "bad"},
                                validRecord, error) &&
          FileStore::loadLines(dataFile, records, error) &&
          records.size() == 2 &&
          records[0] == "12627|Karnataka Express Updated");

    check("Write access/path errors are reported",
          !FileStore::saveLines(invalidPath, {"12627|Train"},
                                validRecord, error) && !error.empty());

    check("Saving without a validator is rejected",
          !FileStore::saveLines(dataFile, {"12627|Train"},
                                FileStore::RecordValidator{}, error));

    std::cout << "\n========== FILE STORE TEST SUMMARY ==========\n"
              << "Passed: " << passed << '\n'
              << "Failed: " << failed << '\n'
              << "Total:  " << passed + failed << '\n';

    std::filesystem::remove_all(testDirectory, filesystemError);
    return failed == 0 ? 0 : 1;
}
