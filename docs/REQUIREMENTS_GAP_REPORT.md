# Requirements Gap Report

## Purpose

This report compares the current repository implementation with the submitted SRS and Software Test Plan. It distinguishes implemented and tested behavior from requirements that are only partially implemented or not present in the inspected branch. A green build does not imply that every SRS feature is complete.

## Confirmed implementation and test evidence

| Requirement area | Current repository evidence | Status |
|---|---|---|
| Passenger registration/login, password hashing, lockout and inactivity expiry (FR-01 to FR-04, SEC-01, SEC-04) | `AuthManager`, `AuthManagerTests`, salted PBKDF2-HMAC-SHA-256 password derivation, and session/lockout tests | Implemented; registered tests pass in the latest completed run |
| Administrator authentication and guarded train menu | `AdminAuthManager`, `AdminTrainMenu`, administrator authentication and menu-security tests | Implemented; authenticated and unauthenticated menu paths are tested |
| Train search (FR-05 to FR-07) | `TrainSearch`, `test_modules`, console booking smoke test | Implemented; search and full booking smoke paths have automated evidence |
| Seat hold logic (FR-08 to FR-09) | `SeatInventoryManager`, `SeatInventoryTests` | In-memory hold/expiry logic exists; end-to-end UI integration and all timeout scenarios need confirmation |
| Passenger details and fare calculation | `PassengerDetails`, `FareCalculator`, `US05Tests` | Implemented and unit tested |
| Confirmed booking and unique PNR (FR-10 to FR-11) | `BookingManager`, `BookingManagerTests`, `BookingIntegrationTest`, `PerformanceTests` | Confirmed-booking path implemented and tested |
| Simulated payment (FR-14 to FR-15) | `Payment.cpp`, `PaymentProcessTests` | Success, failure, invalid amount/method/result paths are tested |
| Cancellation/refund (FR-17 to FR-19) | `Cancellation.cpp`, `CancellationTests` | Confirmed booking cancellation, both 25% and 50% refund slabs, seat restoration, repeat-cancel and post-departure rejection are tested |
| Admin train management | `AdminTrainManager`, `AdminTrainMenu`, `AdminTrainMenuTests` | Add, view, update and delete paths have automated evidence |
| Admin fare/reservation management | `AdminFareReservationManager`, `AdminFareReservationTests` | Existing fare/reservation functions have unit coverage; complete acceptance mapping still needs review |

## Material gaps against the SRS / Test Plan

| Requirement | Finding in inspected branch | Required action |
|---|---|---|
| FR-12 / FR-13: waitlisting and automatic upgrade | `BookingManager::createBooking` rejects insufficient seat availability instead of persisting a Waitlisted booking. Cancellation does not promote a waitlisted booking. | Implement waitlist position/state and promotion on cancellation, or obtain approval to revise the SRS and Test Plan scope. |
| FR-16: persisted payment receipt | `processPayment` simulates a success/failure result. Payment structures can be serialized, but the inspected payment flow does not create and persist a unique transaction receipt as part of booking. | Add a payment transaction/receipt persistence flow and tests, or document an approved scope change. |
| FR-20 to FR-22: PNR enquiry and booking history | No dedicated PNR status or passenger booking-history service/source file or corresponding main-menu flow was found in the inspected repository tree. | Implement PNR lookup/status and passenger-scoped history with sorting/filtering, or revise scope with evaluator approval. |
| FR-23 / FR-24: e-ticket generation/export | No ticket-printer/e-ticket generation module or export flow was found in the inspected repository tree. | Implement formatted ticket generation and local export, or revise scope with evaluator approval. |
| FR-29 / FR-30: administrator reports | No ReportGenerator/report module or report-generation menu flow was found. Existing reservation viewing is not equivalent to occupancy/revenue reports. | Implement the specified reports and tests, or revise scope with evaluator approval. |
| TC-07: waitlist auto-upgrade | Not testable as specified until FR-12/FR-13 are implemented. | Keep TC-07 open; do not count booking/cancellation tests as proof of auto-upgrade. |
| TC-11: PNR/history/e-ticket | No implementation found for the complete scenario. | Keep TC-11 open until the services and end-to-end test exist. |
| TC-13: reports | Train/fare administration exists, but report generation is not evidenced. | Keep the reports portion of TC-13 open. |

## Non-functional and security validation still open

- **NFR-01:** search against 200 seeded trains is timed against the 2-second limit by `PerformanceTests`.
- **NFR-02:** booking against the seeded 200-train file is timed against the 3-second limit by `PerformanceTests`.
- **NFR-03:** startup must be measured against the 3-second limit; the console smoke test proves launch/flow, not a measured startup threshold.
- **NFR-04:** seeded train-file read is timed against the 200-millisecond limit by `PerformanceTests`.
- **NFR-05:** peak process memory must be measured against the 128-MB limit; no reliable cross-platform memory measurement is currently included.
- **SEC-02:** relative `data/` paths do not prove canonical application-directory confinement.
- **SEC-03:** selected invalid/malformed input cases are tested, but the Test Plan's oversized-input and broader fuzz/boundary matrix is not complete.
- **SEC-05:** corrupted passenger-account storage is rejected, but checksum/record-count detection and safe reinitialization for all train/booking stores are not established.
- **Branch coverage:** the required 70% threshold remains open until a completed gcovr report confirms it.

## Tooling deviation

The Test Plan names Jenkins, GoogleTest and SonarQube (or equivalent). The implemented pipeline uses GitHub Actions, CMake/CTest with repository test executables, Cppcheck and gcovr. This substitution is documented in `CI_CD_AND_TESTING.md`; it should be accepted by the evaluator and reflected in the final approved Test Plan revision.

## Closure rule

Do not mark a requirement or test case Done solely because the project builds or related unit tests pass. Close each item only after its acceptance steps have evidence, or after the team/evaluator formally approves a scope change and the SRS, SAD, Test Plan and traceability matrix are updated consistently.
