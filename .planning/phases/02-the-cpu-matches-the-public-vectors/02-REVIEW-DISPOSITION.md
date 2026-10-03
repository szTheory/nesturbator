---
phase: 02
review: 02-REVIEW.md
titles: json
findings:
  - id: CR-01
    severity: critical
    disposition: open
    title: "The in-source guard in fetch_vectors.cmake is lexical, so `file(REMOVE_RECURSE)` can delete the repository's `src/` or any `<dir>/src`"
  - id: WR-01
    severity: warning
    disposition: open
    title: "The nightly's pull_request path filter misses most of the lane's inputs"
  - id: WR-02
    severity: warning
    disposition: open
    title: "The nightly's 3-minute job timeout leaves no room for network variance and overrides the fetch's own 900 s timeout"
  - id: WR-03
    severity: warning
    disposition: open
    title: "The cpu.vectors cleanup misses stray writes, so one failing test can corrupt every later test in the run"
  - id: IN-01
    severity: info
    disposition: open
    title: "A new instance's CPU state is all zeros, and \"only reset clears it\" has no reset to point to"
  - id: IN-02
    severity: info
    disposition: open
    title: "`ticks` will be advanced twice once the CPU runs in frames"
  - id: IN-03
    severity: info
    disposition: open
    title: "Build helpers leak into an embedding project's namespace"
  - id: IN-04
    severity: info
    disposition: open
    title: "Script-mode CMake files set policies inconsistently"
  - id: IN-05
    severity: info
    disposition: open
    title: "vecconv accepts leading zeros, which JSON forbids"
  - id: IN-06
    severity: info
    disposition: open
    title: "Helpers are duplicated across the vector tools"
open: 10
total: 10
recorded: 2026-10-03T16:28:05.093Z
---

# Phase 02: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| CR-01 | critical | open | - |
| WR-01 | warning | open | - |
| WR-02 | warning | open | - |
| WR-03 | warning | open | - |
| IN-01 | info | open | - |
| IN-02 | info | open | - |
| IN-03 | info | open | - |
| IN-04 | info | open | - |
| IN-05 | info | open | - |
| IN-06 | info | open | - |

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.
