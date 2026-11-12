
#include "Cancellation.h"
#include "BookingManager.h"
#include "TrainData.h"
#include "AtomicFileReplace.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <limits>

namespace Cancellation {

const std::string BOOKING_FILE = "data/bookings.dat";
const std::string TRAIN_FILE = "data/trains.dat";
const std::string CANCELLATION_FILE = "data/cancellations.dat";

// SRS default cancellation fee percentages.
const int CANCELLATION_FEE_TIER1_PCT = 25;
const int CANCELLATION_FEE_TIER2_PCT = 50;

struct FileUpdate {
    std::string filename;
    std::vector<std::string> lines;

    FileUpdate(
        const std::string& name,
        const std::vector<std::string>& records
    ) : filename(name), lines(records) {}
};

// Read records from a file.
bool readLines(
    const std::string& filename,
    std::vector<std::string>& lines,
    std::string& errorMessage) {

    std::ifstream file(filename.c_str());

    if (!file.is_open()) {
        errorMessage = "Cannot open file: " + filename;
        return false;
    }

    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty()) {
            lines.push_back(line);
        }
    }

    if (file.bad()) {
        errorMessage = "Error reading file: " + filename;
        return false;
    }

    return true;
}

// Calculate cancellation fee and refund using SRS time slabs.
bool calculateRefund(
    const Train& train,
    double totalFare,
    double& refund,
    double& charge,
    std::string& errorMessage) {

    refund = 0.0;
    charge = 0.0;

    if (train.journeyDate.size() != 10 ||
        train.journeyDate[4] != '-' ||
        train.journeyDate[7] != '-') {
        errorMessage = "Invalid train journey date.";
        return false;
    }

    if (train.departureTime.size() != 5 ||
        train.departureTime[2] != ':') {
        errorMessage = "Invalid train departure time.";
        return false;
    }

    for (std::size_t i = 0; i < train.departureTime.size(); ++i) {
        if (i != 2 &&
            (train.departureTime[i] < '0' ||
             train.departureTime[i] > '9')) {
            errorMessage = "Invalid train departure time.";
            return false;
        }
    }

    int year;
    int month;
    int day;
    int hour;
    int minute;

    try {
        year = std::stoi(train.journeyDate.substr(0, 4));
        month = std::stoi(train.journeyDate.substr(5, 2));
        day = std::stoi(train.journeyDate.substr(8, 2));
        hour = std::stoi(train.departureTime.substr(0, 2));
        minute = std::stoi(train.departureTime.substr(3, 2));
    }
    catch (...) {
        errorMessage = "Invalid journey date or departure time.";
        return false;
    }

    if (hour < 0 || hour > 23 ||
        minute < 0 || minute > 59) {
        errorMessage = "Invalid train departure time.";
        return false;
    }

    std::tm departure = {};

    departure.tm_year = year - 1900;
    departure.tm_mon = month - 1;
    departure.tm_mday = day;
    departure.tm_hour = hour;
    departure.tm_min = minute;
    departure.tm_sec = 0;
    departure.tm_isdst = -1;

    std::time_t departureTime = std::mktime(&departure);

    if (departureTime == static_cast<std::time_t>(-1)) {
        errorMessage = "Could not determine train departure time.";
        return false;
    }

    std::time_t currentTime = std::time(nullptr);

    double hoursRemaining =
        std::difftime(departureTime, currentTime) / 3600.0;

    if (hoursRemaining <= 0.0) {
        errorMessage = "Cannot cancel after train departure.";
        return false;
    }

    int feePercentage;

    if (hoursRemaining > 24.0) {
        feePercentage = CANCELLATION_FEE_TIER1_PCT;
    }
    else {
        feePercentage = CANCELLATION_FEE_TIER2_PCT;
    }

    charge = totalFare * feePercentage / 100.0;
    refund = totalFare - charge;

    return true;
}

