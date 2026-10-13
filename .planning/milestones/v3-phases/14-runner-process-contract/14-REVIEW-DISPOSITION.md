---
phase: 14
review: 14-REVIEW.md
titles: json
findings:
  - id: WR-01
    severity: warning
    disposition: fixed
    title: "Handlers are installed over an inherited SIG_IGN, and restart semantics depend on `signal()` and a feature-test macro"
  - id: WR-02
    severity: warning
    disposition: fixed
    title: "Test timeouts of 1 s and 2 s are far below the helpers' own deadlines and likely to flake on slow hosted runners"
  - id: WR-03
    severity: warning
    disposition: fixed
    title: "A signal that arrives while a `--movie`/`--hash-*` run is blocked on stdout is silently held until the write returns, but the same applies to the first stop check"
  - id: IN-01
    severity: info
    disposition: deferred
    title: "SIGHUP is not handled on POSIX, while the Windows console-close event is"
  - id: IN-02
    severity: info
    disposition: deferred
    title: "The EINTR retry loops in `nesturbator_run_state_read` are inconsistent with the other reads and unreachable under SA_RESTART"
  - id: IN-03
    severity: info
    disposition: deferred
    title: "Duplicated `parse_kind` bodies and a misleading usage message for a repeated `--raise-after`"
  - id: IN-04
    severity: info
    disposition: deferred
    title: "`nesturbator_run_stop_done` is a no-op on POSIX and `close_cap_ms`/`close_grace_ms` are plain statics read by the handler thread"
open: 0
total: 7
recorded: 2026-10-13T00:14:58.501Z
---

# Phase 14: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| WR-01 | warning | fixed | 59902dd: signal() kept; inherited SIG_IGN restored; errno saved (15-01) |
| WR-02 | warning | fixed | b0d18c6: TIMEOUT 30 s hang detectors; 30 s not 10 s because 10 s fires inside the helpers' own deadlines and can orphan the runner (15-01, resolution of D-20) |
| WR-03 | warning | fixed | b0d18c6: README POSIX stdout sentence (15-01) |
| IN-01 | info | deferred | cosmetic, no behaviour change; backlog per 15-CONTEXT deferred ideas |
| IN-02 | info | deferred | cosmetic, no behaviour change; backlog per 15-CONTEXT deferred ideas |
| IN-03 | info | deferred | cosmetic, no behaviour change; backlog per 15-CONTEXT deferred ideas |
| IN-04 | info | deferred | cosmetic, no behaviour change; backlog per 15-CONTEXT deferred ideas |

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.
