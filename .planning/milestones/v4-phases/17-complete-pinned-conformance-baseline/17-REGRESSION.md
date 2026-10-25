---
phase: 17-complete-pinned-conformance-baseline
status: passed
verified: 2026-10-24
backend: github-actions
source_head: c23d0ac845ed405147c7006e87400abeda94b8cb
tested_merge: b52e4766633c75e061ce47079bf19141efad5f7a
run_id: 40075421001
job_id: 120817601380
command: cmake --workflow --preset ci
command_seconds: 630
configured_gate_seconds: 1500
tests_passed: 663
tests_total: 663
prior_phase: 16
prior_test_files: 11
---

# Phase 17 regression gate

The configured one-shot command ran successfully on the exact reviewed source in
[PR CI 40075421001](https://github.com/szTheory/nesturbator/actions/runs/40075421001).
The primary Linux step ran from 07:46:07Z to 07:56:37Z, within the configured
1,500-second gate. It built the three deliverables, passed 663/663 tests
(616.97 seconds of CTest), and completed packaging. All 15 required jobs passed,
including six platform builds, ASan, both no-FP lanes, real protected-main perf
comparison, RetroArch E2E, hash equality and hygiene.

This exact-source hosted execution supplies the serial integration and prior-phase
regression gates. Local full runs were preserved as failures after logged macOS
maintenance sleeps, including an interrupted run with an active idle-sleep
assertion. No test or project timeout was changed to reinterpret those failures.
No additional local full rerun is counted as passing.

## Source and retained evidence

- `git diff --exit-code c23d0ac845ed405147c7006e87400abeda94b8cb -- . ':(exclude).planning/**'`
  exited zero before this record. Later local commits contain review/tracking
  metadata only; the pre-existing planning config edit remains unstaged.
- The current 56-input fingerprint and hosted campaign metadata both equal
  `5aedbb3b04eca54a04d42e4e0ec7851e2e694cd72fd032d2ab1c29dfcbf2afe0`.
- Downloaded Linux artifact `accuracycoin-evidence-linux-x64-40075421001-1`,
  ID `12553018046`, has verified ZIP SHA-256
  `34f1ed7c9f0063181c78970af576b1bb32ae800536a42f2e96f57cd54a7b9b86`.
- Its actual full report has 146 scored entries and five DRAW entries: 101 pass,
  42 fail, zero skip, two not-run and one timeout. Metadata records 22 launched
  and attributed pages, 146 observed scored rows, 143 terminal rows, trustworthy
  observation and complete infrastructure. Report SHA-256 remains
  `6d0a811960f6964e7fc66802c6e4ade0c4ffb8772c40d88d50cf79e1f8463652`.
- Hosted `accuracycoin.regression_controls` passed in 30.70 seconds, including
  the fresh 101-PASS next-main case and CR-01 duplicate/missing-final controls.
  The actual campaign passed in 23.18 seconds. Independent source re-review is
  clean; CR-01's fixed disposition is retained separately.

## Prior-phase test references

These existing references were extracted from the canonical Phase 16 verification
report. The complete current CI suite, not a reduced replacement, ran above:

- `tests/cmake/check_archives.cmake`
- `tests/cmake/fixture_identity.cmake`
- `tests/cmake/info_claims.cmake`
- `tests/core/test_cartridge.c`
- `tests/libretro/libretro_host.c`
- `tests/retroarch/frame_index.c`
- `tests/retroarch/interruption_trace.sh`
- `tests/retroarch/progression.h`
- `tests/retroarch/run_retroarch.cmake`
- `tests/retroarch/state_session.sh`
- `tests/retroarch/test_state_session.sh`

Historical publication claims remain bound to their original release source;
this source verification does not claim a new release or close separate Nightly
vectors debt.
