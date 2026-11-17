#include "BookingManager.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static const std::string BOOKING_FILE = "data/bookings.dat";

void writeBookings(const std::vector<std::string>& records) {
    fs::create_directories("data");

    std::ofstream file(BOOKING_FILE, std::ios::trunc);
    assert(file.is_open());

    for (const auto& record : records) {
        file << record << '\n';
    }
}

void testValidBookingRecord() {
    writeBookings({
        "1234567890|77777|2099-12-31|Sleeper|Alice,30,Female|1|500.00|Confirmed|0"
    });

    Booking booking;
    assert(BookingManager::findBooking("1234567890", booking));
    assert(booking.pnr == "1234567890");
    assert(booking.trainNumber == "77777");
    assert(booking.passengerCount == 1);
    assert(booking.totalFare == 500.00);
    assert(booking.waitlistPosition == 0);
}

void testLegacyEightFieldRecord() {
    writeBookings({
        "1234567891|77777|2099-12-31|AC3|Bob,25,Male|1|750.00|Confirmed"
    });

    Booking booking;
    assert(BookingManager::findBooking("1234567891", booking));
    assert(booking.waitlistPosition == 0);
}

void testMalformedRecordsAreSkipped() {
    writeBookings({
        "too|few|fields",
        "1234567892|77777|2099-12-31|Sleeper|Alice,30,Female|not-a-number|500.00|Confirmed|0",
        "1234567893|77777|2099-12-31|Sleeper|Alice,30,Female|1|not-a-fare|Confirmed|0",
        "1234567894|77777|2099-12-31|Sleeper|Alice,30,Female|1|500.00|Confirmed|not-a-position",
        "1234567895|77777|2099-12-31|Sleeper|Alice,30,Female|1|500.00|Confirmed|0"
    });

    Booking booking;
    assert(!BookingManager::findBooking("1234567892", booking));
    assert(!BookingManager::findBooking("1234567893", booking));
    assert(!BookingManager::findBooking("1234567894", booking));
    assert(BookingManager::findBooking("1234567895", booking));
}

void testInvalidAndUnknownPNRs() {
    writeBookings({
        "1234567896|77777|2099-12-31|Sleeper|Alice,30,Female|1|500.00|Confirmed|0"
    });

    Booking booking;

    assert(!BookingManager::findBooking("", booking));
    assert(!BookingManager::findBooking("123", booking));
    assert(!BookingManager::findBooking("ABCDEFGHIJ", booking));
    assert(!BookingManager::findBooking("9999999999", booking));
}

void testPassengerHistoryMatching() {
    writeBookings({
        "1234567897|77777|2099-12-31|Sleeper|Alice,30,Female|1|500.00|Confirmed|0",
        "1234567898|88888|2099-12-31|AC3|Bob,25,Male;Alice,28,Female|2|1200.00|Confirmed|0",
        "1234567899|99999|2099-12-31|Sleeper|Charlie,40,Male|1|400.00|Confirmed|0"
    });

    const auto history = BookingManager::getBookingHistory("Alice");
    assert(history.size() == 2);

    const auto caseInsensitiveHistory =
        BookingManager::getBookingHistory("alice");
    assert(caseInsensitiveHistory.size() == 2);

    assert(BookingManager::getBookingHistory("").empty());
    assert(BookingManager::getBookingHistory("Nobody").empty());
}

void testMalformedPassengerEntries() {
    writeBookings({
        "1234567801|77777|2099-12-31|Sleeper|MalformedEntry;Alice,30,Female|2|500.00|Confirmed|0",
        "1234567802|77777|2099-12-31|Sleeper|NoSecondComma,30;Alice,25,Female|2|500.00|Confirmed|0",
        "1234567803|77777|2099-12-31|Sleeper|Charlie,40,Male|1|400.00|Confirmed|0"
    });

    const auto history = BookingManager::getBookingHistory("Alice");
    assert(history.size() == 2);
}

void testEncodedAndInvalidEscapeSequences() {
    writeBookings({
        "1234567804|77777|2099-12-31|Sleeper|%41%6Cice,30,Female|1|500.00|Confirmed|0",
        "1234567805|77777|2099-12-31|Sleeper|Bad%GGName,25,Male|1|500.00|Confirmed|0",
        "1234567806|77777|2099-12-31|Sleeper|Incomplete%,20,Other|1|500.00|Confirmed|0"
    });

    const auto decodedHistory = BookingManager::getBookingHistory("Alice");
    assert(decodedHistory.size() == 1);

    const auto invalidEscapeHistory =
        BookingManager::getBookingHistory("Bad%GGName");
    assert(invalidEscapeHistory.size() == 1);

    const auto incompleteEscapeHistory =
        BookingManager::getBookingHistory("Incomplete%");
    assert(incompleteEscapeHistory.size() == 1);
}

void testMissingBookingFile() {
    fs::remove(BOOKING_FILE);

    Booking booking;
    assert(!BookingManager::findBooking("1234567890", booking));
    assert(BookingManager::getBookingHistory("Alice").empty());
}

int main() {
    fs::create_directories("data");

    testValidBookingRecord();
    testLegacyEightFieldRecord();
    testMalformedRecordsAreSkipped();
    testInvalidAndUnknownPNRs();
    testPassengerHistoryMatching();
    testMalformedPassengerEntries();
    testEncodedAndInvalidEscapeSequences();
    testMissingBookingFile();

    std::cout << "All BookingManager coverage-extra tests passed.\n";
    return 0;
}
