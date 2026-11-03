# Requirements Traceability Matrix (RTM)
## Railway Reservation System — Team 12

| ID | Requirement | Related Module | Verification | Status |
|---|---|---|---|---|
| FR-01 | Passenger registration | AuthManager | Registration test | Pending |
| FR-02 | Passenger login | AuthManager | Authentication test | Pending |
| FR-03 | Search trains by source, destination and date | TrainSearch | Automated test | Passed |
| FR-04 | Display train availability and fares | TrainData, TrainSearch | Application test | Partially implemented |
| FR-05 | Validate passenger information | Validation | Automated test | Passed |
| FR-06 | Calculate ticket fare | FareCalculator | Automated test | Passed |
| FR-07 | Generate unique PNR | BookingManager | Booking test | Implemented and merged into main (PR #56) |
| FR-08 | Persist booking records | BookingManager | File verification | Implemented and merged into main (PR #56) |
| FR-09 | Update seat availability | SeatInventory | Booking integration test | Implemented and merged into main (PR #56) |
| FR-10 | Process payment | PaymentModule | Payment test | Pending |
| FR-11 | Cancel tickets and process refunds | CancellationManager | Cancellation test | Pending |
| FR-12 | PNR enquiry and booking history | PNRService | PNR test | Partially implemented |
| FR-13 | Administrator operations | AdminManager | Admin test | Pending |
| FR-14 | Error handling and input validation | Validation | Regression tests | Partially verified |

## Test Traceability

| Test ID | Requirement | Test Description | Result |
|---|---|---|---|
| UT-01 | FR-05 | Valid passenger name | PASS |
| UT-02 | FR-05 | Empty passenger name rejection | PASS |
| UT-03 | FR-05 | Valid date | PASS |
| UT-04 | FR-05 | Invalid date rejection | PASS |
| UT-05 | FR-05 | Invalid date format rejection | PASS |
| UT-06 | FR-05 | Valid passenger count | PASS |
| UT-07 | FR-05 | Zero passengers rejection | PASS |
| UT-08 | FR-05 | More than six passengers rejection | PASS |
| UT-09 | FR-06 | Valid fare input | PASS |
| UT-10 | FR-06 | Negative fare rejection | PASS |
| UT-11 | FR-14 | Valid menu choice | PASS |
| UT-12 | FR-14 | Invalid menu choice rejection | PASS |
| UT-13 | FR-03 | Matching train search | PASS |
| UT-14 | FR-03 | No matching train | PASS |
| UT-15 | FR-06 | Sleeper fare calculation | PASS |
| UT-16 | FR-06 | AC 3-Tier fare calculation | PASS |
| UT-17 | FR-06 | Invalid travel class rejection | PASS |
| UT-18 | FR-05 | Invalid passenger count rejection | PASS |

## Notes
- PASS means verified by the available automated tests.
- Implemented and merged into main (PR #56) means the implementation was tested there but is not necessarily merged into main.
- Pending means implementation or verification remains outstanding.
- This matrix should be updated when additional modules are completed.
