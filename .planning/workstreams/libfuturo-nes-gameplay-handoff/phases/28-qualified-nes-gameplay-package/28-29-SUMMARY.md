---
phase: 28-qualified-nes-gameplay-package
plan: "29"
subsystem: testing
requires: []
provides:
  - Persisted LF/CRLF rejection controls and reduced fixture Git work
  - Reviewed separate D45 receipt and ignored one-shot driver
requirements-completed: []
---

# Plan29 complete — verified prefreeze controls

Exactly the five approved product files changed. Public behavior/header/API,
build/workflow/preset/registration/binding/baseline/ROM/provenance remain exact.
The ignored driver belongs only to the prepared D45 checkout.

Baseline controls passed, receipt4.50seconds; postpatch passed, receipt3.12.
Scoped instrumentation proves28→16 add/commit launches and14→8 commits,
retaining all seven mutation rejections. LF79bytes and CRLF81bytes had distinct
verified hashes and the actual checker rejected both for the unchanged pinned
historical-manifest reason.

Final finalized-source controls passed, receipt1.75seconds under15. Driver
synthetic controls passed in2.247seconds, no actual ledger touched. Baseline
and postpatch used the same delegating Git instrumentation; final checks used
ordinary Git. Local timing is not hosted Intel performance. Task1 results
retain earlier hashes; final results separately bind Task2 final files/driver.
No actual gate failed or was retried.

[Independent review](28-D45-REVIEW.md) approved corrections before final checks.
Whole-source/digest comparison, Python syntax and scoped whitespace passed.
Original dirty MMC2/migration and all prior failed routes remain preserved.

Only30 is next eligible: one freeze and one full local qualification with
configured campaign. None started here; hosted/publication remain pending.
