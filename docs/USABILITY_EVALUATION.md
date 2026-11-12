# RRS-59: First-Time-User Usability Evaluation

## 1. Objective

Evaluate whether a user can understand and use the Railway Reservation System's passenger workflows, including registration, login, train search, booking, payment simulation and cancellation.

## 2. Application Overview

The application is a console-based railway reservation system.

The passenger workflow includes:
- Passenger registration and login.
- Train search using source, destination and journey date.
- Train selection and seat availability display.
- Passenger details entry.
- Travel class selection.
- Booking confirmation and simulated payment.
- Ticket cancellation and refund display.

## 3. Evaluation Method

A manual walkthrough was performed using an isolated test copy of the application and dummy passenger details.

The walkthrough was conducted by a project team member. It was not an independently recruited first-time-user study.

Test data:
- Source: Bengaluru
- Destination: Chennai
- Journey date: 2026-12-01
- Train: Shatabdi Express (12027)
- Test passenger: Usability Tester
- Travel class: AC 3-Tier

No real payment credentials were used. The application’s simulated successful payment option was selected.

## 4. Usability Test Scenarios

| ID | Test Scenario | Actual Result | Status | Observation |
|---|---|---|---|---|
| UT-01 | Open the application menu | Main menu and seven numbered options displayed | PASS | Login status was visible |
| UT-02 | Register and log in | Registration and login succeeded | PASS | Confirmation messages were displayed |
| UT-03 | Enter empty source | Not tested | PENDING | Requires additional validation |
| UT-04 | Enter destination station | Chennai accepted as destination | PASS | Search continued successfully |
| UT-05 | Enter invalid journey date format | Not tested | PENDING | A malformed date was not entered |
| UT-06 | Search for matching trains | Shatabdi Express was displayed | PASS | Route and journey date matched |
| UT-07 | Search when no train records match | No train records found for the tested date | PASS | No-results message was displayed |
| UT-08 | View train details | Train number, fare and seat availability displayed | PASS | Class-wise availability was visible |
| UT-09 | Select a train | Train option 1 accepted | PASS | Booking continued |
| UT-10 | Enter valid passenger count | One passenger accepted | PASS | Passenger details prompt appeared |
| UT-11 | Enter invalid passenger count | Zero rejected with an error message | PASS | Prompt repeated for valid input |
| UT-12 | Select travel class | Unavailable Sleeper class rejected; AC 3-Tier accepted | PASS | User could return and select another class |
| UT-13 | Cancel a ticket | Cancellation confirmation and result displayed | PASS | Refund and seat restoration were reported |

## 5. Booking and Payment Walkthrough

The following steps were completed successfully:

1. Registered a dummy passenger.
2. Logged in with the registered account.
3. Searched for Bengaluru to Chennai on 2026-12-01.
4. Selected Shatabdi Express (12027).
5. Entered an invalid passenger count of zero and observed validation.
6. Selected an unavailable Sleeper class and observed the availability message.
7. Changed the travel class to AC 3-Tier.
8. Entered passenger details and reviewed the booking summary.
9. Selected UPI and simulated successful payment.
10. Received a booking confirmation.

Booking evidence:
- PNR: 1571738030
- Passenger count: 1
- Travel class: AC 3-Tier
- Fare: Rs. 900.00
- Payment status: SUCCESS
- Booking status: Confirmed

## 6. Cancellation Walkthrough

The test booking was cancelled using its PNR.

Observed output:
- PNR: 1571738030
- Cancellation charge: Rs. 225.00
- Refund amount: Rs. 675.00
- Booking status: Cancelled
- Seats restored successfully.
- Cancellation details saved.

The application displayed a successful cancellation message.

## 7. Initial Usability Observations

1. The main menu clearly displayed numbered options and login status.
2. Registration and login displayed confirmation messages.
3. Train search results presented route, date, fare and seat availability together.
4. Invalid passenger count was rejected with a corrective message.
5. The unavailable travel class message provided options to retry or change class.
6. Booking review displayed passenger information and total fare before confirmation.
7. Payment simulation clearly separated successful and failed payment options.
8. Booking confirmation displayed the PNR and payment status.
9. Cancellation displayed the cancellation charge, refund amount and seat restoration result.
10. The password was visibly entered in the terminal during login; hiding password input would improve privacy.
11. An additional independent first-time-user evaluation is still recommended.

## 8. Potential Improvements

- Hide password characters during terminal input.
- Improve validation for empty station names and malformed dates.
- Make no-results messages identify the route and journey date.
- Continue testing with users who have not previously used the application.
- Record task completion time and user feedback in a future independent evaluation.

## 9. Conclusion

The manual walkthrough successfully covered the main passenger registration, login, train search, booking, simulated payment and cancellation workflows.

The tested scenarios passed based on the observed application output. Empty-source validation and malformed-date validation remain pending.

This evaluation is a project-team walkthrough and should not be presented as an independent first-time-user study. Further testing with an independent user is recommended.
