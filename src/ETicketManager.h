
#ifndef ETICKET_MANAGER_H
#define ETICKET_MANAGER_H

#include "BookingManager.h"
#include "Train.h"
#include "PassengerDetails.h"
#include <string>
#include <vector>

// Stores the complete information displayed on a generated e-ticket.
struct ETicket {
    std::string pnr;
    std::string trainNumber;
    std::string trainName;
    std::string source;
    std::string destination;
    std::string journeyDate;
    std::string departureTime;
    std::string arrivalTime;
    std::string travelClass;
    std::vector<Passenger> passengers;
    double totalFare = 0.0;
    std::string bookingStatus;
};

namespace ETicketManager {

    // Decode passenger details stored in bookings.dat.
    bool decodePassengers(
        const std::string& encodedDetails,
        std::vector<Passenger>& passengers,
        std::string& errorMessage
    );

    // Generate a ticket only for a confirmed booking.
    bool generateTicket(
        const std::string& pnr,
        ETicket& ticket,
        std::string& errorMessage
    );

    // Format ticket into readable text.
    bool formatTicket(
        const ETicket& ticket,
        std::string& ticketText,
        std::string& errorMessage
    );

    // Export ticket into a text file.
    bool exportTicket(
        const ETicket& ticket,
        std::string& filePath,
        std::string& errorMessage
    );

    // Display the ticket on the console.
    bool displayTicket(
        const ETicket& ticket,
        std::string& errorMessage
    );

}

#endif
