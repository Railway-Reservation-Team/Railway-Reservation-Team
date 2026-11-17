---
name: coverage-plan
description: Plan to raise GCC branch coverage from 55.4% to 70%
metadata:
  type: project
---

Baseline (this repo's build-branch-coverage): 55.5% (3389/6103). Target 70% = ~4272 covered branches (~883 more).

Per-file uncovered (after batch 1 + smoke test):
- main.cpp: 1132 total, 434 covered (38%) → ~698 uncovered — BIGGEST target
- BookingManager.cpp: 605/344 → ~261 uncovered
- Cancellation.cpp: 607/342 → ~265 uncovered
- AuthManager.cpp: 572/346 → ~226 uncovered
- ETicketManager.cpp: 415/224 → ~191 uncovered
- AdminFareReservationManager.cpp: 390/209 → ~181 uncovered

Strategy:
- main.cpp is interactive (std::cin). Drive it via ConsoleMenuSmoke.cmake piped stdin (already works; covers booking/cancellation/admin/menu paths). Extend input to cover: payment FAILED receipt path, booking declined at confirmation, invalid registration, empty PNR cancellation, cancellation-N, full success+E-ticket.
- BookingManager/Cancellation: waitlisted booking, partial waitlist promotion, deactivated train, rollback paths — added in batch 1 (BookingManagerTests, CancellationTests).
- AuthManager/ETicketManager/AdminFareReservation: direct unit tests on failure/invalid-input branches.

Batches:
1. DONE: BookingManager waitlisted+deactivated-train tests + Cancellation partial-promotion test. Extended ConsoleMenuSmoke input. All 31 CTest pass. Coverage ~55.3% (baseline-equivalent).
2. IN PROGRESS: Extend ConsoleMenuSmoke to cover main.cpp payment-Failed-receipt, booking-declined-at-confirmation, empty-PNR, cancellation-N, success+e-ticket paths.
3. Unit tests for AdminFareReservationManager + ETicketManager failure paths.
4. Rebuild, run full CTest, measure coverage; repeat until 70% or explain blockers.

Rules followed: no behavior changes, no removed assertions, existing tests preserved, coverage measured via supplied gcovr command each batch.