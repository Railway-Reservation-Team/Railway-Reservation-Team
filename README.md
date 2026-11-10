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
- Separate administrator authentication (specified; implementation pending)
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
- **Cryptography:** OpenSSL (libcrypto), used by passenger authentication
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
git clone https://github.com/Railway-Reservation-Team/Railway-Reservation-Team.git
cd Railway-Reservation-Team
```

Configure and build from the repository root:

```bash
cmake -S . -B build
cmake --build build
```

Run the application from the repository root so the relative `data/` paths resolve:

```bash
# Windows PowerShell
.\\build\\Debug\\railway.exe

# Linux / Ubuntu / WSL
./build/railway
```

Run the included module tests:

```bash
ctest --test-dir build --output-on-failure
```

### Using Makefile (Linux / Ubuntu / WSL with GNU Make)

Build the application:

```bash
make
```

Run it from the repository root:

```bash
./railway
```

Build and run the included module tests:

```bash
make test
```

Remove generated binaries:

```bash
make clean
```



## 6. Usage

The application starts with a console-based main menu containing options such as:

```text

1. Passenger Registration
2. Passenger Login
3. Passenger Booking
4. Admin Train Management
5. Passenger Logout
6. Exit
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
<PNR>|<TRAIN_NO>|<DATE>|<CLASS>|<PASSENGER_DETAILS>|<PASSENGER_COUNT>|<TOTAL_FARE>|<STATUS>
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

---

## Development and Testing

Requirements: g++, C++17, Linux/Ubuntu/WSL, Git.

Compile: `g++ -std=c++17 -Wall -Wextra -pedantic -Isrc src/*.cpp -o railway -lcrypto` (requires OpenSSL development libraries)

Run: `./railway`

Regression tests:
`g++ -std=c++17 -Wall -Wextra -pedantic -Isrc tests/test_modules.cpp src/Validation.cpp src/TrainSearch.cpp src/FareCalculator.cpp -o tests/test_modules`

`./tests/test_modules`

Documentation:
- RTM.md
- SRS_Updates.md
- SAD_Updates.md
- FINAL_TEST_RESULTS.md
- TEST_REPORT.md

The passenger authentication module has been implemented and integrated into the main application. Its dedicated test suite reports 20 passed and 0 failed. The complete application builds successfully. Separate administrator authentication remains pending implementation.

### Verified Automated Test Results

| Test Suite | Passed | Failed |
|---|---:|---:|
| Passenger authentication | 20 | 0 |
| US-13 passenger-count validation | 11 | 0 |
| Seat inventory | 42 | 0 |
| Validation, search and fare calculation | 18 | 0 |
| Passenger details and fare calculation | 13 | 0 |
| Booking manager | 8 | 0 |
| Admin train management | 8 | 0 |
| Admin fare and reservations | 6 | 0 |
| **Total** | **126** | **0** |

All eight test suites passed. The complete application compiled successfully.

See `FINAL_TEST_RESULTS.md` and `TEST_REPORT.md` for detailed results.


### Project build configuration (TASK-01)

The repository includes a root-level `CMakeLists.txt` and `Makefile`. Both use the C++17 standard, compile the application sources under `src/`, and provide a target for the existing `tests/test_modules.cpp` module test program. CMake copies the `data/` directory into the build directory; launch the application from the repository root when using the documented commands so runtime data paths remain consistent.
