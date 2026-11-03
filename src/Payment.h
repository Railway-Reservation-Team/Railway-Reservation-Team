
#ifndef PAYMENT_H
#define PAYMENT_H

#include <string>

bool processPayment(
    double totalFare,
    std::string& paymentMethod,
    std::string& paymentStatus
);

#endif
