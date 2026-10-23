# Plan 16-56 Source Acceptance

## Initial candidate state

- Initial HEAD: `af7f9d025ce42dfd18540ad031ff638da75b8eef` on `gsd/phase-16-host-evidence-and-roadmap`.
- Initial index SHA-256: `524c14799e9737c2af19959b56e0686b12259f986bb023a0e07306bb33f07968`; the index had no staged paths.
- `.planning/config.json` SHA-256: `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e`; it is pre-existing dirty input and must remain byte-identical and unstaged.
- Initial candidate source/doc SHA-256:

| Path | SHA-256 |
|---|---|
| `tests/retroarch/state_session.sh` | `1d9574ff863a0757f5a9cf8818e34c7daf6bda9063fabd78f155bf2b1575d560` |
| `tests/retroarch/test_state_session.sh` | `65e772450f992d16cfdbce6ee86e254b2dbc78f0bf1b84cbce619ba46af86e58` |
| `tests/retroarch/interruption_trace.sh` | `a899d87ed398cbfff75b965b01a5575912cb9b0696b048ffdd55732d278ee165` |
| `README.md` | `57f275d7ee2dfb1b2c6f2c31393c8620406992aaa8d172b87ab531af8eebf97e` |
| `include/nesturbator.h` | `aa2e65d8ab258a102ff43d8c9ffeac6161b33a469337a8e5b2b1ae4ebb0295f0` |

## Pre-edit defect reproduction

The tracked harness currently derives `TEST_PROJECT_ROOT` from the caller-supplied `DRIVER` before sourcing `interruption_trace.sh`. A controlled copied driver/include pair under a separate scratch root was supplied to the tracked harness from the configured CTest binary directory with `STATE_SESSION_CONTROL=source-root-preflight`. The harness returned exit 0 and printed `PASS source-root preflight`, accepting the copied source root. This reproduces the identity defect before edits; retained output is in `build/ci/phase16-direct/root-repro.canonical.stdout` and `.stderr`.

The initial reproduction attempt using the `/tmp` spelling returned `CWD_SOURCE_ROOT_MISMATCH` because the test canonicalizes the driver directory but compares the raw driver spelling. A follow-up command then failed to locate the temporary copy because the lookup did not follow the platform temporary-directory alias. Both first logs are retained as `build/ci/phase16-direct/root-repro.stdout`, `.stderr`, `root-repro.retry.stdout`, and `.stderr`. The corrected canonical-path reproduction above isolates the intended defect.

## Execution evidence

### Final tested source manifest

The five behavior/test/documentation paths are committed on the phase branch across `2e9928b7eb68604e6d11ecb5b240e7915b28c4e3` and the review-fix commit `2c356d574290aef1d57122566f7b81beaaa10027`; the latter changes only the driver and its regression test. The tested blobs match the final 11-path source/support manifest. The post-commit index SHA-256 is `688a63724ba498fe11e4ba4fee038d646ce68fde2a48113fd95af452c0ec3085` with no staged paths. `.planning/config.json` remains byte-identical to its initial dirty-input SHA-256 `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e` and unstaged.

| Tested candidate path | SHA-256 |
|---|---|
| `tests/retroarch/state_session.sh` | `ab54e4ce8aed4b24fb871afb3d8c070b0da25823f3d2105446bc9fb26b7361e4` |
| `tests/retroarch/test_state_session.sh` | `6869d8d62b545b7d08b5d12936e5cc1fdedc453175896dd802b886ff5bf92b93` |
| `tests/retroarch/interruption_trace.sh` | `a899d87ed398cbfff75b965b01a5575912cb9b0696b048ffdd55732d278ee165` |
| `README.md` | `c7f4dccde86f0d537c7823f98043240fed3a4e1930307f38efc0e6174b419d9f` |
| `include/nesturbator.h` | `a6d2539797585b6079e445cb772e5d2dc3109f83a47b237e5a0606764c21b137` |

### Source boundary and finalizer evidence

- The tracked harness now derives its canonical path from its own invocation using `Cwd::abs_path`, checks the expected tracked harness/driver/include before sourcing, and rejects a copied driver/include pair before host/send or control markers. The final `source-root-preflight` run passed from repository root, CTest binary directory, and an owned cwd with spaces; all four finalizer faults passed in each cwd with wait status 143, exact first reason, `cleanup_anomaly=0`, oracle disposition `not-run`, no premature oracle outputs, and exact-port reentry after reap. Copied-source rejection passed. Console: `build/ci/phase16-direct/source-root-final/console.log`; per-case retained evidence: `build/ci/phase16 direct source-root final/source-root-preflight-72120/`.
- The final direct `lock-pid-preflight` command passed. Its writer record and observation are under `build/ci/phase16 pid preflight final/valid/`. Driver/passed/bootstrap PID were `76687`, helper PID `76702`; the reserved endpoint used FD 9, port `62551`, `SOCK_STREAM`, and reuse flags zero. Darwin's `device=0,inode=0` observation was accepted as a valid stat identity. The writer's recorded driver source SHA matches this manifest. Console: `build/ci/phase16-direct/lock-pid-preflight-final/console.log`.

