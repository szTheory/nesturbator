---
phase: 16
review: 16-REVIEW.md
reviewed_source_commit: e2bed6aeb85b773e44597e8f5d27de94d2dc383f
titles: json
findings:
  - id: WR-01
    severity: warning
    disposition: fixed
    title: "Setup failure can strand the RetroArch port lock"
  - id: WR-02
    severity: warning
    disposition: fixed
    title: "Semicolons in archive paths bypass the exact file allowlist"
  - id: WR-03
    severity: warning
    disposition: fixed
    title: "Documentation guard ignores later contradictory support claims"
  - id: WR-04
    severity: warning
    disposition: fixed
    title: "Newline in an archive member name can impersonate multiple expected files"
  - id: WR-05
    severity: warning
    disposition: fixed
    title: "Duplicate allowlisted archive entries pass the exact-content gate"
  - id: WR-06
    severity: warning
    disposition: fixed
    title: "Unicode path metadata bypasses the raw archive-name inventory"
  - id: WR-07
    severity: warning
    disposition: fixed
    title: "Source-root preflight is not run by the registered CI tests"
open: 0
total: 7
recorded: 2026-10-23T18:41:00Z
---

# Phase 16: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| WR-01 | warning | fixed | 16-REVIEW-FIX.md (not in the current review) |
| WR-02 | warning | fixed | 16-REVIEW-FIX.md (not in the current review) |
| WR-03 | warning | fixed | 16-REVIEW-FIX.md (not in the current review) |
| WR-04 | warning | fixed | 16-REVIEW-FIX.md (not in the current review) |
| WR-05 | warning | fixed | 16-REVIEW-FIX.md (not in the current review) |
| WR-06 | warning | fixed | 16-REVIEW-FIX.md (not in the current review) |

| WR-07 | warning | fixed | Registered serial retroarch.session.source_root in POSIX CI/ASan; full CI 650/650 and packaging passed; focused ASan 1/1. Independent follow-up review clean. |

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.

## WR-07 verification

`cmake --workflow --preset ci` exited 0: 650/650 tests, no skips, three packages. The registered preflight passed in 6.80 s. `ctest --preset asan -R '^retroarch.session.source_root$' --output-on-failure` exited 0: 1/1, 8.89 s. Logs: `build/phase16-release/coverage-ci-workflow.log` and `coverage-asan-focused.log` (ignored). The correction changes test registration and paired documentation; runtime source and published v0.1.11 bytes are unchanged. The original finding and independent follow-up remain in 16-REVIEW.md.
