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
| TC-08 | Failed payment releases hold; success records receipt | `PaymentStructureTests`, `BookingIntegrationTest` | Payment and integration suites exist; exact failed-payment/receipt sequence requires confirmation. |
| TC-09 | Cancellation refund fee slabs | No dedicated cancellation suite identified in current CMake list | Manual or new automated evidence required for both time bands. |
| TC-10 | Performance and memory targets | No dedicated benchmark suite identified | Measure the NFR thresholds using the Test Plan's seeded workload; do not infer from unit-test success. |
| TC-11 | PNR status, history, e-ticket | `BookingManagerTests`, `BookingIntegrationTest` | Related booking tests exist; PNR/history/e-ticket end-to-end evidence needs confirmation. |
| TC-12 / SEC-05 | Corrupt data and verify integrity/recovery | `FileStoreTests`, `TrainDataFileTests`, `AuthManagerTests` | Corrupted passenger account storage is rejected; checksum/record-count recovery for all stores is not established. |
| TC-13 | Admin train/fare management and reports | `AdminTrainManagerTests`, `AdminFareReservationTests` | Admin management suites exist; report-generator requirements need explicit mapping/evidence. |
| TC-14 / SEC-02 / SEC-03 | Data-file confinement and malformed-input safety | `FileStoreTests`, `TrainDataFileTests`, validation suites | Relative `data/` paths and selected malformed-record checks exist; strict directory confinement and full boundary/fuzz coverage remain open. |
| SEC-04 | Passenger session expiry after 15 minutes | `AuthManagerTests` | Passenger timeout test exists; administrator inactivity timeout not established. |

## Execution status

The latest completed GitHub Actions run is the source of truth for automated suite pass/fail status. This document deliberately does not pre-mark tests as passed. Manual acceptance tests, performance measurements, and security checks not covered by automated assertions must be recorded separately with date, tester, input data, expected result, actual result, and evidence.

## Coverage target

The Test Plan requires at least 70% branch coverage. Record the exact measured branch percentage from the latest completed gcovr report. If it is below 70%, the requirement remains open.
