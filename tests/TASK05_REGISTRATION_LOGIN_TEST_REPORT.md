# TASK-05: Passenger Registration and Login Testing



## 1. Objective



The objective of this task is to verify passenger registration and login functionality in the Railway Reservation System. The testing covers account creation, duplicate usernames, authentication, invalid inputs, password handling and login restrictions.



## 2. Testing Environment



- Project: Railway Reservation System - Team 12

- Language: C++17

- Compiler: g++

- Testing approach: Functional testing and source-code inspection

- Test status: Blocked due to unavailable authentication implementation



## 3. Test Cases



| Test ID | Test Scenario | Expected Result | Actual Result | Status |

|---|---|---|---|---|

| TC-01 | Register with valid passenger details | Account should be created successfully | Registration functionality is unavailable | BLOCKED |

| TC-02 | Register using an existing username | System should reject duplicate username | Username checking functionality is unavailable | BLOCKED |

| TC-03 | Login using valid credentials | User should be authenticated successfully | Login functionality is unavailable | BLOCKED |

| TC-04 | Login using an incorrect password | System should reject the login attempt | Authentication functionality is unavailable | BLOCKED |

| TC-05 | Login using an unregistered username | System should reject the login attempt | User account verification is unavailable | BLOCKED |

| TC-06 | Submit empty registration fields | System should reject empty inputs | Registration form is unavailable | BLOCKED |

| TC-07 | Submit malformed registration inputs | System should display appropriate validation errors | Registration functionality is unavailable | BLOCKED |

| TC-08 | Verify password storage | Passwords should not be stored in plaintext | Password storage implementation is unavailable | BLOCKED |

| TC-09 | Exceed the permitted number of failed login attempts | System should restrict further login attempts | Login restriction mechanism is unavailable | BLOCKED |



## 4. Observations



1. The project README describes an AuthManager module for registration, authentication and session handling.

2. The Requirements Traceability Matrix lists passenger registration (FR-01) and passenger login (FR-02) as pending.

3. Source-code inspection did not identify an AuthManager implementation or registration and login functions in the inspected source and test files.

4. The existing automated regression tests cover passenger validation, date validation, passenger count, fare validation, train search and fare calculation.

5. Authentication-specific behaviour could not be executed or verified because the required functionality is not currently available in the inspected implementation.



## 5. Limitations



- Registration and login functionality is not available for execution.

- Duplicate username handling could not be tested.

- Password hashing and storage could not be verified.

- Login attempt restrictions and account lockout could not be tested.

- No authentication-specific automated test results are claimed.



## 6. Conclusion



The TASK-05 test scenarios were identified and reviewed against the available project implementation. However, registration and login functionality could not be tested because the authentication module is not available in the inspected source code.



Therefore, FR-01 (Passenger registration) and FR-02 (Passenger login) remain pending. These test cases should be executed after the AuthManager module is implemented and integrated into the application.
