# RRS-55 - Operational Availability Assessment

## 1. Requirement
SRS NFR-002 requires 99.5% operational availability during the defined operating period, excluding scheduled maintenance.

## 2. Application context
The Railway Reservation System is an offline, single-user C++17 console application. It is not a continuously running network service and does not currently have a long-running availability monitor.

## 3. Proposed operating period
Operating period: To be confirmed and approved by the project team.
Scheduled maintenance: Excluded from the measurement.
Measurement window: To be agreed before collecting availability evidence.

## 4. Measurement method
Record:
- Application launch date and time.
- Whether the application starts successfully.
- Whether the application remains usable during the agreed test session.
- Unexpected crashes or interruptions.
- Recovery and restart results.
- Scheduled maintenance periods.

Availability (%) =
((Total agreed operating time - Unplanned downtime) /
Total agreed operating time) * 100

## 5. Existing functional test evidence
The existing TEST_REPORT.md records:
- Total automated tests: 74
- Passed: 74
- Failed: 0

These results demonstrate the tested functionality only. They are not uptime measurements.

## 6. Availability evidence
No verified long-running uptime measurements are currently available.
Operating period and measurement window require team approval.

| Measurement | Result |
|---|---|
| Total operating time | Not yet measured |
| Scheduled maintenance | Not yet recorded |
| Unplanned downtime | Not yet measured |
| Calculated availability | Not yet established |
| 99.5% target | Not yet demonstrated |

## 7. Limitations
- No continuous uptime monitor is implemented.
- Automated functional tests do not establish operational availability.
- A short test session cannot establish long-term availability.
- The operating period must be approved before calculating the requirement.

## 8. Conclusion
RRS-55 has a documented proposed measurement method and identified evidence requirements. The 99.5% availability target cannot currently be marked as passed because verified uptime data and an approved operating period are unavailable.

## 9. Next actions
1. Obtain team approval for the operating period.
2. Conduct and record availability test sessions.
3. Record any unplanned downtime.
4. Calculate availability from the collected measurements.
5. Update this assessment with actual results.

## 10. Recorded Runtime Test - 12 November 2026

| Measurement | Observed result |
|---|---|
| Application | railway.exe |
| Start time | 12:16:57 |
| End time | 12:18:17 |
| Elapsed session time | 80.39 seconds |
| Main menu displayed | Yes |
| Normal exit | Yes |
| Process exit code | 0 |
| Test outcome | Passed |

The application launched successfully, displayed its main menu and exited normally. This is one successful runtime observation, not continuous uptime evidence or proof of the 99.5% target.
