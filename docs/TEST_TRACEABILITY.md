# Test Case Traceability and Execution Status

## Basis

This matrix follows the 14 test cases and SEC-01 to SEC-05 checks in the submitted Software Test Plan. It maps them to the current repository's CTest suites where a related test source exists. A related unit suite is not automatically proof that the complete end-to-end test case or every stated security requirement has passed.

| Test Plan ID | Test Plan objective | Related automated suite(s) | Current evidence / gap |
|---|---|---|---|
| TC-01 | Passenger registration and duplicate-email rejection | `AuthManagerTests` | Registration and duplicate-account checks exist; confirm latest CI result. |
| TC-02 | Login, five-failure lockout, session expiry | `AuthManagerTests` | Login, lockout and passenger session timeout checks exist; confirm latest CI result. |
| TC-03 / SEC-01 | Passwords not stored in plaintext | `AuthManagerTests`, `AdminAuthManagerTests` | Passenger and administrator authentication use salted PBKDF2-HMAC-SHA-256 in source; tests inspect account storage. |
| TC-04 | Train search and no-results behavior | `test_modules`, `US05Tests` | Related search/module tests exist; complete scenario coverage needs review against the detailed procedure. |
| TC-05 | Five-minute seat soft-lock expiry | `SeatInventoryTests` | Seat-inventory suite exists; do not mark the five-minute elapsed-time scenario passed without explicit evidence. |
| TC-06 | Unique PNR on confirmed booking | `BookingManagerTests`, `BookingIntegrationTest` | Related booking tests exist; validate exact 10-digit uniqueness and traveller-count assertions. |
| TC-07 | Waitlist auto-upgrade after cancellation | `BookingIntegrationTest`, `BookingManagerTests` | Related integration/booking tests exist; exact auto-upgrade scenario requires confirmation. |
| TC-08 | Failed payment releases hold; success records receipt | `PaymentStructureTests`, `PaymentProcessTests`, `BookingIntegrationTest` | Payment data validation and interactive success/failure simulation are now covered separately. The complete payment-to-booking rollback/receipt scenario still needs end-to-end evidence. |
| TC-09 | Cancellation refund fee slabs | `CancellationTests` | Automated coverage now exercises a future booking cancellation, the over-24-hour 25% fee tier, refund calculation, seat restoration, status update, and repeat-cancellation rejection. The within-24-hour 50% fee tier and post-departure rejection still need explicit tests. |
| TC-10 | Performance and memory targets | No dedicated benchmark suite identified | Measure the NFR thresholds using the Test Plan's seeded workload; do not infer from unit-test success. |
| TC-11 | PNR status, history, e-ticket | `BookingManagerTests`, `BookingIntegrationTest` | Related booking tests exist; PNR/history/e-ticket end-to-end evidence needs confirmation. |
| TC-12 / SEC-05 | Corrupt data and verify integrity/recovery | `FileStoreTests`, `TrainDataFileTests`, `AuthManagerTests` | Corrupted passenger account storage is rejected; checksum/record-count recovery for all stores is not established. |
| TC-13 | Admin train/fare management and reports | `AdminTrainManagerTests`, `AdminFareReservationTests` | Admin management suites exist; report-generator requirements need explicit mapping/evidence. |
| TC-14 / SEC-02 / SEC-03 | Data-file confinement and malformed-input safety | `FileStoreTests`, `TrainDataFileTests`, validation suites | Relative `data/` paths and selected malformed-record checks exist; strict directory confinement and full boundary/fuzz coverage remain open. |
| SEC-04 | Passenger session expiry after 15 minutes | `AuthManagerTests` | Passenger timeout test exists; administrator inactivity timeout not established. |

| Console menu smoke | `ConsoleMenuSmoke` | Exercises invalid menu choice, passenger registration, login, logout, and exit through the built console executable. |

## Execution status

Workflow run [43022855092](https://github.com/Railway-Reservation-Team/Railway-Reservation-Team/actions/runs/43022855092) passed all **14/14 registered CTest suites** on Ubuntu and Windows. Cppcheck and coverage report generation also completed successfully. This is evidence that the registered suites passed; it is not evidence that every manual step in all 14 Test Plan cases has been executed.

The measured coverage in that run was **56.6% line**, **83.1% function**, and **33.7% branch**. The required 70% branch-coverage target remains open. This document deliberately does not pre-mark tests as passed. Manual acceptance tests, performance measurements, and security checks not covered by automated assertions must be recorded separately with date, tester, input data, expected result, actual result, and evidence.

## Coverage target

The Test Plan requires at least 70% branch coverage. Record the exact measured branch percentage from the latest completed gcovr report. If it is below 70%, the requirement remains open.
