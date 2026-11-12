
#include "PaymentReceiptManager.h"
#include "Payment.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>
#include <cmath>
#include <cstdio>

namespace PaymentReceiptManager {

static const std::string RECEIPT_FILE = "data/payment_receipts.dat";

// Read existing receipt records.
static bool loadReceipts(
    std::vector<PaymentReceipt>& receipts,
    std::string& errorMessage) {

    receipts.clear();

    std::ifstream file(RECEIPT_FILE);

    if (!file.is_open()) {
        // A missing file is acceptable when no receipts exist yet.
        return true;
    }

    std::string line;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        PaymentReceipt receipt;

        if (!parseReceipt(line, receipt, errorMessage, receipts)) {
            errorMessage = "Invalid existing receipt record: " + errorMessage;
            return false;
        }

        receipts.push_back(receipt);
    }

    if (file.bad()) {
        errorMessage = "Error reading payment receipt file.";
        return false;
    }

    return true;
}

// Generate a unique transaction ID.
bool generateTransactionId(
    std::string& transactionId,
    std::string& errorMessage) {

    errorMessage.clear();
    transactionId.clear();

    std::vector<PaymentReceipt> existingReceipts;

    if (!loadReceipts(existingReceipts, errorMessage)) {
        return false;
    }

    std::random_device rd;
    std::mt19937_64 generator(rd());

    std::uniform_int_distribution<unsigned long long> distribution(
        100000000000ULL, 999999999999ULL
    );

    for (int attempt = 0; attempt < 1000; ++attempt) {
        const std::string candidate =
            "TXN" + std::to_string(distribution(generator));

        bool unique = true;

        for (const PaymentReceipt& receipt : existingReceipts) {
            if (receipt.transactionId == candidate) {
                unique = false;
                break;
            }
        }

        if (unique) {
            transactionId = candidate;
            return true;
        }
    }

    errorMessage = "Unable to generate a unique transaction ID.";
    return false;
}

// Validate and serialize a receipt.
bool serializeReceipt(
    const PaymentReceipt& receipt,
    std::string& record,
    std::string& errorMessage,
    const std::vector<PaymentReceipt>& existingReceipts) {

    record.clear();
    errorMessage.clear();

    if (receipt.transactionId.empty() ||
        receipt.transactionId.find('|') != std::string::npos ||
        receipt.transactionId.find('\n') != std::string::npos ||
        receipt.transactionId.find('\r') != std::string::npos) {

        errorMessage = "Invalid transaction ID.";
        return false;
    }

    if (receipt.status != "SUCCESS" &&
        receipt.status != "FAILED") {

        errorMessage = "Invalid receipt payment status.";
        return false;
    }

    if (receipt.status == "SUCCESS" &&
        !isValidPaymentPNR(receipt.pnr)) {

        errorMessage = "Successful payment receipt requires a valid PNR.";
        return false;
    }

    if (receipt.status == "FAILED" &&
        !receipt.pnr.empty() &&
        !isValidPaymentPNR(receipt.pnr)) {

        errorMessage = "Invalid PNR for failed payment.";
        return false;
    }

    if (!isValidPaymentMethod(receipt.paymentMethod)) {
        errorMessage = "Invalid payment method.";
        return false;
    }

    if (!isValidPaymentDateTime(receipt.paymentDateTime)) {
        errorMessage = "Invalid payment date and time.";
        return false;
    }

    if (!std::isfinite(receipt.amount) || receipt.amount <= 0.0) {
        errorMessage = "Receipt amount must be positive and finite.";
        return false;
    }

    for (const PaymentReceipt& existing : existingReceipts) {
        if (existing.transactionId == receipt.transactionId) {
            errorMessage = "Duplicate transaction ID.";
            return false;
        }
    }

    std::ostringstream output;

    output << receipt.transactionId << '|'
           << receipt.pnr << '|'
           << std::fixed << std::setprecision(2)
           << receipt.amount << '|'
           << receipt.paymentMethod << '|'
           << receipt.status << '|'
           << receipt.paymentDateTime;

    record = output.str();

    return true;
}

// Parse a stored receipt.
bool parseReceipt(
    const std::string& record,
    PaymentReceipt& receipt,
    std::string& errorMessage,
    const std::vector<PaymentReceipt>& existingReceipts) {

    errorMessage.clear();

    std::stringstream input(record);
    std::vector<std::string> fields;
    std::string field;

    while (std::getline(input, field, '|')) {
        fields.push_back(field);
    }

    if (fields.size() != 6) {
        errorMessage = "Receipt must contain exactly six fields.";
        return false;
    }

    PaymentReceipt parsed;

    parsed.transactionId = fields[0];
    parsed.pnr = fields[1];
    parsed.paymentMethod = fields[3];
    parsed.status = fields[4];
    parsed.paymentDateTime = fields[5];

    try {
        std::size_t position = 0;
        parsed.amount = std::stod(fields[2], &position);

        if (position != fields[2].size()) {
            errorMessage = "Invalid receipt amount.";
            return false;
        }
    }
    catch (...) {
        errorMessage = "Invalid receipt amount.";
        return false;
    }

    std::string serialized;

    if (!serializeReceipt(
            parsed, serialized, errorMessage, existingReceipts)) {
        return false;
    }

    receipt = parsed;
    return true;
}

// Save a receipt persistently.
bool saveReceipt(
    const PaymentReceipt& receipt,
    std::string& errorMessage) {

    errorMessage.clear();

    std::vector<PaymentReceipt> existingReceipts;

    if (!loadReceipts(existingReceipts, errorMessage)) {
        return false;
    }

    std::string record;

    if (!serializeReceipt(
            receipt, record, errorMessage, existingReceipts)) {
        return false;
    }

    std::ofstream file(RECEIPT_FILE, std::ios::app);

    if (!file.is_open()) {
        errorMessage = "Unable to open payment receipt file.";
        return false;
    }

    file << record << '\n';
    file.close();

    if (!file) {
        errorMessage = "Failed to save payment receipt.";
        return false;
    }

    return true;
}

// Find a successful receipt using its PNR.
bool findReceiptByPNR(
    const std::string& pnr,
    PaymentReceipt& receipt,
    std::string& errorMessage) {

    errorMessage.clear();

    if (!isValidPaymentPNR(pnr)) {
        errorMessage = "Invalid PNR.";
        return false;
    }

    std::vector<PaymentReceipt> receipts;

    if (!loadReceipts(receipts, errorMessage)) {
        return false;
    }

    for (const PaymentReceipt& existing : receipts) {
        if (existing.pnr == pnr &&
            existing.status == "SUCCESS") {

            receipt = existing;
            return true;
        }
    }

    errorMessage = "Successful payment receipt not found for this PNR.";
    return false;
}

// Generate current local date and time.
std::string getCurrentDateTime() {

    const std::time_t now = std::time(nullptr);

    std::tm* localTime = std::localtime(&now);

    if (!localTime) {
        return "";
    }

    std::ostringstream output;

    output << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");

    return output.str();
}

}
