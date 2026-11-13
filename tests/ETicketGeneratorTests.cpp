#include "ETicketGenerator.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <chrono>

int passed = 0;
int failed = 0;

void test(const std::string& name, bool condition) {
    if (condition) {
        std::cout << "[PASS] " << name << '\n';
        ++passed;
    } else {
        std::cout << "[FAIL] " << name << '\n';
        ++failed;
    }
}

void writeTrain(const std::string& trainNumber) {
    std::ofstream file("data/trains.dat");
    file << trainNumber
         << "|Karnataka Express|Bengaluru|New Delhi"
         << "|20:00|10:30|14h 30m"
         << "|1200|1800|2400|900"
         << "|100|50|30|40|2026-11-01\n";
}

void writeBooking(
    const std::string& pnr,
    const std::string& passengers) {

    std::ofstream file("data/bookings.dat");

    file << pnr
         << "|12627|2026-11-01|Sleeper|"
         << passengers
         << "|2|2400|Confirmed\n";
}

int main() {
    namespace fs = std::filesystem;

    fs::path originalDirectory = fs::current_path();

    fs::path testDirectory =
        fs::temp_directory_path() /
        ("rrs_eticket_tests_" +
         std::to_string(
             std::chrono::steady_clock::now()
                 .time_since_epoch().count()));

    fs::create_directories(testDirectory / "data");
    fs::current_path(testDirectory);

    writeTrain("12627");

    std::string ticketPath;
    std::string errorMessage;

    // Test 1: Valid PNR
    writeBooking(
        "1234567890",
        "Dinesh,20,Male;Abhay,20,Male"
    );

    bool generated = ETicketGenerator::generateETicket(
        "1234567890", ticketPath, errorMessage
    );

    test("Valid PNR generates e-ticket",
         generated && fs::exists(ticketPath));

    // Test 2: Verify ticket contents
    std::ifstream ticket(ticketPath);
    std::stringstream contents;
    contents << ticket.rdbuf();

    std::string text = contents.str();

    test("Ticket contains correct booking details",
         text.find("1234567890") != std::string::npos &&
         text.find("Karnataka Express") != std::string::npos &&
         text.find("Dinesh") != std::string::npos &&
         text.find("Abhay") != std::string::npos &&
         text.find("Rs. 2400.00") != std::string::npos &&
         text.find("Passenger Count  : 2") != std::string::npos);

    writeBooking(
        "1234567893",
        "Anne-Marie,20,Female;O'Neil,30,Other"
    );
    ticketPath.clear();
    errorMessage.clear();

    const bool validNamesResult = ETicketGenerator::generateETicket(
        "1234567893", ticketPath, errorMessage
    );

    test("Valid passenger names are accepted",
         validNamesResult && fs::exists(ticketPath));


    std::ostringstream displayedOutput;
    std::streambuf* originalOutput = std::cout.rdbuf(
        displayedOutput.rdbuf()
    );

    bool displayed = ETicketGenerator::displayETicket(
        "1234567890", errorMessage
    );

    std::cout.rdbuf(originalOutput);

    test("Existing e-ticket displayed successfully",
         displayed &&
         displayedOutput.str().find("1234567890") != std::string::npos &&
         displayedOutput.str().find("Karnataka Express") != std::string::npos);

    bool invalidDisplay = ETicketGenerator::displayETicket(
        "INVALID", errorMessage
    );

    test("Invalid PNR rejected during display",
         !invalidDisplay &&
         errorMessage == "Invalid PNR format.");

    bool missingDisplay = ETicketGenerator::displayETicket(
        "9999999999", errorMessage
    );

    test("Missing e-ticket handled correctly",
         !missingDisplay &&
         errorMessage.find("E-ticket not found") != std::string::npos);

    // Test 3: Invalid PNR
    ticketPath.clear();
    errorMessage.clear();

    bool invalidResult = ETicketGenerator::generateETicket(
        "INVALID000", ticketPath, errorMessage
    );

    test("Invalid PNR is rejected",
         !invalidResult &&
         errorMessage.find("Booking not found") != std::string::npos);

    // Test 4: Invalid passenger data
    writeBooking("1234567891", "InvalidPassenger");

    ticketPath.clear();
    errorMessage.clear();

    bool malformedResult = ETicketGenerator::generateETicket(
        "1234567891", ticketPath, errorMessage
    );

    test("Invalid passenger data is rejected",
         !malformedResult &&
         errorMessage.find("Invalid passenger details") != std::string::npos &&
         !fs::exists("tickets/ETicket_1234567891.txt"));

    struct InvalidPassengers {
        const char* label;
        const char* records;
        const char* error;
    };
    const InvalidPassengers cases[] = {
        {"Fewer passengers than booked", "Dinesh,20,Male", "Passenger count mismatch"},
        {"More passengers than booked", "Dinesh,20,Male;Abhay,20,Male;Pooja,21,Female", "Passenger count mismatch"},
        {"Zero passenger records", "", "Passenger count mismatch"},
        {"Empty passenger entry", "Dinesh,20,Male;;Abhay,20,Male", "Invalid passenger details"},
        {"Trailing empty entry", "Dinesh,20,Male;", "Invalid passenger details"},
        {"Empty name", "Dinesh,20,Male;,20,Male", "Invalid passenger details"},
        {"Punctuation-only name", "---,20,Male;Abhay,20,Male", "Invalid passenger details"},
        {"Special-character-only name", "!!!,20,Male;Abhay,20,Male", "Invalid passenger details"},
        {"Empty age", "Dinesh,20,Male;Abhay,,Male", "Invalid passenger details"},
        {"Invalid age", "Dinesh,20,Male;Abhay,20x,Male", "Invalid passenger details"},
        {"Out of range age", "Dinesh,20,Male;Abhay,121,Male", "Invalid passenger details"},
        {"Empty gender", "Dinesh,20,Male;Abhay,20,", "Invalid passenger details"},
        {"Invalid gender", "Dinesh,20,Male;Abhay,20,Unknown", "Invalid passenger details"},
        {"Malformed second record", "Dinesh,20,Male;InvalidPassenger", "Invalid passenger details"}
    };
    for (const auto& invalid : cases) {
        writeBooking("1234567892", invalid.records);
        ticketPath = "stale output path";
        errorMessage.clear();
        const bool result = ETicketGenerator::generateETicket(
            "1234567892", ticketPath, errorMessage);
        test(invalid.label,
             !result && ticketPath.empty() &&
             errorMessage.find(invalid.error) != std::string::npos &&
             !fs::exists("tickets/ETicket_1234567892.txt"));
    }

    // A failed regeneration must not truncate an existing complete ticket.
    writeBooking("1234567890", "Dinesh,20,Male");
    const bool regenerated = ETicketGenerator::generateETicket(
        "1234567890", ticketPath, errorMessage);
    std::ifstream preserved("tickets/ETicket_1234567890.txt");
    std::stringstream preservedContents;
    preservedContents << preserved.rdbuf();
    test("Rejected regeneration preserves existing complete ticket",
         !regenerated && ticketPath.empty() && preservedContents.str() == text);

    std::cout << "\n========== E-TICKET TEST SUMMARY ==========\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "Total:  " << passed + failed << '\n';

    fs::current_path(originalDirectory);
    fs::remove_all(testDirectory);

    return failed == 0 ? 0 : 1;
}
