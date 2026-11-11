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
| TC-07 | Waitlist auto-upgrade after cancellation | No qualifying suite | Current `BookingManager::createBooking` rejects insufficient seats; no waitlist persistence or cancellation-triggered promotion was found. Keep this test open until FR-12/FR-13 are implemented. |
| TC-08 | Failed payment releases hold; success records receipt | `PaymentStructureTests`, `PaymentProcessTests`, `BookingIntegrationTest` | Interactive success/failure simulation and payment-record serialization are tested. The application flow does not yet persist a unique transaction receipt or demonstrate a complete seat-hold retry/release cycle. |
| TC-09 | Cancellation refund fee slabs | `CancellationTests` | Automated coverage now exercises a future booking cancellation, the over-24-hour 25% fee tier, refund calculation, seat restoration, status update, and repeat-cancellation rejection. The within-24-hour 50% fee tier and post-departure rejection still need explicit tests. |
| TC-10 | Performance and memory targets | `PerformanceTests`, `ConsoleMenuSmoke` | `PerformanceTests` seeds 200 trains and checks search ≤2s, booking ≤3s, and train-file read ≤200ms. Console startup is exercised end-to-end, but startup timing ≤3s and peak memory ≤128MB still need explicit process-level measurement. |
| TC-11 | PNR status, history, e-ticket | No dedicated service or end-to-end suite found | The inspected repository has no PNR status/history service or e-ticket export flow. Keep TC-11 open; booking lookup by PNR alone does not satisfy the complete test case. |
| TC-12 / SEC-05 | Corrupt data and verify integrity/recovery | `FileStoreTests`, `TrainDataFileTests`, `AuthManagerTests` | Corrupted passenger account storage is rejected; checksum/record-count recovery for all stores is not established. |
| TC-13 | Admin train/fare management and reports | `AdminTrainManagerTests`, `AdminFareReservationTests`, `AdminTrainMenuTests` | Train add/view/update/delete and fare/reservation functions have tests. No ReportGenerator or occupancy/revenue report flow was found; the reports portion remains open. |
| TC-14 / SEC-02 / SEC-03 | Data-file confinement and malformed-input safety | `FileStoreTests`, `TrainDataFileTests`, `ValidationTests` | Validation boundary tests now cover names, dates, fares, menu choices and passenger counts. Strict data-directory confinement and broad file-record fuzzing remain open. |
| SEC-04 | Passenger session expiry after 15 minutes | `AuthManagerTests` | Passenger timeout test exists; administrator inactivity timeout not established. |

| Console menu smoke | `ConsoleMenuSmoke` | Exercises invalid menu choice, passenger registration, login, full successful ticket booking and simulated payment, logout, and exit through the built console executable. |
| Administrator menu flow | `AdminTrainMenuTests`, `AdminTrainMenuSecurityTests` | Exercises unauthenticated access denial and authenticated add, view, update, delete, invalid-choice, and return paths. |

See [REQUIREMENTS_GAP_REPORT.md](REQUIREMENTS_GAP_REPORT.md) for features that are absent or only partially implemented, including waitlist auto-upgrade, PNR/history/e-ticket, and report generation.

## Execution status

Workflow run [43025657211](https://github.com/Railway-Reservation-Team/Railway-Reservation-Team/actions/runs/43025657211) passed all **19/19 registered CTest tests** on Ubuntu and Windows. Cppcheck and coverage report generation also completed successfully. This is evidence that the registered tests passed; it is not evidence that every manual step in all 14 Test Plan cases has been executed.

The measured coverage in that run was **77.0% line**, **96.9% function**, and **48.0% branch**. The required 70% branch-coverage target remains open. This document deliberately does not pre-mark tests as passed. Manual acceptance tests, performance measurements, and security checks not covered by automated assertions must be recorded separately with date, tester, input data, expected result, actual result, and evidence.

## Coverage target

The Test Plan requires at least 70% branch coverage. Record the exact measured branch percentage from the latest completed gcovr report. If it is below 70%, the requirement remains open.
