# Software Requirements Specification — Updates
## Railway Reservation System | Team 12

### 1. Project Overview
The Railway Reservation System is a standalone console-based application developed using C++17.

### 2. Functional Requirements and Implementation Status

| Requirement | Description | Current Status |
|---|---|---|
| Passenger registration | Create passenger accounts | Pending |
| Passenger authentication | Login and session handling | Pending |
| Train search | Search using source, destination and date | Implemented |
| Passenger validation | Validate passenger input | Implemented |
| Fare calculation | Calculate fare based on class and passenger count | Implemented |
| Booking | Create and persist booking records | Implemented and merged into main (PR #56) |
| PNR generation | Generate unique 10-digit PNR | Implemented and merged into main (PR #56) |
| Seat inventory | Update available seats after booking | Implemented and merged into main (PR #56) |
| Payment | Simulated payment processing | Pending |
| Cancellation | Cancel tickets and calculate refunds | Pending |
| Administrator | Train and schedule management | Pending |

### 3. Non-Functional Requirements
- Portability: C++17 application intended for Windows and Linux.
- Reliability: Validate inputs and handle errors.
- Data persistence: Store application records in local data files.
- Maintainability: Organize functionality into separate modules.

### 4. Implementation Notes
The current implementation provides train search, input validation and fare calculation. Booking persistence, PNR generation and seat inventory updates have been implemented and tested after TASK-09 safe integration (PR #56) was merged into main.

Authentication, payment, cancellation and administrator functionality remain pending implementation or verification.

### 5. Revision Note
This document supplements the original SRS. It does not replace the original requirements specification.
