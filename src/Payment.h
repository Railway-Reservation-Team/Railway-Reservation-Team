
#ifndef PAYMENT_H
#define PAYMENT_H

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

// Payment processing function
bool processPayment(
    double totalFare,
    std::string& paymentMethod,
    std::string& paymentStatus
);

// Payment data structure
struct Payment {
    std::string transactionId;
    std::string pnr;
    double amount = 0.0;
    std::string paymentMethod;
    std::string status;
    std::string paymentDateTime;
};

// Validate payment date and time: YYYY-MM-DD HH:MM:SS
inline bool isValidPaymentDateTime(const std::string& value) {
    if (value.size() != 19 ||
        value[4] != '-' || value[7] != '-' ||
        value[10] != ' ' || value[13] != ':' ||
        value[16] != ':') {
        return false;
    }

    for (std::size_t i = 0; i < value.size(); ++i) {
        if (i != 4 && i != 7 && i != 10 && i != 13 && i != 16 &&
            (value[i] < '0' || value[i] > '9')) {
            return false;
        }
    }

    const int year = std::stoi(value.substr(0, 4));
    const int month = std::stoi(value.substr(5, 2));
    const int day = std::stoi(value.substr(8, 2));
    const int hour = std::stoi(value.substr(11, 2));
    const int minute = std::stoi(value.substr(14, 2));
    const int second = std::stoi(value.substr(17, 2));

    if (year < 1 || month < 1 || month > 12 ||
        hour > 23 || minute > 59 || second > 59) {
        return false;
    }

    const int daysInMonth[] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    int maxDays = daysInMonth[month - 1];

    const bool leapYear = (year % 400 == 0) ||
                          (year % 4 == 0 && year % 100 != 0);

    if (month == 2 && leapYear) {
        maxDays = 29;
    }

    return day >= 1 && day <= maxDays;
}

inline bool isValidPaymentPNR(const std::string& pnr) {
    if (pnr.size() != 10) {
        return false;
    }

    for (unsigned char ch : pnr) {
        if (ch < '0' || ch > '9') {
            return false;
        }
    }

    return true;
}

inline bool isValidPaymentMethod(const std::string& method) {
    return method == "UPI" ||
           method == "Card" ||
           method == "Net Banking" ||
           method == "Cash";
}

inline bool isValidPaymentStatus(const std::string& status) {
    return status == "PENDING" ||
           status == "SUCCESS" ||
           status == "FAILED";
}

inline bool isValidPaymentField(const std::string& value) {
    return !value.empty() &&
           value.find('|') == std::string::npos &&
           value.find('\n') == std::string::npos &&
           value.find('\r') == std::string::npos;
}

inline bool isUniqueTransactionId(
    const std::string& transactionId,
    const std::vector<Payment>& existingPayments) {

    for (const Payment& existing : existingPayments) {
        if (existing.transactionId == transactionId) {
            return false;
        }
    }

    return true;
}

// Serialize payment into a pipe-separated record.
inline bool serializePayment(
    const Payment& payment,
    std::string& record,
    std::string& errorMessage,
    const std::vector<Payment>& existingPayments = {}) {

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

    if (!isValidPaymentPNR(payment.pnr)) {
        errorMessage = "PNR must contain exactly 10 digits.";
        return false;
    }

    if (!isValidPaymentMethod(payment.paymentMethod)) {
        errorMessage = "Unsupported payment method.";
        return false;
    }

    if (!isValidPaymentStatus(payment.status)) {
        errorMessage = "Invalid payment status.";
        return false;
    }

    if (!isValidPaymentDateTime(payment.paymentDateTime)) {
        errorMessage = "Payment date and time must use YYYY-MM-DD HH:MM:SS and be valid.";
        return false;
    }

    if (!std::isfinite(payment.amount) || payment.amount <= 0.0) {
        errorMessage = "Payment amount must be a positive finite value.";
        return false;
    }

    if (!isUniqueTransactionId(payment.transactionId, existingPayments)) {
        errorMessage = "Duplicate transaction ID.";
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

// Parse a pipe-separated payment record.
inline bool parsePaymentRecord(
    const std::string& record,
    Payment& payment,
    std::string& errorMessage,
    const std::vector<Payment>& existingPayments = {}) {

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

    if (!serializePayment(
            parsedPayment, serialized, errorMessage, existingPayments)) {
        return false;
    }

    payment = parsedPayment;
    return true;
}

#endif
