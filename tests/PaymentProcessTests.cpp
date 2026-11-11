#include "Payment.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace {
bool runPayment(
    double fare,
    const std::string& inputText,
    std::string& method,
    std::string& status,
    std::string& outputText) {
    std::istringstream input(inputText);
    std::ostringstream output;
    std::streambuf* oldInput = std::cin.rdbuf(input.rdbuf());
    std::streambuf* oldOutput = std::cout.rdbuf(output.rdbuf());

    const bool result = processPayment(fare, method, status);

    std::cin.rdbuf(oldInput);
    std::cout.rdbuf(oldOutput);
    std::cin.clear();
    outputText = output.str();
    return result;
}
}

int main() {
    std::string method;
    std::string status;
    std::string output;

    // Invalid monetary values must be rejected before reading user input.
    for (double fare : {
             0.0,
             -10.0,
             std::numeric_limits<double>::infinity(),
             std::numeric_limits<double>::quiet_NaN()}) {
        assert(!runPayment(fare, "", method, status, output));
        assert(method.empty());
        assert(status == "FAILED");
        assert(output.find("Invalid payment amount.") != std::string::npos);
    }

    // Invalid method choices and non-numeric input are rejected.
    for (const std::string choice : {"x", "0", "4"}) {
        assert(!runPayment(500.0, choice, method, status, output));
        assert(method.empty());
        assert(status == "FAILED");
        assert(output.find("Invalid payment method.") != std::string::npos);
    }

    // All supported methods can complete a successful simulated payment.
    const std::pair<std::string, std::string> methods[] = {
        {"1 1", "UPI"},
        {"2 1", "Card"},
        {"3 1", "Net Banking"}
    };

    for (const auto& testCase : methods) {
        assert(runPayment(
            1250.50, testCase.first, method, status, output));
        assert(method == testCase.second);
        assert(status == "SUCCESS");
        assert(output.find("Payment Successful!") != std::string::npos);
    }

    // A valid method with a failed simulated payment returns false.
    assert(!runPayment(800.0, "1 2", method, status, output));
    assert(method == "UPI");
    assert(status == "FAILED");
    assert(output.find("Payment Failed!") != std::string::npos);
    assert(output.find("NOT been confirmed") != std::string::npos);

    // Invalid result choices and non-numeric result input are rejected.
    for (const std::string choice : {"1 x", "1 0", "1 3"}) {
        assert(!runPayment(800.0, choice, method, status, output));
        assert(method == "UPI");
        assert(status == "FAILED");
        assert(output.find("Invalid payment result.") != std::string::npos);
    }

    std::cout << "Payment processing tests passed.\n";
    std::cout << "Invalid amounts: PASS\n";
    std::cout << "Invalid payment methods: PASS\n";
    std::cout << "UPI, card and net banking success: PASS\n";
    std::cout << "Simulated payment failure: PASS\n";
    std::cout << "Invalid payment results: PASS\n";
    return 0;
}
