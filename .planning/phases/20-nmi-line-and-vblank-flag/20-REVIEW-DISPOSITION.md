---
phase: 20
review: 20-REVIEW.md
titles: json
findings:
  - id: WR-01
    severity: warning
    disposition: fixed
    title: "Public API comment says disabling NMI cancels a captured request"
  - id: WR-02
    severity: warning
    disposition: fixed
    title: "Split `$2002` tests do not exercise changing sprite flags across M2"
open: 0
total: 2
recorded: 2026-10-27T03:26:03Z
---

# Phase 20: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| WR-01 | warning | fixed | `include/nesturbator.h` public NMI comments and README contract; `test_captured_request_survives_clear` retains the request after line deassertion. |
| WR-02 | warning | fixed | `test_sprite_status_changes_between_m2_samples` witnesses `0:130` clear and `0:131` set through `$2002` and `$3FFA`; wrong-sampler control fails at the returned `0x35` assertion. |

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.
