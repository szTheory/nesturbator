---
phase: 25
review: 25-REVIEW.md
titles: json
findings:
  - id: CR-01
    severity: critical
    disposition: fixed
    title: "PAL five-step states exceed the APU state bound"
  - id: CR-02
    severity: critical
    disposition: fixed
    title: "Unbounded state frame number can skip PAL frame execution"
  - id: WR-01
    severity: warning
    disposition: fixed
    title: "API guide reports the wrong current behavior revision"
  - id: WR-02
    severity: warning
    disposition: deferred
    title: "PAL soft-reset write-inhibit duration lacks a PAL-backed rule"
open: 0
total: 4
recorded: 2026-11-03T02:45:38.408Z
---

# Phase 25: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| CR-01 | critical | fixed | Closed by Phase 27.1 loader and regression work; see [verification truths 1–2](../27.1-close-state-01-regional-state-validation-gaps/27.1-VERIFICATION.md). |
| CR-02 | critical | fixed | Closed by Phase 27.1 pre-apply frame bounds and regression work; see [verification truths 3–5](../27.1-close-state-01-regional-state-validation-gaps/27.1-VERIFICATION.md). |
| WR-01 | warning | fixed | Updated `docs/api.md` to behavior revision 16 and added the revision-16 history entry in `5a17229`; `docs.links` passed 1/1. (not in the current review) |
| WR-02 | warning | deferred | PAL source clears `reset_flag` at pre-render dot 1 (line 311), but pinned research does not establish a PAL reset-release duration. Header and API docs now identify this as an unverified model behavior; retained as a flagged PAL adjacency/reset/reload limitation. (not in the current review) |

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.
