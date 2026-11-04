---
phase: 28
review: 28-REVIEW.md
titles: json
findings:
  - id: CR-01
    severity: critical
    disposition: fixed
    title: "Windows moved-prefix test rejects the normal library filename"
  - id: CR-02
    severity: critical
    disposition: fixed
    title: "Profile self-test can recursively delete an arbitrary path"
  - id: WR-01
    severity: warning
    disposition: fixed
    title: "Pixel-difference evidence compares frame 80 with frame 110, not frame 95"
  - id: WR-02
    severity: warning
    disposition: fixed
    title: "Provider JSON checks accept values with the wrong JSON type"
  - id: WR-03
    severity: warning
    disposition: fixed
    title: "Consumer trusts the library's audio count as an array bound"
  - id: IN-01
    severity: info
    disposition: fixed
    title: "Missing-attestation errors can name the previous archive"
open: 0
total: 6
recorded: 2026-11-03T19:36:31.092Z
---

# Phase 28: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| CR-01 | critical | fixed | 28-REVIEW-FIX.md (not in the current review) |
| CR-02 | critical | fixed | 28-REVIEW-FIX.md (not in the current review) |
| WR-01 | warning | fixed | 28-REVIEW-FIX.md (not in the current review) |
| WR-02 | warning | fixed | 28-REVIEW-FIX.md (not in the current review) |
| WR-03 | warning | fixed | 28-REVIEW-FIX.md (not in the current review) |
| IN-01 | info | fixed | 28-REVIEW-FIX.md (not in the current review) |

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.
