# TASK-09: Integration, Regression and System Testing Report

## 1. Objective

To evaluate the Railway Reservation System through automated testing, manual functional testing, security checks and performance testing.

## 2. Testing Environment

- Language: C++17
- Compiler: MSYS2 GCC 16.2.0
- Build System: CMake
- Testing Framework: CTest
- Operating System: Windows
- Application: Console-based Railway Reservation System

## 3. Automated Test Results

Command executed:

`ctest --test-dir build --output-on-failure`

| Metric | Result |
|---|---:|
| Total tests | 24 |
| Passed | 24 |
| Failed | 0 |
| Success rate | 100% |
| Execution time | 9.22 seconds |

All 24 registered CTest cases passed, including booking integration, cancellation, authentication, payment, seat inventory, admin security, validation and performance tests.

## 4. Manual Functional Testing

| Test Case | Description | Actual Result | Status |
|---|---|---|---|
| TC-03 | Successful passenger booking | Booking confirmed and PNR generated | PASS |
| TC-05 | Passenger login | Valid credentials accepted | PASS |
| TC-06 | Cancellation after train departure | Cancellation correctly rejected | PASS |
| TC-07 | Successful cancellation and refund | ₹450 charge and ₹1,350 refund calculated | PASS |
| TC-08 | Seat restoration | AC 3-Tier availability restored to 80 | PASS |
| TC-09 | Unauthorized booking | Booking blocked without login | PASS |
| TC-10 | Password storage verification | Salted PBKDF2-HMAC-SHA-256 implementation verified | PASS |
| TC-11 | Invalid password login | Incorrect password rejected | PASS |

## 5. Security Testing

The following security checks were performed:

- Passenger registration and login.
- Rejection of incorrect passwords.
- Prevention of booking without authentication.
- Password hashing using PBKDF2-HMAC-SHA-256.
- Random 16-byte salt generation.
- 600,000 password-hashing iterations.
- 32-byte derived password hash.
- Constant-time password hash comparison.

The inspected passenger account storage contains a salt and password hash rather than the plaintext password.

Note: These checks do not constitute a complete security audit of the application.

## 6. Performance Testing

Recorded results from `performance-results.txt`:

| Performance Metric | Recorded Result | Limit |
|---|---:|---:|
| Train search | 0 ms | 2,000 ms |
| Booking | 1 ms | 3,000 ms |
| Train-file read | 0 ms | 200 ms |
| Startup time | Not measured | Not established |
| Peak memory | Not measured | Not established |

The measured operations were within their recorded limits.

## 7. Limitations and Pending Verification

- Startup time was not measured.
- Peak memory usage was not measured.
- 99.5% availability was not established through uptime monitoring.
- The 100-concurrent-reservation target was not verified.
- External-user usability testing was not completed.
- Complete restart-based persistence verification was not demonstrated.
- Full security audit and all admin authorization scenarios were not manually verified.
- Some functional requirements remain outside the scope of the manual tests performed.

## 8. Evidence Collected

Manual testing evidence should be maintained in the `Manual_Testing_Evidence` folder, including screenshots for completed booking, cancellation, refund, seat restoration, authentication and security checks.

Automated test output was also captured separately.

## 9. Conclusion

The latest automated regression run completed successfully with 24 out of 24 tests passing and zero failures.

Manual testing verified successful booking, passenger authentication, cancellation rejection after departure, successful cancellation, refund calculation, seat restoration and selected security controls.

Performance measurements recorded train search, booking and train-file read times within their specified limits.

However, availability, peak memory, external-user usability, concurrency and complete persistence testing remain unverified. The results demonstrate successful testing of the listed functionality but do not establish that every system requirement has been fully implemented and verified.

**Overall Status: Testing completed for the verified scope; remaining requirements and limitations are documented above.**
