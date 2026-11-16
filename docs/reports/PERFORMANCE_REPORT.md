# Performance Validation Report

Source: GitHub Actions workflow run [#43073959982](https://github.com/Railway-Reservation-Team/Railway-Reservation-Team/actions/runs/43073959982).

The automated performance checks used a seeded dataset of **200 trains**.

| Requirement | Measured operation | Measured time | Limit | Result |
|---|---|---:|---:|---|
| NFR-01 | Train search | 0 ms | 2,000 ms | Within limit |
| NFR-02 | Booking | 2 ms | 3,000 ms | Within limit |
| NFR-04 | Train-file read | 0 ms | 200 ms | Within limit |
| NFR-03 | Application startup | Not measured | — | Open |
| NFR-05 | Peak memory | Not measured | — | Open |

## Conclusion
The measured NFR-01, NFR-02 and NFR-04 operations were within their stated limits for this automated run. Startup timing and peak memory were not measured by this test and remain open.
