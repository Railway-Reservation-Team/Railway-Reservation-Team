# Integration Testing Report

## 1. Objective
Integration testing checks whether connected Railway Reservation System components work together correctly, particularly across the booking workflow and the services it depends on.

## 2. Test evidence
The GitHub Actions run [#43073959982](https://github.com/Railway-Reservation-Team/Railway-Reservation-Team/actions/runs/43073959982) executed the registered CTest suite on the Ubuntu build. The dedicated `BookingIntegrationTest` completed successfully.

| Test case | Integrated area | Expected outcome | Observed result |
|---|---|---|---|
| BookingIntegrationTest | Booking workflow and its collaborating components | The integration test completes without failure | Passed |

## 3. Execution summary
- Integration test cases explicitly identified in the CI test list: **1**
- Passed: **1**
- Failed: **0**
- CI run: **43073959982**
- Overall registered CTest suite: **20/20 passed**

## 4. Conclusion
The automated booking integration test passed in the referenced CI run. This is evidence that the tested booking-component integration behaved as expected under that test's conditions.

**Scope limitation:** This report does not claim that every end-to-end use case (UC-01 to UC-13), all external interactions, or manual acceptance scenarios have been integration-tested. Those walkthroughs remain separate validation activities.
