# Test Case Traceability and Execution Status

## Requirement baseline

The authoritative submitted SRS v1.0 (06-Sep-2026) defines 21 functional requirements (RRS-F-001–021), 6 non-functional requirements (RRS-NF-001–006) and 6 security requirements (RRS-SR-001–006). The Software Test Plan contains 14 test cases and additional module scope. Where the Test Plan uses different NFR names/targets or adds waitlisting, payment receipts, e-ticket export and reports, those items must be recorded as Test Plan scope or approved SRS extensions—not silently mapped to a different SRS FR.

See [REQUIREMENTS_GAP_REPORT.md](REQUIREMENTS_GAP_REPORT.md) for the full SRS ID-by-ID status and cross-document discrepancies.

## Test Plan scenario traceability

| Test Plan ID | Test objective | Related automated suite(s) | Current evidence / gap |
|---|---|---|---|
| TC-01 | Passenger registration and duplicate-email rejection | `AuthManagerTests` | Registration and duplicate-account checks exist; latest baseline CI passed |
| TC-02 | Login, five-failure lockout, session expiry | `AuthManagerTests` | Login, lockout and passenger inactivity timeout tests exist; verify exact 15-minute threshold |
| TC-03 / SEC-01 | Passwords not stored in plaintext | `AuthManagerTests`, `AdminAuthManagerTests` | Auth modules use salted PBKDF2-HMAC-SHA-256; inspect both account stores and logs |
| TC-04 | Train search and no-results behavior | `test_modules`, `US05Tests` | Related search/module tests exist; manually verify all required result fields |
| TC-05 | Five-minute seat soft-lock expiry | `SeatInventoryTests` | Seat-inventory suite exists; do not claim elapsed five-minute expiry without explicit evidence |
| TC-06 | Unique PNR on confirmed booking | `BookingManagerTests`, `BookingIntegrationTest` | Related booking tests exist; verify required PNR length/format and uniqueness in persisted data |
| TC-07 | Waitlist auto-upgrade after cancellation | No qualifying suite confirmed | Waitlist is not a numbered FR in SRS v1.0; it is added Test Plan scope. Keep open unless approved extension is implemented and FIFO promotion is tested |
| TC-08 | Failed payment releases hold; success records receipt | `PaymentStructureTests`, `PaymentProcessTests`, `BookingIntegrationTest` | Simulated payment paths and record serialization are tested; unique persisted receipt and full hold-release/retry scenario remain to be demonstrated if required |
| TC-09 | Cancellation refund fee slabs | `CancellationTests` | Cancellation/refund and seat-restoration tests exist; explicitly verify all configured slabs, boundary dates, post-departure rejection and repeated cancellation |
| TC-10 | Performance and memory targets | `PerformanceTests`, `ConsoleMenuSmoke` | Current tests check search <=2s, booking <=3s and train-file read <=200ms on seeded data. Startup <=3s and peak memory <=128MB are not measured. These Test Plan targets do not directly equal SRS NF-001/NF-002/NF-004 |
| TC-11 | PNR status, history, e-ticket | No complete dedicated end-to-end suite confirmed | SRS RRS-F-012 is PNR/account retrieval and RRS-F-021 is passenger status. History/e-ticket export are additional Test Plan scope. Keep open until retrieval, access control, history and export flow are verified |
| TC-12 / SEC-05 | Corrupt data and verify integrity/recovery | `FileStoreTests`, `TrainDataFileTests`, `AuthManagerTests` | Corrupted passenger-account storage rejection exists; checksum/record-count detection and safe recovery for all stores not established |
| TC-13 | Admin train/fare management and reports | `AdminTrainManagerTests`, `AdminFareReservationTests`, `AdminTrainMenuTests` | Admin train/fare/reservation paths have tests; report generation is extra Test Plan scope and remains open pending ReportGenerator evidence |
| TC-14 / SEC-02 / SEC-03 | Data-file confinement and malformed-input safety | `FileStoreTests`, `TrainDataFileTests`, `ValidationTests` | Selected validation boundary tests exist; strict application-directory confinement and broad malformed/oversized file-record fuzzing remain open |
| SEC-04 | Passenger session expiry after 15 minutes | `AuthManagerTests` | Passenger timeout test exists; explicitly verify 15-minute boundary and admin inactivity behavior only if required by approved scope |

