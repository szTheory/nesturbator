---
phase: 11
review: 11-REVIEW.md
titles: json
findings:
  - id: WR-01
    severity: warning
    disposition: open
    title: "save.h promises a symlink at the .tmp name is \"never removed\", but the helper removes it"
  - id: WR-02
    severity: warning
    disposition: open
    title: "A dangling symlink at the .tmp name fails every save permanently"
  - id: WR-03
    severity: warning
    disposition: open
    title: "\"sync\" is not a durable flush on macOS, and the CI RetroArch job runs only on macOS"
  - id: WR-04
    severity: warning
    disposition: open
    title: "The chmod sub-case and the symlink sub-case in test_save_replace.c can pass without testing anything"
  - id: IN-01
    severity: info
    disposition: open
    title: "nesturbator_load_cartridge duplicates the power-on reset the new helper was created to share"
  - id: IN-02
    severity: info
    disposition: open
    title: "The libretro adapter's failure path leaves inst NULL, so av info is zeroed"
  - id: IN-03
    severity: info
    disposition: open
    title: "Over-long comment line in runner/main.c"
open: 7
total: 7
recorded: 2026-10-11T21:09:19.233Z
---

# Phase 11: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| WR-01 | warning | open | - |
| WR-02 | warning | open | - |
| WR-03 | warning | open | - |
| WR-04 | warning | open | - |
| IN-01 | info | open | - |
| IN-02 | info | open | - |
| IN-03 | info | open | - |

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.
