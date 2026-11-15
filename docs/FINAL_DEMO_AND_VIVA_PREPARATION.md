# Final Demo and Viva Preparation — Railway Reservation System
## Team 12 | C++17 | PES University

This is a preparation guide, not a substitute for running the current application. Rehearse against the exact build that will be shown. Use disposable demo data and do not expose real passwords or personal records.

## 1. Project in one minute

The Railway Reservation System is a standalone, single-user-per-running-instance, console-based application developed in C++17. It allows passengers to register and log in, search trains by route and date, check class-wise availability, enter passenger details, calculate fares, make a simulated payment, create a reservation with a unique 10-digit PNR, view booking history, generate an e-ticket and cancel eligible bookings with refund calculation and seat restoration. An authenticated administrator can manage train records and fares and access reservation-related operations.

The application persists records in local files under `data/`; it does not use a remote database, network server or real payment gateway.

## 2. Core architecture

`main.cpp` is the console coordinator. It reads menu/input and calls dedicated modules rather than implementing all business logic itself.

| Module | Explain it like this |
|---|---|
| AuthManager | Registers and authenticates passengers, verifies salted password hashes, tracks failed logins and session activity |
| AdminAuthManager | Keeps administrator authentication separate from passenger login and protects admin entry |
| TrainData / TrainSearch | Reads train records and finds trains matching source, destination and journey date |
| Validation | Rejects malformed or out-of-range input before business logic uses it |
| SeatInventory | Manages seat availability, holds and inventory-related operations |
| PassengerDetails | Collects and validates each passenger's details |
| FareCalculator | Computes total fare for the selected train, class and passenger count |
| Payment | Simulates payment success/failure; it is not connected to a bank/payment gateway |
| BookingManager | Validates booking, generates a unique 10-digit PNR, updates inventory or waitlist state, and saves the booking |
| Cancellation | Checks cancellation eligibility, calculates fee/refund, changes booking state and restores seats |
| PaymentReceiptManager | Creates transaction IDs and records payment-attempt receipts |
| ETicketManager | Generates and saves e-ticket output |
| AdminTrainManager / AdminTrainMenu | Performs protected train add/view/update/delete/deactivate operations |
| AdminFareReservationManager | Handles administrator fare and reservation operations |
| FileStore / AtomicFileReplace | Supports validated local file persistence and safer file replacement |

## 3. Passenger booking flow — explain in order

1. The passenger registers and logs in.
2. The booking menu checks that the passenger session is active.
3. The passenger enters source, destination and journey date.
4. Validation checks the input; train data is loaded and matching trains are searched.
5. The passenger selects a train, class and passenger count.
6. Passenger details are collected and validated.
7. FareCalculator computes the total fare.
8. The passenger reviews the summary and confirms.
9. Payment is simulated. If payment fails, no booking is confirmed; a failed-payment receipt may be recorded.
10. If payment succeeds, BookingManager validates the request, generates a unique 10-digit PNR and reads current train inventory.
11. If seats are available, the inventory is reduced and the confirmed booking is persisted. If not, the booking can be stored with a waitlist position.
12. The application displays the booking result and PNR.

**Important viva nuance:** Payment occurs before BookingManager persists the reservation. If payment succeeds but booking persistence fails, the current console reports that a manual reversal may be required. Do not claim the external payment and booking are one atomic transaction.

## 4. Cancellation flow — explain in order

1. An authenticated passenger selects cancellation and enters the PNR.
2. The application asks for confirmation.
3. Cancellation validates the PNR and retrieves the booking and corresponding train.
4. It rejects invalid, already-cancelled or post-departure cancellation attempts.
5. It calculates the cancellation charge and refund using the configured time slabs.
6. It updates the booking status, restores seats and writes cancellation details.
7. The application displays the PNR, cancellation charge, refund and cancelled status.

## 5. Data storage and persistence

The project uses local data files, not MySQL/MongoDB/SQLite.

- `data/trains.dat`: train number/name, route, schedule, fares, seat counts and active state.
- `data/users.dat`: passenger account information including salt and password hash fields.
- `data/bookings.dat`: PNR, train/date/class, passenger details, count, fare, status and waitlist position where applicable.
- `data/cancellations.dat`: cancellation/refund records.
- Additional receipt and audit files are maintained by their modules.

Run the executable from the project/application directory so relative `data/` paths resolve.

## 6. Security questions you must answer

**How are passwords stored?** Passenger authentication uses a random per-user salt and PBKDF2-HMAC-SHA-256 via OpenSSL (600,000 iterations in the current implementation). The password itself is not stored as plaintext.

**Why use a salt?** It makes identical passwords produce different stored hashes and makes precomputed hash tables less useful.

**What is session timeout?** Passenger session activity is tracked; inactivity beyond the configured timeout requires re-authentication. The project also has failed-login lockout logic.

**How is admin access protected?** Admin authentication is separate from passenger authentication; the admin menu checks authenticated state before performing protected operations.

**What is input validation for?** It prevents empty, malformed, out-of-range and invalid values from reaching business logic or persistent records.

