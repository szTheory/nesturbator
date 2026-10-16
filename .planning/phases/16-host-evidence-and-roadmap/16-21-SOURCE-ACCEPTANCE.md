---
phase: 16-host-evidence-and-roadmap
plan: "21"
status: blocked-after-invalid-fixture-red
date: 2026-10-16
runtime_acceptance: false
red_slot: consumed-invalid-fixture
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

## Invalid fixture RED and scoped correction

The subsequent resumed execution consumed RED21 at `build/phase16-acceptance21-execute/red` but failed before fault injection: a precreated quit sentinel caused HOST_DIED with an empty transport log. Its reproduction banner and completed ledger were false; control and primary-result evidence were absent and finalization failed. All original raw files remain untouched. The disposition sidecar binds their digests. Production, README and header remain unchanged; only the regression harness is uncommitted. No causal RED, GREEN, PREFLIGHT, FULL, source review, SUMMARY or requirement credit exists. The substantive fixture amendment identifies the corrections and authorizes one distinct corrected RED root, preserving this failed allowance.

## Second invalid local RED

The corrected-fixture root ended READINESS_TRANSPORT before injection because fake_nc looked beneath an extra attempts directory. control.json/evidence.json were absent; ledger planned/consumed. The shell wrapper lost its own wait code by assigning zsh readonly status. No causal proof, production correction or GREEN/FULL/review is credited. Raw bytes and their disposition sidecar remain retained. The bounded development amendment now permits source-bound local fixture corrections in four distinct rounds; formal acceptance counts remain global and unchanged.

## Four-round local development outcome — not accepted

All four permitted roots under `build/phase16-acceptance21-development` are retained. Round1 RED exited2 because the non-foreign case referenced an uninitialized foreign_path. Round2 RED exited2 because proof finalization lacked driver/fixture/frame environment, after directly observing READINESS_REPLY and a surviving owned host/lock. Round3 RED exited1 with causal observations; its GREEN exited1 because the complete-case override variable did not match run_fault, placing the real invocation outside its canonical case root. Each subsequent round changed the specifically diagnosed harness defect; no unchanged retry occurred.

Round4 RED exited1 and recorded actual request-directory injection after genuine readiness, with the owned host and endpoint lock surviving before emergency cleanup. Its GREEN exited0 and printed the eight-case banner. Primary observations were EXIT1/READINESS_REPLY879ms, INT130/INTERRUPTED_INT941ms, TERM130/INTERRUPTED_TERM894ms, HUP130/INTERRUPTED_HUP858ms, writable1/READINESS_REPLY829ms, already-quit1/QUIT_DEADLINE57560ms, foreign-endpoint1/ENDPOINT_BUSY52ms and foreign-owner-replacement1/READINESS_REPLY892ms. The complete case reached all sixteen boundaries and recorded ordinary quit/OK then quit/QUIT_DEADLINE with one physical QUIT. These are raw observations, not accepted matrix evidence.

The extracted read-only saved-proof gate rejects round4: the matrix duration is negative because Perl CLOCK_MONOTONIC and Python time.monotonic use different epochs on this host; the stdout digest predates the final banner; and four zero-send cleanup rows retain the original reason instead of CLEANUP_REQUEST_WRITE_FAILED. Individual assertion diagnostics found no additional rejection categories in the case identities, raw roles, lifecycle, foreign preservation or send contracts. No formal Task2 command, preflight, FULL or review ran.

After the invocation ended, the local repair sets the cleanup row's distinct reason while leaving FIRST_FAILURE intact. The harness now uses the same Perl monotonic clock for start/end, rejects nonpositive/negative elapsed accounting and enforces the exact zero-send diagnostic row before accepting the four write-failure cases. The foreign fixture installs actual INT/TERM/HUP logging traps before PID publication. Shell syntax, twelve embedded Python compilations and diff whitespace checks pass. Full stream/status hashes still require outer finalization after actual process wait. Four rounds are consumed; no further driver was launched, no source commit/candidate/SUMMARY exists, and no Task1 acceptance is credited. A new bounded continuation must be reconciled by the parent before another coherent unchanged-harness RED/GREEN pair.
