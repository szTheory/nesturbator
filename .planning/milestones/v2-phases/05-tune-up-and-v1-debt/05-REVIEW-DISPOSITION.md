# Phase 05: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| CR-01 | critical | fixed | 05-REVIEW.md; no longer reproduces at the phase 10 core; guarded by core.reset check_reset_sentinel_frame (0xFFFF pre-fill, pre-reset frames 0-120); evidence in 10-RESEARCH.md |
| WR-01 | warning | open | 05-REVIEW.md |
| WR-02 | warning | fixed | 05-REVIEW.md; fixed by 1a8fc55 (05-08) |
| WR-03 | warning | open | 05-REVIEW.md |
| IN-01 | info | open | 05-REVIEW.md |
| IN-02 | info | open | 05-REVIEW.md |
| IN-03 | info | open | 05-REVIEW.md |

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.
