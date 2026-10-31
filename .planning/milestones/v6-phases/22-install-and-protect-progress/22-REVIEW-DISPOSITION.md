---
phase: 22
review: 22-REVIEW.md
titles: json
findings:
  - id: CR-01
    severity: critical
    disposition: fixed
    title: "Failed scratch creation can redirect progress checks to shared paths"
  - id: WR-01
    severity: warning
    disposition: fixed
    title: "Reproduction contract accepts a noncanonical document path"
  - id: WR-02
    severity: warning
    disposition: fixed
    title: "Reproduction contract does not verify POSIX and PowerShell parity"
open: 0
total: 3
recorded: 2026-10-31
---

# Phase 22 refreshed review disposition

| Finding | Resolution | Verification |
| --- | --- | --- |
| CR-01 | Guard scratch creation and stop on failed commands in the progress subshell. | Published block runs with four injected failures and a successful backup-double control. |
| WR-01 | Normalize and require the canonical guide path in normal check/self-test mode; isolate mutation mode. | Complete off-path guide copy is refused. |
| WR-02 | Require matching shell target lists and CTest selections before generated-metadata checks. | PowerShell-only missing target and missing test each fail for their own diagnostic. |

The final five document checks passed after these corrections. See
[refresh evidence](22-REFRESH-EVIDENCE.md). The original three fixed findings
have different titles despite reused warning IDs; their decisions are preserved
in [the initial ledger](22-REVIEW-DISPOSITION-INITIAL.md), [initial review](22-REVIEW-INITIAL.md)
and [original corrections](22-REVIEW-FIX.md). No finding was silently discarded.