// Write multiple files with temporary files and rollback support.
bool writeFilesAtomically(
    std::vector<FileUpdate>& updates,
    std::string& errorMessage) {

    std::vector<std::string> originalContents(updates.size());
    std::vector<bool> existed(updates.size(), false);

    // Prepare temporary files before replacing originals.
    for (std::size_t i = 0; i < updates.size(); ++i) {

        std::ifstream original(
            updates[i].filename.c_str(),
            std::ios::binary
        );

        if (original.is_open()) {
            std::ostringstream content;
            content << original.rdbuf();

            if (original.bad()) {
                errorMessage =
                    "Could not read " + updates[i].filename;
                return false;
            }

            originalContents[i] = content.str();
            existed[i] = true;
        }

        std::ofstream temp(
            (updates[i].filename + ".tmp").c_str(),
            std::ios::binary | std::ios::trunc
        );

        if (!temp.is_open()) {
            errorMessage =
                "Could not create temporary file for " +
                updates[i].filename;

            for (const FileUpdate& update : updates) {
                std::remove((update.filename + ".tmp").c_str());
            }

            return false;
        }

        for (const std::string& record : updates[i].lines) {
            temp << record << '\n';
        }

        temp.close();

        if (!temp) {
            errorMessage =
                "Failed writing temporary file for " +
                updates[i].filename;

            for (const FileUpdate& update : updates) {
                std::remove((update.filename + ".tmp").c_str());
            }

            return false;
        }
    }

    // Replace original files.
    std::size_t committed = 0;

    for (; committed < updates.size(); ++committed) {

        const std::string tempName =
            updates[committed].filename + ".tmp";

        if (!replaceFileAtomically(
                tempName,
                updates[committed].filename)) {

            errorMessage =
                "Could not replace " +
                updates[committed].filename;

            break;
        }
    }

    if (committed == updates.size()) {
        return true;
    }

    // Roll back files already replaced.
    bool rollbackSucceeded = true;

    while (committed > 0) {
        --committed;

        const std::string filename =
            updates[committed].filename;

        if (existed[committed]) {

            const std::string rollbackFile =
                filename + ".rollback";

            std::ofstream rollback(
                rollbackFile.c_str(),
                std::ios::binary | std::ios::trunc
            );

            if (!rollback.is_open()) {
                rollbackSucceeded = false;
                continue;
            }

            rollback << originalContents[committed];
            rollback.close();

            if (!rollback ||
                !replaceFileAtomically(
                    rollbackFile,
                    filename)) {

                rollbackSucceeded = false;
            }
        }
        else {
            if (std::remove(filename.c_str()) != 0) {
                rollbackSucceeded = false;
            }
        }
    }

    for (const FileUpdate& update : updates) {
        std::remove((update.filename + ".tmp").c_str());
    }

    if (!rollbackSucceeded) {
        errorMessage +=
            " WARNING: Rollback incomplete. "
            "Please inspect the affected files.";
    }

    return false;
}

