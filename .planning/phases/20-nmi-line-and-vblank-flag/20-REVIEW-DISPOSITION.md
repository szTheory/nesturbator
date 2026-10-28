---
phase: 20
review: 20-REVIEW.md
titles: json
findings:
  - id: CR-01
    severity: critical
    disposition: deferred
    title: "Hosted source validation ignores Git file modes"
  - id: WR-03
    severity: warning
    disposition: deferred
    title: "Scoreboard lookup recursively scans ignored build output"
  - id: WR-04
    severity: warning
    disposition: deferred
    title: "Nightly validator is defined twice"
  - id: WR-01
    severity: warning
    disposition: fixed
    title: "Public API comment says disabling NMI cancels a captured request"
  - id: WR-02
    severity: warning
    disposition: fixed
    title: "Split `$2002` tests do not exercise changing sprite flags across M2"
open: 0
total: 5
recorded: 2026-10-28T18:46:30.401Z
---

# Phase 20: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| CR-01 | critical | deferred | Recurring collector mode-check enforcement requires an effective-source change and fresh delivery, outside 20-10's read-only contract. Supplemental automated comparison of all 417 local Git index modes/blob IDs with candidate, CI-tested and Nightly-tested trees passes; mode/blob manifest SHA-256 `d799defc0b6be54d5e9ff8e00beae68fb003fe06e78bb4d6c240afd055117818`. Historical 417-input evidence remains source-specific. The repair refresh independently matches all 418 current Git index modes/blob IDs against candidate, CI-tested and Nightly-tested trees; `build/phase20-refresh-source-modes.tsv` SHA-256 `98f1978ae7a3ead9f8ce22b62a4f6d5950165abeddb03523e57bfe696e4fce7f`. Recurring enforcement remains deferred for independent verification to assess. |
| WR-03 | warning | deferred | Actual failure and successful absolute clean-equivalent-checkout workaround are preserved in 20-10-SUMMARY.md. A direct tracked-scoreboard copy and normalized source path need a later source-changing repair and new hosted evidence. |
| WR-04 | warning | deferred | The two bodies are identical; keep the tested effective source intact during read-only acceptance. Remove the shadowed definition in a later source-changing maintenance change. |
| WR-01 | warning | fixed | `include/nesturbator.h` public NMI comments and README contract; `test_captured_request_survives_clear` retains the request after line deassertion. (not in the current review) |
| WR-02 | warning | fixed | `test_sprite_status_changes_between_m2_samples` witnesses `0:130` clear and `0:131` set through `$2002` and `$3FFA`; wrong-sampler control fails at the returned `0x35` assertion. (not in the current review) |

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.
