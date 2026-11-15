# Security Requirements Review

## 1. Objective

This review records the security controls visible in the current CI/CD implementation branch. It is a source-code review, not a penetration test or a complete runtime security assessment.

## 2. Security Requirements Verification

| Requirement | Current evidence | Status / limitation |
|---|---|---|
| Passenger password protection | `AuthManager` uses a random 16-byte salt and PBKDF2-HMAC-SHA-256 (600,000 iterations) to derive a 32-byte hash. Password verification uses constant-time comparison. | Implemented in source. The passenger authentication tests check registration, login, session expiry, lockout, and corrupted account storage. |
| Administrator authentication | `AdminAuthManager` stores a salted PBKDF2-HMAC-SHA-256 password hash, limits failed attempts, and maintains an authenticated state. | Implemented in source; a dedicated administrator authentication test suite is registered in CTest. |
| Administrator-only train management | `AdminTrainMenu::run` checks `AdminAuthManager::isAuthenticated()` before displaying the menu. | Implemented in source; unauthenticated access is covered by `AdminTrainMenuSecurityTests`. |
| Five failed login attempts / 15-minute lockout | Passenger and administrator authentication modules include failed-attempt lockout logic. Passenger tests use deterministic timestamps; administrator tests verify the lockout behavior. | Implemented in source and covered by dedicated tests. |
| Passenger session inactivity timeout | Passenger authentication defines a 15-minute session timeout and supports activity refresh. | Implemented in source and covered by passenger authentication tests. A corresponding administrator inactivity timeout is not evidenced by the inspected interface. |
| Data-file location (SEC-02) | Authentication and train data use relative paths under the application's `data/` directory. | Partially evidenced. Relative paths depend on the process working directory; this review does not establish a sandbox or prove that all data access is confined to a canonical application directory. |
| Input and malformed-record handling (SEC-03) | Validation helpers and malformed account-storage checks exist. | Partially implemented. No comprehensive fuzzing campaign or full malformed-record test matrix has been demonstrated. |
| Corrupted-file recovery / integrity (SEC-05) | Passenger authentication tests deliberately append a malformed record and verify that login rejects corrupted account storage. | Partial coverage only. This does not establish checksum/record-count validation and safe reinitialization for every data file. |
| Booking retrieval authorization | Booking lookup by PNR must be reviewed separately from authentication. | Do not assume passenger ownership authorization from the existence of login. The inspected authentication interfaces alone do not prove that every booking retrieval path checks ownership. |

## 3. Positive Controls Identified

1. Passenger passwords are not stored as plaintext in the authentication account record; a per-account salt and PBKDF2-HMAC-SHA-256 hash are persisted.
2. Administrator credentials use salted PBKDF2-HMAC-SHA-256 password hashing.
3. Password comparisons use OpenSSL's constant-time comparison function.
4. Passenger login includes failed-attempt tracking, temporary lockout, session expiry, and activity refresh.
5. Administrator login includes failed-attempt lockout.
6. The administrator train menu rejects calls when no administrator session is authenticated.
7. Existing validation, booking, file-handling, and PNR checks remain relevant controls and should continue to receive separate tests.

## 4. Remaining Risks and Required Follow-up

- **Data-directory confinement:** Relative `data/` paths follow the current working directory. Resolve and enforce a canonical application data directory if SEC-02 requires strict confinement.
- **Booking ownership checks:** Verify that passenger booking-history and PNR enquiry flows authorize access to the requested booking, not merely that the caller has logged in.
- **SEC-03 test depth:** Add boundary and malformed-input tests for all persisted record formats and interactive input paths; run fuzz testing if time and tooling permit.
- **SEC-05 requirement mismatch:** The current authentication test demonstrates rejection of a corrupted account record. It does not demonstrate checksums/record counts or safe reinitialization of trains and bookings data. Confirm whether the SRS requires those exact mechanisms and implement/test them if they remain mandatory.
- **Administrator session lifecycle:** The inspected administrator interface does not expose an inactivity timeout equivalent to the passenger's 15-minute timeout. Confirm whether this is required for administrator sessions.
- **Stored passenger data:** Password hashing protects credentials, not all passenger booking data. Apply appropriate OS file permissions and consider encryption at rest if required by the security requirements.
- **PNR collision:** A check-then-write PNR allocation sequence may not be safe under simultaneous access. The SAD describes a single-user offline application, but the limitation should remain documented.

## 5. Test Evidence

The CMake/CTest pipeline registers dedicated passenger authentication, administrator authentication, and unauthenticated administrator-menu tests. The current workflow's result must be consulted before marking these suites as passed. The separate CI test report records the actual run outcome.

## 6. Scope and Limitations

This review reflects source files inspected on the `fix/cmake-missing-core-sources` branch. It does not claim a penetration test, complete SEC-01 to SEC-05 acceptance, booking-ownership enforcement, or full data-store confinement.
