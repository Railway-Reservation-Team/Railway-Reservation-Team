# Railway Reservation System

## Team 12 — Software Engineering Mini-Project

**PES University, Bangalore**  
**Department of Computer Science and Engineering**  
**Semester:** 5th Semester  
**Language:** C++17  
**Interface:** Console-based  
**Platform:** Windows 10/11 or Ubuntu 20.04 LTS  

## Team Members

| S.No | Name | SRN |
|---|---|---|
| 1 | PAVANKUMAR H | PES2UG24CS345 |
| 2 | POOJA KOPPAD | PES2UG24CS352 |
| 3 | POOJA KANAKAPPANAVARA | PES2UG24CS351 |
| 4 | POTHINENI DINESH | PES2UG24CS354 |

## 1. Project Overview

The Railway Reservation System is a standalone, single-user, console-based ticket reservation application developed using C++17 as part of the Software Engineering Mini-Project at PES University.

The system allows passengers to search trains, check seat availability, book tickets, make simulated payments, cancel bookings, receive refunds, check PNR status, view booking history, and generate e-tickets.

Administrators can manage train schedules, fares and seat quotas, and generate occupancy and revenue reports.

The application uses local data files for persistent storage and does not require network connectivity or an external database.

## 2. Main Features

### Passenger Features
- Passenger registration and login
- Account lockout after repeated failed login attempts
- Search trains by source, destination and journey date
- View class-wise fares and seat availability
- Temporary seat hold during checkout
- Book tickets for up to 6 travellers
- Generate a unique 10-digit PNR
- Confirmed, Waitlisted and RAC booking status
- Simulated payment processing
- Payment receipt generation
- Ticket cancellation and policy-based refunds
- Automatic waitlist upgrade after cancellation
- PNR status enquiry
- Booking history with upcoming/past filters
- E-ticket generation and local saving/printing

### Administrator Features
- Separate administrator login
- Add, edit and delete train schedules
- Manage train fares
- Configure seat quotas
- Generate seat-occupancy reports
- Generate revenue reports

## 3. System Modules

- **AuthManager** — registration, authentication and session handling
- **TrainSearch** — train search and fare/availability display
- **SeatInventory** — seat availability and temporary holds
- **BookingManager** — booking creation, PNR generation and status management
- **PaymentModule** — fare calculation, simulated payment and receipts
- **CancellationManager** — cancellation, refunds and seat release
- **PNRService** — PNR status enquiry
- **HistoryService** — booking history
- **TicketPrinter** — e-ticket generation
- **AdminTrainManager** — train schedule management
- **AdminFareManager** — fare and quota management
- **ReportGenerator** — occupancy and revenue reports
- **FileStore** — local persistent data storage

## 4. Technology Stack

- **Programming Language:** C++17
- **Build System:** CMake 3.16+ or Makefile
- **Compiler:** GCC 9+, MSVC 2019+, or equivalent
- **Testing:** GoogleTest 1.14+
- **CI/CD:** Jenkins
- **Static Analysis:** SonarQube or equivalent
- **Coverage:** gcov/lcov
- **Storage:** Local text/binary data files
- **Version Control:** Git and GitHub

## 5. Build Instructions

### Using CMake

Clone the repository:

```bash
git clone <repository-url>
cd Railway-Reservation-System
```

Create and enter the build directory:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

Run the generated executable according to the executable name configured in `CMakeLists.txt`.

### Using Makefile

If a Makefile is provided:

```bash
make
```

Then run the generated executable.

> Replace `<repository-url>` with your actual GitHub repository URL.

## 6. Usage

The application starts with a console-based main menu containing options such as:

```text
1. Register
2. Login
3. Search Trains
4. Admin Login
5. Exit
```

### Passenger Booking Flow

```text
Search Train
     ↓
Select Train/Class
     ↓
Check Seat Availability
     ↓
Select Seats
     ↓
Enter Passenger Details
     ↓
Make Simulated Payment
     ↓
Booking Confirmation
     ↓
Generate PNR / E-Ticket
```

### Cancellation Flow

```text
Enter PNR
     ↓
Validate Booking
     ↓
Calculate Refund
     ↓
Cancel Ticket
     ↓
Release Seat
     ↓
Check Waitlist Upgrade
```

## 7. Data Storage

The system uses local files instead of an external database.

Main data files include:

```text
users.dat
trains.dat
bookings.dat
```

Booking records use the format:

```text
<PNR>|<TRAIN_NO>|<DATE>|<CLASS>|<STATUS>|<PASSENGER_COUNT>|<TOTAL_FARE>
```

Example:

```text
1042837591|12801|2026-11-14|3A|Confirmed|2|1780
```

## 8. Default Configuration

| Parameter | Default |
|---|---:|
| Maximum passengers per booking | 6 |
| Seat hold duration | 300 seconds |
| PNR length | 10 digits |
| Session timeout | 15 minutes |
| Maximum login attempts | 5 |
| Account lockout duration | 15 minutes |
| Cancellation fee (>24h before departure) | 25% |
| Cancellation fee (within 24h) | 50% |

## 9. Security

The system specifies:

- Salted password hashing; plaintext passwords are not stored.
- Input validation and length checking.
- Separate administrator authentication.
- 15-minute inactive-session timeout.
- Local data files restricted to the application's own directory.
- Checksum or record-count checks for detecting accidental file corruption.

## 10. Testing and Code Quality

The project is designed to support:

- Unit testing using GoogleTest
- At least 70% branch coverage using gcov/lcov
- Jenkins-based automated build and testing
- SonarQube static analysis
- Branch-based development and pull-request reviews
- Modular C++ design with testable business logic

## 11. Performance Targets

- Train search: **≤ 2 seconds**
- Booking transaction: **≤ 3 seconds**
- Application startup: **≤ 3 seconds**
- Data file read/write: **≤ 200 ms**
- Active-session memory footprint: **≤ 128 MB**

## 12. Scope and Limitations

This version is an academic, standalone application.

It does **not** support:

- Multi-user concurrent access across different machines
- Network connectivity
- Real payment-gateway integration
- External database systems

All reservation and user data is stored locally.

## 13. GitHub Contribution

All source code should be maintained in the team's GitHub repository.

The project follows branch-based development with pull-request reviews and meaningful commit messages.

**Team requirement:** Each team member must make at least one GitHub commit contributing to at least one functional feature.

## 14. Documentation

The project SRS contains:

- Introduction and project scope
- Overall system description
- External interface requirements
- Use-case and booking state models
- Functional requirements
- Non-functional requirements
- Security requirements
- Business rules
- Glossary
- Data field layouts
- Requirement Traceability Matrix (RTM)

## 15. Academic Note

This project is developed exclusively for academic evaluation under the Software Engineering course at PES University. It is not intended for commercial distribution.

All team members should understand and be able to explain the submitted code during evaluation.

## 16. References

- ISO/IEC/IEEE 29148:2018 — Systems and Software Engineering: Requirements Engineering
- C++17 Standard
- C++ Standard Template Library (STL) Reference
- PES University SE Mini-Project Guidelines, Jackfruit 2026
- GitHub Education
- SonarQube Documentation
