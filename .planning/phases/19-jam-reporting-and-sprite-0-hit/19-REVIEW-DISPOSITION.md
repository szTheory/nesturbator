---
phase: 19
review: 19-REVIEW.md
titles: json
findings:
  - id: WR-01
    severity: warning
    disposition: open
    title: "Sprite-0 hit relies on an implicit coupling to the video path"
  - id: WR-02
    severity: warning
    disposition: open
    title: "The libretro cache treats all later JAM frames as one identity"
  - id: IN-01
    severity: info
    disposition: open
    title: "A full state save for every converted frame"
open: 3
total: 3
recorded: 2026-10-25T18:58:36.378Z
---

# Phase 19: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| WR-01 | warning | open | - |
| WR-02 | warning | open | - |
| IN-01 | info | open | - |

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.
