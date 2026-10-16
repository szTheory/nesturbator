---
phase: 16-host-evidence-and-roadmap
plan: "21"
status: blocked-before-runtime
date: 2026-10-16
runtime_acceptance: false
red_slot: unconsumed
green_slot: unconsumed
full_slot: unconsumed
review_slot: unconsumed
---

# Plan 21 source acceptance stop

Plan 21 stopped before any source edit or runtime slot was consumed. The task's required `already-quit` case has to drive the existing production session through all sixteen marker boundaries to the ordinary terminal `send_once QUIT` path. The same acceptance criteria cap that case's measured driver-plus-cleanup elapsed time at 5,000 ms.

The unchanged production path performs the complete sixteen-boundary progression, including two native anchor validations, before terminal QUIT. The checked-in README records the existing portable complete fake-host control at 53.2 seconds, before isolated case setup, lifecycle capture, or endpoint reacquisition. The driver also fixes its forty rewind delays at 50 ms each. This conflicts with the 5,000 ms per-case bound; the overall 60,000 ms matrix bound does not resolve the per-case conflict. Reporting a smaller elapsed value would misstate the actual invocation. Bypassing the marker path, extracting or directly mutating `QUIT_SENT`, or adding a production test mode would violate the plan's stated evidence contract.

## Evidence and disposition

- The recorded complete-path measurement is in `README.md` under the portable POSIX session-control description.
- `tests/retroarch/state_session.sh` sends ordinary QUIT only after the prescribed complete path; its fixed rewind gap is 50 ms and the loop sends forty rewinds.
- `16-21-PLAN.md` requires the existing complete fake-host fixture, ordinary terminal QUIT, and `elapsed_ms <= 5000` for every GREEN case.
- No RED, GREEN, preflight, FULL, or independent review command was launched. All five finite runtime allowances remain unconsumed; no result was adopted from Plan 20.
- `tests/retroarch/state_session.sh`, `tests/retroarch/test_state_session.sh`, `README.md`, `include/nesturbator.h`, `.planning/config.json`, and all historical Plan 20 evidence remain unchanged.

## Required next step

Revise and independently check Plan 21 so its already-quit observation can be both authentic and truthfully bounded. The amendment must reconcile the per-case and matrix limits with the measured complete-path cost, or specify an authorized, production-faithful bounded route to ordinary terminal QUIT. Then obtain a fresh Plan 21 scope check before dispatch. Until then, Plan 21 remains incomplete with no SUMMARY, requirement credit, FULL acceptance, review, or handoff to Plan 07. All mandatory parent, native, owner-consent, and release gates remain pending.
