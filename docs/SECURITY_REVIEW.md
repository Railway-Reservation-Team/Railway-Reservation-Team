\# Security Requirements Review



\## 1. Objective



The purpose of this review is to examine the existing Railway Reservation System and identify whether the required security controls are present. The review focuses on password protection, passenger and administrator access restrictions, login attempt limits, input validation, and possible security improvements.



\## 2. Security Requirements Verification



| Requirement | Review Result | Observation |

|---|---|---|

| Password protection | Not implemented | No login or password authentication mechanism was found in the inspected source files. |

| Passenger/Admin access restrictions | Not implemented | No role-based access control or separate passenger and administrator roles were found. |

| Login attempt limits | Not applicable in the current implementation | The application does not currently have a login mechanism, so login attempt limits cannot be verified. |

| Input validation | Partially implemented | The application validates passenger count, passenger name, age, gender, journey date, fare, and menu choices. |

| Security issues and improvements | Reviewed | The findings and suggested improvements are documented below. |



\## 3. Positive Security Controls Identified



1\. \*\*Passenger validation:\*\* Passenger count is restricted to 1–6. Passenger names, ages, and gender values are checked before booking.

2\. \*\*Date validation:\*\* Journey dates are checked for the correct format, valid month, valid day, and leap-year conditions.

3\. \*\*Fare validation:\*\* Fare values are checked to ensure they are finite and greater than zero.

4\. \*\*Booking validation:\*\* The booking module checks passenger details and fare values before creating a booking.

5\. \*\*Special-character encoding:\*\* Passenger details are encoded before being stored in the pipe-separated booking file. This helps prevent special characters from interfering with the record format.

6\. \*\*PNR generation:\*\* The system generates a random 10-digit PNR and checks existing booking records before accepting a candidate.

7\. \*\*File error handling:\*\* The booking module checks whether files can be opened and whether booking data can be written. It also attempts to restore train inventory if saving a booking fails.



\## 4. Security Issues Identified



\### Issue 1: Missing Authentication



\*\*Severity:\*\* Medium



The current application does not contain a login or password authentication mechanism. Users can access the booking workflow without first verifying their identity.



\*\*Suggested improvement:\*\* Introduce secure user authentication. Passwords should be stored using a suitable password-hashing algorithm rather than as plain text.



\### Issue 2: Missing Role-Based Access Control



\*\*Severity:\*\* Medium



The inspected source files do not implement separate passenger and administrator roles. As a result, the application does not demonstrate role-based restrictions for different operations.



\*\*Suggested improvement:\*\* Introduce passenger and administrator roles and restrict sensitive administrative operations to authorized administrators.



\### Issue 3: No Login Attempt Limiting



\*\*Severity:\*\* Not applicable currently



Since the application does not have a login system, there is no login attempt counter or account lockout mechanism to verify.



\*\*Suggested improvement:\*\* When authentication is introduced, implement rate limiting, temporary lockouts, and appropriate logging of repeated failed login attempts.



\### Issue 4: Booking Retrieval Access Control



\*\*Severity:\*\* Medium



The `findBooking()` function retrieves a booking by matching the requested PNR. The function itself does not verify the identity or role of the person requesting the booking.



\*\*Suggested improvement:\*\* Before displaying booking details, verify that the requester is authorized to access the booking. An administrator should have separately controlled access.



\### Issue 5: Integer Overflow Risk in Numeric Input Parsing



\*\*Severity:\*\* Medium



The `parseSeatCount()` and `parseMenuChoice()` functions build integer values by multiplying the current value by 10 and adding the next digit. Extremely long numeric input may overflow the integer before the maximum-value check is reached.



\*\*Suggested improvement:\*\* Check whether the next multiplication and addition would exceed the permitted range before performing the operation. Reject excessively long or out-of-range input safely.



\### Issue 6: Passenger Data Is Not Encrypted



\*\*Severity:\*\* Medium



Passenger details are encoded to preserve the booking file format, but this encoding is not encryption. The booking records are stored in a local data file without application-level encryption.



\*\*Suggested improvement:\*\* Protect booking files using appropriate operating-system file permissions. If sensitive passenger information needs stronger protection, use encryption at rest and restrict access to authorized components.



\### Issue 7: Possible PNR Collision During Concurrent Bookings



\*\*Severity:\*\* Low



The application checks whether a generated PNR already exists before saving a booking. However, checking and writing are separate operations. Two simultaneous booking requests could theoretically select the same PNR.



\*\*Suggested improvement:\*\* Use an atomic booking identifier allocation mechanism or a database constraint that guarantees PNR uniqueness.



\## 5. Recommendations



\- Implement secure login and password authentication.

\- Introduce passenger and administrator roles.

\- Restrict booking retrieval to authorized users.

\- Add login rate limiting when authentication is implemented.

\- Improve numeric input parsing to prevent integer overflow.

\- Protect stored passenger information through suitable file permissions and encryption where required.

\- Ensure PNR uniqueness even when multiple bookings are processed simultaneously.

\- Add security-focused test cases for invalid and boundary-value inputs.



\## 6. Conclusion



The review found that the Railway Reservation System already includes several useful input-validation and error-handling measures. However, authentication and role-based access control are not present in the inspected implementation. The numeric input parsing logic and protection of stored passenger information also require attention.



The identified issues and recommendations provide a starting point for improving the security and reliability of the system.



\## 7. Scope and Limitations



This review is based on static inspection of the source files in the current TASK-08 branch. It does not constitute a penetration test or a complete runtime security assessment. Requirements marked as not implemented refer to the inspected application code.

