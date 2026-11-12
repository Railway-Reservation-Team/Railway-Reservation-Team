# First-Time User Usability Checklist

Note: This checklist reflects a manual walkthrough by a project team member. Independent first-time-user validation is still pending.

## Application Access
- [x] Application starts successfully.
- [x] Main menu is displayed clearly.
- [x] Menu options are understandable.
- [x] Passenger registration and login work.

## Train Search
- [x] User can enter source and destination.
- [x] User can enter a journey date.
- [x] Matching trains are displayed.
- [x] No-result message is displayed when no records match.
- [ ] Empty source input is handled and tested.
- [ ] Invalid date format is rejected and tested.

## Booking
- [x] User can select a train.
- [x] Seat availability is displayed.
- [x] Passenger details can be entered.
- [x] Valid passenger count is accepted.
- [x] Invalid passenger count is rejected.
- [x] Unavailable travel class is handled.
- [x] User can change travel class.
- [x] Booking summary is displayed before confirmation.
- [x] Simulated successful payment completes booking.
- [x] Booking confirmation and PNR are displayed.

## Cancellation
- [x] User can enter a booking PNR.
- [x] Cancellation confirmation is requested.
- [x] Cancellation result is displayed.
- [x] Refund amount is displayed.
- [x] Seat restoration is reported.

## Error Handling
- [x] Invalid passenger count is rejected.
- [x] Unavailable travel class is handled.
- [ ] Empty inputs are fully tested.
- [ ] Invalid date formats are tested.
- [ ] Invalid train selections are tested.

## Evaluation Notes

- Test environment: Isolated temporary application copy.
- Test route: Bengaluru to Chennai.
- Test train: Shatabdi Express (12027).
- Test booking: One AC 3-Tier passenger.
- Payment: Simulated successful UPI payment.
- Cancellation: Successful; refund of Rs. 675.00 displayed.
- Independent first-time-user evaluation: Pending.

Do not treat this project-team walkthrough as an independent usability study.
