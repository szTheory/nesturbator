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
- `cmake --workflow --preset ci`: 344/346 passed. `retroarch.testframe` and `retroarch.game` both exited with `Subprocess aborted` and empty stdout/stderr on the local macOS GUI environment.
- `cmake --workflow --preset asan`: 344/344 passed.
- `cmake --workflow --preset nofp`: failed `abi.float_scan` on pre-existing comments in `src/cartridge.c`, `src/palette_ntsc.c`, and `include/nesturbator.h`; no files from this plan introduced those lines.
- `cmake --workflow --preset hygiene`: 7/8 passed; `hygiene.format` reports pre-existing formatting violations across unrelated tracked source and test files.
- Hosted `macos-15` / `macos-15-intel` candidate trial: not run. No release compatibility or pixel-equality claim is made.

## Blocking Checkpoint

Pushing the candidate commit to the configured GitHub remote was rejected by automatic approval review as exporting repository contents to an unverified remote; the review stated that trusted user messages did not specifically authorize that export. Do not retry through another path until explicit user approval is obtained.

After approval, push `phase/03-real-game-in-retroarch`, create or use its PR, and run the exact-commit CI workflow with `retroarch_trial=true`. Verify the candidate's measured checksum, exact version, core/game load, nonempty screenshot, and pixel equality on `macos-15`. If that candidate fails, run the bounded `macos-15-intel` candidate with its official architecture-specific asset and measured SHA-256. Only after a hosted candidate passes should the workflow promote it to required job `retroarch-e2e`, add it to `ci-required`, retain `retroarch-e2e-frames`, and collect the final exact-commit Actions evidence.

The existing user edits to `.planning/config.json` and deletion of `.planning/HANDOFF.json` are preserved and unstaged.
