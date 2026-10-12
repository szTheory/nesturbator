---
phase: 12
review: 12-REVIEW.md
titles: json
findings:
  - id: CR-01
    severity: critical
    disposition: open
    title: "A CRC-valid state with `ticks` near 2^64 makes `nesturbator_run_frame` loop about 2^61 times"
  - id: WR-01
    severity: warning
    disposition: open
    title: "The frame-range checks now run after ROM load, so a usage error can turn into a ROM or I/O exit code"
  - id: WR-02
    severity: warning
    disposition: open
    title: "`cpu.halted_in_read` is validated as exactly 0 although the field is documented as used"
  - id: IN-01
    severity: info
    disposition: open
    title: "VALIDATE reads the instance through `*v`, contradicting the stated invariant"
open: 4
total: 4
recorded: 2026-10-12T12:50:42.685Z
---

# Phase 12: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| CR-01 | critical | open | - |
| WR-01 | warning | open | - |
| WR-02 | warning | open | - |
| IN-01 | info | open | - |

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.
