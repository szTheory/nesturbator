---
phase: 21
review: 21-REVIEW.md
titles: json
findings:
  - id: WR-01
    severity: warning
    disposition: deferred
    title: "Worktree executable-mode changes are absent from source identity (WARNING)"
  - id: WR-02
    severity: warning
    disposition: open
    title: "Measurement narrative attributes a different campaign to the canonical baseline (WARNING)"
open: 1
total: 2
recorded: 2026-10-30T13:13:16Z
---

# Phase 21: Code Review Disposition

| Finding | Severity | Disposition | Source |
|---------|----------|-------------|--------|
| WR-01 | warning | deferred | Collector maintenance; actual filesystem modes and raw blobs independently matched all 428 inputs in both checkouts to candidate and tested merge. (not in the current incremental review) |
| WR-02 | warning | open | - |

## Current-run mitigation and retained history

WR-01 is valid and remains unfixed. Repairing the tracked source-identity helper
would change accepted effective inputs and require new hosted CI/Nightly runs.
This evidence-only closeout preserves the work alongside Phase 20 CR-01/WR-03/WR-04;
it does not waive source identity for this run.

The 428-input comparison above is the mitigation recorded for that historical
source-bound run. It does not requalify a hosted bundle against the current
checkout. WR-01 remains deferred and was carried forward because its source was
outside this review's incremental scope.

An independent scratch verifier reads each regular file's actual owner-executable
permission (or symlink type), hashes raw file/link bytes as Git blobs, and compares
every path against candidate `54c7db2`, tested merge `faf1b91` and the checkout
HEAD at that time. The checkout and its absolute clean-source checkout then
matched all 428 inputs.
Their sorted mode/blob/path inventories have SHA-256
`d62c2f7e3f5b27653f2edcd9f9fef77d272bef6465b8afb161f8b76186899fe3`.
Reproduction and raw output are retained under
`build/phase21-hosted-evidence/verification/current-source-identity.py` and
`current-source-identity.json`; the two `*-worktree-modes-blobs.tsv` files retain
the actual filesystem observations. This supplements committed/index inventories
without changing production code or an acceptance threshold.

The original detailed Phase 21 CR-01 rejection, CR-02 fix, source citations and
revert/reapply evidence are preserved verbatim in
[21-REVIEW-HISTORY.md](21-REVIEW-HISTORY.md). CR-01 remains rejected; CR-02 remains
fixed. Neither finding is reopened or silently dropped by the incremental review.

Dispositions: `open` (recorded, not yet triaged), `fixed`, `skipped`, `deferred`.
Set `deferred` by hand and put the reason in the Source cell; both are preserved. A `|` in the reason is kept as prose and escaped on the next run.
Re-running the gate keeps every row it can. A row the current review no longer reports is kept and its Source cell flagged, so a finding does not leave this record silently. ONE exception: when a finding id is REUSED by a different finding, the earlier decision cannot keep a row — the id is taken — and it is dropped. A RECORDED decision (anything but `open`) is named on the console when that happens; a row still at `open` is replaced silently, because `open` records no decision to lose.
