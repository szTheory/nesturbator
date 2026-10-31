---
phase: 23
review: 23-REVIEW.md
titles: json
findings:
  - id: CR-01
    severity: critical
    disposition: fixed
    title: "Scratch creation failure can redirect the POSIX walkthrough to `/build`"
  - id: WR-01
    severity: warning
    disposition: fixed
    title: "The documentation contract fails under the ASan preset"
  - id: WR-02
    severity: warning
    disposition: fixed
    title: "Preflight success text asserts provenance the function does not establish"
open: 0
total: 3
recorded: 2026-10-31T13:07:43.912Z
---

# Phase 23: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| CR-01 | critical | fixed | POSIX subshell stops on failure and checks scratch creation before deriving paths; injected mktemp/configure failures stopped before later commands. PowerShell stops on directory errors and checks each native exit. A removed scratch guard is rejected by the document self-test. (not in the current review) |
| WR-01 | warning | fixed | The checker receives SHORT_RUNS and recognizes only the intentional played-input omission; all documented CI names remain required. CMake resolves the played-input target independently. CI and ASan document checks and mutation controls passed. (not in the current review) |
| WR-02 | warning | fixed | Generic preflight reports supplied digests and caller-declared origin without authentication. Only the fixture creation path reports actual same-invocation creation. Positive/negative preflight checks require the limited result wording. (not in the current review) |

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.