// Cancel a confirmed booking.
bool cancelBooking(
    const std::string& pnr,
    double& refundAmount,
    double& cancellationCharge,
    std::string& errorMessage) {

    refundAmount = 0.0;
    cancellationCharge = 0.0;
    errorMessage.clear();

    Booking booking;

    if (!BookingManager::findBooking(pnr, booking)) {
        errorMessage = "Booking not found for the given PNR.";
        return false;
    }

    if (booking.status != "Confirmed") {
        errorMessage = "Only confirmed bookings can be cancelled.";
        return false;
    }

    if (booking.passengerCount <= 0 ||
        !std::isfinite(booking.totalFare) ||
        booking.totalFare <= 0.0) {

        errorMessage = "Invalid booking details.";
        return false;
    }

    // Read existing booking records.
    std::vector<std::string> bookingLines;

    if (!readLines(
            BOOKING_FILE,
            bookingLines,
            errorMessage)) {
        return false;
    }

    bool bookingUpdated = false;

    for (std::string& line : bookingLines) {

        std::stringstream stream(line);
        std::vector<std::string> fields;
        std::string field;

        while (std::getline(stream, field, '|')) {
            fields.push_back(field);
        }

        if (fields.size() == 8 && fields[0] == pnr) {

            if (fields[7] != "Confirmed") {
                errorMessage =
                    "Booking is no longer eligible for cancellation.";
                return false;
            }

            fields[7] = "Cancelled";

            std::ostringstream updatedLine;

            for (std::size_t i = 0; i < fields.size(); ++i) {
                if (i > 0) {
                    updatedLine << '|';
                }

                updatedLine << fields[i];
            }

            line = updatedLine.str();
            bookingUpdated = true;
            break;
        }
    }

    if (!bookingUpdated) {
        errorMessage = "Booking record could not be updated.";
        return false;
    }

    // Read train records.
    std::vector<std::string> trainLines;

    if (!readLines(
            TRAIN_FILE,
            trainLines,
            errorMessage)) {
        return false;
    }

    bool trainUpdated = false;

    for (std::string& line : trainLines) {

        Train train;

        if (!parseTrainRecord(line, train)) {
            errorMessage = "Invalid train record.";
            return false;
        }

        if (train.trainNumber != booking.trainNumber ||
            train.journeyDate != booking.journeyDate) {
            continue;
        }

        if (!calculateRefund(
                train,
                booking.totalFare,
                refundAmount,
                cancellationCharge,
                errorMessage)) {

            refundAmount = 0.0;
            cancellationCharge = 0.0;
            return false;
        }

        int* availableSeats = nullptr;

        if (booking.travelClass == "Sleeper") {
            availableSeats = &train.sleeperSeatsAvailable;
        }
        else if (booking.travelClass == "AC 3-Tier") {
            availableSeats = &train.ac3TierSeatsAvailable;
        }
        else if (booking.travelClass == "AC 2-Tier") {
            availableSeats = &train.ac2TierSeatsAvailable;
        }
        else if (booking.travelClass == "Chair Car") {
            availableSeats = &train.chairCarSeatsAvailable;
        }

        if (availableSeats == nullptr) {
            errorMessage = "Invalid travel class in booking.";
            refundAmount = 0.0;
            cancellationCharge = 0.0;
            return false;
        }

        if (*availableSeats < 0 ||
            *availableSeats >
                std::numeric_limits<int>::max() -
                booking.passengerCount) {

            errorMessage = "Invalid seat restoration values.";
            refundAmount = 0.0;
            cancellationCharge = 0.0;
            return false;
        }

        // Restore seats.
        *availableSeats += booking.passengerCount;

        // Preserve active/deactivated state while updating seat inventory.
        line = serializeTrainRecord(train);

        trainUpdated = true;
        break;
    }

    if (!trainUpdated) {
        errorMessage =
            "Train record for this booking was not found.";

        refundAmount = 0.0;
        cancellationCharge = 0.0;
        return false;
    }

    // Read previous cancellation records.
    std::vector<std::string> cancellationLines;

    std::ifstream cancellationInput(
        CANCELLATION_FILE.c_str()
    );

    if (cancellationInput.is_open()) {

        std::string line;

        while (std::getline(cancellationInput, line)) {
            if (!line.empty()) {
                cancellationLines.push_back(line);
            }
        }

        // Close before replacing the cancellation file.
        cancellationInput.close();

        if (cancellationInput.bad()) {
            errorMessage =
                "Could not read cancellation records.";

            refundAmount = 0.0;
            cancellationCharge = 0.0;
            return false;
        }
    }

    // Create cancellation record.
    std::ostringstream cancellationRecord;

    cancellationRecord
        << std::fixed << std::setprecision(2)
        << pnr << '|'
        << booking.trainNumber << '|'
        << booking.journeyDate << '|'
        << booking.travelClass << '|'
        << booking.passengerCount << '|'
        << booking.totalFare << '|'
        << cancellationCharge << '|'
        << refundAmount;

    cancellationLines.push_back(cancellationRecord.str());

    // Prepare all three file updates.
    std::vector<FileUpdate> updates;

    updates.push_back(
        FileUpdate(CANCELLATION_FILE, cancellationLines)
    );

    updates.push_back(
        FileUpdate(TRAIN_FILE, trainLines)
    );

    updates.push_back(
        FileUpdate(BOOKING_FILE, bookingLines)
    );

    // Save updates with rollback support.
    if (!writeFilesAtomically(updates, errorMessage)) {

        refundAmount = 0.0;
        cancellationCharge = 0.0;

        errorMessage +=
            " Cancellation was not completed successfully.";

        return false;
    }

    return true;
}

} // namespace Cancellation
