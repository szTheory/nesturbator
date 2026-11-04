---
phase: 21-interrupt-latency-and-hijack
plan: "03"
subsystem: testing
tags: [cpu, irq, nmi, interrupt-entry, conformance]
requires:
  - phase: 21-02
    provides: poll-time IRQ history and the Phase 21 test baseline
provides:
  - Before-fix evidence for BRK, IRQ, and NMI entry origins and access contracts
  - Source-shaped BRK32 and IRQ64 paired table captures through the production bus
  - C1-C7 arrival, lifetime, wrapping, and finite-margin cutoff discriminators
affects: [21-04, interrupt-entry, phase-21-conformance]
actuals:
  tokens: 15059
  tasks: 2
  commits: 2
tech-stack:
  added: []
  patterns:
    - Compile-time test wrapper delegates scheduled accesses to the production bus
    - Paired answer tables compare captured guest observations under one shared alignment
key-files:
  created:
    - tests/core/test_interrupt_entry.c
  modified:
    - tests/interrupt_fixture.h
    - tests/cpu/test_cpu_unit.c
    - tests/CMakeLists.txt
    - .planning/phases/21-interrupt-latency-and-hijack/21-INTERRUPT-TIMING-ANNEX.md
    - tests/accuracy/baseline-metadata.txt
key-decisions:
  - "Treat the T5-phi1 after-C4/before-C5 mapping as an inference, tested with finite margin; no physical exact-tie winner is claimed."
  - "Compare each pair of complete guest-captured tables using one common exact or +1 alignment; reject mixed and per-byte alignments."
  - "Keep INT-02 and KEEP-01 open: this plan records before-fix evidence and runs the local CI lane, while later implementation and cross-lane evidence remain required."
patterns-established:
  - "Schedule interrupt inputs through a test-only wrapper that still executes production bus actions."
  - "Keep correct source rows as ordinary guards and confine demonstrated baseline mismatches to named expected-red assertions."
requirements-completed: []
coverage:
  - id: D1
    description: "Before-fix CPU/bus entry evidence covers origin, C1-C7 arrivals, finite-margin cutoff, paired status tables, request lifetime, and PC/stack wrapping."
    requirement: INT-02
    verification:
      - kind: integration
        ref: "core.interrupt_entry and cpu.unit; cmake --workflow --preset ci"
        status: pass
    human_judgment: true
    rationale: "The named expected-red guards deliberately record unresolved overlap behavior. INT-02 closes only after the implementation removes those expected-red wrappers and the source tables pass."
  - id: D2
    description: "All 114 AccuracyCoin starting-PASS records remain protected by unchanged campaign observations while this local CI run completes."
    requirement: KEEP-01
    verification:
      - kind: integration
        ref: "cmake --workflow --preset ci (local macOS arm64 lane)"
        status: pass
    human_judgment: true
    rationale: "KEEP-01 requires the listed checks across every CI lane; this execution provides local-lane evidence only."
duration: 41min
completed: 2026-10-28
status: complete
plan_head_before: ef825b22693fa72c504053c22130dd15c6887022
plan_head_after: 0599c319de06b416e5f5f742fbff063975ee0759
---

# Phase 21 Plan 03: Interrupt Entry Evidence Summary

**Source-shaped BRK32 and IRQ64 guest sweeps now expose the full paired pushed-status tables through the production bus, alongside finite-margin entry cutoffs and request-lifetime controls.**

## Performance

- **Duration:** 41 minutes
- **Started:** 2026-10-29T02:55:08Z
- **Completed:** 2026-10-29T03:36:19Z
- **Tasks:** 2
- **Files modified:** 6

## Accomplishments

- Added `core.interrupt_entry` plus CPU-origin checks for BRK, IRQ, and NMI through `cpu_step`, including exact seven-access expectations, status/return-PC checks, and PC/stack wrapping.
- Captured all 32 BRK and 64 IRQ rows from owned source-shaped guests running against production bus actions. Each paired table is checked against the complete source answer and one common +1 alternative; actual pushed-P observations feed the comparators.
- The captured pre-fix outcomes are explicit: BRK/NMI observes 10×`$A4` then 22×`$26` (source expects 10×`$A4`, 15×`$36`, 7×`$26`); BRK/lower observes 32×`$36` (source expects 10×`$36`, 15×`$00`, 7×`$36`); IRQ/NMI observes 10×`$A4` then 54×`$26` (source match); IRQ/lower observes 64×`$26` (source expects 10×`$26`, 26×`$00`, 28×`$26`). The affected table verdicts are wrapped as expected-red evidence.
- Added all C1-C7 arrivals, finite-margin before/after selection discriminators, late sustained and missed short pulse cases, held-level and new-edge behavior, APU IRQ acknowledgement cases, and named wrong-model controls with clean siblings.
- Preserved all 114 AccuracyCoin starting-PASS records and their campaign report/counts; only the stale build-input fingerprint was refreshed after adding the registered test source.
- Ran the focused set successfully (5/5) and the required `cmake --workflow --preset ci` successfully (691/691). The workflow also completed its package steps.

