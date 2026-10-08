---
phase: 01
review: 01-REVIEW.md
titles: json
findings:
  - id: WR-01
    severity: warning
    disposition: open
    title: "Calling the allocator member can expand a function-like `free` macro"
  - id: WR-02
    severity: warning
    disposition: open
    title: "The vector harness count parser can accept overflowing inputs"
  - id: CR-01
    severity: critical
    disposition: fixed
    title: "hygiene.sh never scans tracked files whose names contain non-ASCII bytes"
  - id: CR-02
    severity: critical
    disposition: fixed
    title: "Under a UTF-8 locale, `grep -I` skips non-UTF-8 text, so the home-path and email scans miss it"
  - id: WR-03
    severity: warning
    disposition: fixed
    title: "The size-tag rule rejects larger structs whose padding bytes are not zero, and callers cannot control those bytes"
  - id: WR-04
    severity: warning
    disposition: fixed
    title: "The allocator contract states no alignment requirement, but arena allocators are explicitly invited"
  - id: WR-05
    severity: warning
    disposition: fixed
    title: "Configuring the project from another CMake project fails on any OS or CPU outside the six release targets"
  - id: WR-06
    severity: warning
    disposition: fixed
    title: "`--history` checks commit identities but never checks commit message bodies"
  - id: WR-07
    severity: warning
    disposition: fixed
    title: "The float scan misses hex floating literals"
  - id: WR-08
    severity: warning
    disposition: skipped
    title: "The one-time `release-as: \"0.1.0\"` pin has only a manual todo to remove it, and nothing checks it"
  - id: WR-09
    severity: warning
    disposition: fixed
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
open: 12
total: 21
recorded: 2026-10-08T15:12:26.578Z
---

# Phase 01: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| WR-01 | warning | open | - |
| WR-02 | warning | open | - |
| CR-01 | critical | fixed | 01-REVIEW-FIX.md (not in the current review) |
| CR-02 | critical | fixed | 01-REVIEW-FIX.md (not in the current review) |
| WR-03 | warning | fixed | 01-REVIEW-FIX.md (not in the current review) |
| WR-04 | warning | fixed | 01-REVIEW-FIX.md (not in the current review) |
| WR-05 | warning | fixed | 01-REVIEW-FIX.md (not in the current review) |
| WR-06 | warning | fixed | 01-REVIEW-FIX.md (not in the current review) |
| WR-07 | warning | fixed | 01-REVIEW-FIX.md (not in the current review) |
| WR-08 | warning | skipped | 01-REVIEW-FIX.md (not in the current review) |
| WR-09 | warning | fixed | 01-REVIEW-FIX.md (not in the current review) |
| IN-01 | info | open | - (not in the current review) |
| IN-02 | info | open | - (not in the current review) |
| IN-03 | info | open | - (not in the current review) |
| IN-04 | info | open | - (not in the current review) |
| IN-05 | info | open | - (not in the current review) |
| IN-06 | info | open | - (not in the current review) |
| IN-07 | info | open | - (not in the current review) |
| IN-08 | info | open | - (not in the current review) |
| IN-09 | info | open | - (not in the current review) |
| IN-10 | info | open | - (not in the current review) |

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.
