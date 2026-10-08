# Phase 03 Plan 08: Hosted RetroArch Trial Checkpoint

**Status:** Incomplete pending the final required CI run. The six-platform hash inventory is implemented, and hosted RetroArch frame equality passed on exact commit `c5ec53b`; local workflow changes promote that check to `ci-required`.

## Completed Work

- Commit `d671a65` validates the exact 30 game and movie hash keys, rejects incomplete or duplicate generated frame rows, and checks six nonempty byte-identical CI artifacts. It also reports nightly ROM-loader fuzz outcomes.
- Commit `b3b94fd` adds a real-game frame-60 RetroArch driver path, a `retroarch.game` CTest registration, and an opt-in `macos-15` candidate job excluded from `ci-required`.
- Candidate asset: official RetroArch v1.22.2 universal macOS DMG at `https://buildbot.libretro.com/stable/1.22.2/apple/osx/universal/RetroArch_Metal.dmg`.
- Locally measured SHA-256: `81b79121ba26d539064ae13b4d0419a120c3d165afbe656cf5f5412b15fdb434`. The complete 232,801,022-byte image passes `hdiutil verify`; its embedded app reports version 1.22.2 and architectures `x86_64 arm64`.
- Frame 60 of the manifest-listed Nesteroids image is its visible title screen and is the runner/RetroArch comparison target.

## Local Verification

- `runner.write_hashes` and `runner.write_hashes.content`: passed.
- `retroarch.compare` and `retroarch.compare.cli`: passed.
- `cmake --workflow --preset ci`: 346/346 passed, including packaging, after the hosted-build fixes. `retroarch.testframe` and `retroarch.game` skipped only because the local macOS GUI session aborted RetroArch with empty stdout/stderr; hosted `retroarch-e2e` supplies required released-app proof.
- `cmake --workflow --preset asan`: 344/344 passed.
- `cmake --workflow --preset nofp`: 5/5 passed.
- `cmake --workflow --preset hygiene`: 8/8 passed after formatting `runner/main.c` with clang-format 18.
- `git diff --check`: passed.
- Hosted run `37857200684` tested commit `389e4c7e6cf5d7783ae85aabec1465de5fafedb4`. Its candidate job stopped in `accuracy.scoreboard` because the job did not run `prepare_scoreboard_baseline.cmake`; the upload contains no asset evidence. This is an invalid/inconclusive RetroArch trial, not an arm64 compatibility result.
- The same hosted run exposed GCC 14 `-Wconversion` and `-Wformat-truncation` errors, plus MSVC C4310 in NMI status masking. These are fixed locally with explicit bounded arithmetic/string formats and a representable status mask.
- Hosted `macos-15` candidate success and screenshot equality remain unverified; no release compatibility or pixel-equality claim is made yet.
- The first hosted run also hit the ASan job's two-minute timeout (2m14s). The next PR run increases that job ceiling to five minutes based on this observed duration.
- Hosted retry run `37858136711` was dispatched against exact commit `f5888f216e7e3edd95e13bfb3956f2e7a805a101` with `retroarch_trial=true`. Its Linux and Windows matrix exposed strict-warning errors in test fixtures (`test_cartridge.c`, `test_sprites.c`, and the MSVC CRT warning in `accuracy.scoreboard`); these fixes pass all relevant local presets.
- Candidate run `37859023685` on exact commit `c5ec53b5402b3141f0fef13d4724f436d13f48c6` completed its local suite, verified the pinned DMG SHA-256 and RetroArch 1.22.2, launched the game in an isolated HOME/CFFIXED_USER_HOME/XDG environment, left the real home unchanged, captured a 1503-byte screenshot, and passed exact pixel comparison for frame 60. Artifact `retroarch-candidate-37859023685` (ID `11584984604`) preserves the evidence. Local workflow changes promote this passing job to required CI as `retroarch-e2e` and retain `retroarch-e2e-frames`.
- Hosted full CI runs `37858960926` and `37859023685` also found: the ASan job lacked its protected-main scoreboard baseline, the six-platform hash step omitted `MOVIE_WRITER`, and the movie test helper/writer lacked target-scoped MSVC CRT definitions. Those workflow and target fixes are local; the exact `write_hashes.cmake` invocation passed locally with the writer argument.

## Next

Publish the required-gate promotion and matrix fixes. Verify the exact-head PR run passes the six-platform builds, protected-main scoreboard, hash equality, ASan, no-float, hygiene, title, and required `retroarch-e2e`; confirm `retroarch-e2e-frames` is retained for that same head. The candidate already passes on its exact implementation commit, so no architecture fallback or owner gameplay check is needed.

The existing user edits to `.planning/config.json` and deletion of `.planning/HANDOFF.json` are preserved and unstaged.
