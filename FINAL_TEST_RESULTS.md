# Final Test Results
## Railway Reservation System — Team 12

### 1. Test Environment
- Language: C++17
- Compiler: g++
- Flags: -Wall -Wextra -pedantic
- Test framework: Custom C++ automated test program

### 2. Automated Regression Results

| Test Category | Tests Passed |
|---|---:|
| Passenger validation | 2 |
| Date validation | 3 |
| Passenger count validation | 3 |
| Fare input validation | 2 |
| Menu validation | 2 |
| Train search | 2 |
| Fare calculation | 4 |
| **Total** | **18/18** |

Failed tests: 0

### 3. Booking Integration Evidence (TASK-09 branch)

| Test | Observed Result |
|---|---|
| Train search | Successful |
| Passenger entry | Successful |
| Fare calculation | Rs. 3,600 for two AC 3-Tier passengers |
| Booking persistence | Successful |
| PNR generation | 10-digit PNR generated |
| PNR lookup | Successful |
| Seat inventory update | AC 3-Tier seats reduced from 80 to 78 after two separate two-passenger bookings |

### 4. Outstanding Test Scenarios
- Passenger authentication
- Payment success and failure
- Cancellation and refund calculation
- Administrator operations
- Complete end-to-end regression testing across all integrated modules

### 5. Conclusion
The available automated regression suite passed all 18 tests. Booking persistence, PNR retrieval and seat inventory updates were also manually verified after TASK-09 safe integration (PR #56) was merged into main.

The results do not establish that every system requirement has been implemented or tested.
