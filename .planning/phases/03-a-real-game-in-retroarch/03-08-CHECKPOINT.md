# Phase 03 Plan 08: Hosted RetroArch Trial Checkpoint

**Status:** Incomplete. The six-platform hash inventory task is committed. The real RetroArch check is implemented as an opt-in hosted candidate trial and must not become a required gate until a hosted macOS runner proves it.

## Completed Work

- Commit `d671a65` validates the exact 30 game and movie hash keys, rejects incomplete or duplicate generated frame rows, and checks six nonempty byte-identical CI artifacts. It also reports nightly ROM-loader fuzz outcomes.
- Commit `b3b94fd` adds a real-game frame-60 RetroArch driver path, a `retroarch.game` CTest registration, and an opt-in `macos-15` candidate job excluded from `ci-required`.
- Candidate asset: official RetroArch v1.22.2 universal macOS DMG at `https://buildbot.libretro.com/stable/1.22.2/apple/osx/universal/RetroArch_Metal.dmg`.
- Locally measured SHA-256: `81b79121ba26d539064ae13b4d0419a120c3d165afbe656cf5f5412b15fdb434`. The complete 232,801,022-byte image passes `hdiutil verify`; its embedded app reports version 1.22.2 and architectures `x86_64 arm64`.
- Frame 60 of the manifest-listed Nesteroids image is its visible title screen and is the runner/RetroArch comparison target.

## Local Verification

- `runner.write_hashes` and `runner.write_hashes.content`: passed.
- `retroarch.compare` and `retroarch.compare.cli`: passed.
- `cmake --workflow --preset ci`: 346/346 passed, including packaging, after the hosted-build fixes. `retroarch.testframe` and `retroarch.game` skipped only because the local macOS GUI session aborted RetroArch with empty stdout/stderr.
- `cmake --workflow --preset asan`: 344/344 passed.
- `cmake --workflow --preset nofp`: 5/5 passed.
- `cmake --workflow --preset hygiene`: 8/8 passed after formatting `runner/main.c` with clang-format 18.
- `git diff --check`: passed.
- Hosted run `37857200684` tested commit `389e4c7e6cf5d7783ae85aabec1465de5fafedb4`. Its candidate job stopped in `accuracy.scoreboard` because the job did not run `prepare_scoreboard_baseline.cmake`; the upload contains no asset evidence. This is an invalid/inconclusive RetroArch trial, not an arm64 compatibility result.
- The same hosted run exposed GCC 14 `-Wconversion` and `-Wformat-truncation` errors, plus MSVC C4310 in NMI status masking. These are fixed locally with explicit bounded arithmetic/string formats and a representable status mask.
- Hosted `macos-15` candidate success and screenshot equality remain unverified; no release compatibility or pixel-equality claim is made yet.

## Next

Publish the local fixes to `phase/03-real-game-in-retroarch` and rerun CI plus `retroarch_trial=true` on that exact commit. The candidate job now prepares its protected-main scoreboard baseline before running the CI preset. If RetroArch itself fails after the pinned asset has installed and launched, run the bounded `macos-15-intel` trial with its official architecture-specific asset and measured SHA-256. Promote only a passing hosted candidate to required job `retroarch-e2e`, make `ci-required` depend on it, retain `retroarch-e2e-frames`, and verify the final exact-commit checks and artifact.

The existing user edits to `.planning/config.json` and deletion of `.planning/HANDOFF.json` are preserved and unstaged.
