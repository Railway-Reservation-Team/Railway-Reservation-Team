# Performance Benchmark Report — RRS-56

## 1. Objective

Evaluate train search performance and selected booking and file-read
operations against the performance requirements in the SRS.

## 2. Test Configuration

- Search algorithm: Linear search
- Iterations: 100 per dataset
- Dataset sizes: 1,000, 10,000 and 50,000 trains
- Typical SRS dataset: 200 trains
- Framework: CMake and CTest
- Timing: C++ steady clock with fractional millisecond precision

## 3. Train Search Benchmark Results

| Dataset Size | Average Search Time |
|---|---:|
| 1,000 trains | 0.1480 ms |
| 10,000 trains | 0.8767 ms |
| 50,000 trains | 4.3865 ms |

### Observations

- Search time increases as dataset size increases.
- All three benchmark searches returned the expected result.
- The results provide a baseline for future optimization.

## 4. SRS Performance Verification

| Requirement | SRS Limit | Measured Result | Status |
|---|---:|---:|---|
| NFR-01: Search (200 trains) | 2,000 ms | 0.0286 ms | PASS |
| NFR-02: Booking creation | 3,000 ms | 1.3492 ms | Partial |
| NFR-03: Application startup | 3,000 ms | Not measured | Pending |
| NFR-04: Train-file read | 200 ms | 0.0190 ms | PASS |
| NFR-04: Other required file I/O | 200 ms | Not fully measured | Pending |
| NFR-05: Peak memory | 128 MB | Not measured | Pending |

### Verification Notes

- NFR-01 was tested using a typical dataset of 200 trains.
- NFR-02 measures BookingManager booking creation. Complete seat-hold-to-confirmation timing still needs verification.
- NFR-04 currently measures train-file reading only.
- NFR-03 and NFR-05 require separate startup and memory measurements.

## 5. Test Execution

Command:

`ctest --test-dir build -V -R '^PerformanceTests$'`

Results:

- PerformanceTests: PASSED
- Checks passed: 16
- Checks failed: 0
- CTest execution time: approximately 0.57 seconds

Explicit pass/fail checks are used instead of assert(), so checks remain active in Release builds.

## 6. Conclusion

The latest performance test verified train search response time for a
typical 200-train dataset against NFR-01. Booking creation and train-file
reading were also within their respective SRS limits.

Startup time, the complete booking transaction, remaining file I/O
operations and peak memory still require measurement before all
performance requirements can be considered fully verified.
