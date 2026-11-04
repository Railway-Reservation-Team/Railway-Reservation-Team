
#include "Payment.h"

#include <iostream>
#include <iomanip>
#include <cmath>

bool processPayment(
    double totalFare,
    std::string& paymentMethod,
    std::string& paymentStatus
) {
    int methodChoice;
    int resultChoice;

    // Initialize payment details.
    paymentMethod.clear();
    paymentStatus = "FAILED";

    // Validate the fare before processing payment.
    if (!std::isfinite(totalFare) || totalFare <= 0.0) {
        std::cout << "\nInvalid payment amount.\n";
        return false;
    }

    std::cout << std::fixed << std::setprecision(2);

    std::cout << "\n========== PAYMENT DETAILS ==========\n";
    std::cout << "Total Payable Amount: Rs. " << totalFare << '\n';

    std::cout << "\nSelect Payment Method:\n";
    std::cout << "1. UPI\n";
    std::cout << "2. Debit/Credit Card\n";
    std::cout << "3. Net Banking\n";

    std::cout << "Enter payment choice (1-3): ";

    if (!(std::cin >> methodChoice) ||
        methodChoice < 1 || methodChoice > 3) {
        std::cout << "\nInvalid payment method.\n";
        return false;
    }

    switch (methodChoice) {
        case 1:
            paymentMethod = "UPI";
            break;
        case 2:
            paymentMethod = "Card";
            break;
        case 3:
            paymentMethod = "Net Banking";
            break;
    }

    std::cout << "\nPayment Method: " << paymentMethod << '\n';
    std::cout << "Amount: Rs. " << totalFare << '\n';

    std::cout << "\nSIMULATED PAYMENT RESULT\n";
    std::cout << "1. Simulate Successful Payment\n";
    std::cout << "2. Simulate Failed Payment\n";

    std::cout << "Enter payment result (1-2): ";

    if (!(std::cin >> resultChoice) ||
        resultChoice < 1 || resultChoice > 2) {
        std::cout << "\nInvalid payment result.\n";
        return false;
    }

    if (resultChoice == 1) {
        paymentStatus = "SUCCESS";

        std::cout << "\nPayment Successful!\n";
        return true;
    }

    std::cout << "\nPayment Failed!\n";
    std::cout << "Your booking has NOT been confirmed.\n";

    return false;
}