### CR-01 same-oracle causal proof (initial candidate; superseded)

This first pair was valid for the initial `2e9928b` candidate. The later independent-review fix changed the driver/test bytes, so the final-source pair below supersedes this evidence for acceptance.

The final harness was used for both sides of the causal pair. The old side used only the pinned `3643d6ac04c8c09391a6f2ea68c7217ba4c5ba25` driver/include fixture in ignored build output. Both runs used harness SHA-256 `c5c121a991df31695fe0c6430996cb900f5d40d2df4fc17eaea1bba39e4d4a31` and the same frozen oracle SHA-256 `1963fbb71f538a7b14ff6c10a679ee09f992528151204b25d219c97c6ecf0c7b8`.

- RED: old driver `0829053b2054025711aa768caf20cd52baf30ce50f3371d2ab04be267fbe8757`, old include `ab11f15c1e77e7d5a9655f37c7a76992222226cf3b550fb8ef805df2f388198a`, driver status 143, oracle status 1, `FOREIGN_OBJECT_CHANGED`.
- GREEN: current driver `49a3dd1829c3d651aa9efa48036b3a7678c2b7dc0bbf28e14aa628c4c7a71d31`, current include `a899d87ed398cbfff75b965b01a5575912cb9b0696b048ffdd55732d278ee165`, driver status 143, oracle status 0, `FOREIGN_OBJECT_PRESERVED`.

The five target classes and their reacquisition/lifecycle records are retained under `build/ci/phase16-direct/cr01-causal-final/{tracer-red,tracer-green}`; raw evidence manifests were checked by the harness. Captured console output is in `build/ci/phase16-direct/cr01-causal-final/logs/`.

### Focused verification

- `sh -n tests/retroarch/state_session.sh tests/retroarch/test_state_session.sh tests/retroarch/interruption_trace.sh` and `git diff --check` passed.
- Ordinary focused CI passed both registered tests: `ctest --preset ci -R '^retroarch[.]session[.](controls|readiness)$' --output-on-failure`. Readiness took 18.72s; default-all controls took 311.89s and included lock PID preflight, the 44-case progression controls, 20 fault controls, wrapper/interruption/concurrency controls, and complete 16-boundary `VERDICT_OK`. Final log/exit: `build/ci/phase16-direct/ci-focused-session-after-interrupt-root/ctest.log` and `ctest.exit` (`0`).
- `cmake --preset asan` and `cmake --build --preset asan` passed. The same two focused session tests passed under ASan/UBSan; readiness took 23.32s and controls 384.24s. Logs/exits are under `build/phase16-direct/asan-{configure,build}.log`, `build/phase16-direct/asan-focused-session/ctest.log`, and matching `.exit` files (all `0`).

### Diagnosed first failures

1. The first focused CI run reached the existing production-prefix test but the driver rejected its fresh lock-root directory because the current candidate no longer created that directory before endpoint preflight. Added bounded `mkdir -p "$LOCK_ROOT"` with refusal on failure. First log: `build/ci/phase16-direct/ci-focused-session/ctest.log` (exit 8).
2. The next run passed the production-prefix test and exposed that the setup-failure fixture retried the same RA root while retaining its O_EXCL test reservation observation. The test now proves that observation exists and moves it to `failed-setup-lock-reservation.tsv` before retrying. Log: `build/ci/phase16-direct/ci-focused-session-after-lock-root/ctest.log` (exit 8).
3. The following run passed the prior controls and exposed an unbound `interrupt_lock_root` in the candidate's interruption fixture under `set -u`. The fixture now defines `$interrupt_dir/locks` once and uses it for setup and driver environment. Log: `build/ci/phase16-direct/ci-focused-session-after-reservation-observation/ctest.log` (exit 8).

All three failures were retained; affected checks were rerun on the final manifest and passed. No test was removed, skipped, weakened, or given a larger timeout.

### Independent-review correction and final-source verification

The first exact-source review found one material gap: bootstrapped production re-entry trusted a matching environment-supplied alternate port if FD 9 held a valid socket there. Added a production-mode fixed-port check and a regression that constructs a genuine non-listening TCP reservation on an ephemeral port, supplies matching PID/start/device/inode metadata, and requires `LOCK_OWNER_UNAVAILABLE` before the reservation record, host marker, or transport marker. The existing valid test-mode port path remains available for isolated controls. The reviewer confirmed the suspected busy-port ordering issue was a line-order misread: required inputs and budgets at lines 17–23 execute before the bootstrap branch.

