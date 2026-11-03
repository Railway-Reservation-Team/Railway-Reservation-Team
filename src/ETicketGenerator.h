#ifndef ETICKET_GENERATOR_H
#define ETICKET_GENERATOR_H

#include <string>

namespace ETicketGenerator {
    bool generateETicket(
        const std::string& pnr,
        std::string& ticketPath,
        std::string& errorMessage
    );
}

#endif
