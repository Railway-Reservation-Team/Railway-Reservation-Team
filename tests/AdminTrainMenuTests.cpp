#include "AdminAuthManager.h"
#include "AdminTrainMenu.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

const std::string TRAIN_FILE = "data/trains.dat";

bool contains(const std::string& text, const std::string& value) {
    return text.find(value) != std::string::npos;
}

void prepareTrainFile(const std::string& content = "") {
    fs::create_directories("data");
    std::ofstream file(TRAIN_FILE, std::ios::trunc);
    file << content;
}

std::string validTrainInput(
    const std::string& number = "77777",
    const std::string& name = "Menu Test Express") {

    return number + "\n" +
           name + "\n" +
           "Bengaluru\n" +
           "Chennai\n" +
           "10:00\n" +
           "18:00\n" +
           "8h\n" +
           "2099-12-31\n" +
           "500\n800\n1000\n400\n" +
           "20\n20\n20\n20\n";
}

std::string runMenu(const std::string& menuInput) {
    std::istringstream input(menuInput);
    std::ostringstream output;

    AdminTrainMenu::run(input, output);

    return output.str();
}

bool testInvalidChoice() {
    prepareTrainFile();

    std::string result = runMenu("9\n6\n");

    return contains(result, "Invalid choice");
}

bool testSuccessfulAddAndDuplicate() {
    prepareTrainFile();

    std::string input =
        "1\n" + validTrainInput() +
        "1\n" + validTrainInput() +
        "6\n";

    std::string result = runMenu(input);

    return contains(result, "Train added successfully") &&
           contains(result, "Train number already exists.");
}

bool testViewEmptyTrains() {
    prepareTrainFile();

    std::string result = runMenu("2\n6\n");

    return contains(result, "No trains available.");
}

bool testViewMalformedTrainFile() {
    prepareTrainFile("invalid-train-record\n");

    std::string result = runMenu("2\n6\n");

    return contains(result, "Invalid train record found.");
}

bool testInvalidFareInputs() {
    const std::vector<std::string> invalidValues = {
        "abc",
        "-100",
        "500abc",
        "",
        "nan",
        "inf"
    };

    for (const std::string& value : invalidValues) {
        prepareTrainFile();

        std::string input =
            std::string("1\n") +
            "77777\n"
            "Test Express\n"
            "Bengaluru\n"
            "Chennai\n"
            "10:00\n"
            "18:00\n"
            "8h\n"
            "2099-12-31\n" +
            value + "\n"
            "6\n";

        std::string result = runMenu(input);

        if (!contains(result, "Invalid input. Train was not added.")) {
            return false;
        }
    }

    return true;
}

bool testInvalidSeatInputs() {
    const std::vector<std::string> invalidValues = {
        "abc",
        "-5",
        "20abc",
        ""
    };

    for (const std::string& value : invalidValues) {
        prepareTrainFile();

        std::string input =
            std::string("1\n") +
            "77777\n"
            "Test Express\n"
            "Bengaluru\n"
            "Chennai\n"
            "10:00\n"
            "18:00\n"
            "8h\n"
            "2099-12-31\n"
            "500\n800\n1000\n400\n" +
            value + "\n"
            "6\n";

        std::string result = runMenu(input);

        if (!contains(result, "Invalid input. Train was not added.")) {
            return false;
        }
    }

    return true;
}

bool testIncompleteTrainInput() {
    prepareTrainFile();

    std::string input =
        "1\n"
        "77777\n"
        "Test Express\n"
        "Bengaluru\n";

    std::string result = runMenu(input);

    return contains(result, "Invalid input. Train was not added.");
}

bool testUpdateMissingTrain() {
    prepareTrainFile();

    std::string input =
        "3\n99999\n" +
        validTrainInput("99999", "Missing Express") +
        "6\n";

    std::string result = runMenu(input);

    return contains(result, "Train not found.");
}

