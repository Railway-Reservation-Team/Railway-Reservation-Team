
# RRS-59: First-Time-User Usability Evaluation

## 1. Objective

Evaluate whether a first-time user can understand and use the Railway Reservation System without external assistance.

## 2. Application Overview

The application is a console-based railway reservation system.

The main menu provides:
- Book Ticket
- Cancel Ticket

The booking process asks for:
- Source station
- Destination station
- Journey date
- Train selection
- Number of passengers
- Travel class

The system displays matching trains, fares and seat availability.

## 3. Usability Test Scenarios

| ID | Test Scenario | Expected Result |
|---|---|---|
| UT-01 | Open the booking menu | User can identify available options |
| UT-02 | Enter source station | System accepts valid input |
| UT-03 | Enter empty source | Clear validation message appears |
| UT-04 | Enter destination station | System accepts valid input |
| UT-05 | Enter invalid journey date | System displays an error |
| UT-06 | Search for matching trains | Matching trains are displayed |
| UT-07 | Search for unavailable route | Informative no-trains message appears |
| UT-08 | View train details | Fare and seat availability are visible |
| UT-09 | Select a train | System accepts a valid selection |
| UT-10 | Enter passenger count | System accepts 1–6 passengers |
| UT-11 | Enter invalid passenger count | System displays an error |
| UT-12 | Select travel class | System displays relevant availability |
| UT-13 | Cancel a ticket | User receives a confirmation prompt |

## 4. Evaluation Method

A first-time user should attempt the scenarios without guidance.

Record:
- Whether the user completed the task.
- Time taken to complete the task.
- Errors encountered.
- Whether instructions were understandable.
- Suggestions from the user.

## 5. Evaluation Results

Actual tester observations must be recorded after running the application.

| Test ID | Actual Result | Status | Observation |
|---|---|---|---|
| UT-01 | Pending manual evaluation | Pending | |
| UT-02 | Pending manual evaluation | Pending | |
| UT-03 | Pending manual evaluation | Pending | |
| UT-04 | Pending manual evaluation | Pending | |
| UT-05 | Pending manual evaluation | Pending | |
| UT-06 | Pending manual evaluation | Pending | |
| UT-07 | Pending manual evaluation | Pending | |
| UT-08 | Pending manual evaluation | Pending | |
| UT-09 | Pending manual evaluation | Pending | |
| UT-10 | Pending manual evaluation | Pending | |
| UT-11 | Pending manual evaluation | Pending | |
| UT-12 | Pending manual evaluation | Pending | |
| UT-13 | Pending manual evaluation | Pending | |

## 6. Initial Usability Observations

The following are areas to evaluate during manual testing:

1. Whether menu instructions are easy to understand.
2. Whether input errors explain how to correct the problem.
3. Whether train details are easy to compare.
4. Whether fare and seat information are clearly displayed.
5. Whether the booking process is easy to follow.
6. Whether cancellation confirmation is understandable.

## 7. Potential Improvements

- Provide clearer prompts for station and date inputs.
- Display consistent validation messages.
- Make train information easier to compare.
- Provide clear confirmation messages after booking and cancellation.
- Help users understand available travel classes.

## 8. Conclusion

The usability evaluation covers the main passenger booking and cancellation workflows.

Final usability findings and completion status will be updated after manual testing with a first-time user.
