#ifndef PAYMENT_H
#define PAYMENT_H

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

struct Payment {
    std::string transactionId;
    std::string pnr;
    double amount = 0.0;
    std::string paymentMethod;
    std::string status;
    std::string paymentDateTime;
};

// Check whether a field can safely be stored in a pipe-separated record.
inline bool isValidPaymentField(const std::string& value) {
    return !value.empty() &&
           value.find('|') == std::string::npos &&
           value.find('\n') == std::string::npos &&
           value.find('\r') == std::string::npos;
}

// Convert a Payment object into a local text-file record.
// Format: TransactionID|PNR|Amount|Method|Status|DateTime
inline bool serializePayment(
    const Payment& payment,
    std::string& record,
    std::string& errorMessage) {

    errorMessage.clear();
    record.clear();

    if (!isValidPaymentField(payment.transactionId) ||
        !isValidPaymentField(payment.pnr) ||
        !isValidPaymentField(payment.paymentMethod) ||
        !isValidPaymentField(payment.status) ||
        !isValidPaymentField(payment.paymentDateTime)) {
        errorMessage = "Payment fields must be non-empty and cannot contain record delimiters.";
        return false;
    }

    if (!std::isfinite(payment.amount) || payment.amount <= 0.0) {
        errorMessage = "Payment amount must be a positive finite value.";
        return false;
    }

    std::ostringstream output;
    output << payment.transactionId << '|'
           << payment.pnr << '|'
           << std::fixed << std::setprecision(2) << payment.amount << '|'
           << payment.paymentMethod << '|'
           << payment.status << '|'
           << payment.paymentDateTime;

    record = output.str();
    return true;
}

// Convert a pipe-separated local text-file record back into a Payment object.
inline bool parsePaymentRecord(
    const std::string& record,
    Payment& payment,
    std::string& errorMessage) {

    errorMessage.clear();

    std::stringstream input(record);
    std::vector<std::string> fields;
    std::string field;

    while (std::getline(input, field, '|')) {
        fields.push_back(field);
    }

    if (fields.size() != 6) {
        errorMessage = "Invalid payment record: expected 6 fields.";
        return false;
    }

    Payment parsedPayment;

    parsedPayment.transactionId = fields[0];
    parsedPayment.pnr = fields[1];
    parsedPayment.paymentMethod = fields[3];
    parsedPayment.status = fields[4];
    parsedPayment.paymentDateTime = fields[5];

    try {
        std::size_t position = 0;
        parsedPayment.amount = std::stod(fields[2], &position);

        if (position != fields[2].size()) {
            errorMessage = "Invalid payment amount.";
            return false;
        }
    } catch (...) {
        errorMessage = "Invalid payment amount.";
        return false;
    }

    std::string serialized;
    if (!serializePayment(parsedPayment, serialized, errorMessage)) {
        return false;
    }

    payment = parsedPayment;
    return true;
}

#endif
