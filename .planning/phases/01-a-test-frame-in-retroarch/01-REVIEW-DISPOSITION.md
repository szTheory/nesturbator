---
phase: 01
review: 01-REVIEW.md
titles: json
findings:
  - id: CR-01
    severity: critical
    disposition: fixed
    title: "hygiene.sh never scans tracked files whose names contain non-ASCII bytes"
  - id: CR-02
    severity: critical
    disposition: open
    title: "Under a UTF-8 locale, `grep -I` skips non-UTF-8 text, so the home-path and email scans miss it"
  - id: WR-01
    severity: warning
    disposition: open
    title: "`--frames 4294967295` makes the runner loop forever"
  - id: WR-02
    severity: warning
    disposition: open
    title: "The runner exits 0 and does nothing when `--frames` is missing; the README says it is required"
  - id: WR-03
    severity: warning
    disposition: open
    title: "The size-tag rule rejects larger structs whose padding bytes are not zero, and callers cannot control those bytes"
  - id: WR-04
    severity: warning
    disposition: open
    title: "The allocator contract states no alignment requirement, but arena allocators are explicitly invited"
  - id: WR-05
    severity: warning
    disposition: open
    title: "Configuring the project from another CMake project fails on any OS or CPU outside the six release targets"
  - id: WR-06
    severity: warning
    disposition: open
    title: "`--history` checks commit identities but never checks commit message bodies"
  - id: WR-07
    severity: warning
    disposition: fixed
    title: "The float scan misses hex floating literals"
  - id: WR-08
    severity: warning
    disposition: open
    title: "The one-time `release-as: \"0.1.0\"` pin has only a manual todo to remove it, and nothing checks it"
  - id: WR-09
    severity: warning
    disposition: open
    title: "CI installs Ninja without `apt-get update`, and its one-minute timeouts sit on the only required check"
  - id: IN-01
    severity: info
    disposition: open
    title: "The test-card comment says every native value has a 16x7 cell; the border trims two columns of cells"
  - id: IN-02
    severity: info
    disposition: open
    title: "The header suggests `audio` may be NULL, but a NULL buffer always fails"
  - id: IN-03
    severity: info
    disposition: open
    title: "`a.free(...)` breaks if `<stdlib.h>` defines `free` as a function-like macro"
  - id: IN-04
    severity: info
    disposition: open
    title: "A failed `--dump-frame` stops the run, drops later hash lines and leaves a partial file"
  - id: IN-05
    severity: info
    disposition: open
    title: "With `argc == 0` the runner reports \"out of memory\""
  - id: IN-06
    severity: info
    disposition: open
    title: "palgen's MSVC flag does not forbid contraction on older toolsets"
  - id: IN-07
    severity: info
    disposition: open
    title: "The BMP reader reads BI_BITFIELDS masks at offset 54 even when pixel data starts there"
  - id: IN-08
    severity: info
    disposition: open
    title: "`--history` splits binary and ROM path names on whitespace"
  - id: IN-09
    severity: info
    disposition: open
    title: "`nesturbator_get_info` needs an instance only to return constants"
  - id: IN-10
    severity: info
    disposition: open
    title: "The no-global-state and allowed-symbol checks run only on Linux and miss some nm symbol types"
open: 19
total: 21
recorded: 2026-10-03T01:56:45.076Z
---

# Phase 01: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| CR-01 | critical | fixed | 01-REVIEW-FIX.md |
| CR-02 | critical | open | - |
| WR-01 | warning | open | - |
| WR-02 | warning | open | - |
| WR-03 | warning | open | - |
| WR-04 | warning | open | - |
| WR-05 | warning | open | - |
| WR-06 | warning | open | - |
| WR-07 | warning | fixed | 01-REVIEW-FIX.md |
| WR-08 | warning | open | - |
| WR-09 | warning | open | - |
| IN-01 | info | open | - |
| IN-02 | info | open | - |
| IN-03 | info | open | - |
| IN-04 | info | open | - |
| IN-05 | info | open | - |
| IN-06 | info | open | - |
| IN-07 | info | open | - |
| IN-08 | info | open | - |
| IN-09 | info | open | - |
| IN-10 | info | open | - |

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.
