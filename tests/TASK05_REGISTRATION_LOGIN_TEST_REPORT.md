# TASK-05: Passenger Registration and Login Testing

## 1. Objective

The objective of this task is to verify passenger registration, login, password handling, account lockout, session management and authentication data integrity in the Railway Reservation System.

## 2. Testing Environment

- Project: Railway Reservation System - Team 12
- Language: C++17
- Compiler: g++
- Authentication module: AuthManager
- Cryptographic library: OpenSSL (libcrypto)
- Testing approach: Automated functional testing
- Test executable: `tests/AuthManagerTests.exe`
- Test status: COMPLETED

## 3. Test Cases and Results

| Test ID | Test Scenario | Expected Result | Actual Result | Status |
|---|---|---|---|---|
| TC-01 | Register with valid passenger details | Account should be created successfully | Passenger account was created | PASS |
| TC-02 | Register using an existing email | Duplicate account should be rejected | Duplicate email was rejected | PASS |
| TC-03 | Verify password storage | Plaintext password should not be stored | Password hash and salt are stored instead | PASS |
| TC-04 | Verify account storage integrity header | Account file should contain an integrity header | Integrity header was found | PASS |
| TC-05 | Register with an invalid email | Invalid email should be rejected | Invalid email was rejected | PASS |
| TC-06 | Register with a password shorter than the minimum length | Short password should be rejected | Short password was rejected | PASS |
| TC-07 | Register with an invalid phone number | Invalid phone number should be rejected | Invalid phone number was rejected | PASS |
| TC-08 | Login with an incorrect password | Login should be rejected | Incorrect password was rejected | PASS |
| TC-09 | Login with valid credentials | Passenger should be authenticated | Login was accepted | PASS |
| TC-10 | Check session before timeout | Session should remain active | Session was active | PASS |
| TC-11 | Record passenger activity | Session activity should be refreshed | Activity refresh succeeded | PASS |
| TC-12 | Check session after activity refresh | Session should remain active | Session remained active | PASS |
| TC-13 | Check session after 15 minutes of inactivity | Session should expire | Session expired | PASS |
| TC-14 | Access authentication state after expiry | Expired session should no longer be authenticated | Session was inactive | PASS |
| TC-15 | Enter an incorrect password five consecutive times | Login attempts should be rejected | All five attempts were rejected | PASS |
| TC-16 | Attempt login after five failed attempts | Account should be locked | Account lockout was enforced | PASS |
| TC-17 | Login after the 15-minute lockout expires | Login should be permitted with correct credentials | Login was accepted | PASS |
| TC-18 | Verify account lockout audit event | Lockout should be recorded | Audit event was recorded | PASS |
| TC-19 | Logout from an active session | Session should terminate | Session was terminated | PASS |
| TC-20 | Load corrupted account storage | Corrupted account data should be rejected | Corrupted storage was rejected | PASS |

## 4. Main Menu Integration Verification

The authentication functionality was also checked through the application menu.

| Scenario | Expected Result | Actual Result | Status |
|---|---|---|---|
| Attempt booking before login | Booking should be blocked | Booking was blocked | PASS |
| Register through the application menu | Account should be created | Registration succeeded | PASS |
| Login through the application menu | Passenger should be authenticated | Login succeeded | PASS |
| Logout through the application menu | Session should terminate | Logout succeeded | PASS |
| Attempt booking after logout | Booking should be blocked | Booking was blocked | PASS |
| Login again and enter booking | Authenticated passenger should enter booking flow | Booking flow was reached | PASS |

The menu integration test confirmed that a passenger cannot enter the booking flow without an active session. It did not execute a complete end-to-end booking transaction.

## 5. Observations

1. The AuthManager module was implemented and integrated into the main application.
2. Passenger accounts are registered using name, email, phone number and password.
3. Duplicate email registration is rejected.
4. Passwords are protected using PBKDF2-HMAC-SHA256 with a per-account random salt.
5. Five consecutive failed login attempts trigger a 15-minute account lockout.
6. Account lockout events are recorded in the authentication audit log.
7. Passenger sessions expire after 15 minutes of inactivity.
8. Account storage includes an integrity check to detect accidental corruption.
9. Booking access is checked against the passenger's active session.
10. The authentication test suite completed with 20 passed and 0 failed.

## 6. Limitations

- The authentication module covers passenger registration and passenger login.
- The current email address is used as the passenger login identifier.
- The integrity checksum detects accidental corruption; it is not a cryptographic protection against deliberate file tampering.
- The menu integration test verified access control and entry into the booking flow, not a complete booking transaction.
- Separate administrator authentication has not yet been implemented or verified.
- The 20 automated authentication tests do not establish that every security requirement in the SRS has been independently verified.

## 7. Conclusion

The AuthManager module has been implemented, integrated into the application and tested successfully. All 20 authentication-specific automated tests passed.

Passenger registration, duplicate email rejection, password protection, login validation, failed-attempt lockout, session timeout, logout and account-storage integrity checks were verified.

Therefore, passenger registration and login functionality can be reported as implemented and tested. Separate administrator authentication remains a distinct pending requirement and should not be considered complete based on these results.
