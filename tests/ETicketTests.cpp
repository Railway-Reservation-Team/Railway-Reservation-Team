
#include "ETicketManager.h"
#include "BookingManager.h"
#include "TrainData.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static void updateBookingStatus(
    const std::string& status) {

    std::ifstream input("data/bookings.dat");
    assert(input.is_open());

    std::string line;
    assert(std::getline(input, line));
    input.close();

    const std::size_t separator = line.rfind('|');
    assert(separator != std::string::npos);

    line = line.substr(0, separator + 1) + status;

    std::ofstream output("data/bookings.dat", std::ios::trunc);
    assert(output.is_open());

    output << line << '\n';
    output.close();
}

int main() {
    const fs::path originalDirectory = fs::current_path();

    const fs::path testDirectory =
        fs::temp_directory_path() / "railway_eticket_tests";

    fs::remove_all(testDirectory);
    fs::create_directories(testDirectory / "data");

    fs::current_path(testDirectory);

    std::cout << "Starting E-Ticket Tests...\n";

    std::string error;

    // Test passenger detail decoding.
    std::vector<Passenger> passengers;

    assert(ETicketManager::decodePassengers(
        "A%2CB,20,Female;Second Passenger,21,Male",
        passengers, error));

    assert(passengers.size() == 2);
    assert(passengers[0].name == "A,B");
    assert(passengers[0].age == 20);
    assert(passengers[0].gender == "Female");

    std::cout << "[PASS] Passenger detail decoding\n";

    // Test malformed passenger details.
    assert(!ETicketManager::decodePassengers(
        "InvalidPassengerRecord",
        passengers, error));

    std::cout << "[PASS] Invalid passenger detail rejection\n";

    // Prepare isolated train data.
    const std::string trainRecord =
        "99999|Test Express|Bengaluru|Chennai|10:00|18:00|8h|"
        "500|800|1000|400|20|20|20|20|2026-11-01";

    Train train;
    assert(parseTrainRecord(trainRecord, train));

    {
        std::ofstream trainFile("data/trains.dat");
        assert(trainFile.is_open());
        trainFile << trainRecord << '\n';
    }

    // Create an empty booking file.
    {
        std::ofstream bookingFile("data/bookings.dat");
        assert(bookingFile.is_open());
    }

    std::cout << "[PASS] Isolated test data preparation\n";

    // Create a confirmed booking.
    std::vector<Passenger> bookingPassengers = {
        {"Test Passenger", 20, "Male"},
        {"Second Passenger", 21, "Female"}
    };

    std::string pnr;

    assert(BookingManager::createBooking(
        train,
        "AC 2-Tier",
        bookingPassengers,
        2000.0,
        pnr,
        error));

    std::cout << "[PASS] Test booking creation\n";

    // Generate an e-ticket for the confirmed booking.
    ETicket ticket;

    assert(ETicketManager::generateTicket(
        pnr, ticket, error));

    assert(ticket.pnr == pnr);
    assert(ticket.trainNumber == "99999");
    assert(ticket.trainName == "Test Express");
    assert(ticket.source == "Bengaluru");
    assert(ticket.destination == "Chennai");
    assert(ticket.passengers.size() == 2);
    assert(ticket.bookingStatus == "Confirmed");

    std::cout << "[PASS] Confirmed booking ticket generation\n";

    // Test readable ticket formatting.
    std::string ticketText;

    assert(ETicketManager::formatTicket(
        ticket, ticketText, error));

    assert(ticketText.find(pnr) != std::string::npos);
    assert(ticketText.find("Test Express") != std::string::npos);
    assert(ticketText.find("Test Passenger") != std::string::npos);
    assert(ticketText.find("Bengaluru") != std::string::npos);
    assert(ticketText.find("Chennai") != std::string::npos);

    std::cout << "[PASS] Ticket formatting and details verification\n";

    // Test ticket export.
    std::string filePath;

    assert(ETicketManager::exportTicket(
        ticket, filePath, error));

    assert(fs::exists(filePath));

    std::ifstream exportedTicket(filePath);
    assert(exportedTicket.is_open());

    std::string exportedContent(
        (std::istreambuf_iterator<char>(exportedTicket)),
        std::istreambuf_iterator<char>());

    assert(exportedContent.find(pnr) != std::string::npos);

    // Close the file before deleting the test directory.
    exportedTicket.close();

    std::cout << "[PASS] Ticket file export\n";

    // Test invalid PNR.
    assert(!ETicketManager::generateTicket(
        "INVALID", ticket, error));

    std::cout << "[PASS] Invalid PNR rejection\n";

    // Test cancelled booking rejection.
    updateBookingStatus("Cancelled");

    assert(!ETicketManager::generateTicket(
        pnr, ticket, error));

    std::cout << "[PASS] Cancelled booking ticket rejection\n";

    // Test waitlisted booking rejection.
    updateBookingStatus("Waitlisted");

    assert(!ETicketManager::generateTicket(
        pnr, ticket, error));

    std::cout << "[PASS] Waitlisted booking ticket rejection\n";

    // Restore the original working directory.
    fs::current_path(originalDirectory);
    fs::remove_all(testDirectory);

    std::cout << "\nALL E-TICKET TESTS PASSED!\n";

    return 0;
}
