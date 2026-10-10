---
phase: 02
review: 02-REVIEW.md
titles: json
findings:
  - id: CR-01
    severity: critical
    disposition: open
    title: "Single-sample libretro audio callback is never called"
  - id: WR-01
    severity: warning
    disposition: fixed
    title: "[WARNING] In-progress nightly run is classified as a failure"
  - id: WR-02
    severity: warning
    disposition: fixed
    title: "[WARNING] Required no-FP CI job has little timeout headroom"
  - id: WR-04
    severity: warning
    disposition: fixed
    title: "The nightly's path filter still misses inputs that change the CPU build the lane runs"
  - id: IN-01
    severity: info
    disposition: fixed
    title: "A new instance's CPU state is all zeros, and \"only reset clears it\" has no reset to point to"
  - id: IN-02
    severity: info
    disposition: fixed
    title: "`ticks` will be advanced twice once the CPU runs in frames"
  - id: IN-03
    severity: info
    disposition: fixed
    title: "Build helpers leak into an embedding project's namespace"
  - id: IN-04
    severity: info
    disposition: fixed
    title: "Script-mode CMake files set policies inconsistently"
  - id: IN-05
    severity: info
    disposition: fixed
    title: "vecconv accepts leading zeros, which JSON forbids"
  - id: IN-06
    severity: info
    disposition: fixed
    title: "Helpers are duplicated across the vector tools"
  - id: IN-07
    severity: info
    disposition: fixed
    title: "The fetch creates DIR before its resolved-path check, so a refused DIR still leaves directories in the source tree"
  - id: IN-08
    severity: info
    disposition: fixed
    title: "The lower-casing step is redundant, and `vectors.fetch.guard` cannot detect its removal"
  - id: IN-09
    severity: info
    disposition: fixed
    title: "Fetched files in the old `DIR/src` layout are orphaned with no note for users"
  - id: WR-03
    severity: warning
    disposition: fixed
    title: "The cpu.vectors cleanup misses stray writes, so one failing test can corrupt every later test in the run"
open: 1
total: 14
recorded: 2026-10-09T13:14:10.919Z
---

# Phase 02: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| CR-01 | critical | open | - |
| WR-01 | warning | fixed | 02-REVIEW-FIX.md (not in the current review) |
| WR-02 | warning | fixed | 02-REVIEW-FIX.md (not in the current review) |
| WR-04 | warning | fixed | 02-REVIEW-FIX.md (not in the current review) |
| IN-01 | info | fixed | 02-REVIEW-FIX.md (not in the current review) |
| IN-02 | info | fixed | 02-REVIEW-FIX.md (not in the current review) |
| IN-03 | info | fixed | 02-REVIEW-FIX.md (not in the current review) |
| IN-04 | info | fixed | 02-REVIEW-FIX.md (not in the current review) |
| IN-05 | info | fixed | 02-REVIEW-FIX.md (not in the current review) |
| IN-06 | info | fixed | 02-REVIEW-FIX.md (not in the current review) |
| IN-07 | info | fixed | 02-REVIEW-FIX.md (not in the current review) |
| IN-08 | info | fixed | 02-REVIEW-FIX.md (not in the current review) |
| IN-09 | info | fixed | 02-REVIEW-FIX.md (not in the current review) |
| WR-03 | warning | fixed | 02-REVIEW-FIX.md (not in the current review) |

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.