## Task Commits

Each task was committed atomically:

1. **Task 1: Constrain entry origin, cutoff and recognition before the fix** - `10ac503` (test)
2. **Task 2: Cover arrival sweeps, request lifetime and complete paired tables** - `0599c31` (test)

**Plan metadata:** `cfbed76` (summary, STATE, and ROADMAP).

## Files Created/Modified

- `tests/core/test_interrupt_entry.c` - Production-bus entry schedules, complete table observations, cutoff/lifetime controls.
- `tests/interrupt_fixture.h` - Source-pinned expected tables and schedule metadata.
- `tests/cpu/test_cpu_unit.c` - Origin, access sequence, stack and return-PC checks.
- `tests/CMakeLists.txt` - Registers the interrupt-entry test.
- `.planning/phases/21-interrupt-latency-and-hijack/21-INTERRUPT-TIMING-ANNEX.md` - Corrected source sentinels and documented the mapping evidence and limits.
- `tests/accuracy/baseline-metadata.txt` - Updated only the build-input fingerprint; campaign report digest and PASS/FAIL/not-run counts remain unchanged.

## Decisions Made

- The source's T5-phi1 mapping to after C4 PCL and before C5 P is treated as an inference, not as a measured physical tie. Finite-margin cases distinguish it from entry-start and external-C6 selection; exact zero-margin precedence remains unmeasured.
- Guest observations, not constants compared with themselves, form both complete paired tables. A single alignment applies to both handlers in a pair, and mixed/per-byte choices are rejected.
- The expected-red wrappers remain limited to demonstrated pre-fix divergences and assert the intended failure; ordinary correct rows and clean sibling cases remain unwrapped.
- INT-02 and KEEP-01 are intentionally not marked complete in this plan. Phase 21-04 must close the overlap behavior, and KEEP-01 also requires all listed checks on every CI lane.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Incorrect source-order sentinels] Corrected annex sentinels against the pinned AccuracyCoin source**
- **Found during:** Task 1 (Constrain entry origin, cutoff and recognition before the fix)
- **Issue:** The existing annex sentinel rows did not match the pinned complete source tables.
- **Fix:** Audited the source and corrected the BRK/IRQ NMI and lower-handler sentinels before using them as expected results.
- **Files modified:** `.planning/phases/21-interrupt-latency-and-hijack/21-INTERRUPT-TIMING-ANNEX.md`
- **Verification:** Guest-captured full tables and source comparison are exercised by `core.interrupt_entry` in the passing full workflow.
- **Committed in:** `10ac503`

**2. [Rule 3 - Blocking CI fingerprint] Refreshed the baseline build-input fingerprint after registering the test**
- **Found during:** Task 2 (Cover arrival sweeps, request lifetime and complete paired tables)
- **Issue:** The first full workflow run reported only `accuracycoin.activation` failing because its recorded build-input fingerprint was stale after `tests/CMakeLists.txt` changed.
- **Fix:** Updated `build_inputs_sha256` to `755f9ccce31bdeb069e3dbdc175078439b46ce67a6e87e628c0e35f5288702ef`; retained the campaign report digest and the 114 PASS / 30 FAIL / 2 not-run observations.
- **Files modified:** `tests/accuracy/baseline-metadata.txt`
- **Verification:** Activation and campaign focused tests passed 2/2; final `cmake --workflow --preset ci` passed 691/691.
- **Committed in:** `0599c31`

**Total deviations:** 2 auto-fixed (1 Rule 1, 1 Rule 3)
**Impact on plan:** Both corrections were required to keep source expectations factual and restore the metadata gate; no production code or campaign observations changed.

## Issues Encountered

- The first full workflow run stopped at the stale fingerprint check (690/691). After refreshing the input fingerprint, the complete workflow passed (691/691).
- Source-pinned overlap rows still expose the known pre-fix divergences behind named expected-red wrappers. These remain explicit inputs for Plan 21-04, not claimed fixes.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

Plan 21-04 can use the runnable entry and lifetime discriminators to implement the shared entry path. The table captures now make BRK and IRQ behavior independently observable and preserve the exact/source alternatives. No physical exact-tie conclusion is made. INT-02 and KEEP-01 remain pending until the implementation removes the named expected-red cases, the campaign rows pass, and every required CI lane is verified.

## Self-Check: PASSED

- Summary file exists at `.planning/phases/21-interrupt-latency-and-hijack/21-03-SUMMARY.md`.
- Task commits `10ac503` and `0599c31` are ancestors of HEAD.

---
*Phase: 21-interrupt-latency-and-hijack*
*Completed: 2026-10-28*
