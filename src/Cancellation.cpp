
#include "Cancellation.h"
#include "BookingManager.h"
#include "TrainData.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <windows.h>

namespace Cancellation {

const std::string BOOKING_FILE = "data/bookings.dat";
const std::string TRAIN_FILE = "data/trains.dat";
const std::string CANCELLATION_FILE = "data/cancellations.dat";

// Provisional capacities. Confirm these with the project team.
const int MAX_SLEEPER_SEATS = 100;
const int MAX_AC3_SEATS = 80;
const int MAX_AC2_SEATS = 60;
const int MAX_CHAIR_CAR_SEATS = 50;

// Structure for updating files.
struct FileUpdate {
    std::string filename;
    std::vector<std::string> lines;
    bool existed = false;
};

// Read records from a file.
bool readLines(
    const std::string& filename,
    std::vector<std::string>& lines,
    std::string& errorMessage) {

    std::ifstream file(filename);

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

    file.close();
    return true;
}

// Update multiple files with backup and rollback protection.
bool writeFilesAtomically(
    std::vector<FileUpdate>& updates,
    std::string& errorMessage) {

    errorMessage.clear();

    std::vector<bool> backupsCreated(updates.size(), false);
    std::vector<bool> filesExisted(updates.size(), false);

    // Step 1: Prepare backups and temporary files.
    for (std::size_t i = 0; i < updates.size(); ++i) {

        const std::string filename = updates[i].filename;
        const std::string tempFile = filename + ".tmp";
        const std::string backupFile = filename + ".bak";

        DWORD attributes = GetFileAttributesA(filename.c_str());

        if (attributes == INVALID_FILE_ATTRIBUTES) {
            DWORD error = GetLastError();

            if (error != ERROR_FILE_NOT_FOUND &&
                error != ERROR_PATH_NOT_FOUND) {

                errorMessage =
                    "Could not check file: " + filename;

                goto preparation_failed;
            }

            filesExisted[i] = false;
        }
        else {
            filesExisted[i] = true;
        }

        if (filesExisted[i]) {

            if (!CopyFileA(
                    filename.c_str(),
                    backupFile.c_str(),
                    FALSE)) {

                errorMessage =
                    "Could not create backup for " + filename;

                goto preparation_failed;
            }

            backupsCreated[i] = true;
        }

        {
            std::ofstream file(tempFile, std::ios::trunc);

            if (!file.is_open()) {
                errorMessage =
                    "Could not create temporary file: " + tempFile;

                goto preparation_failed;
            }

            for (const std::string& line : updates[i].lines) {
                file << line << '\n';
            }

            file.flush();

            if (!file.good()) {
                errorMessage =
                    "Failed to write temporary file: " + tempFile;

                file.close();
                goto preparation_failed;
            }

            file.close();
        }
    }

    // Step 2: Replace original files.
    {
        std::size_t committed = 0;

        for (; committed < updates.size(); ++committed) {

            const std::string tempFile =
                updates[committed].filename + ".tmp";

            if (!MoveFileExA(
                    tempFile.c_str(),
                    updates[committed].filename.c_str(),
                    MOVEFILE_REPLACE_EXISTING |
                    MOVEFILE_WRITE_THROUGH)) {

                DWORD windowsError = GetLastError();

                errorMessage =
                    "Could not update " +
                    updates[committed].filename +
                    ". Windows error: " +
                    std::to_string(windowsError);

                break;
            }
        }

        // Step 3: Roll back previously replaced files.
        if (committed != updates.size()) {

            bool rollbackSucceeded = true;

            while (committed > 0) {

                --committed;

                const std::string filename =
                    updates[committed].filename;

                if (filesExisted[committed]) {

                    const std::string backupFile =
                        filename + ".bak";

                    if (!MoveFileExA(
                            backupFile.c_str(),
                            filename.c_str(),
                            MOVEFILE_REPLACE_EXISTING |
                            MOVEFILE_WRITE_THROUGH)) {

                        rollbackSucceeded = false;
                    }
                    else {
                        backupsCreated[committed] = false;
                    }
                }
                else {

                    if (!DeleteFileA(filename.c_str()) &&
                        GetLastError() != ERROR_FILE_NOT_FOUND) {

                        rollbackSucceeded = false;
                    }
                }
            }

            // Remove temporary files.
            for (const FileUpdate& update : updates) {
                DeleteFileA((update.filename + ".tmp").c_str());
            }

            // Delete backups only if rollback succeeded.
            if (rollbackSucceeded) {

                for (std::size_t i = 0; i < updates.size(); ++i) {

                    if (backupsCreated[i]) {
                        DeleteFileA(
                            (updates[i].filename + ".bak").c_str());
                    }
                }
            }
            else {
                errorMessage +=
                    " WARNING: Rollback was incomplete. "
                    "Backup files have been retained for manual recovery.";
            }

            return false;
        }
    }

    // Step 4: All replacements succeeded.
    for (std::size_t i = 0; i < updates.size(); ++i) {

        DeleteFileA((updates[i].filename + ".tmp").c_str());

        if (backupsCreated[i]) {
            DeleteFileA((updates[i].filename + ".bak").c_str());
        }
    }

    return true;

preparation_failed:

    // No original file has been replaced yet.
    for (std::size_t i = 0; i < updates.size(); ++i) {

        DeleteFileA((updates[i].filename + ".tmp").c_str());

        if (backupsCreated[i]) {
            DeleteFileA((updates[i].filename + ".bak").c_str());
        }
    }

    return false;
}

// Cancel a confirmed booking and calculate refund.
bool cancelBooking(
    const std::string& pnr,
    double& refundAmount,
    double& cancellationCharge,
    std::string& errorMessage) {

    refundAmount = 0.0;
    cancellationCharge = 0.0;
    errorMessage.clear();

    // Step 1: Find booking using PNR.
    Booking booking;

    if (!BookingManager::findBooking(pnr, booking)) {
        errorMessage = "Booking not found for the given PNR.";
        return false;
    }

    // Step 2: Check cancellation eligibility.
    if (booking.status != "Confirmed") {
        errorMessage = "Only confirmed bookings can be cancelled.";
        return false;
    }

    if (booking.passengerCount <= 0 ||
        booking.totalFare <= 0.0) {

        errorMessage = "Invalid booking details.";
        return false;
    }

    // Step 3: Calculate refund.
    // Confirm the 80/20 policy with the project team.
    refundAmount = booking.totalFare * 0.80;
    cancellationCharge = booking.totalFare * 0.20;

    // Step 4: Read booking records.
    std::vector<std::string> bookingLines;

    if (!readLines(BOOKING_FILE, bookingLines, errorMessage)) {
        refundAmount = 0.0;
        cancellationCharge = 0.0;
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

                refundAmount = 0.0;
                cancellationCharge = 0.0;
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
        refundAmount = 0.0;
        cancellationCharge = 0.0;
        return false;
    }

    // Step 5: Read train records.
    std::vector<std::string> trainLines;

    if (!readLines(TRAIN_FILE, trainLines, errorMessage)) {
        refundAmount = 0.0;
        cancellationCharge = 0.0;
        return false;
    }

    bool trainUpdated = false;

    for (std::string& line : trainLines) {

        Train train;

        if (!parseTrainRecord(line, train)) {
            errorMessage = "Invalid train record.";
            refundAmount = 0.0;
            cancellationCharge = 0.0;
            return false;
        }

        if (train.trainNumber != booking.trainNumber ||
            train.journeyDate != booking.journeyDate) {
            continue;
        }

        int* availableSeats = nullptr;
        int maximumSeats = 0;

        if (booking.travelClass == "Sleeper") {
            availableSeats = &train.sleeperSeatsAvailable;
            maximumSeats = MAX_SLEEPER_SEATS;
        }
        else if (booking.travelClass == "AC 3-Tier") {
            availableSeats = &train.ac3TierSeatsAvailable;
            maximumSeats = MAX_AC3_SEATS;
        }
        else if (booking.travelClass == "AC 2-Tier") {
            availableSeats = &train.ac2TierSeatsAvailable;
            maximumSeats = MAX_AC2_SEATS;
        }
        else if (booking.travelClass == "Chair Car") {
            availableSeats = &train.chairCarSeatsAvailable;
            maximumSeats = MAX_CHAIR_CAR_SEATS;
        }

        if (availableSeats == nullptr) {
            errorMessage = "Invalid travel class in booking.";
            refundAmount = 0.0;
            cancellationCharge = 0.0;
            return false;
        }

        // Step 6: Validate maximum seat capacity.
        if (booking.passengerCount > maximumSeats ||
            *availableSeats > maximumSeats - booking.passengerCount) {

            errorMessage =
                "Seat restoration exceeds maximum capacity.";

            refundAmount = 0.0;
            cancellationCharge = 0.0;
            return false;
        }

        // Restore available seats.
        *availableSeats += booking.passengerCount;

        std::ostringstream updatedTrain;

        updatedTrain
            << train.trainNumber << '|'
            << train.trainName << '|'
            << train.source << '|'
            << train.destination << '|'
            << train.departureTime << '|'
            << train.arrivalTime << '|'
            << train.journeyDuration << '|'
            << train.sleeperFare << '|'
            << train.ac3TierFare << '|'
            << train.ac2TierFare << '|'
            << train.chairCarFare << '|'
            << train.sleeperSeatsAvailable << '|'
            << train.ac3TierSeatsAvailable << '|'
            << train.ac2TierSeatsAvailable << '|'
            << train.chairCarSeatsAvailable << '|'
            << train.journeyDate;

        line = updatedTrain.str();
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

    // Step 7: Read existing cancellation records.
    std::vector<std::string> cancellationLines;

    std::ifstream cancellationInput(CANCELLATION_FILE);

    if (cancellationInput.is_open()) {

        std::string line;

        while (std::getline(cancellationInput, line)) {
            if (!line.empty()) {
                cancellationLines.push_back(line);
            }
        }

        if (cancellationInput.bad()) {
            cancellationInput.close();

            errorMessage = "Could not read cancellation records.";

            refundAmount = 0.0;
            cancellationCharge = 0.0;
            return false;
        }

        cancellationInput.close();
    }

    // Step 8: Prepare cancellation record.
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

    // Step 9: Update all three files with rollback protection.
    std::vector<FileUpdate> updates = {
        {CANCELLATION_FILE, cancellationLines},
        {TRAIN_FILE, trainLines},
        {BOOKING_FILE, bookingLines}
    };

    if (!writeFilesAtomically(updates, errorMessage)) {

        refundAmount = 0.0;
        cancellationCharge = 0.0;

        errorMessage +=
            " Cancellation was not completed successfully.";

        return false;
    }

    // Step 10: Cancellation completed successfully.
    return true;
}

} // namespace Cancellation