bool testInvalidUpdateInput() {
    prepareTrainFile();

    std::string input =
        "3\n77777\n"
        "77777\n"
        "Test Express\n"
        "Bengaluru\n"
        "Chennai\n"
        "10:00\n"
        "18:00\n"
        "8h\n"
        "2099-12-31\n"
        "-500\n";

    std::string result = runMenu(input);

    return contains(result, "Invalid input. Train was not updated.");
}

bool testDeleteMissingTrain() {
    prepareTrainFile();

    std::string result = runMenu("4\n99999\n6\n");

    return contains(result, "Train not found.");
}

bool testDeactivateMissingTrain() {
    prepareTrainFile();

    std::string result = runMenu("5\n99999\n6\n");

    return contains(result, "Train not found.");
}

bool testMenuEOF() {
    prepareTrainFile();

    std::string result = runMenu("");

    return contains(result, "ADMIN TRAIN MANAGEMENT");
}

bool testUnauthenticatedAccess() {
    AdminAuthManager::logout();

    std::string result = runMenu("1\n");

    return contains(result, "Access denied. Administrator login required.") &&
           !contains(result, "ADMIN TRAIN MANAGEMENT");
}

bool testFullTrainManagementFlow() {
    prepareTrainFile();

    std::string input =
        "1\n" + validTrainInput() +
        "2\n"
        "3\n77777\n" +
        validTrainInput("77777", "Updated Express") +
        "2\n"
        "5\n77777\n"
        "2\n"
        "4\n77777\n"
        "2\n"
        "6\n";

    std::string result = runMenu(input);

    return contains(result, "Train added successfully") &&
           contains(result, "Train updated successfully") &&
           contains(result, "Train deactivated successfully") &&
           contains(result, "Status: Deactivated") &&
           contains(result, "Train deleted successfully") &&
           contains(result, "ALL TRAINS");
}

int main() {
    const fs::path original = fs::current_path();
    const fs::path work =
        fs::temp_directory_path() / "RRS_AdminTrainMenu_Flow";

    std::error_code ec;

    fs::remove_all(work, ec);
    fs::create_directories(work / "data", ec);

    if (ec) {
        std::cerr << "Unable to create admin menu test directory.\n";
        return 1;
    }

    fs::current_path(work);

    std::string error;

    if (!AdminAuthManager::setupAdmin(
            "railadmin", "SecureRailwayPass123", error) ||
        !AdminAuthManager::login(
            "railadmin", "SecureRailwayPass123", error)) {

        std::cerr << "Unable to authenticate test administrator: "
                  << error << '\n';

        fs::current_path(original);
        fs::remove_all(work, ec);
        return 1;
    }

    int failures = 0;

    auto check = [&](const std::string& name, bool passed) {
        std::cout << (passed ? "[PASS] " : "[FAIL] ")
                  << name << '\n';

        if (!passed) {
            ++failures;
        }
    };

    check("Invalid menu choice", testInvalidChoice());
    check("Add train and reject duplicate", testSuccessfulAddAndDuplicate());
    check("View empty train list", testViewEmptyTrains());
    check("Reject malformed train file", testViewMalformedTrainFile());
    check("Reject invalid fare inputs", testInvalidFareInputs());
    check("Reject invalid seat inputs", testInvalidSeatInputs());
    check("Reject incomplete train input", testIncompleteTrainInput());
    check("Reject update for missing train", testUpdateMissingTrain());
    check("Reject invalid update input", testInvalidUpdateInput());
    check("Reject deletion of missing train", testDeleteMissingTrain());
    check("Reject deactivation of missing train", testDeactivateMissingTrain());
    check("Handle menu EOF", testMenuEOF());
    check("Reject unauthenticated access", testUnauthenticatedAccess());

    AdminAuthManager::login(
        "railadmin", "SecureRailwayPass123", error);

    check("Complete train management workflow",
          testFullTrainManagementFlow());

    AdminAuthManager::logout();

    fs::current_path(original);
    fs::remove_all(work, ec);

    if (failures > 0) {
        std::cerr << "\n" << failures
                  << " AdminTrainMenu test(s) failed.\n";
        return 1;
    }

    std::cout << "\nAll AdminTrainMenu tests passed.\n";
    return 0;
}
