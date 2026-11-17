
#include "../src/TrainData.h"

#include <iostream>
#include <string>
#include <vector>

namespace {
int passed = 0;
int failed = 0;

void check(const std::string& description, bool condition) {
    if (condition) {
        std::cout << "[PASS] " << description << '\n';
        ++passed;
    } else {
        std::cout << "[FAIL] " << description << '\n';
        ++failed;
    }
}

std::vector<std::string> validFields() {
    return {
        "TEST101", "Test Express", "Bengaluru", "Chennai",
        "08:00", "14:00", "6h",
        "1000", "1500", "2000", "800",
        "10", "10", "10", "10",
        "2026-12-15"
    };
}

std::string makeRecord(const std::vector<std::string>& fields) {
    std::string result;
    for (std::size_t i = 0; i < fields.size(); ++i) {
        if (i > 0) result += "|";
        result += fields[i];
    }
    return result;
}
}

int main() {
    std::cout << "\n=== EXTRA TRAIN DATA COVERAGE TESTS ===\n";

    // Date validation tests.
    check("Reject short date", !isValidDate("2026-12"));
    check("Reject incorrect first separator", !isValidDate("2026/12-15"));
    check("Reject incorrect second separator", !isValidDate("2026-12/15"));
    check("Reject non-numeric date", !isValidDate("202A-12-15"));
    check("Reject year zero", !isValidDate("0000-12-15"));
    check("Reject month zero", !isValidDate("2026-00-15"));
    check("Reject month above twelve", !isValidDate("2026-13-15"));
    check("Reject day zero", !isValidDate("2026-12-00"));
    check("Reject day above month limit", !isValidDate("2026-04-31"));
    check("Accept leap year 2000", isValidDate("2000-02-29"));
    check("Reject leap day in non-leap year 1900",
          !isValidDate("1900-02-29"));
    check("Accept leap year 2024", isValidDate("2024-02-29"));
    check("Accept normal date", isValidDate("2026-12-15"));

    // Record field-count tests.
    Train train{};
    auto fields = validFields();

    check("Reject record with too few fields",
          !parseTrainRecord("TEST101|Test Express", train));

    fields.push_back("1");
    fields.push_back("EXTRA");
    check("Reject record with too many fields",
          !parseTrainRecord(makeRecord(fields), train));

    // Numeric conversion failures.
    fields = validFields();
    fields[7] = "invalid";
    check("Reject invalid fare conversion",
          !parseTrainRecord(makeRecord(fields), train));

    fields = validFields();
    fields[11] = "invalid";
    check("Reject invalid seat conversion",
          !parseTrainRecord(makeRecord(fields), train));

    // Empty required fields.
    const std::vector<int> requiredFields = {0, 1, 2, 3};
    const std::vector<std::string> fieldNames = {
        "train number", "train name", "source", "destination"
    };

    for (std::size_t i = 0; i < requiredFields.size(); ++i) {
        fields = validFields();
        fields[requiredFields[i]] = "";
        check("Reject empty " + fieldNames[i],
              !parseTrainRecord(makeRecord(fields), train));
    }

    // Invalid journey dates in records.
    fields = validFields();
    fields[15] = "2026-02-30";
    check("Reject impossible journey date",
          !parseTrainRecord(makeRecord(fields), train));

    fields = validFields();
    fields[15] = "invalid";
    check("Reject malformed journey date",
          !parseTrainRecord(makeRecord(fields), train));

    // Negative fare validation.
    for (int i = 7; i <= 10; ++i) {
        fields = validFields();
        fields[i] = "-1";
        check("Reject negative fare field " + std::to_string(i - 7 + 1),
              !parseTrainRecord(makeRecord(fields), train));
    }

    // Negative seat validation.
    for (int i = 11; i <= 14; ++i) {
        fields = validFields();
        fields[i] = "-1";
        check("Reject negative seat field " + std::to_string(i - 10),
              !parseTrainRecord(makeRecord(fields), train));
    }

    // Valid boundary values.
    fields = validFields();
    fields[7] = "0";
    fields[11] = "0";
    check("Accept zero fare and zero seats",
          parseTrainRecord(makeRecord(fields), train));

    fields = validFields();
    fields.push_back("1");
    check("Accept active flag one",
          parseTrainRecord(makeRecord(fields), train) && train.active);

    std::cout << "\nPassed: " << passed
              << "\nFailed: " << failed << '\n';

    return failed == 0 ? 0 : 1;
}
