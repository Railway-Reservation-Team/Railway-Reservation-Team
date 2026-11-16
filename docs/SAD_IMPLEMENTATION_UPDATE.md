# Software Architecture and Design — Implementation Update
## Railway Reservation System | Team 12

**Baseline:** TEAM12_SAD_Railway_Reservation_System.pdf  
**Implementation:** C++17, standalone console application, local file persistence.

## 1. Architecture overview

The system uses a modular, layered console architecture. `main.cpp` coordinates user interaction and delegates work to focused modules. Business logic is separated into authentication, search, validation, fare, booking, payment, cancellation, inventory and administration modules. Persistent records are stored in local files under `data/`.

This is a standalone single-user application per running instance. It is not a network-hosted service and does not integrate a live payment gateway or external database.

## 2. Implemented modules

| Module | Main responsibility | Implementation |
|---|---|---|
| AuthManager | Passenger registration/login, salted password hashing, lockout and session timeout | `src/AuthManager.*` |
| AdminAuthManager | First-time admin setup, admin login/logout and protected admin entry | `src/AdminAuthManager.*` |
| TrainData / TrainSearch | Load train records and search by route/date; exclude deactivated trains from passenger search | `src/TrainData.h`, `src/TrainSearch.*` |
| Validation | Validate user input, dates, passenger counts and menu selections | `src/Validation.*` |
| PassengerDetails | Collect and validate passenger information | `src/PassengerDetails.*` |
| FareCalculator | Calculate fare for selected class and passenger count | `src/FareCalculator.*` |
| SeatInventory | Check availability, hold/release/confirm seats and concurrency-related inventory operations | `src/SeatInventory.*` |
| BookingManager | Validate booking, generate unique 10-digit PNR, update inventory, persist booking and waitlist state | `src/BookingManager.*` |
| Payment | Simulated payment processing | `src/Payment.*` |
| PaymentReceiptManager | Generate transaction IDs and persist payment receipts | `src/PaymentReceiptManager.*` |
| Cancellation | Validate cancellation, calculate fee/refund, update records and restore inventory | `src/Cancellation.*` |
| ETicketManager | Generate/save e-ticket output | `src/ETicketManager.*` |
| AdminTrainManager / AdminTrainMenu | Add, view, update, delete and deactivate train records behind admin authentication | `src/AdminTrainManager.*`, `src/AdminTrainMenu.*` |
| AdminFareReservationManager | Administrator fare and reservation operations | `src/AdminFareReservationManager.*` |
| FileStore / atomic replacement helpers | Validated local record persistence and safe file replacement utilities | `src/FileStore.h`, `src/AtomicFileReplace.h` |

## 3. Data persistence

Primary data files include:
- `data/users.dat` — passenger account records with salt and password hash fields.
- `data/trains.dat` — train schedule, fare, availability and active/deactivated state.
- `data/bookings.dat` — PNR, train/date/class, passenger details, passenger count, fare, booking status and waitlist position where applicable.
- `data/cancellations.dat` — cancellation/refund records.
- Payment and authentication audit records are stored separately by their respective modules.

The application uses relative paths under the repository/application working directory. Run it from the project root or packaged application directory so its `data/` paths resolve correctly.

## 4. Main interactions

1. Passenger registration and login.
2. Search train by source, destination and journey date.
3. Select travel class and check seat availability.
4. Enter passenger details and calculate fare.
5. Confirm booking and complete simulated payment.
6. Create booking, generate PNR and update seat inventory or waitlist state.
7. Retrieve booking history and generate e-ticket output.
8. Cancel eligible booking, calculate refund and restore inventory.
9. Administrator authentication and protected train/fare/reservation operations.

## 5. Security and error handling

- Passenger and administrator authentication are separate.
- Passenger passwords use salted PBKDF2-HMAC-SHA-256 via OpenSSL; do not describe this as plain SHA-256.
- Passenger sessions include an inactivity timeout and failed-login lockout.
- Inputs are validated before relevant operations.
- Booking and cancellation paths validate records and report failures.
- File replacement helpers are used to reduce partial-write risk in critical record updates.
- Administrator actions and authentication events have audit logging support.

## 6. Sequence diagrams

Two implementation-oriented sequence diagrams are maintained in [SAD_SEQUENCE_DIAGRAMS.md](SAD_SEQUENCE_DIAGRAMS.md):
1. Passenger ticket booking, including simulated payment and PNR persistence.
2. Ticket cancellation, refund calculation and seat restoration.

These diagrams should be rendered and inserted into the final SAD PDF before submission.

## 7. Known architectural limitations and scope decisions

- One local application instance is the supported deployment model; no cross-machine shared database or hosted multi-user service is provided.
- Payment is simulated; no real payment gateway or automatic external reversal exists.
- Availability is sampled application-startup responsiveness, not continuous hosted-service uptime. NFR-002 remains unverified pending mentor-approved measurement scope and genuine evidence.
- The Test Plan names Jenkins, GoogleTest and SonarQube/equivalents. The actual pipeline uses GitHub Actions, CMake/CTest, custom C++ test executables, Cppcheck and gcovr. Final document revisions should record this tooling deviation and obtain evaluator acceptance.
- SRS v1.0 and the Test Plan contain conflicting NFR labels/targets and additional features not individually numbered in the SRS. Refer to [REQUIREMENTS_GAP_REPORT.md](REQUIREMENTS_GAP_REPORT.md) and [TEST_TRACEABILITY.md](TEST_TRACEABILITY.md) for the current reconciliation.

## 8. Revision note

This document supersedes the earlier brief `SAD_Updates.md` implementation note. It records the current repository architecture and explicitly identifies the diagrams and PDF integration step still required.
