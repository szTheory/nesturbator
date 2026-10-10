---
phase: 03
review: 03-REVIEW.md
titles: json
findings:
  - id: IN-01
    severity: info
    disposition: fixed
    title: "Duplicate PPU timing comments"
  - id: WR-01
    severity: warning
    disposition: fixed
    title: "Sprite overflow scan misses hardware false positives"
  - id: CR-01
    severity: critical
    disposition: deferred
    title: "Per-sample audio callback is never used"
  - id: CR-02
    severity: critical
    disposition: deferred
    title: "Libretro reset leaves a running cartridge untouched"
  - id: CR-03
    severity: critical
    disposition: fixed
    title: "Sprite rendering omits scanline 0"
open: 0
total: 5
recorded: 2026-10-09T21:43:47.732Z
---

# Phase 03: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| IN-01 | info | fixed | src/ppu.c duplicate timing comments removed; clean re-review (not in the current review) |
| WR-01 | warning | fixed | 03-15-SUMMARY.md diagonal overflow correction; clean re-review (not in the current review) |
| CR-01 | critical | deferred | ROADMAP.md Phase 04.2 closes SND-01; outside Phase 03 GAME requirements (not in the current review) |
| CR-02 | critical | deferred | 03-VERIFICATION.md: reset behavior is outside Phase 03 requirements (not in the current review) |
| CR-03 | critical | fixed | 03-14-SUMMARY.md: commits 58fd898/41d0807; ppu.sprites passes (not in the current review) |

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.
