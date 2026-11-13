
#ifndef CANCELLATION_H
#define CANCELLATION_H

#include <string>

// Handles booking cancellation and refund calculation.
namespace Cancellation {

    bool cancelBooking(
        const std::string& pnr,
        double& refundAmount,
        double& cancellationCharge,
        std::string& errorMessage
    );

}

#endif
