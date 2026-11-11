# Requirements Gap Report — SRS v1.0 Baseline

## Purpose and authority

This report uses the submitted **Railway Reservation System SRS v1.0 dated 06-Sep-2026** as the authoritative baseline. That SRS defines **21 functional requirements (RRS-F-001–RRS-F-021), 6 non-functional requirements (RRS-NF-001–RRS-NF-006), and 6 security requirements (RRS-SR-001–RRS-SR-006)**.

The Software Test Plan and Architecture & Design Specification contain additional scope and some inconsistent numbering/targets. Those additions must be mapped to an approved SRS revision or recorded as approved scope changes. Do not silently relabel them as SRS requirements.

A green build does not mean all requirements have passed.

## Functional requirement traceability

| SRS ID | Requirement | Current repository evidence / Jira | Assessment |
|---|---|---|---|
| RRS-F-001 | Passenger registration with unique passenger ID and contact/password | `AuthManager`, `AuthManagerTests`, Jira RRS-10 | Implemented; registered tests pass in latest completed baseline CI |
| RRS-F-002 | Passenger login with valid credentials | `AuthManager`, `AuthManagerTests`, Jira RRS-11 | Implemented; invalid credentials/lockout covered |
| RRS-F-003 | Search by source, destination and journey date | `TrainSearch`, search tests, Jira RRS-2 | Implemented; verify complete acceptance data |
| RRS-F-004 | Display train number/name, route, departure/arrival and available classes | `TrainSearch`, console/search tests, Jira RRS-2 | Implemented in search flow; verify every required field in manual demo |
| RRS-F-005 | Display current seat availability | `SeatInventoryManager`, `SeatInventoryTests`, Jira RRS-13/RRS-43 | Implemented; in-memory hold/expiry and UI integration need full scenario verification |
| RRS-F-006 | Select train/class only when seats are available | booking/search flow, Jira RRS-13/RRS-43 | Partial pending end-to-end unavailable-class test |
| RRS-F-007 | Collect passenger name, age, gender and mandatory details | `PassengerDetails`, Jira RRS-14 | Implemented and unit tested |
| RRS-F-008 | Calculate fare from train, class and passenger details | `FareCalculator`, Jira RRS-15 | Implemented and unit tested |
| RRS-F-009 | Create reservation only when sufficient seats are available | `BookingManager`, booking tests, Jira RRS-25 | Confirmed-booking path implemented; seat hold/transaction edge cases still need end-to-end verification |
| RRS-F-010 | Generate unique PNR for each successful reservation | `BookingManager`, Jira RRS-16 | Implementation exists; Jira remains In Progress; verify uniqueness and format against the approved SRS/STP |
| RRS-F-011 | Display confirmation with PNR, train, date, passenger and fare | booking integration/console smoke, Jira RRS-25 | Implemented in successful booking flow; capture full demo evidence |
| RRS-F-012 | Retrieve booking using PNR or authenticated account details | Jira RRS-30 tracks integration; no complete dedicated passenger-scoped PNR/history flow established in the inspected tree | Not demonstrated end-to-end; do not confuse this with waitlisting |
| RRS-F-013 | Cancel eligible reservation | `Cancellation.cpp`, `CancellationTests`, Jira RRS-61 | Implemented for tested cancellation scenarios |
| RRS-F-014 | Calculate configured refund | `Cancellation.cpp`, `CancellationTests`, Jira RRS-61 | Partially evidenced; test all configured refund slabs and boundary dates |
| RRS-F-015 | Restore cancelled seats to corresponding train/class | `Cancellation.cpp`, `CancellationTests`, Jira RRS-61 | Implemented in tested cancellation flow; verify exactly-once restoration and persistence |
| RRS-F-016 | Administrator adds train records | `AdminTrainManager`, `AdminTrainMenu`, Jira RRS-28 | Implemented and tested |
| RRS-F-017 | Administrator edits train schedule/route/class/availability | `AdminTrainManager`, Jira RRS-28 | Update path implemented and tested; verify every field and authorization |
| RRS-F-018 | Authorized administrator deactivates a train from future bookings | Jira RRS-65 created for the explicit gap | Not implemented by destructive delete; deactivation must preserve historical bookings |
| RRS-F-019 | Authorized administrator views reservation/cancellation records | `AdminFareReservationManager`, Jira RRS-62 | Viewing/search functions exist; verify role-gated menu and complete records |
| RRS-F-020 | Record booking/cancellation transactions with timestamp and status | Jira RRS-62; administrator audit changes proposed in PR #81 | Partial. PR #81 records administrator actions; passenger booking transaction audit must also be verified/implemented |
| RRS-F-021 | Passenger views reservation status (e.g. Confirmed/Cancelled) | Jira RRS-63 | Not demonstrated end-to-end; status retrieval must be passenger-authenticated and match persisted state |

### Scope items present in later planning documents but not separate numbered SRS v1.0 functional requirements

- **Waitlist / automatic upgrade:** not a numbered functional requirement in SRS v1.0. It appears in the later Test Plan/module scope. RRS-F-012 is PNR/account booking retrieval, not waitlisting. Obtain approval to add waitlisting to the SRS or record it as an approved extension; if retained, implement FIFO waiting, promotion on cancellation and tests.
- **Payment transaction receipt:** simulated payment is in project/test scope, but a persisted unique receipt is not a separate numbered SRS v1.0 FR. Treat as an approved extension if required by the evaluator.
- **E-ticket generation/export:** SRS interface section 3.2 says ticket information may be printed or saved; the SRS does not assign it a separate FR ID. Jira RRS-27 tracks the additional deliverable.
- **Admin occupancy/revenue reports:** present in the Test Plan/module ownership but not a separate numbered SRS v1.0 FR. Jira report scope belongs under the administration epic and must be confirmed as an approved extension.

