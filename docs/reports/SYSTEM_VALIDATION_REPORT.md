# System Validation Report

Source: GitHub Actions workflow run [#43073959982](https://github.com/Railway-Reservation-Team/Railway-Reservation-Team/actions/runs/43073959982).

## Automated validation
The CI workflow validates:
- Project compilation.
- Registered CTest suites.
- Cppcheck static analysis.
- Code coverage generation.
- Automated NFR-01, NFR-02 and NFR-04 measurements.

The registered CTest suite passed **20 out of 20 tests** in the referenced run.

## Validation items still open
The following activities are not established as passed by this automated report:
- NFR-03 application startup timing.
- NFR-05 peak memory measurement.
- Manual end-to-end acceptance walkthroughs for UC-01 to UC-13.
- Security checks not represented by automated tests.

## Conclusion
The automated CI checks completed for the referenced run. The open items above require separate measurement or manual validation and are not claimed as completed.
