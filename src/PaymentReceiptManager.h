
#ifndef PAYMENT_RECEIPT_MANAGER_H
#define PAYMENT_RECEIPT_MANAGER_H

#include <string>
#include <vector>

struct PaymentReceipt {
    std::string transactionId;
    std::string pnr;
    double amount = 0.0;
    std::string paymentMethod;
    std::string status;
    std::string paymentDateTime;
};

namespace PaymentReceiptManager {

    // Generate a unique transaction reference.
    bool generateTransactionId(
        std::string& transactionId,
        std::string& errorMessage
    );

    // Validate and serialize a receipt record.
    bool serializeReceipt(
        const PaymentReceipt& receipt,
        std::string& record,
        std::string& errorMessage,
        const std::vector<PaymentReceipt>& existingReceipts = {}
    );

    // Parse a stored receipt record.
    bool parseReceipt(
        const std::string& record,
        PaymentReceipt& receipt,
        std::string& errorMessage,
        const std::vector<PaymentReceipt>& existingReceipts = {}
    );

    // Save a receipt to the persistent receipt file.
    bool saveReceipt(
        const PaymentReceipt& receipt,
        std::string& errorMessage
    );

    // Retrieve a successful receipt using its PNR.
    bool findReceiptByPNR(
        const std::string& pnr,
        PaymentReceipt& receipt,
        std::string& errorMessage
    );

    // Create a timestamp in YYYY-MM-DD HH:MM:SS format.
    std::string getCurrentDateTime();

}

#endif
