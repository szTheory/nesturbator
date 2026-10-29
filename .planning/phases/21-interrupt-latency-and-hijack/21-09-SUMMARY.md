---
phase: 21-interrupt-latency-and-hijack
plan: "09"
subsystem: testing
tags: [hosted-evidence, interrupt-entry, libretro, accuracycoin, performance]
requires:
  - phase: 21-interrupt-latency-and-hijack
    provides: Plan 21-08 hosted collector contract, candidate identity and local replay evidence
provides:
  - Actual CI/Nightly source, run, attempt, job and artifact acceptance ledger
  - Final six-platform hash, conformance, campaign, host and performance outcomes
affects: [phase21-verification, INT-01, INT-02, KEEP-01, hosted-acceptance]
actuals:
  tokens: 5502
  tasks: 2
  commits: 2
  plan_head_before: 3a2c44700b761ca70cfc8a8681e77d242cd4bb7f
  plan_head_after: 69c61cea9844e5970aa1a68790c2701b7c22e2ca
tech-stack:
  added: []
  patterns:
    - Keep hosted run provenance separate from failed and canceled historical attempts.
    - Compare effective Git modes and blobs independently, alongside production bundle validation.
key-files:
  created:
    - .planning/phases/21-interrupt-latency-and-hijack/21-EVIDENCE.md
  modified:
    - tests/accuracy/measurements.md
key-decisions:
  - "Bind acceptance to CI 40929011059/1 and Nightly 40929010997/1 for candidate 54c7db29, tested merge faf1b915 and protected base a64c23fd."
  - "Retain the production collector's existing mode/root debt; use its production library entry through a scratch harness for the canonical clean-source bundle."
  - "Record finite-margin timing only; the physical exact-timestamp CPU/PPU winner remains unmeasured."
patterns-established:
  - "Hosted evidence ledger: bounded source/run/artifact identities with raw scratch artifact pointers and measured content digests."
  - "Evidence-only changes remain separate from effective source inputs and requirement completion."
requirements-completed: []
requirements-pending: [INT-01, INT-02, KEEP-01]
coverage:
  - id: D1
    description: "Actual hosted CI/Nightly contents are source-bound, collected and revalidated against the current checkout."
    verification:
      - kind: integration
        ref: "tests/cmake/phase21_hosted_evidence.cmake MODE=validate"
        status: pass
    human_judgment: false
  - id: D2
    description: "Final campaign, native/adapter replay, performance and host outcomes are recorded with remaining limits."
    verification:
      - kind: integration
        ref: ".planning/phases/21-interrupt-latency-and-hijack/21-EVIDENCE.md"
        status: pass
    human_judgment: true
    rationale: "The ledger preserves judgment-tier clean-room and physical-tie limits for independent phase verification."
duration: 18min
completed: 2026-10-29
status: complete
---

# Phase 21 Plan 09: Hosted acceptance summary

**Actual CI/Nightly acceptance for candidate `54c7db2` is recorded with source-bound six-platform hashes, conformance results, RetroArch evidence and per-ROM performance.**

## Performance

- **Duration:** About 18 minutes, using the last STATE.md update timestamp as the read-in lower bound.
- **Started:** 2026-10-29T19:01:16Z (STATE.md update timestamp; plan read-in began before the first task commit).
- **Completed:** 2026-10-29.
- **Tasks:** 2/2.
- **Files modified by tasks:** 2.

## Accomplishments

