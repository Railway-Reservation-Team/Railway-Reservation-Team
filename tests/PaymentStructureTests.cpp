#include "Payment.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>

int main() {
    Payment original{
        "TXN1001",
        "1234567890",
        2500.50,
        "UPI",
        "SUCCESS",
        "2026-11-05 20:00:00"
    };

    std::string record;
    std::string error;

    // Test serialization.
    assert(serializePayment(original, record, error));

    // Test deserialization.
    Payment restored;
    assert(parsePaymentRecord(record, restored, error));

    // Verify that all fields are preserved.
    assert(restored.transactionId == original.transactionId);
    assert(restored.pnr == original.pnr);
    assert(std::fabs(restored.amount - original.amount) < 0.001);
    assert(restored.paymentMethod == original.paymentMethod);
    assert(restored.status == original.status);
    assert(restored.paymentDateTime == original.paymentDateTime);

    // Test invalid amount.
    Payment invalid = original;
    invalid.amount = -100.0;

    assert(!serializePayment(invalid, record, error));

    // Test invalid record.
    Payment invalidRecord;
    assert(!parsePaymentRecord("TXN1001|1234567890|ABC", invalidRecord, error));

    std::cout << "Payment structure tests passed.\n";
    std::cout << "Serialization and deserialization: PASS\n";
    std::cout << "Field preservation: PASS\n";
    std::cout << "Invalid amount validation: PASS\n";
    std::cout << "Invalid record validation: PASS\n";

    return 0;
}
