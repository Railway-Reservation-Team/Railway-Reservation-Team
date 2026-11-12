# Seat Inventory Test Guide

## Objective
Verify seat availability, reservation holds and inventory consistency.

## Test Categories

### 1. Availability
- Check availability for Sleeper.
- Check AC 3-Tier and AC 2-Tier.
- Check Chair Car.
- Reject invalid travel classes.

### 2. Reservation Validation
- Accept valid seat requests.
- Reject zero and negative requests.
- Enforce the six-passenger booking limit.
- Reject requests exceeding availability.

### 3. Temporary Holds
- Create a temporary seat hold.
- Verify availability decreases.
- Release a hold and restore availability.
- Verify five-minute hold expiry.

### 4. Concurrency
- Test simultaneous seat reservations.
- Test concurrent hold releases.
- Test concurrent hold confirmations.
- Run repeated reservation stress tests.

## Conclusion
These tests help verify inventory correctness, reservation validation and thread safety.
