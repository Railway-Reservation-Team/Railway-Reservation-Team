# Performance Benchmark Report — RRS-56

## 1. Objective

Verify train-search response time against the SRS and Jira performance requirements.

## 2. Test Environment

- Environment: Ubuntu WSL
- Dataset: 200 trains for representative testing
- Search samples: 100
- Timing method: C++ `std::chrono::steady_clock`
- Statistic: Nearest-rank 90th percentile (p90)

## 3. Search Scalability Benchmarks

| Dataset | Average Search Time |
|---|---:|
| 1,000 trains | 0.1003 ms |
| 10,000 trains | 0.8204 ms |
| 50,000 trains | 4.2734 ms |

## 4. Jira RRS-56 Response-Time Verification

Acceptance criteria: At least 90% of requests must complete within 3 seconds under normal load.

| Metric | Measured Result | Requirement | Status |
|---|---:|---:|---|
| Sample count | 100 | 100 | PASS |
| Average latency | 0.0175 ms | SRS limit: 2,000 ms | PASS |
| P90 latency | 0.0178 ms | 3,000 ms | PASS |
| Requests within 3 seconds | 100/100 | At least 90/100 | PASS |
| Compliance | 100% | At least 90% | PASS |

## 5. Additional SRS Checks

| Requirement | Measurement | Status |
|---|---|---|
| NFR-01: Search response | 0.0175 ms average | PASS |
| NFR-02: Booking operation | 1.3492 ms in earlier run | Partial verification |
| NFR-03: Startup time | Not measured | Pending |
| NFR-04: Train-file read | 0.0190 ms in earlier run | PASS |
| NFR-04: Other file operations | Not fully measured | Pending |
| NFR-05: Peak memory | Not measured | Pending |

## 6. Test Execution

- Performance test suite: Passed.
- Checks passed: 20.
- Checks failed: 0.
- CTest execution time: Approximately 0.56 seconds.

## 7. Limitations

The measurements were collected in a local Ubuntu WSL environment. They represent a controlled test run and do not establish production performance under real network or multi-user load.

Menu-operation timing, application startup time, peak memory, full booking transaction timing and additional file I/O measurements require separate verification.

## 8. Conclusion

The train-search performance acceptance criteria for Jira RRS-56 were satisfied in the recorded 100-sample test. All 100 requests completed within 3 seconds, and p90 latency was 0.0178 ms.

Additional SRS performance requirements remain partially verified or pending.
