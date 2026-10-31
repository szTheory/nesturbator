---
phase: 24-correct-remaining-ntsc-dma-behavior
reviewed: 2026-10-31T21:32:50Z
depth: deep
files_reviewed: 8
files_reviewed_list:
  - src/apu.c
  - src/bus.c
  - src/internal.h
  - src/state.c
  - include/nesturbator.h
  - tests/cpu/test_bus.c
  - tests/core/test_state_fork.c
  - tests/core/test_state_reject.c
findings:
  critical: 0
  warning: 0
  info: 0
  total: 0
status: clean
---

# Phase 24 Plan 03: Code Review Report

**Reviewed:** 2026-10-31T21:32:50Z
**Depth:** deep
**Files Reviewed:** 8
**Status:** clean

## Summary

Reviewed the approved abort countdown, request lifecycle, bus timing, native serialization, and focused regression controls. The countdown is captured before the current CPU clock expires it; a due read repeats once, a due write consumes it without normal DMA retry, and synchronous OAM consumes the countdown without stealing a memory slot. The new native fork reaches the countdown through guest execution at a public frame boundary and checks resumed output and state. No findings in this scope. No tests were run for this review; bounded passing results were reported by the implementation owner. Consumer pin and documentation updates were still in progress and are not covered by this review.

## Narrative Findings (AI reviewer)

No findings.

---

_Reviewed: 2026-10-31T21:32:50Z_
_Reviewer: gsd-code-reviewer agent_
_Depth: deep_

## Addendum: Consumer and documentation review

**Reviewed:** 2026-10-31

The current-source docs and consumers now consistently report format 6 and
behavior revision 13: README, public header, API/progress/reproduction guides,
API and C/C++ header assertions, state-range rejection, the Phase 21 hosted
contract, reproduction contract, vector API policy, and `tests/runner/hashes.txt`.
The format-5 and revision-12 mentions in the API history, header history,
README's revision-12 entry, and historical campaign records describe prior
behavior and remain correctly labeled as history. The AccuracyCoin revision-9
campaign metadata was not relabeled.

The vector API declaration change is limited to the approved version macros;
the owner reports an independent normalized-stream proof. The hash-table diff
updates only the six native-state hash/size groups and `state_format`; ticks and
non-state rows are unchanged. The owner's normalization check reports that
removing the new abort byte and restoring the prior format/revision/CRC
reproduces every previous native-state hash. I inspected the edited rows but
did not rerun that check.

`.planning/STATE.md` and `24-EXECUTION-RESUME.md` still contain pre-mutation
checkpoint wording. The owner is refreshing those tracking artifacts after
the coherent code commit and bounded checks; this is pending handoff bookkeeping,
not a defect in the current-source version consumers reviewed here. Documentation
and selected version/API/header/state/bus/APU checks were reported by the owner;
I did not run them, and the remaining documentation/short-stress/reject checks
were still in progress at review time.

No findings added. This addendum supersedes the initial summary's note that
the consumer and documentation updates were still in progress.

## Tracking follow-through

The implementation owner refreshed STATE.md and24-EXECUTION-RESUME.md to the actual committed format6/behavior13 outcome and Plan04 runtime-authority checkpoint before handoff. The earlier pending tracking note is resolved; the historical state decisions remain preserved in their checkpoint records. No full acceptance claim is added.
