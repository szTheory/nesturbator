---
quick_id: 261031-ecd
mode: quick
description: Fix release-only AccuracyCoin fingerprint mismatch and avoid redundant Nightly work
branch: gsd/phase-23-reproduce-and-report-a-scene
base: af0d656eed63f5df0757aaabb2acf0ee3f7bdfd3
assumption_delta_decision: no-change; preserve the measured AccuracyCoin report and every required CI lane
---

# Fix release CI and trim redundant Nightly work

Keep this as one GSD quick task on the current phase branch. The release PR
changes only the public patch-version macro, which currently changes the strict
AccuracyCoin build-input fingerprint. Preserve strict checking for all other
header and emulator inputs. Keep the required six-platform CI matrix unchanged.
The extended Nightly workflow is non-required; skip its lanes for Release Please
PRs because those PRs already run required CI and main pushes still trigger the
Nightly lanes after merge. Preserve scheduled, manual, main-push, and other
relevant PR runs.

## Task 1 — Make release version numbers fingerprint-neutral

Files: `include/nesturbator.h`, `tests/cmake/accuracycoin_input_fingerprint.cmake`,
`tests/cmake/accuracycoin_input_fingerprint_controls.cmake`,
`tests/CMakeLists.txt`, `tests/accuracy/baseline-metadata.txt`.

Normalize only the numeric value of the three `NESTURBATOR_VERSION_*` macros
when hashing `include/nesturbator.h`. Keep every other header byte and every
other fingerprinted input exact. Add an offline regression test proving a
version-number change leaves the fingerprint stable while a non-version header
change still changes it. Clarify the public-header comment that these version
numbers identify a release and do not alter emulated output. Recompute and
record the metadata input fingerprint from the current source tree; do not
change the campaign report hash or measured scoreboard.

Verify with `cmake --preset ci` followed by
`ctest --preset ci --no-tests=error -R '^accuracycoin\\.input_fingerprint$'`.
Also verify the stored `build_inputs_sha256` and count match the current
fingerprint. Do not run `accuracycoin.campaign` or the full CI workflow in this
task: the existing stop remains until the owner separately authorizes that
run.

Done: the release macro bump no longer invalidates the measured baseline, and
changes to any other fingerprinted header content remain detectable.

## Task 2 — Skip redundant Nightly lanes on Release Please PRs

Files: `.github/workflows/nightly.yml`,
`tests/cmake/nightly_workflow_policy.cmake`.

Add a job-level condition to each non-required Nightly lane so it skips only for
the generated Release Please PR. Keep it enabled for scheduled, manual, main
push, and ordinary relevant PR events. Extend the live workflow policy and its
mutation self-test to require the guard on every affected lane and to prove
that removing one lane's guard is rejected. Do not add filters that skip
Nightly coverage on ordinary source changes.

Verify with
`ctest --preset hygiene --no-tests=error -R '^hygiene\\.nightly_workflow_policy(\\.selftest)?$'`.

Done: a release PR does not start the extended vector, oracle, fuzz, flake, or
Intel session lanes, while every other existing Nightly trigger retains them.

## Task 3 — Document the CI contract and run targeted checks

Files: `README.md`, `docs/testing.md`.

Document that public release-version numbers are excluded from the AccuracyCoin
input fingerprint, that the remaining fingerprint stays exact, and that the
non-required Nightly lanes skip generated release PRs while running on main.
Keep release publication gated on all required checks for the exact release
head.

Run the two targeted checks from Tasks 1 and 2 and `git diff --check`. Do not
run `cmake --workflow --preset ci`, the AccuracyCoin campaign, or push the
branch. Report those as pending the explicit owner decision required by the
preserved stop.

Done: the docs match the guarded workflows, both focused policy tests pass, and
the full-CI decision boundary remains explicit.
