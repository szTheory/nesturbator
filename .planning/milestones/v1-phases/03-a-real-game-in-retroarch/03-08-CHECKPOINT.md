# Phase 03 Plan 08: Completion Checkpoint

**Status:** Complete. The phase's recurring game, movie, six-platform hash, and released-RetroArch checks are automated; no owner gameplay or screenshot UAT remains.

## Required Hosted Evidence

- Implementation commit: `0ba7b8127f13bd06a6d1d35f7ff3dee6aa7c95d2`.
- CI run `37860982545`: all six platform build/test legs, hash equality, ASan, no-float, hygiene, the Conventional Commit title check, `retroarch-e2e`, and `CI required` passed.
- Nightly run `37860982588` on the same commit: `vectors-full` and `rom-loader-fuzz` passed.
- The official RetroArch 1.22.2 DMG checksum matched `81b79121ba26d539064ae13b4d0419a120c3d165afbe656cf5f5412b15fdb434`. Nesteroids frame 60 produced a 1503-byte screenshot that compared equal to the runner's frame 60.
- Artifact `retroarch-e2e-frames`, ID `11585683202`, retains the asset record, runner image, and RetroArch screenshot.
- The six platform hash artifacts each contain the same sorted 30-row inventory; `hash-equality` validates row shape, expected keys, and byte equality.

## Local Verification

- `cmake --workflow --preset ci`: 346/346 passed. The two optional local RetroArch launches skipped after the GUI session aborted; the required hosted E2E passed.
- `cmake --workflow --preset asan`: 344/344 passed.
- `cmake --workflow --preset nofp`: 5/5 passed.
- `cmake --workflow --preset hygiene`: 8/8 passed.
- The six downloaded hash artifacts also passed the corrected field/schema and byte-equality check locally.

## Follow-up Fixes

- Windows exposed shell-quoting failures in the movie and libretro test subprocesses. `tests/test_process.h` now launches children directly on Windows and POSIX, preserving test output capture without invoking a command shell.
- The hosted aggregate validator expected seven whitespace fields even though the generated rows contain six. It now validates the actual `key frame ticks value sha256 digest` row shape.
- The formatter failure during the first helper run was fixed before the passing exact-head CI run.

## Continuation

The D-11 automation-first policy remains in `.planning/PROJECT.md` and Phase 03 context. Do not repeat a manual RetroArch trial or ask for owner gameplay UAT; the required machine evidence is retained in CI.

PR #18 was squash-merged as `5919ac14`, and the release automation advanced `main` to `93942b7a` (v0.1.4). Phase 04 discussion and planning are complete; the next GSD command is `$gsd-execute-phase 04`.

The pre-existing local `.planning/config.json` edit and `.planning/HANDOFF.json` deletion were preserved and excluded from the PR.
