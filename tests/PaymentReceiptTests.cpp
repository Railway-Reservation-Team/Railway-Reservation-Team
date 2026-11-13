
#include "PaymentReceiptManager.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main() {
    const std::string testFile = "data/payment_receipts.dat";

    // Start with an isolated receipt file.
    std::remove(testFile.c_str());

    std::string error;
    std::string transactionId;

    // Test transaction ID generation.
    assert(PaymentReceiptManager::generateTransactionId(
        transactionId, error));

    assert(transactionId.size() == 15);
    assert(transactionId.substr(0, 3) == "TXN");

    std::cout << "[PASS] Transaction ID generation\n";

    PaymentReceipt valid{
        transactionId,
        "1234567890",
        2500.50,
        "UPI",
        "SUCCESS",
        "2026-11-11 16:30:00"
    };

    // Test serialization.
    std::string record;

    assert(PaymentReceiptManager::serializeReceipt(
        valid, record, error));

    std::cout << "[PASS] Receipt serialization\n";

    // Test parsing.
    PaymentReceipt restored;

    assert(PaymentReceiptManager::parseReceipt(
        record, restored, error));

    assert(restored.transactionId == valid.transactionId);
    assert(restored.pnr == valid.pnr);
    assert(std::fabs(restored.amount - valid.amount) < 0.001);
    assert(restored.paymentMethod == valid.paymentMethod);
    assert(restored.status == valid.status);

    std::cout << "[PASS] Receipt parsing\n";

    // Reject duplicate transaction IDs.
    std::vector<PaymentReceipt> existing{valid};

    PaymentReceipt duplicate = valid;
    duplicate.amount = 3000.0;

    assert(!PaymentReceiptManager::serializeReceipt(
        duplicate, record, error, existing));

    std::cout << "[PASS] Duplicate transaction ID rejection\n";

    // Reject successful receipts without a PNR.
    PaymentReceipt invalid = valid;
    invalid.pnr.clear();

    assert(!PaymentReceiptManager::serializeReceipt(
        invalid, record, error));

    std::cout << "[PASS] Successful receipt requires valid PNR\n";

    // Reject unsupported status.
    invalid = valid;
    invalid.status = "REFUNDED";

    assert(!PaymentReceiptManager::serializeReceipt(
        invalid, record, error));

    std::cout << "[PASS] Invalid status rejection\n";

    // Reject invalid amount.
    invalid = valid;
    invalid.amount = -100.0;

    assert(!PaymentReceiptManager::serializeReceipt(
        invalid, record, error));

    std::cout << "[PASS] Invalid amount rejection\n";

    // Reject malformed date.
    invalid = valid;
    invalid.paymentDateTime = "2026-15-99 25:70:00";

    assert(!PaymentReceiptManager::serializeReceipt(
        invalid, record, error));

    std::cout << "[PASS] Invalid date/time rejection\n";

    // Test persistent receipt storage.
    assert(PaymentReceiptManager::saveReceipt(valid, error));

    std::cout << "[PASS] Receipt persistence\n";

    // Test retrieval using PNR.
    PaymentReceipt retrieved;

    assert(PaymentReceiptManager::findReceiptByPNR(
        valid.pnr, retrieved, error));

    assert(retrieved.transactionId == valid.transactionId);
    assert(retrieved.pnr == valid.pnr);

    std::cout << "[PASS] Receipt retrieval using PNR\n";

    // Test invalid PNR lookup.
    assert(!PaymentReceiptManager::findReceiptByPNR(
        "INVALID", retrieved, error));

    std::cout << "[PASS] Invalid PNR rejection\n";

    // Verify stored record exists.
    std::ifstream file(testFile);

    assert(file.is_open());

    std::string savedRecord;
    assert(std::getline(file, savedRecord));

    file.close();

    assert(!savedRecord.empty());

    std::cout << "[PASS] Stored receipt record verification\n";

    // Clean up test data.
    std::remove(testFile.c_str());

    std::cout << "\nALL PAYMENT RECEIPT TESTS PASSED!\n";

    return 0;
}
