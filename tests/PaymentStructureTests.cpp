#include "Payment.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

int main() {
    const Payment valid{
        "TXN1001",
        "1234567890",
        2500.50,
        "UPI",
        "SUCCESS",
        "2026-11-05 20:00:00"
    };

    std::string record;
    std::string error;

    // Valid payment serialization and deserialization.
    assert(serializePayment(valid, record, error));
    Payment restored;
    assert(parsePaymentRecord(record, restored, error));
    assert(restored.transactionId == valid.transactionId);
    assert(restored.pnr == valid.pnr);
    assert(std::fabs(restored.amount - valid.amount) < 0.001);
    assert(restored.paymentMethod == valid.paymentMethod);
    assert(restored.status == valid.status);
    assert(restored.paymentDateTime == valid.paymentDateTime);

    // All supported payment statuses.
    for (const std::string status : {"PENDING", "SUCCESS", "FAILED"}) {
        Payment candidate = valid;
        candidate.status = status;
        assert(serializePayment(candidate, record, error));
    }

    // All supported payment methods.
    for (const std::string method : {"UPI", "Card", "Net Banking", "Cash"}) {
        Payment candidate = valid;
        candidate.paymentMethod = method;
        assert(serializePayment(candidate, record, error));
    }

    // Invalid status and payment method.
    Payment invalid = valid;
    invalid.status = "REFUNDED";
    assert(!serializePayment(invalid, record, error));

    invalid = valid;
    invalid.status = "success";
    assert(!serializePayment(invalid, record, error));

    invalid = valid;
    invalid.paymentMethod = "Crypto";
    assert(!serializePayment(invalid, record, error));

    // PNR must be exactly 10 numeric digits.
    for (const std::string pnr : {
             "123456789", "12345678901", "12345A7890", "123 567890", ""}) {
        invalid = valid;
        invalid.pnr = pnr;
        assert(!serializePayment(invalid, record, error));
    }

    // Reject invalid calendar dates and times.
    for (const std::string dateTime : {
             "2026-02-29 20:00:00",
             "2025-02-29 20:00:00",
             "2026-13-01 20:00:00",
             "2026-04-31 20:00:00",
             "2026-11-05 24:00:00",
             "2026-11-05 20:60:00",
             "2026-11-05",
             "05-11-2026 20:00:00"}) {
        invalid = valid;
        invalid.paymentDateTime = dateTime;
        assert(!serializePayment(invalid, record, error));
    }

    // Accept a leap-day date in a leap year.
    invalid = valid;
    invalid.paymentDateTime = "2024-02-29 23:59:59";
    assert(serializePayment(invalid, record, error));

    // Reject duplicate transaction IDs against already-loaded records.
    const std::vector<Payment> existingPayments{valid};
    invalid = valid;
    invalid.amount = 3000.00;
    assert(!serializePayment(invalid, record, error, existingPayments));

    // Reject a duplicate transaction ID while parsing stored records.
    const std::string duplicateRecord =
        "TXN1001|1234567890|2500.50|UPI|SUCCESS|2026-11-05 20:00:00";
    Payment duplicateParsed;
    assert(!parsePaymentRecord(
        duplicateRecord, duplicateParsed, error, existingPayments));

    // Invalid amount and malformed record.
    invalid = valid;
    invalid.amount = -100.0;
    assert(!serializePayment(invalid, record, error));

    invalid.amount = 0.0;
    assert(!serializePayment(invalid, record, error));

    Payment invalidRecord;
    assert(!parsePaymentRecord("TXN1001|1234567890|ABC", invalidRecord, error));

    std::cout << "Payment structure tests passed.\n";
    std::cout << "Serialization/deserialization: PASS\n";
    std::cout << "Status and payment method validation: PASS\n";
    std::cout << "PNR format validation: PASS\n";
    std::cout << "Date/time validation: PASS\n";
    std::cout << "Duplicate transaction ID validation: PASS\n";
    std::cout << "Amount and malformed record validation: PASS\n";
    return 0;
}