The same frozen causal oracle was rerun after the correction against the final source. RED used the exact pinned old driver/include from `3643d6ac04c8c09391a6f2ea68c7217ba4c5ba25` (`0829053b2054025711aa768caf20cd52baf30ce50f3371d2ab04be267fbe8757`, `ab11f15c1e77e7d5a9655f37c7a76992222226cf3b550fb8ef805df2f388198a`); GREEN used final driver SHA-256 `ab54e4ce8aed4b24fb871afb3d8c070b0da25823f3d2105446bc9fb26b7361e4`. Both used final harness SHA-256 `6869d8d62b545b7d08b5d12936e5cc1fdedc453175896dd802b886ff5bf92b93` and oracle SHA-256 `1963fbb71f538a7b14ff6c10a679ee09f992528151204b25d219c97c6ecf0c7b8`. RED recorded status 143 / oracle status 1 / `FOREIGN_OBJECT_CHANGED`; GREEN recorded status 143 / oracle status 0 / `FOREIGN_OBJECT_PRESERVED`. Both five-target suites passed the same raw-evidence checks; GREEN also proved held-port refusal and exact-port re-entry after owned reap. Logs and retained case artifacts are under `build/ci/phase16-direct/cr01-final-candidate2/{logs,tracer-red,tracer-green}`; exact result records are in each `result.env`.

- Direct final lock-PID/source-root preflight passed, including the alternate-port forged re-entry and all finalizer controls: `build/ci/phase16-direct/review-alt-port-reentry-rerun/console.log` (exit `0`). The first wrapper attempt ran the preflight successfully but its zsh post-command recorder assigned to read-only `$status`; the bash recorder then reused that preflight’s scratch path and correctly refused the existing root. Both tool/fixture issues were isolated; the rerun used a fresh case root and retained its log.
- Final ordinary focused CI passed `retroarch.session.controls` and `retroarch.session.readiness` (2/2, no skips); controls took 300.62s. Log/exit: `build/ci/phase16-direct/review-alt-port-focused-ci/ctest.log` and `.exit` (`0`).
- Final ASan/UBSan configure and build passed. The same focused tests passed 2/2, no skips; controls took 385.86s. Logs/exits: `build/phase16-direct/review-alt-port-asan/{configure,build}.log` and `.exit`, and `build/phase16-direct/review-alt-port-asan/focused-session/ctest.log` and `.exit` (all `0`).
- Final `cmake --workflow --preset ci` passed all four configure/build/test/package stages on the final 11-path manifest with four CTest jobs and 649/649 tests, no skips. Controls took 310.94s; CTest took 314.77s. Workflow log/exit: `build/phase16-parent56/review-final/ci-workflow.log` and `.exit` (`0`).
- The workflow freshly generated all three 0.1.10-macos-arm64 packages. Final archive SHA-256: library `cef2539204b3a460e36bf844b941611b3ca90e9bb3a654bc15d57b14c2031e2f`, libretro `878f33210400e288fb4bbb6942e6297cc522716cc79eafda5f869bbf2cc02068`, runner `dbecf9b1223073b62e579ac18c4ac98fa07d070a11c9ced85edd4eff2b48ee79`. `cmake -DDIR=build/ci -P tests/cmake/check_archives.cmake` passed all three package metadata/content checks; log/exit: `build/phase16-parent56/review-final/archive-check/packages.log` and `.exit` (`0`). Final exact 11-path hashes are retained in `build/phase16-parent56/review-final/source-manifest.json`.
- Final `scripts/hygiene.sh --tree` and `git diff --check af7f9d0 -- <five candidate paths>` passed; exits are in `build/phase16-parent56/review-final/hygiene/tree.exit` and `build/phase16-parent56/review-final/diff-check.exit` (both `0`).

### Final parent acceptance on the exact candidate

The parent reran its configured acceptance directly on candidate `2c356d574290aef1d57122566f7b81beaaa10027`; no candidate source changed afterward. The parent source manifest at `build/phase16-parent56/final-parent/source-manifest.json` matches the worktree and HEAD for all 11 source/support paths. `cmake --build --preset ci` passed (`build.log`, `build.exit=0`), followed by `cmake --workflow --preset ci` passing configure, build, test and package (`ci-workflow.log`, `ci-workflow.exit=0`). CTest passed 649/649 with zero skips and four configured CTest jobs; controls took 320.45s and total workflow/test time was 324.18s. The parent package validator passed all three fresh packages (`packages.log`, `packages.exit=0`); `packages.json` records current archive SHA-256 values and members, with README/header/info matching current source. Final parent package hashes: library `af1a780dbbb7db312357858b8263f159d7cb3a3b007673d5caa30e87fa4606f7`, libretro `cae36175eba8496dcbe5997ac36be574f7450ce9b9da2d29b7c620903ad6e5c6`, runner `dc5ed75d5ce9a2efce20e0753ff88fa8acdd6a83d860d3aa00d8a4dd1bbeeccd`. Parent hygiene, `git diff --check 227bd6a`, and `sh -n` passed (`hygiene.exit`, `diffcheck.exit`, `sh-n.exit`, all `0`). `.planning/config.json` remains unchanged at SHA-256 `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e`; the index was empty before metadata staging.

