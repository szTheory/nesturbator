---
phase: 15
review: 15-REVIEW.md
titles: json
findings:
  - id: WR-01
    severity: warning
    disposition: fixed
    title: "Perf gate passes silently when the head measurement is missing and the base is also missing"
  - id: WR-02
    severity: warning
    disposition: fixed
    title: "`file(STRINGS)` on the allow file mangles reasons that contain `;` or `[` `]`"
  - id: WR-03
    severity: warning
    disposition: fixed
    title: "An inherited SIG_IGN is honoured but not documented, and it can hang the stop tests"
  - id: WR-04
    severity: warning
    disposition: skipped
    title: "One-minute job timeouts sit at about 2 to 3 times a single sample"
  - id: IN-01
    severity: info
    disposition: open
    title: "`timeout_comments.cmake` accepts any 9-digit number"
  - id: IN-02
    severity: info
    disposition: open
    title: "`bench.h` helpers have undefined behaviour for n == 0"
  - id: IN-03
    severity: info
    disposition: open
    title: "Fixed 512-byte report buffer truncates silently"
open: 3
total: 7
recorded: 2026-10-13T12:39:04.673Z
---

# Phase 15: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| WR-01 | warning | fixed | 15-REVIEW-FIX.md |
| WR-02 | warning | fixed | 15-REVIEW-FIX.md |
| WR-03 | warning | fixed | 15-REVIEW-FIX.md |
| WR-04 | warning | skipped | 15-REVIEW-FIX.md |
| IN-01 | info | open | - |
| IN-02 | info | open | - |
| IN-03 | info | open | - |

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.