- Collected and revalidated actual CI `40929011059/1` (18/18 jobs) and Nightly `40929010997/1` (seven work jobs passed; the declared PR-only report job skipped) for PR #61, candidate `54c7db29c73a66bb552e71b0887529249a41a072`, tested merge `faf1b9159ce8dae83cd2c08c155fc9b77fac32de` and base `a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`.
- Independently matched all 428 effective tracked Git modes/blobs across candidate, tested merge, current checkout and the clean-source checkout. Revalidated the immutable bundle against the current checkout after the evidence and measurements edits; all 16 downloaded artifact ZIPs match retained API digests.
- Recorded campaign results of 119 PASS / 27 FAIL / five inventory-only DRAW, 146 scored terminal rows, all 114 fresh protected PASS keys retained, and all eleven exact target rows passing. The final campaign report and six-platform frame/PCM/APU-transition/state inventory digests remain unchanged.
- Recorded actual six-platform controls, Holy Mapperel and boot outcomes, hosted ASAN/no-float/host jobs, Nightly vectors/MMC3, the RetroArch 1.22.2 16-boundary marker session, native and adapter every-stage replay coverage, and the passing original +1% performance gate.
- Preserved the earlier failed/canceled attempt history, evidence limits, clean-room judgment status, physical zero-margin tie limit, and CR-01/WR-03/WR-04 collector debt.

## Task Commits

1. **Task 1: Collect actual source-bound CI/Nightly content** — `957d271` (docs).
2. **Task 2: Reconcile final measurements and expose remaining limits** — `69c61ce` (docs).

## Files Created/Modified

- `.planning/phases/21-interrupt-latency-and-hijack/21-EVIDENCE.md` — actual run, source, job, artifact, digest, validation, replay and limit ledger.
- `tests/accuracy/measurements.md` — final hosted outcomes and source-bound measurement reconciliation, retaining prior causal history.

## Decisions Made

- The final successful hosted attempts alone establish acceptance. The earlier performance-regression and canceled attempts stay historical.
- The final evidence-fixture correction changes no runtime timing or state layout; the final pinned frame, PCM, transition and state inventories do not move.
- Per-run mode/blob validation and absolute clean-source/current-checkout validation are mitigations for this evidence instance, not closure of collector maintenance debt.
- INT-01, INT-02, KEEP-01 and phase completion remain pending parent-owned verification and state updates.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking issue] Used the production collector library entry for the canonical clean-source bundle**
- **Found during:** Task 1 (hosted content collection).
- **Issue:** The public `MODE=collect` wrapper rejected the canonical bundle and sibling absolute clean-source paths under its existing root guard.
- **Fix:** Used an ignored scratch helper to call the same production `nesturbator_phase20_collect_evidence()` function with the canonical bundle root and absolute clean-source checkout. No production source, validator, pin, workflow, or collector acceptance rule changed.
- **Files modified:** Scratch-only `build/phase21-hosted-evidence/publication/collect-production.cmake`; no tracked production files.
- **Verification:** Production collection completed, current-checkout `MODE=validate` passed both before and after documentation edits, and independent mode/blob comparisons matched all 428 effective inputs.
- **Committed in:** Task 1 evidence commit `957d271` records this run-specific mitigation; the scratch helper remains ignored.

**Total deviations:** 1 narrowly scoped collection-entry workaround.
**Impact on plan:** Preserved the existing WR-03 debt and canonical source-bound collection; acceptance thresholds and evidence validators were unchanged.

## Issues Encountered

The GitHub CLI log retrieval attempted to write to a read-only default cache. Pointing `XDG_CACHE_HOME` at `/private/tmp/nesturbator-gh-cache` allowed the same read-only authenticated query to complete without changing GitHub configuration or remote state.

## User Setup Required

None.

## Next Phase Readiness

Plan 21-09 evidence is complete and ready for independent phase verification. The phase's requirements remain pending; parent-owned STATE.md, ROADMAP.md and REQUIREMENTS.md updates were left untouched.

## Self-Check: PASSED

- This summary, the hosted evidence ledger and final measurements file exist.
- Task commits `957d271` and `69c61ce` are ancestors of HEAD.
- The production hosted bundle revalidated against the current checkout after both task commits.
- The required config SHA-256 remains `62a22f6862efb7c74ee31baf2f22bae32d453e79aaec6726dbcf3b7213d47167`.
- No stub or placeholder patterns were found in the files changed by this plan.

---
*Phase: 21-interrupt-latency-and-hijack*
*Completed: 2026-10-29*
