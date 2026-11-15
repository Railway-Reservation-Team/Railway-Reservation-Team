# SAD Supplement — UML Sequence Diagrams
## Railway Reservation System | Team 12

These sequence diagrams supplement the submitted SAD. They describe the current C++ implementation in `src/main.cpp`, `BookingManager.cpp`, `Cancellation.cpp`, `Payment.cpp`, and the related modules. They are source diagrams for inclusion in the final SAD PDF.

## 1. Passenger ticket booking

```mermaid
sequenceDiagram
    actor Passenger
    participant Main as Console Application (main.cpp)
    participant Auth as AuthManager
    participant Search as TrainSearch / TrainData
    participant Validate as Validation
    participant Details as PassengerDetails
    participant Fare as FareCalculator
    participant Payment as Payment
    participant Booking as BookingManager
    participant TrainFile as data/trains.dat
    participant BookingFile as data/bookings.dat
    participant Receipt as PaymentReceiptManager

    Passenger->>Main: Select Passenger Booking
    Main->>Auth: isSessionActive()
    Auth-->>Main: Active session / reject
    Passenger->>Main: Enter source, destination, journey date
    Main->>Validate: Validate route/date/menu input
    Validate-->>Main: Valid / error
    Main->>Search: Search matching trains
    Search-->>Main: Matching train options and class availability
    Passenger->>Main: Select train, class and passenger count
    Main->>Validate: Validate selection and passenger count
    Validate-->>Main: Valid / error
    Main->>Details: collect passenger details
    Details-->>Main: Passenger records
    Main->>Fare: calculateFare(train, class, seats)
    Fare-->>Main: Total fare
    Main-->>Passenger: Display booking summary
    Passenger->>Main: Confirm booking
    Main->>Auth: isSessionActive()
    Auth-->>Main: Active / expired
    Main->>Payment: processPayment(totalFare)
    alt Payment fails
        Payment-->>Main: FAILED
        Main->>Receipt: Save failed-payment receipt (when applicable)
        Receipt-->>Main: Receipt result
        Main-->>Passenger: Booking not confirmed
    else Payment succeeds
        Payment-->>Main: SUCCESS
        Main->>Booking: createBooking(train, class, passengers, fare)
        Booking->>TrainFile: Read current train and seat inventory
        TrainFile-->>Booking: Train records
        Booking->>Booking: Validate, generate unique 10-digit PNR
        alt Seats available
            Booking->>TrainFile: Atomically update seat count
        else Insufficient seats
            Booking->>Booking: Assign FIFO waitlist position
        end
        Booking->>BookingFile: Persist booking and status
        Booking-->>Main: PNR and booking result
        Main-->>Passenger: Confirmation / waitlist status and ticket details
    end
```

**Important implementation note:** Payment is simulated. The application processes payment before calling `BookingManager::createBooking()`. If payment succeeds but booking persistence subsequently fails, the console reports that a manual payment reversal may be required; do not describe this as a fully atomic external payment transaction.

## 2. Ticket cancellation and seat restoration

```mermaid
sequenceDiagram
    actor Passenger
    participant Main as Console Application (main.cpp)
    participant Auth as AuthManager
    participant Cancel as Cancellation
    participant Booking as data/bookings.dat
    participant Train as data/trains.dat
    participant CancelFile as data/cancellations.dat

    Passenger->>Main: Choose Cancel Ticket
    Main->>Auth: Check active passenger session
    Auth-->>Main: Active session / reject
    Passenger->>Main: Enter PNR
    Main-->>Passenger: Ask for cancellation confirmation
    Passenger->>Main: Confirm Y
    Main->>Cancel: cancelBooking(PNR)
    Cancel->>Booking: Read and locate booking
    Booking-->>Cancel: Booking record and status
    Cancel->>Train: Read matching train/date/class
    Train-->>Cancel: Train and departure details
    Cancel->>Cancel: Validate eligibility and calculate refund
    alt Invalid PNR, ineligible or already cancelled
        Cancel-->>Main: Failure reason
        Main-->>Passenger: Cancellation failed; show reason
    else Eligible cancellation
        Cancel->>Cancel: Prepare updated booking, inventory and cancellation records
        Cancel->>Booking: Atomically persist Cancelled status
        Cancel->>Train: Restore seats / process waitlist promotion
        Cancel->>CancelFile: Persist cancellation and refund details
        Cancel-->>Main: Success, refund and cancellation charge
        Main-->>Passenger: Display cancelled status and refund details
    end
```

## 3. Diagram-to-code mapping

| Sequence participant | Implementation |
|---|---|
| Console Application | `src/main.cpp` |
| AuthManager | `src/AuthManager.cpp` |
| TrainSearch / TrainData | `src/TrainSearch.cpp`, `src/TrainData.h` |
| Validation | `src/Validation.cpp` |
| PassengerDetails | `src/PassengerDetails.cpp` |
| FareCalculator | `src/FareCalculator.cpp` |
| Payment | `src/Payment.cpp` |
| BookingManager | `src/BookingManager.cpp` |
| Cancellation | `src/Cancellation.cpp` |
| Persistent records | `data/trains.dat`, `data/bookings.dat`, `data/cancellations.dat` |
| Payment receipt | `src/PaymentReceiptManager.cpp` |

The diagrams are implementation-oriented. Before submitting the final SAD PDF, render both Mermaid diagrams as UML sequence diagrams and insert them under the SAD detailed-design section. Review the final PDF to ensure labels remain readable.
