
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

// These capacities are assumptions based on your sample trains.dat.
// Confirm them with your team before final submission.
const int MAX_SLEEPER_SEATS = 100;
const int MAX_AC3_SEATS = 80;
const int MAX_AC2_SEATS = 60;
const int MAX_CHAIR_CAR_SEATS = 50;

// Read all records from a file.
bool readLines(
    const std::string& filename,
    std::vector<std::string>& lines,
    std::string& errorMessage
) {
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

// Write records using a temporary file and safe Windows replacement.
bool writeLines(
    const std::string& filename,
    const std::vector<std::string>& lines,
    std::string& errorMessage
) {
    const std::string tempFile = filename + ".tmp";

    {
        std::ofstream file(tempFile, std::ios::trunc);

        if (!file.is_open()) {
            errorMessage = "Cannot create temporary file: " + tempFile;
            return false;
        }

        for (const std::string& line : lines) {
            file << line << '\n';
        }

        file.flush();

        if (!file.good()) {
            file.close();
            DeleteFileA(tempFile.c_str());
            errorMessage = "Failed to write temporary file.";
            return false;
        }

        file.close();
    }

    if (!MoveFileExA(
            tempFile.c_str(),
            filename.c_str(),
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {

        DWORD windowsError = GetLastError();

        DeleteFileA(tempFile.c_str());

        errorMessage =
            "Could not safely update file: " + filename +
            ". Windows error: " + std::to_string(windowsError);

        return false;
    }

    return true;
}

// Cancel a confirmed booking and calculate refund.
bool cancelBooking(
    const std::string& pnr,
    double& refundAmount,
    double& cancellationCharge,
    std::string& errorMessage
) {
    refundAmount = 0.0;
    cancellationCharge = 0.0;
    errorMessage.clear();

    // 1. Find booking using PNR.
    Booking booking;

    if (!BookingManager::findBooking(pnr, booking)) {
        errorMessage = "Booking not found for the given PNR.";
        return false;
    }

    // 2. Check booking status.
    if (booking.status != "Confirmed") {
        errorMessage = "Only confirmed bookings can be cancelled.";
        return false;
    }

    if (booking.passengerCount <= 0 ||
        booking.totalFare <= 0.0) {
        errorMessage = "Invalid booking details.";
        return false;
    }

    // 3. Calculate refund.
    // Confirm the 80% refund rule with your team.
    refundAmount = booking.totalFare * 0.80;
    cancellationCharge = booking.totalFare * 0.20;

    // 4. Read booking records.
    std::vector<std::string> bookingLines;

    if (!readLines(BOOKING_FILE, bookingLines, errorMessage)) {
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

    // 5. Read train records.
    std::vector<std::string> trainLines;

    if (!readLines(TRAIN_FILE, trainLines, errorMessage)) {
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
            return false;
        }

        // 6. Validate seat restoration.
        if (booking.passengerCount > maximumSeats ||
            *availableSeats > maximumSeats - booking.passengerCount) {
            errorMessage =
                "Seat restoration exceeds maximum capacity.";
            return false;
        }

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
        return false;
    }

    // 7. Read existing cancellation records.
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
            return false;
        }

        // Important: close before replacing the same file on Windows.
        cancellationInput.close();
    }

    // 8. Prepare cancellation record.
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

    // 9. Save cancellation record.
    if (!writeLines(
            CANCELLATION_FILE,
            cancellationLines,
            errorMessage)) {
        refundAmount = 0.0;
        cancellationCharge = 0.0;
        return false;
    }

    // 10. Update train availability.
    if (!writeLines(TRAIN_FILE, trainLines, errorMessage)) {
        errorMessage +=
            " Cancellation record was saved, but train data was not updated.";
        refundAmount = 0.0;
        cancellationCharge = 0.0;
        return false;
    }

    // 11. Update booking status.
    if (!writeLines(BOOKING_FILE, bookingLines, errorMessage)) {
        errorMessage +=
            " Train availability was updated, but booking status was not updated. Manual recovery may be required.";
        refundAmount = 0.0;
        cancellationCharge = 0.0;
        return false;
    }

    return true;
}

} // namespace Cancellation
