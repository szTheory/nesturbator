---
phase: 24
review: 24-REVIEW.md
titles: json
findings:
  - id: WR-01
    severity: warning
    disposition: fixed
    title: "Long DMA timing test silently loses the trace tail"
    source: "24-ACCEPTANCE.md#plan24-05-local-gap-closure"
open: 0
fixed: 1
total: 1
recorded: 2026-11-01T02:24:19Z
---

# Phase 24: Code Review Disposition

| Finding | Severity | Disposition | Evidence |
|---------|----------|-------------|----------|
| WR-01 | warning | fixed | [Plan24-05 local gap closure](24-ACCEPTANCE.md#plan24-05-local-gap-closure) |

WR-01 is fixed by checking each retained trace window for overflow before
reset, assertion, count, or print; the forced-overflow child exits nonzero
with one specific diagnostic. The focused landing run retains 24,741 events
through the actual final sample GET and resumed CPU read. See the linked
acceptance record for commands and bounded results.

## 2026-11-01 follow-up review findings

The first pass found three issues, all fixed in commit `dfd0bfc`; the final
standard-depth re-review recorded in `24-REVIEW.md` is clean. The IDs below
are scoped to that follow-up pass; its WR-01 is separate from the earlier
trace-tail WR-01 above.

| Follow-up ID | Initial finding | Disposition | Evidence |
|--------------|-----------------|-------------|----------|
| CR-01 | Evidence ROOT could traverse a symlink outside ignored build output | fixed | Physical path containment and symlink rejection control in `tests/cmake/phase24_dma_gap_evidence.cmake` |
| WR-01 | Test 9 trace printer dereferenced a missing completed-read event | fixed | Null-safe diagnostic and missing-read control in `tests/cpu/test_bus.c` and evidence controls |
| WR-02 | Optional CPU-PC trace mode referenced an absent field | fixed | Conditional field capture in `tests/cpu/test_bus.c`; compiled with `ROM_ABORT_TRACE_CPU_PC` |

`bus.unit`, the synthetic evidence controls, and the symlink rejection control
passed. These findings affect test diagnostics and evidence tooling only; the
emulation core and saved-state format did not change.

## Plan24-04 prohibition flags — human judgment remains unresolved

These rows supersede only the unverified prohibition projection in
Plan24-04. The plan itself remains unchanged. Observable machine evidence is
listed separately from the reviewer judgment still required; no automated
check can supply that judgment.

| Flag | Verification | Status | Authority / observable machine evidence | Judgment still required |
|------|--------------|--------|-----------------------------------------|-------------------------|
| Stopped-campaign authority | `human_judgment` | unresolved | D-08 governs authority. This step ran only local checks; no hosted workflow, push, PR, merge, or release occurred. | A reviewer must judge whether the prepared remote request is acceptable; the decision is recorded separately in `24-ACCEPTANCE.md`. |
| Oracle and target overclaim | `human_judgment` | unresolved | D-03 limits the claim to pinned test compatibility. The early-G ordering remains explicitly inferred; local ROM, owned timing, audio, and state checks are not silicon evidence. | A reviewer must judge whether public claims stay within those oracle limits. |
| Native compatibility overclaim | `human_judgment` | unresolved | D-05/D-06 retain native format 6 and ABI 1. This plan changes test harness and evidence files only; it adds no converter, loader policy, or compatibility claim. | A reviewer must judge whether compatibility wording stays within the exact documented contract. |

The remote-authority decision alone cannot resolve the oracle or native
compatibility judgments. All three remain unresolved until their respective
review judgments are explicitly recorded.
