#include "AdminAuthManager.h"
#include "AdminTrainMenu.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

int main() {
    namespace fs = std::filesystem;
    const fs::path original = fs::current_path();
    const fs::path work = fs::temp_directory_path() / "RRS_AdminTrainMenu_Flow";
    std::error_code ec;
    fs::remove_all(work, ec);
    fs::create_directories(work / "data", ec);
    if (ec) {
        std::cerr << "Unable to create admin menu test directory.\n";
        return 1;
    }
    fs::current_path(work);

    {
        std::ofstream trains("data/trains.dat");
    }

    std::string error;
    if (!AdminAuthManager::setupAdmin("railadmin", "SecureRailwayPass123", error) ||
        !AdminAuthManager::login("railadmin", "SecureRailwayPass123", error)) {
        std::cerr << "Unable to authenticate test administrator: " << error << '\n';
        fs::current_path(original);
        fs::remove_all(work, ec);
        return 1;
    }

    const std::string menuInput =
        "9\n"
        "1\n77777\nMenu Test Express\nBengaluru\nChennai\n"
        "10:00\n18:00\n8h\n2099-12-31\n"
        "500\n800\n1000\n400\n20\n20\n20\n20\n"
        "2\n"
        "3\n77777\n77777\nUpdated Express\nBengaluru\nMysuru\n"
        "11:00\n16:00\n5h\n2099-12-31\n"
        "600\n900\n1100\n450\n18\n18\n18\n18\n"
        "2\n"
        "5\n77777\n"
        "2\n"
        "4\n77777\n"
        "2\n"
        "6\n";

    std::istringstream input(menuInput);
    std::ostringstream output;
    AdminTrainMenu::run(input, output);
    const std::string result = output.str();

    const bool invalidChoice = result.find("Invalid choice") != std::string::npos;
    const bool added = result.find("Train added successfully") != std::string::npos;
    const bool updated = result.find("Train updated successfully") != std::string::npos;
    const bool deactivated = result.find("Train deactivated successfully") != std::string::npos;
    const bool deleted = result.find("Train deleted successfully") != std::string::npos;
    const bool viewed = result.find("ALL TRAINS") != std::string::npos;

    AdminAuthManager::logout();
    fs::current_path(original);
    fs::remove_all(work, ec);

    if (!(invalidChoice && added && updated && deactivated && deleted && viewed)) {
        std::cerr << "Admin menu flow failed. Output:\n" << result << '\n';
        return 1;
    }

    std::cout << "Administrator train menu flow passed.\n";
    std::cout << "Invalid choice, add, view, update, deactivate, delete and back paths: PASS\n";
    return 0;
}