## Non-functional requirement traceability

| SRS ID | Exact SRS target | Evidence / Jira | Assessment |
|---|---|---|---|
| RRS-NF-001 | Train search and normal menu operations: 90th percentile <=3 seconds for at least 90% of requests under normal load | `PerformanceTests`; Jira RRS-56 | Partial. Existing test checks search <=2s on 200 seeded trains, booking <=3s and train-file read <=200ms; this is not a measured p90 of >=90% of normal menu/search requests |
| RRS-NF-002 | 99.5% operational availability during defined operating period, excluding scheduled maintenance | Jira RRS-55 | Not demonstrated. Define operating period and monitoring method; reconcile with local offline console application |
| RRS-NF-003 | Passwords never stored in plaintext; secure protection used | PBKDF2-HMAC-SHA-256 with salt in auth modules; auth tests; Jira RRS-57 | Implementation present; attach explicit passenger/admin storage and log inspection evidence |
| RRS-NF-004 | At least 100 concurrent reservation operations without incorrect seat counts or duplicate confirmed bookings | Jira RRS-58 | Not demonstrated and conflicts with SRS description of a local offline single-user app. Requires approved interpretation/scope change or concurrency architecture and test |
| RRS-NF-005 | First-time users understand interface and receive clear validation/error messages | validation tests; Jira RRS-59 | Partial. Automated input checks exist; structured first-time-user usability evaluation is missing |
| RRS-NF-006 | Reservation records remain available after restart through persistent storage | local file storage and store tests; Jira RRS-60 | Partial. Need explicit create-booking/restart/retrieve-same-PNR and cancellation-persistence evidence |

## Security requirement traceability

| SRS ID | Requirement | Evidence / Jira | Assessment |
|---|---|---|---|
| RRS-SR-001 | Authentication before protected booking information is accessed | passenger auth and admin guards | Partial until PNR/history access is implemented and access-control tested |
| RRS-SR-002 | Passwords protected; never displayed/logged in plaintext | salted PBKDF2-HMAC-SHA-256; auth tests; Jira RRS-57 | Implemented in inspected auth modules; verify all credential files and logs |
| RRS-SR-003 | Role-based authorization restricts admin functions | `AdminAuthManager`, `AdminTrainMenu`, security tests | Implemented for tested admin menu; verify every admin operation |
| RRS-SR-004 | Validate all user input before processing/storing | validation modules/tests, Jira RRS-51 | Partial; malformed/oversized/file-record fuzz matrix remains incomplete |
| RRS-SR-005 | Booking/cancellation/seat updates preserve transaction integrity and prevent duplicate commitment | booking/cancellation/inventory tests | Partial; verify rollback/failure injection, duplicate requests and persistence consistency |
| RRS-SR-006 | Authentication failures and administrator actions recorded as timestamped audit events | PR #81 adds admin audit helper/events and test assertions | Pending CI/review/merge; verify timestamp parsing, admin train/fare/reservation event tests, passenger booking/cancellation transaction coverage and audit failure behavior |

## CI/CD evidence and quality gaps

Latest previously completed baseline run: [GitHub Actions run 43027487428](https://github.com/Railway-Reservation-Team/Railway-Reservation-Team/actions/runs/43027487428). It reported:
- 20/20 registered CTest tests passing on Ubuntu and Windows.
- Cppcheck passed.
- Line coverage 79.7%; function coverage 96.9%; branch coverage 49.9%.
- Branch coverage remains below the 70% target stated in the Test Plan.
- Existing performance checks use 200 seeded trains for search, booking and train-file read; they do not prove every SRS NFR.

PR #81 ([administrator audit logging](https://github.com/Railway-Reservation-Team/Railway-Reservation-Team/pull/81)) is a proposed change, not merged evidence. Its latest CI must pass and the PR must be reviewed before claiming the audit work is integrated.

## Cross-document inconsistencies requiring an approved decision

1. The SRS has six NFRs with IDs RRS-NF-001–006 and targets listed above. The Test Plan instead refers to five NFR-01–05 targets including startup <=3s, booking <=3s, file read <=200ms and memory <=128MB. These are not equivalent mappings.
2. The Test Plan says 70% branch coverage is required by an SRS section 6.4; SRS v1.0 has a Quality Attributes section 6 but no section 6.4. Treat 70% as a Test Plan/evaluation criterion unless the SRS is revised.
3. The Test Plan includes waitlist, payment receipt, e-ticket/history and admin reports beyond the numbered SRS FR list.
4. The SRS says local offline single-user operation, while RRS-NF-004 asks for 100 concurrent reservation operations and RRS-NF-002 asks for 99.5% availability. The evaluator/team must clarify the intended deployment/measurement scope.

## Test tooling substitution

The Test Plan names Jenkins, GoogleTest and SonarQube (or equivalent). The implemented pipeline uses GitHub Actions, CMake/CTest repository test executables, Cppcheck and gcovr. This substitution is documented in `CI_CD_AND_TESTING.md`; obtain evaluator acceptance and update the approved Test Plan if necessary.

## Closure rule

Every requirement must have one explicit disposition: **Pass (with evidence), Fail, Not Demonstrated, or Approved Scope Change**. Do not mark a Jira item or epic Done solely because the project builds or related unit tests pass.