### Initial parent workflow, packages and hygiene (superseded by the final-source run below)

The parent ran cmake --workflow --preset ci against this unchanged 11-path source/support manifest. All four workflow stages (configure, build, test, package) succeeded; CTest retained four jobs; CTest passed 649/649 with no skips. Session controls passed in 305.80s; total workflow/test time was 309.46s. Exit is recorded in build/phase16-parent56/ci-workflow.exit (0), with full output in build/phase16-parent56/ci-workflow.log.

The tested 11-path source/support manifest is build/phase16-parent56/source-manifest.json:

| Path | SHA-256 |
|---|---|
| tests/retroarch/state_session.sh | 49a3dd1829c3d651aa9efa48036b3a7678c2b7dc0bbf28e14aa628c4c7a71d31 |
| tests/retroarch/test_state_session.sh | c5c121a991df31695fe0c6430996cb900f5d40d2df4fc17eaea1bba39e4d4a31 |
| tests/retroarch/interruption_trace.sh | a899d87ed398cbfff75b965b01a5575912cb9b0696b048ffdd55732d278ee165 |
| README.md | c7f4dccde86f0d537c7823f98043240fed3a4e1930307f38efc0e6174b419d9f |
| include/nesturbator.h | a6d2539797585b6079e445cb772e5d2dc3109f83a47b237e5a0606764c21b137 |
| tests/retroarch/frame_index.c | 6c828e1829f8df03a617acd99fee7c67bd8f8f71d126ff0a3b2104448eb55cdf |
| tests/retroarch/CMakeLists.txt | 84bbaf464d812c1142e85851203f56e41f04511fee8bc16cf09bcc898400ec0e |
| tests/retroarch/run_retroarch.cmake | 6a387dc0ac1c2be498f5375d597e4d84ae69763a471e73e80c6a5400972e9581 |
| CMakePresets.json | a97957fcd87052b863be85a26609c0c985402b8eac645748907cec6ea237a024 |
| tests/cmake/check_archives.cmake | 67fbab69fffe5660b17d6be2ef0b7f3c0414f8e876f4105e85fd36ef2e69fc86 |
| libretro/nesturbator_libretro.info | b8cab0d6084b919eed0028086e2d8a8e2fd1a8b42f9196bfd35b90fdd7bbaa23 |

The earlier workflow freshly generated all three 0.1.10-macos-arm64 packages. `build/phase16-parent56/packages.json` binds their archive SHA-256 values and member lists; `packages.log` records `check_archives.cmake` accepting all three. Library archive SHA-256 was `48cc4f5e6e59a6f329e49dc94f84928591217587a510499caccbcfdc04ca7b45`; libretro was `e40b24fb73a994f5d7f5e562e81b75d0de67fa33c4a8f644e88da47082d31921`; runner was `8cefb54aa3191b005ada8a4603196a3fba04582232e10ae31dce4e50b18a1729`. This earlier run was on the initial `2e9928b` candidate and is superseded for final-source acceptance by the final run above.

The initial candidate's `scripts/hygiene.sh --tree` and `git diff --check 227bd6a` also passed. Parent verified no source-manifest drift on that candidate; the final workflow/source manifest and configuration SHA-256 are recorded above. The post-commit index is SHA-256 `688a63724ba498fe11e4ba4fee038d646ce68fde2a48113fd95af452c0ec3085`; no path is staged.

### Commit and independent review status

Source commits `2e9928b7eb68604e6d11ecb5b240e7915b28c4e3` and `2c356d574290aef1d57122566f7b81beaaa10027` contain the five tested behavior/test/documentation paths; final workflow/archive/hygiene/diff checks and same-oracle CR-01 proof passed on final bytes, and the latest source commit ran the normal noreply hooks. The post-commit SHA-256 manifest and blob equality are verified; there are no tracked deletions. The standard independent exact-source review of candidate `2c356d574290aef1d57122566f7b81beaaa10027` is clean with zero findings, recorded in `16-56-REVIEW.md`; `16-56-REVIEW-DISPOSITION.md` closes original CR-01 and records the resolved alternate-port blocker. Parent-owned configured acceptance passed on the unchanged candidate, with its manifest/logs and package hashes recorded above. The only remaining commit is the four-file Plan 16-56 metadata commit. Configuration remains unstaged and byte-identical to its original hash.
