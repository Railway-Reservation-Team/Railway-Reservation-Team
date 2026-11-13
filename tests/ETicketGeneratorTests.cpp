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

void writeTrain() {
    std::ofstream file("data/trains.dat");
    file << "12627|Karnataka Express|Bengaluru|New Delhi"
         << "|20:00|10:30|14h 30m"
         << "|1200|1800|2400|900"
         << "|100|50|30|40|2026-11-01\n";
}

void writeBooking(
    const std::string& pnr,
    const std::string& passengers,
    int count = 2) {

    std::ofstream file("data/bookings.dat");
    file << pnr << "|12627|2026-11-01|Sleeper|"
         << passengers << "|" << count
         << "|2400|Confirmed\n";
}

bool rejectedWithoutTicket(
    const std::string& pnr,
    const std::string& passengers,
    int count) {

    writeBooking(pnr, passengers, count);

    std::string path;
    std::string error;

    bool result = ETicketGenerator::generateETicket(pnr, path, error);

    return !result &&
           !std::filesystem::exists("tickets/ETicket_" + pnr + ".txt");
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

    writeTrain();

    std::string ticketPath;
    std::string errorMessage;

    writeBooking(
        "TEST000001",
        "Dinesh,20,Male;Abhay,20,Male",
        2
    );

    bool generated = ETicketGenerator::generateETicket(
        "TEST000001", ticketPath, errorMessage
    );

    test("Valid PNR generates e-ticket",
         generated && fs::exists(ticketPath));

    std::ifstream ticket(ticketPath);
    std::stringstream contents;
    contents << ticket.rdbuf();

    std::string text = contents.str();

    test("Ticket contains correct booking details",
         text.find("TEST000001") != std::string::npos &&
         text.find("Karnataka Express") != std::string::npos &&
         text.find("Dinesh") != std::string::npos &&
         text.find("Abhay") != std::string::npos &&
         text.find("Rs. 2400.00") != std::string::npos);

    ticketPath.clear();
    errorMessage.clear();

    bool invalidResult = ETicketGenerator::generateETicket(
        "INVALID000", ticketPath, errorMessage
    );

    test("Invalid PNR is rejected", !invalidResult);

    test("Empty passenger name is rejected",
         rejectedWithoutTicket("TEST000002", ",20,Male;Abhay,20,Male", 2));

    test("Empty passenger age is rejected",
         rejectedWithoutTicket("TEST000003", "Dinesh,,Male;Abhay,20,Male", 2));

    test("Empty passenger gender is rejected",
         rejectedWithoutTicket("TEST000004", "Dinesh,20,;Abhay,20,Male", 2));

    test("Invalid passenger age is rejected",
         rejectedWithoutTicket("TEST000005", "Dinesh,abc,Male;Abhay,20,Male", 2));

    test("Passenger age outside valid range is rejected",
         rejectedWithoutTicket("TEST000006", "Dinesh,121,Male;Abhay,20,Male", 2));

    test("Too few passengers are rejected",
         rejectedWithoutTicket("TEST000007", "Dinesh,20,Male", 2));

    test("Too many passengers are rejected",
         rejectedWithoutTicket("TEST000008",
                               "Dinesh,20,Male;Abhay,20,Male;Rahul,21,Male",
                               2));

    test("Invalid gender is rejected",
         rejectedWithoutTicket("TEST000009",
                               "Dinesh,20,Unknown;Abhay,20,Male", 2));

    test("Malformed passenger record is rejected",
         rejectedWithoutTicket("TEST000010",
                               "Dinesh,20;Abhay,20,Male", 2));

    std::cout << "\n========== E-TICKET TEST SUMMARY ==========\n";
    std::cout << "Passed: " << passed << '\n';
    std::cout << "Failed: " << failed << '\n';
    std::cout << "Total:  " << passed + failed << '\n';

    fs::current_path(originalDirectory);
    fs::remove_all(testDirectory);

    return failed == 0 ? 0 : 1;
}
