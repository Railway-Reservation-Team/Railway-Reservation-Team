
#include "AdminAuthManager.h"
#include "AdminTrainMenu.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

bool check(const std::string& name, bool condition) {
    std::cout << (condition ? "[PASS] " : "[FAIL] ")
              << name << '\n';
    return condition;
}

std::string runMenu(const std::string& commands) {
    std::istringstream input(commands);
    std::ostringstream output;
    AdminTrainMenu::run(input, output);
    return output.str();
}

std::string validTrainInput(const std::string& number) {
    return number + "\nTest Express\nBengaluru\nChennai\n"
           "10:00\n18:00\n8h\n2099-12-31\n"
           "500\n800\n1000\n400\n20\n20\n20\n20\n";
}

} // namespace

int main() {
    namespace fs = std::filesystem;

    const fs::path original = fs::current_path();
    const fs::path work =
        fs::temp_directory_path() / "RRS_AdminTrainMenu_Expanded";

    std::error_code ec;
    fs::remove_all(work, ec);
    fs::create_directories(work / "data", ec);

    if (ec) {
        std::cerr << "Unable to create test directory.\n";
        return 1;
    }

    fs::current_path(work);

    {
        std::ofstream trains("data/trains.dat");
    }

    std::string error;

    if (!AdminAuthManager::setupAdmin(
            "railadmin", "SecureRailwayPass123", error) ||
        !AdminAuthManager::login(
            "railadmin", "SecureRailwayPass123", error)) {
        std::cerr << "Admin authentication failed: "
                  << error << '\n';
        fs::current_path(original);
        fs::remove_all(work, ec);
        return 1;
    }

    int passed = 0;
    int total = 0;

    auto test = [&](const std::string& name, bool condition) {
        ++total;
        if (check(name, condition)) {
            ++passed;
        }
    };

    // 1. Empty train list
    {
        const std::string result = runMenu("2\n6\n");
        test("Empty train list",
             result.find("No trains available.") != std::string::npos);
    }

    // 2. Invalid menu choice
    {
        const std::string result = runMenu("9\n6\n");
        test("Invalid menu choice",
             result.find("Invalid choice") != std::string::npos);
    }

    // 3. Invalid fare inputs
    const std::vector<std::pair<std::string, std::string>> badFares = {
        {"text", "abc"},
        {"negative", "-10"},
        {"not finite", "nan"},
        {"trailing characters", "500abc"}
    };

    for (const auto& item : badFares) {
        const std::string commands =
            "1\n" + validTrainInput("BADFARE") .substr(0, 0);
        (void)commands;

        const std::string input =
            "1\nBADFARE\nTest Express\nBengaluru\nChennai\n"
            "10:00\n18:00\n8h\n2099-12-31\n" +
            item.second + "\n";

        const std::string result = runMenu(input);
        test("Reject " + item.first + " fare",
             result.find("Invalid input. Train was not added.") !=
                 std::string::npos);
    }

    // 4. Invalid seat count inputs
    const std::vector<std::pair<std::string, std::string>> badSeats = {
        {"text", "abc"},
        {"negative", "-5"},
        {"trailing characters", "20abc"}
    };

    for (const auto& item : badSeats) {
        const std::string input =
            "1\nBADSEAT\nTest Express\nBengaluru\nChennai\n"
            "10:00\n18:00\n8h\n2099-12-31\n"
            "500\n800\n1000\n400\n" +
            item.second + "\n";

        const std::string result = runMenu(input);
        test("Reject " + item.first + " seat count",
             result.find("Invalid input. Train was not added.") !=
                 std::string::npos);
    }

    // 5. Interrupted input while entering train details
    {
        const std::string result =
            runMenu("1\nPARTIAL\nTest Express\n");

        test("Handle interrupted train input",
             result.find("Invalid input. Train was not added.") !=
                 std::string::npos);
    }

    // 6. Missing train update
    {
        const std::string result =
            runMenu("3\nNOTFOUND\n" + validTrainInput("NOTFOUND") + "6\n");

        test("Update nonexistent train",
             result.find("Train updated successfully.") ==
                 std::string::npos &&
             result.find("Error:") != std::string::npos);
    }

    // 7. Missing train delete
    {
        const std::string result = runMenu("4\nNOTFOUND\n6\n");

        test("Delete nonexistent train",
             result.find("Train deleted successfully.") ==
                 std::string::npos &&
             result.find("Error:") != std::string::npos);
    }

    // 8. Missing train deactivate
    {
        const std::string result = runMenu("5\nNOTFOUND\n6\n");

        test("Deactivate nonexistent train",
             result.find("Train deactivated successfully.") ==
                 std::string::npos &&
             result.find("Error:") != std::string::npos);
    }

    // 9. Normal add, view, update, deactivate and delete flow
    {
        const std::string commands =
            "1\n" + validTrainInput("77777") +
            "2\n"
            "3\n77777\n" + validTrainInput("77777") +
            "2\n"
            "5\n77777\n"
            "2\n"
            "4\n77777\n"
            "2\n"
            "6\n";

        const std::string result = runMenu(commands);
"1\n" + validTrainInput("BADFARE").substr(0, 0);
(void)commands;

        test("Normal add operation",
             result.find("Train added successfully.") !=
                 std::string::npos);

        test("Normal view operation",
             result.find("ALL TRAINS") != std::string::npos);

        test("Normal update operation",
             result.find("Train updated successfully.") !=
                 std::string::npos);

        test("Normal deactivate operation",
             result.find("Train deactivated successfully.") !=
                 std::string::npos);

        test("Deactivated status displayed",
             result.find("Status: Deactivated") !=
                 std::string::npos);

        test("Normal delete operation",
             result.find("Train deleted successfully.") !=
                 std::string::npos);
    }

    AdminAuthManager::logout();
    fs::current_path(original);
    fs::remove_all(work, ec);

    std::cout << "\nAdminTrainMenu tests: "
              << passed << "/" << total << " passed.\n";

    return passed == total ? 0 : 1;
}
