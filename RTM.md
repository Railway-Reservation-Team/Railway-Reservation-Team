
# Requirements Traceability Matrix (RTM) — Current Implementation Baseline

## Railway Reservation System | Team 12

**Authoritative requirements baseline:** Submitted SRS v1.0 (06-Sep-2026).

**Status rule:** A requirement is not Passed merely because a related module exists or CI is green. See the detailed evidence and limitations in [docs/REQUIREMENTS_GAP_REPORT.md](docs/REQUIREMENTS_GAP_REPORT.md) and [docs/TEST_TRACEABILITY.md](docs/TEST_TRACEABILITY.md).

## 1. Functional requirements

| SRS ID | Requirement (short form) | Main implementation / Jira | Current assessment |
|---|---|---|---|
| RRS-F-001 | Passenger registration | AuthManager; RRS-10 | Implemented; automated tests exist |
| RRS-F-002 | Passenger login | AuthManager; RRS-11 | Implemented; invalid-login and lockout tests exist |
| RRS-F-003 | Search by source, destination and date | TrainSearch; RRS-2 | Implemented; search tests exist |
| RRS-F-004 | Display train details and classes | TrainSearch; RRS-2 | Implemented in search flow; verify all fields in demo |
| RRS-F-005 | Display current seat availability | SeatInventory; RRS-13/RRS-43 | Implemented; complete UI/integration scenario needs verification |
| RRS-F-006 | Select only available train/class | Booking/search; RRS-13/RRS-43 | Partial; unavailable-class end-to-end case needs evidence |
| RRS-F-007 | Collect passenger details | PassengerDetails; RRS-14 | Implemented and unit tested |
| RRS-F-008 | Calculate fare | FareCalculator; RRS-15 | Implemented and unit tested |
| RRS-F-009 | Create reservation when seats are available | BookingManager; RRS-25 | Implemented; transaction edge cases need end-to-end evidence |
| RRS-F-010 | Generate unique PNR | BookingManager; RRS-16 | Implementation exists; verify persisted uniqueness/format |
| RRS-F-011 | Display booking confirmation | BookingManager / console; RRS-25 | Implemented; capture full demo evidence |
| RRS-F-012 | Retrieve booking by PNR/account | BookingManager; RRS-30 | Not demonstrated end-to-end; authenticated passenger-scoped retrieval needs verification |
| RRS-F-013 | Cancel eligible reservation | Cancellation; RRS-61 | Implemented for tested scenarios |
| RRS-F-014 | Calculate refund | Cancellation; RRS-61 | Partial; all refund slabs and boundaries need explicit test evidence |
| RRS-F-015 | Restore cancelled seats | Cancellation / SeatInventory; RRS-61 | Implemented in tested flow; persistence and exactly-once restoration need evidence |
| RRS-F-016 | Admin adds train records | AdminTrainManager; RRS-28 | Implemented and tested |
| RRS-F-017 | Admin edits train details | AdminTrainManager; RRS-28 | Implemented and tested; verify authorization and all fields |
| RRS-F-018 | Admin deactivates train for future bookings | AdminTrainManager; RRS-65 | Implemented and tested in merged PR #83 |
| RRS-F-019 | Admin views reservation/cancellation records | AdminFareReservationManager; RRS-62 | Functions exist; full role-gated record coverage needs verification |
| RRS-F-020 | Timestamped booking/cancellation audit | AdminAuditLog; RRS-62 | Partial; passenger booking/cancellation audit coverage must be confirmed |
| RRS-F-021 | Passenger views reservation status | Booking retrieval; RRS-63 | Not demonstrated end-to-end; authenticated persisted-status flow needs evidence |

## 2. Non-functional requirements

| SRS ID | Acceptance target | Evidence / Jira | Current assessment |
|---|---|---|---|
| RRS-NF-001 | 90th percentile <=3s for >=90% of train-search and normal menu requests under normal load | PerformanceTests; RRS-56 | Partial; existing individual timing checks do not establish the full p90/90% metric |
| RRS-NF-002 | 99.5% operational availability during defined operating period, excluding maintenance | Availability monitor; RRS-55 | Not demonstrated; mentor approval, real observation window and approved calculation required |
| RRS-NF-003 | Passwords never stored in plaintext; secure protection | AuthManager/AdminAuthManager; RRS-57 | Implementation present; inspect passenger/admin stores and logs for evidence |
| RRS-NF-004 | At least 100 concurrent reservation operations without inconsistent seats/duplicate confirmations | Seat inventory tests; RRS-58 | Not demonstrated; reconcile with standalone single-user architecture before claiming compliance |
| RRS-NF-005 | First-time users understand interface and receive clear feedback | Usability checklist; RRS-59 | Partial; structured non-team user evaluation needed |
| RRS-NF-006 | Reservation data persists across application restart | File persistence; RRS-60 | Partial; record create/restart/retrieve/cancel/restart evidence |

## 3. Security requirements

| SRS ID | Requirement | Evidence / Jira | Current assessment |
|---|---|---|---|
| RRS-SR-001 | Authentication before protected booking information | AuthManager and admin guards | Partial; passenger PNR/history access control needs end-to-end verification |
| RRS-SR-002 | Passwords protected; never displayed/logged in plaintext | Salted PBKDF2-HMAC-SHA-256; RRS-57 | Implementation present; inspect both account stores and logs |
| RRS-SR-003 | Role-based admin authorization | AdminAuthManager/AdminTrainMenu | Implemented for tested admin entry; verify all admin operations |
| RRS-SR-004 | Validate user input before processing/storage | Validation tests; RRS-51 | Partial; malformed/oversized input and file-record fuzz matrix incomplete |
| RRS-SR-005 | Preserve booking/cancellation/inventory transaction integrity | Booking/Cancellation/SeatInventory tests | Partial; failure-injection, duplicate requests and rollback consistency need broader evidence |
| RRS-SR-006 | Timestamp authentication failures and admin actions | AdminAuditLog | Partial; passenger booking/cancellation audit evidence needs confirmation |

## 4. Test Plan traceability

The submitted Software Test Plan contains 14 test cases plus security checks. Their current mapping, automation and gaps are recorded in [docs/TEST_TRACEABILITY.md](docs/TEST_TRACEABILITY.md). Waitlisting, receipts, e-ticket export and reports appear in later project scope but are not all separate numbered SRS v1.0 functional requirements; treat them as approved extensions, not silently renumbered SRS items.

## 5. CI and measured quality evidence

The GitHub Actions pipeline uses GitHub Actions, CMake/CTest, custom C++ test executables, Cppcheck and gcovr. It does not run Jenkins, GoogleTest or SonarQube as named in the original Test Plan.

The last documented baseline in the repository reported:
- 20/20 registered CTest tests passing on Ubuntu and Windows.
- 79.3% line coverage.
- 97.0% function coverage.
- 50.0% branch coverage.

The Test Plan's minimum 70% branch-coverage target is **not met**. The current merged PR #96 adds availability-monitor unit tests and the main-branch CI run must be used for any refreshed coverage figures.

## 6. Final evidence fields

For each manual acceptance test, attach:
- SRS/Test Plan ID and test-case ID.
- Build commit SHA and build date.
- Tester, operating system and compiler.
- Test data and exact input steps.
- Expected and actual result.
- Pass / Fail / Not Demonstrated.
- Screenshot, video, log or CI artifact reference.

Never change a Partial/Not Demonstrated row to Passed without attaching the evidence above.
