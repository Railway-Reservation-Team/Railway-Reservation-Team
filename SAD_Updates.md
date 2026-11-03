# Software Architecture and Design — Updates
## Railway Reservation System | Team 12

### 1. Architecture Overview
The application uses a modular C++17 console-based architecture.

### 2. Existing Modules

| Module | Responsibility |
|---|---|
| TrainSearch | Search trains by source, destination and journey date |
| TrainData | Load and parse train records |
| Validation | Validate passenger input, dates, fares and menu selections |
| FareCalculator | Calculate fares for different travel classes |
| main.cpp | Coordinate the console application workflow |

### 3. TASK-09 Integration Modules

| Module | Responsibility |
|---|---|
| BookingManager | Create booking records and generate PNRs |
| SeatInventory | Update seat availability in train data |
| Booking data file | Persist booking records in data/bookings.dat |

These modules are implemented on the TASK-09 development branch.

### 4. Data Storage

**Train data:** data/trains.dat

Format:
TRAIN_NO|TRAIN_NAME|SOURCE|DESTINATION|DEPARTURE|ARRIVAL|DURATION|FARES|SEAT_COUNTS|DATE

**Booking data:** data/bookings.dat

Format:
PNR|TRAIN_NO|DATE|CLASS|STATUS|PASSENGER_COUNT|TOTAL_FARE

### 5. Booking Workflow
1. Passenger enters journey details.
2. Application searches matching trains.
3. Passenger selects a train.
4. Passenger enters name and passenger count.
5. Passenger selects travel class.
6. Application calculates total fare.
7. Seat inventory is updated.
8. Booking record is saved.
9. A 10-digit PNR is displayed.

### 6. Error Handling
The application validates passenger input, passenger counts, dates, travel class and seat availability. Booking creation includes a seat rollback attempt if saving the booking fails.

### 7. Pending Architecture Components
Authentication, payment processing, cancellation, administrator management and complete booking history remain pending.

### 8. Revision Note
This document supplements TEAM12_SAD.pdf and describes the implementation currently available on the development branches.
