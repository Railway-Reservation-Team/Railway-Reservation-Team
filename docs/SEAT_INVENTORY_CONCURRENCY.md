
# Seat Inventory Concurrency Testing

## 1. Objective

The objective is to verify that seat inventory operations work correctly when multiple threads access the reservation system simultaneously.

## 2. Thread Safety

The SeatInventoryManager uses a recursive mutex to protect shared seat inventory data and prevent race conditions.

## 3. Test Scenarios

| Test | Description | Expected Result |
|---|---|---|
| Concurrent reservation | 20 threads request 10 available seats | Only 10 succeed |
| Stress testing | 50 threads compete for 25 seats across 20 iterations | No overbooking |
| Concurrent release | 20 threads release the same hold | Only one succeeds |
| Concurrent confirmation | 20 threads confirm the same hold | Only one succeeds |

## 4. Boundary Testing

The following conditions are also verified:

- Requests exceeding available seats are rejected.
- Zero-seat requests are rejected.
- Negative-seat requests are rejected.
- Requests exceeding six passengers are rejected.
- Releasing a hold restores seat availability.

## 5. Results

The concurrency and stress test executables passed their individual test runs.

The complete automated test suite previously reported 28 out of 28 tests passing.

## 6. Conclusion

The tests help verify that simultaneous reservation operations do not cause duplicate hold release, duplicate confirmation, or seat overbooking.
