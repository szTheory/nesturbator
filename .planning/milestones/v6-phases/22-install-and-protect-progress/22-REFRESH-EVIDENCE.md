# Phase 22 verification refresh — 2026-10-31

This explicitly invoked execution step skipped all four completed plans and
resumed aggregation, verify-post hooks, review, bounded regression and verification.
The original report is archived as `22-VERIFICATION-INITIAL.md`; the original
review/disposition are archived with `-INITIAL.md` names. Original execution
and provenance failure/repair records remain unchanged in 22-EXECUTION-EVIDENCE.md.

## Review and corrections

The incremental scope resolver returned `phase-range/no-task-commits-since`:
all Phase 22 summaries were unchanged since review. Its broader ten-file scope
included shared Phase 23 guides, registration, replay checks and existing
measurement metadata. No active verify-post security hook or execute-post TDD
gate was configured. The code-review hook ran at standard depth.

The refreshed review found one critical and two warning findings. All three
were corrected:

- The POSIX progress walkthrough now runs in a subshell with `set -e`, stops
  when scratch creation fails, and checks the directory before deriving paths.
  README describes this failure behavior. The public header is unchanged
  because no runtime behavior, ABI or format changed.
- Normal reproduction checks require the canonical docs/reproduction.md path;
  deliberately broken documents use the explicit test-only mutation mode.
- Both shell build target lists and both anchored CTest selections must agree;
  existing required-target/test and generated-metadata checks still apply.

The existing registered docs.reproduction.self_test now executes the actual
published progress shell block with harmless doubles. Scratch creation,
invalid-directory, configure and CTest failures stop before copying/hashing;
configure/test failures must retain their injected exit values. A successful
control reaches backup doubles. No real save, host configuration or CMake
build command is invoked by those doubles. Seventeen document mutations
include a noncanonical complete copy and PowerShell-only missing target/test.
POSIX controls are explicitly unavailable when no sh executable is installed.
These controls run through the CI preset's existing CTest registrations.

## Fresh automated checks

Every CTest invocation used `--output-on-failure --no-tests=error
-FA accuracycoin_full_report`. No campaign fixture was enabled; no timeout,
threshold, baseline or provenance was changed. The configured full workflow
would launch a campaign outside the preserved authority, so this refresh used
the explicitly bounded contract/regression selection instead of full CI.

| Selection | Outcome |
| --- | --- |
| docs.reproduction, docs.reproduction.self_test, docs.links, docs.links.self_test, cmake.script_policy | Final run passed 5/5 in 28.62 seconds. Positive document check 8.80 s; self-test 19.75 s, within unchanged 60-second bounds. |
| Inherited status/API, runner options/exit, release marker/version, libretro info/archive checks; cold movie/comparator, positive/negative checkpoint preflight, NMI continuation/idempotence/wrong-load, boot-to-play, native format refusal, generation and controller | Passed 31/31 including five required fixture setup tests, 6.28 seconds. |
| git diff --check | Passed after removing review-report trailing whitespace. |

The focused replay/state/input and inherited checks use the same scoped
contracts recorded in the Phase 23 verification. The five document checks ran
again after the final control changes. The earlier Phase 23 failed timeout
attempts and its earlier successful runs remain historical evidence; these
fresh passes do not diagnose the intermittent timing problem or establish a
full CI workflow pass. No broader campaign, platform matrix or package rebuild
was performed. A process listing attempt was denied by the sandbox; no
process cleanup or restart was needed or attempted.

## Boundaries and next step

No runtime source, public ABI, save format, ROM pin or hash baseline changed.
No network release observation, collector/provenance refresh, real host/device,
private content, daily progress, publication, push or PR occurred. PowerShell
was checked as text and by mutation; it was not executed on Windows.
Historical collector/platform/host-recovery debt and runtime safety stops remain.

The fixes necessarily changed three files covered by Phase 23's verification:
README.md, docs/progress.md and tests/cmake/reproduction_contract.cmake.
Its status query is now stale. Do not silently update that report's fingerprint.
After this Phase 22 step, the next separate command is `$gsd-execute-phase 23`;
only after its passing refresh should the owner invoke milestone audit.
Owner edits to AGENTS.md and .planning/config.json remain uncommitted.
