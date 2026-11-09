#include "AdminTrainMenu.h"
#include "AdminAuthManager.h"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

int main() {
    namespace fs = std::filesystem;

    const fs::path originalDirectory = fs::current_path();
    const fs::path testDirectory =
        fs::temp_directory_path() / "RRS_AdminTrainMenuSecurity_Test";

    std::error_code ec;
    fs::remove_all(testDirectory, ec);
    fs::create_directories(testDirectory, ec);

    if (ec) {
        std::cerr << "Unable to create test directory.\n";
        return 1;
    }

    fs::current_path(testDirectory);

    AdminAuthManager::logout();

    std::istringstream input("1\n");
    std::ostringstream output;

    AdminTrainMenu::run(input, output);

    const bool accessDenied =
        output.str().find("Access denied. Administrator login required.") !=
        std::string::npos;

    const bool menuNotDisplayed =
        output.str().find("ADMIN TRAIN MANAGEMENT") == std::string::npos;

    std::cout << (accessDenied
        ? "[PASS] Unauthenticated admin access is rejected.\n"
        : "[FAIL] Unauthenticated admin access was not rejected.\n");

    std::cout << (menuNotDisplayed
        ? "[PASS] Admin menu is not displayed without authentication.\n"
        : "[FAIL] Admin menu was displayed without authentication.\n");

    fs::current_path(originalDirectory);
    fs::remove_all(testDirectory, ec);

    return accessDenied && menuNotDisplayed ? 0 : 1;
}
