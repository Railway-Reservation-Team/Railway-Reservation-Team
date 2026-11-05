
#ifndef CANCELLATION_H
#define CANCELLATION_H

#include <string>

namespace Cancellation {

    bool cancelBooking(
        const std::string& pnr,
        double& refundAmount,
        double& cancellationCharge,
        std::string& errorMessage
    );

}

#endif