**Do not overclaim:** The SRS also contains security requirements around file-directory confinement, corruption checks and complete audit coverage. Refer to the RTM and gap report for evidence still needing verification.

## 7. Testing and CI/CD

- CMake configures the C++17 application and test executables.
- CTest runs the registered test suites.
- GitHub Actions builds/tests on Ubuntu and Windows.
- Cppcheck runs static analysis.
- gcovr generates line, function and branch coverage reports.
- Workflow artifacts include test and coverage reports.

The latest repository-documented baseline is 20/20 registered CTest tests passing on Ubuntu and Windows, with 79.3% line coverage, 97.0% function coverage and 50.0% branch coverage. The Test Plan's >=70% branch coverage target is not met yet. Always use the newest CI run's report rather than quoting historical test totals.

## 8. Your individual contribution record

GitHub lists the following pull requests as authored by `poojakoppad04`. Use the PR title and actual diff as evidence of what you personally changed; distinguish work you implemented from work you coordinated, reviewed or documented.

- **Train search and availability:** PR #10 (train data and route search), PR #42/#43 (class-wise seat availability).
- **Validation and security testing:** PR #62 (seat-count validation), PR #65 (security requirements review), PR #66 (passenger registration/login tests).
- **Project foundation and authentication:** PR #71 (core data structures), PR #72 (passenger registration/authentication), PR #73 (C++17 project setup), PR #74 (validated file-storage foundation).
- **Integration and quality:** PR #76 (session activity/test isolation), PR #77 (integer-overflow input validation), PR #79 (GitHub Actions CI/CD), PR #80 (waitlist/automatic promotion), PR #81 (administrator audit logging), PR #82 (SRS/test traceability), PR #83 (FR-018 train deactivation), PR #84/#85 (traceability and coverage evidence).
- **Final verification:** PR #94 (passenger booking/cancellation audit), PR #95 (password-storage/audit privacy verification), PR #96 (availability monitoring), PR #97 (current SAD/RTM/diagram documentation; pending review/CI at time of writing).

You also personally worked on TASK-02 payment data structures and validation tests in `src/Payment.h` and `tests/PaymentStructureTests.cpp`, including required payment-field validation, transaction uniqueness and duplicate-transaction tests (PR #71).

For the viva, prepare to open at least two of your own PRs and explain: the problem, files changed, key logic, tests added, reviewer feedback and how you verified the fix.

## 9. Demo rehearsal plan (about 6–8 minutes)

Prepare a clean disposable working directory with known demo data. Build and run before recording; never debug dependencies during the evaluated demo.

1. **Opening (30 sec):** State project goal, C++17, console interface, local-file storage and single-instance scope.
2. **Passenger registration/login (45 sec):** Create a demo passenger and log in.
3. **Search/availability (60 sec):** Search a known route/date; show train options, classes, fares and seat counts.
4. **Booking (90 sec):** Enter passenger details, explain fare calculation, confirm simulated payment, show PNR and persisted booking.
5. **History/e-ticket (45 sec):** Show the saved reservation and generated ticket output.
6. **Cancellation (75 sec):** Cancel a demo PNR; explain fee/refund and seat restoration.
7. **Admin (60 sec):** Authenticate as administrator; demonstrate a safe train/fare operation and explain deactivation preserves historical bookings.
8. **Engineering evidence (60 sec):** Show Jira sprint board, PRs, CI run, test results and coverage report. Be honest that branch coverage and some NFR evidence remain open.

For a two-minute Sprint 2 recording, select a concise representative flow rather than trying to show every feature. Keep the recording focused on the working application, not slides.

## 10. Viva questions to practise

1. Why did you choose C++17 and a modular architecture?
2. Why local files instead of a database?
3. What is the responsibility of `main.cpp` versus BookingManager?
4. How is a unique PNR generated and checked?
5. How do you prevent a booking from reducing seats incorrectly?
6. What happens when payment fails?
7. What happens if payment succeeds but saving the booking fails?
8. How is cancellation eligibility determined?
9. How are refunds calculated?
10. How are seats restored after cancellation?
11. How does waitlisting work and how is queue order maintained?
12. How are passwords protected?
13. What is the difference between passenger and administrator authentication?
14. What is unit testing versus integration testing?
15. What does CTest do?
16. What does CI/CD do in this project?
17. Why are line coverage and branch coverage different?
18. Why is 50% branch coverage below the Test Plan target?
19. What are the limitations of the current architecture?
20. Which exact files and PRs did you personally work on?

## 11. Claims to avoid in the viva

- Do not say the application is a 24/7 hosted service; it is a local console application.
- Do not say a 99.5% availability result has been measured; the operating window and observation evidence still require approval.
- Do not claim >=70% branch coverage; the latest documented baseline is 50%.
- Do not claim a real payment gateway or automatic bank refund; payments are simulated.
- Do not claim 100 concurrent users across machines; the SRS architecture describes a single-user local instance and the concurrency NFR needs scope reconciliation.
- Do not claim every SRS/Test Plan case is passed because CI is green.
