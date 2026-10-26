# Phase 20 Plan Check

**Status:** VERIFICATION PASSED
**Plans:** 20-01 through 20-06
**Reviewed:** 2026-10-26

The plans cover NMI-01, NMI-02, KEEP-02 and KEEP-03. They preserve the source-grounded status tables, 108-PASS protection, causal format 1→2 / behavior revision 7→8 changes, exact set/clear/edge-before-poll state evidence, and core/runner/libretro replay. Same-edge ordering and alignment remain explicitly unresolved until the bounded experiments; no pass result is assumed. Phase 21 IRQ/hijack work and extra APIs/dependencies remain excluded.

The initial review found four verifier gaps; revision 1 resolves them with isolated `ctest -R` invocations and `--no-tests=error`:

- Plan 20-01 Task 1 isolates `ppu.nmi_races`.
- Plan 20-01 Task 2 isolates `ppu.nmi_races` and `core.nmi_poll`.
- Plan 20-05 Task 1 isolates `core.state_fork.nmi`.
- Plan 20-06 Task 1 isolates `accuracycoin.nmi_targets`.

All six plans pass `verify.plan-structure`; every task has files, action, verify, and done fields. The parser's one-way-door warnings for Plan 20-02 are covered by locked decision D-14, which explicitly approves format 1→2 and requires no additional checkpoint.

Probe results: verify-paths reported 24 CMake-form commands as `not_applicable` with severity `none` (not positive path validation); failure-directions reported 24/24 `ok`, with no blocker or warning.