## SRS non-functional requirement tests

| SRS ID | Acceptance target | Current evidence | Status |
|---|---|---|---|
| RRS-NF-001 | 90th percentile <=3 seconds for >=90% of train-search and normal menu requests under normal load | Search <=2s test on 200 seeded trains exists; no complete p90/90%-of-requests measurement | Partial |
| RRS-NF-002 | 99.5% operational availability over defined period excluding maintenance | No continuous availability monitoring/measurement for local offline console app | Not demonstrated; clarify scope |
| RRS-NF-003 | No plaintext passwords; secure protection | PBKDF2-HMAC-SHA-256 salted storage in auth modules; test storage/log inspection | Implemented; evidence review required |
| RRS-NF-004 | 100 concurrent reservation operations without inconsistency/duplicate confirmed booking | No 100-operation concurrency test; offline single-user SRS context conflicts | Not demonstrated; clarify scope |
| RRS-NF-005 | First-time-user usability checklist passes | Validation messages exist; no documented non-team user evaluation | Partial |
| RRS-NF-006 | Reservation records persist after application restart | File storage exists; complete booking/restart/retrieve same PNR scenario needs explicit evidence | Partial |

## SRS security requirement tests

| SRS ID | Security objective | Current evidence / gap |
|---|---|---|
| RRS-SR-001 | Authentication before protected booking details | Passenger authentication exists; PNR/history access control needs end-to-end verification |
| RRS-SR-002 | No plaintext password display/storage/logging | Salted PBKDF2-HMAC-SHA-256 in passenger/admin auth modules; inspect logs and persisted stores |
| RRS-SR-003 | Role-based admin authorization | Admin auth and guarded train menu tests exist; verify all admin operations |
| RRS-SR-004 | Validate every user input before processing/storage | Validation suite exists; oversized and malformed-file fuzz matrix incomplete |
| RRS-SR-005 | Transaction integrity/no duplicate commitment | Booking, cancellation and seat tests exist; failure-injection, duplicate requests and rollback consistency need broader evidence |
| RRS-SR-006 | Timestamped authentication failures and administrator actions | PR #81 merged; CI run 43071499990 passed. Tests cover administrator audit events; passenger booking/cancellation transaction coverage remains to be confirmed |

## Execution status

Latest previously completed baseline run: [GitHub Actions run 43027487428](https://github.com/Railway-Reservation-Team/Railway-Reservation-Team/actions/runs/43027487428). It passed all **20/20 registered CTest tests** on Ubuntu and Windows. Cppcheck and coverage generation completed.

Measured baseline coverage:
- Line coverage: **79.7%**
- Function coverage: **96.9%**
- Branch coverage: **49.9%**

The Test Plan's 70% branch coverage target remains open. Passing registered tests do not prove every manual step in the 14 Test Plan cases or every SRS requirement.

## Tooling deviation

PR #83 (SRS RRS-F-018 train deactivation) is merged; CI run 43073959982 passed on Ubuntu and Windows with 20/20 CTest tests and static-analysis/coverage successful. The post-merge main run is tracked separately.

The Test Plan names Jenkins, GoogleTest and SonarQube (or equivalent). The implemented pipeline uses GitHub Actions, CMake/CTest repository test executables, Cppcheck and gcovr. The substitution is documented in `CI_CD_AND_TESTING.md`; evaluator acceptance and an approved Test Plan revision should be recorded.

## Evidence recording rule

For every manual or automated acceptance test, record: requirement/test ID, build SHA, date, tester, OS/toolchain, test data, exact input, expected result, actual result, Pass/Fail/Not Demonstrated, and evidence link. Do not mark a requirement Done only because the application compiles or a related unit test passes.