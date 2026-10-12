# Phase 13: Rewind and run-ahead in RetroArch - Research

**Researched:** 2026-10-12
**Domain:** libretro serialization, RetroArch 1.22.2 behaviour (rewind, run-ahead, network commands), `.info` capability claims, libFuzzer over a state loader
**Confidence:** HIGH (RetroArch behaviour measured on the pinned 1.22.2 binary; hosted-runner equivalence is assumption A1)

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

The owner selected all four areas and asked for a research fan-out per area (every
stakeholder lens, primary sources, adversarial pass) and one coherent recommendation per area,
followed automatically. The decisions below are those recommendations, reconciled with each
other.

### Serialize entry points and their tests (LR-01)
- **D-01:** `retro_serialize_size` returns `nesturbator_state_size(inst)` with content loaded
  and 0 otherwise (no content, after a failed load, after `retro_unload_game`). The test
  card (`retro_load_game(NULL)`) counts as no content: size 0.
- **D-02:** `retro_serialize` returns false with no content or `len < size`, writes nothing in
  that case, and on success writes exactly `size` bytes (bytes past `size` untouched). Two saves
  with no frame between give identical bytes.
- **D-03:** `retro_unserialize` accepts only `len == size` and a buffer the library accepts.
  It returns false for: no content, NULL, 0, 63, size-1, size+1, 2*size, flipped magic,
  version, identity or body byte (CRC), and a valid state from another ROM. After every
  rejected call the next frame's video and audio hashes and the next state bytes equal a
  lockstep control run that never made the call, and the call emits no callbacks.
- **D-04:** Size invariants are asserted in `tests/libretro/libretro_host.c` through the real
  entry points: equal to the library size for the loaded board, constant across 120+ frames,
  `retro_reset`, a CPU jam and a controller-port change; 0 again after unload; correct after
  unload and reload of a different board.
- **D-05:** `retro_unserialize` before the first `retro_run` (right after `retro_load_game`, and
  right after `retro_reset`) must give a next frame equal to the saved run's continuation.
- **D-06:** No serialization quirks: the adapter never calls
  `RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS`, and a test checks the adapter source for it.
- **D-07:** The host answers `RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT` with each context in
  turn (normal, run-ahead same instance, run-ahead second instance, rollback netplay); the state
  bytes must be identical for every context. The adapter ignores the context.
- **D-08:** The host answers `RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE` with audio and video
  off during speculative frames. Emulation never depends on the flags: state bytes at each real
  boundary are identical with the flags on or off. Skipping the callbacks is allowed only if
  trivial.
- **D-09:** A load-per-frame loop on the battery ROM `M1_P128K_C32K_W8K` must not raise the save
  generation, and `retro_get_memory_data`/`_size` stay unchanged across loads.

### Run-ahead host (LR-02 gate for `deterministic`)
- **D-10:** The gating host runs in `libretro_host.c` through the real `retro_*` entry points
  (single instance). Depths N = 1, 2, 3. Per displayed frame t: run the real frame with input
  `in[t]` (keep its audio, drop its video), `retro_serialize` R(t+1), run N speculative frames
  with audio dropped, show the last speculative frame's video, `retro_unserialize` R(t+1).
- **D-11:** Oracle wording (resolves the "equal to an uninterrupted run" ambiguity). The gating
  host feeds the speculative frames the script's real future inputs `in[t+1..t+N]`. Then the
  visible frame at tick t must equal the uninterrupted run's frame t+N at every tick, and the
  real-timeline audio, the state bytes at every real boundary and the final state must equal
  the uninterrupted run exactly. Any lossy save or load shows as a mismatch on every tick,
  not only on stable-input windows.
- **D-12:** A second mode repeats `in[t]` into the speculative frames, as RetroArch does. Its
  visible frames are compared tick by tick with a library-level two-instance model in
  `tests/core/test_state_runahead.c`: instance A runs the real timeline, and instance B
  receives A's state each frame, runs N frames and shows the last one. This covers RetroArch's
  second-instance mode as a library property. Running two dlopened copies of the adapter is
  not built (deferred).
- **D-13:** Input script: fixed-seed integer LCG on ports 0 and 1 covering directions, A, B,
  Select and Start. There are two scripts: one changes every frame, and one holds each input
  1..5 frames at random. Together they place changes at every alignment mod N+1, including the
  frame right after a load.
- **D-14:** ROM set: one per board (`M1_P128K_C128K_S8K`, `M2_P128K_CR8K_V`,
  `M3_P32K_C32K_H`, `M4_P128K_CR8K`, `M7_P128K_CR8K`, one NROM), plus `nesteroids.nes`, plus
  the Phase 12 stress images if `state_test.h` can be reused by the host. Otherwise the stress
  images stay in the library-level test. 180 frames per ROM, script and depth. The wall time
  is recorded in the PR; if `libretro.runahead` exceeds about 10 s on the slowest runner, trim
  script 2 first.
- **D-15:** Hashing uses the existing FNV-1a helper. The oracle is an in-process uninterrupted
  run, so no new pinned hash keys are added. Six-platform equality comes from the existing
  pinned hashes plus this test running in all six `build` jobs.
- **D-16:** Anti-vacuity controls run as expected failures of the same comparator: a host that
  skips the load, loads the stale state from t-1, applies the input one frame late, or lets
  speculative audio into the real stream. The normal run also asserts that the bytes before
  each load differ from R(t+1).
- **D-17:** CTests: `libretro.serialize` (D-01..D-09), `libretro.runahead` (D-10, D-11, D-13..D-16),
  `core.state_runahead` (D-12). All run in `ci`, `ci-msvc` and `asan`.

### `.info` claims and the six-platform proof order (LR-02)
- **D-18:** Squash merge hides commit order on `main`, so the guarantee is a property of the
  tree and holds on every platform. A configure-time guard in `tests/CMakeLists.txt` (run after
  all `add_test` calls, via `cmake_language(DEFER)` or at the end of the file) parses
  `libretro/nesturbator_libretro.info`. It fails configuration if a claimed level's proving
  tests are not registered: `serialized` needs `libretro.serialize`; `deterministic` needs
  `libretro.serialize`, `libretro.runahead` and `core.state_runahead`. The claim-to-tests table
  lives in one place.
- **D-19:** The `libretro.info_claims` test parses the `.info` file as `key = "value"` lines
  and asserts the exact capability key set and values, failing on any unknown capability key.
  `savestate = "true"` holds exactly when a level is claimed. The final state is
  `savestate = "true"` and `savestate_features = "deterministic"`, because the levels are
  cumulative: basic < serialized < deterministic. `libretro_saves`, `supports_no_game`,
  `memory_descriptors`, `core_options` and `disk_control` keep their current values. A negative
  self-test feeds the parser a bad file and expects a failure. The `.info` path is passed as a
  test argument.
- **D-20:** The proving tests may never skip or run zero iterations. They assert minimum counts
  of frames and states compared, and use no `SKIP_RETURN_CODE` and no `DISABLED`.
- **D-21:** Commit order on the phase branch (review discipline; the guard is the guarantee):
  1. Adapter serialize, `libretro.serialize`, guard and `libretro.info_claims`, with the `.info`
     still `savestate = "false"`.
  2. Push and open a draft PR. Once all six `build` jobs are green, commit the `serialized`
     claim (`savestate = "true"`, `savestate_features = "serialized"`).
  3. Run-ahead host, two-instance test and controls.
  4. The `retroarch-e2e` state step and the fuzz target.
  5. The last commit: `savestate_features = "deterministic"`, together with the README and
     adapter comments.

  A revert of that last commit leaves an honest tree.
- **D-22:** Push cadence:
  - Draft PR after step 1. Push after each claim commit and at the end.
  - Merge only when every check on the final head is green.
  - The PR title is a Conventional Commit.
  - Phase 12 showed GCC 14 `-Werror`, clang-format 18.1.3 vs 18.1.8 and the nofp float scan
    catching what macOS clang did not. Run the `hygiene` preset locally before each push, and
    compile changed C files with GCC 14 in Docker (`gcc:14` image, the `ci` warning flags,
    `-O3`).

### RetroArch e2e oracle (LR-03)
- **D-23:** The oracle identifies frames by matching. One RetroArch session is paused and
  stepped over network commands. Every screenshot is matched against the runner's continuous
  frames 1..W to find its frame index, and the assertions are on relative indices, so wall-clock
  timing only shifts the starting index. Content: `nesteroids.nes`, after asserting that its
  runner frame hashes for 1..W (W of about 900) are pairwise distinct. No input commands are
  needed.
- **D-24:** Transport: `printf %s CMD | nc -u -w1 127.0.0.1 55355` (macOS `/usr/bin/nc`),
  wrapped in one CMake function `ra_cmd` in `tests/retroarch/run_retroarch.cmake`. No owned
  UDP sender and no new dependency. Fence: `GET_CONFIG_PARAM savestate_directory`, which
  replies. Never use `GET_STATUS`: it segfaulted 1.22.2 in a probe. Don't use `READ_CORE_*`:
  the core has no memory map.
- **D-25:** The sequence runs in one session:
  1. Boot without `--max-frames`, and fence on the `[NetCMD]` log line.
  2. `PAUSE_TOGGLE`, then two identical screenshots more than 1 s apart, which proves the pause.
     Identify the frame as m0.
  3. `SAVE_STATE`. Assert one state file under `states/` with the core's state magic and a size
     of at least `nesturbator_state_size`.
  4. `FRAMEADVANCE` x5, then a screenshot, which must be m0+5.
  5. `LOAD_STATE`, then `FRAMEADVANCE` x1, then a screenshot, which must be the saved frame
     plus the measured offset.
  6. Resume and run about 3 s (frame m1). Send a `REWIND` burst, pause and take a screenshot:
     the frame must be at most m1 minus 60. Then `FRAMEADVANCE` x3 must give m2+3.
  7. Run-ahead. With `run_ahead_enabled`, a stepped frame must equal m3+1+k, where k is the
     measured, pinned display offset.
  8. `QUIT`, sent twice (or with `quit_press_twice = "false"`).

  Offsets that depend on RetroArch are measured once in a spike and pinned with a comment.
- **D-26:** Config added to `tests/retroarch/test.cfg.in` (only for the state session or
  generally, as the planner decides):
  - Verified in a 1.22.2 probe: `network_cmd_enable`, `network_cmd_port = "55355"`,
    `rewind_enable`, `run_ahead_enabled`, `run_ahead_frames = "1"`, `user_language = "0"`
    (English logs, required for grepping).
  - Unverified, so each needs an outcome assertion: `run_ahead_secondary_instance = "false"`,
    `savestate_file_compression = "false"`, `rewind_granularity = "1"`, `rewind_buffer_size`,
    `quit_press_twice = "false"`, `savestate_thumbnail_enable = "false"`.
- **D-27:** Screenshots come from the `SCREENSHOT` command. The script polls `shots/` for a new
  png under a deadline (0.1 s steps with `cmake -E sleep`, never a fixed sleep), renames it to
  `shot-<tag>.png`, then uses the existing `sips` → BMP conversion. `compare_frame` gains an
  identify mode that hashes the BMP's RGB bytes against the runner's `--hash-frame` values,
  provided both hash the same bytes. If they don't, it uses `--dump-frame` with a pixel compare
  per candidate.
- **D-28:** The session fails on any of:
  - a non-zero exit;
  - a WARN or ERROR line tagged `[Rewind]` or `[Run-Ahead]`;
  - a core `RETRO_LOG_ERROR` line;
  - "Failed to save state" or "Failed to load state";
  - the absence of the `[NetCMD]` line.

  The benign `[ERROR] failed_to_start_audio_driver` line is allowed. The existing check that
  nothing changes in the user's directory is kept, with `$TMPDIR` added if second-instance
  mode is ever enabled.
- **D-29:** CI: a new step "Run the RetroArch state, rewind and run-ahead commands" in the
  `retroarch-e2e` job, with its own `RA_DIR` (`build/retroarch-e2e/run-state`). It uploads
  `shot-*.png`, the state file and the verbose log on failure. The existing game and battery
  sessions are unchanged.

### State fuzz target (LR-04)
- **D-30:** Synthetic in-memory carts. The cart is an iNES header plus PRG and CHR filled by an
  integer LCG, built in the harness, so no ROM bytes or ROM-derived RAM reach the corpus and
  there are no file paths or environment variables. Real ROM seeds are deferred.
- **D-31:** `tests/fuzz/state_loader.c` holds a single `LLVMFuzzerTestOneInput`.
  - Byte 0, bits 0-3: one of 12 carts (6 boards × small/maximum PRG and CHR).
  - Byte 0, bit 4: raw mode, with no CRC fix-up.
  - Byte 0, bits 5-7: length mode (exact, -1, +1, header only).
  - Bytes 1 and up: a patch laid over a freshly saved baseline state.

  Each iteration uses a fresh instance.
- **D-32:** CRC fix-up lives in the harness. It is never a fuzz-only flag in `src/state.c`.
  Outside raw mode the harness restores the identity bytes (magic, version, header size, total
  size, mapper, submapper, region, ROM CRC) and leaves the behaviour revision, frame, ticks and
  reserved bytes mutated. It then recomputes the state CRC with a CRC-32 copied into the
  harness.
- **D-33:** Oracles use `abort()`, never `assert`.
  - A rejected load leaves `save_state` output byte-identical and returns a documented status.
  - An accepted load must survive 2 frames with a fixed pad, with status OK or STOP_JAM.
  - After those frames, `save_state` must succeed and reload into a fresh instance of the same
    cart.
  - Save, load, save again must give identical bytes.
  - Two fresh instances loading the same state and running the same frames must give equal
    frame and audio hashes.
  - `save(load(S)) == S` is not asserted until the researcher confirms whether validation
    canonicalises masked bits.
- **D-34:** Corpus: `tests/fuzz/corpus_state/` holds `valid-<cart>` (12), a few `reject-*`
  seeds and later `regress-<slug>`, under 100 KB in total.
  - The seeds are generated by the replay build's `--write-seeds <dir>` mode, never written by
    hand.
  - Replay asserts that every `valid-*` seed loads in fix-up mode, which catches seed drift
    when the layout changes.
  - The manifest lines are generated if hygiene needs them.
- **D-35:** Move the existing `replay_file`/`main` from `tests/fuzz/rom_loader.c` into a shared
  `tests/fuzz/replay_main.h` (sharing existing code, not a new dependency).
  - The CTests are `fuzz.state` and `fuzz.state.regress`: a `CONFIGURE_DEPENDS` glob over the
    corpus that fails if empty, with `TIMEOUT 30`, run in `ci`, `ci-msvc` and `asan`.
  - `tests/cmake/fuzz_registration.cmake` requires the new test.
  - libFuzzer target `fuzz.state.libfuzzer` under the existing `NESTURBATOR_LIBFUZZER`.
- **D-36:** A nightly job `state-fuzz` is copied from `rom-loader-fuzz` (duplication over
  abstraction).
  - It runs `-max_total_time=60 -timeout=5 -max_len=32768` (or a larger `-max_len` if the
    maximum state is bigger) on `tests/fuzz/corpus_state`.
  - It sets `-artifact_prefix`, uploads `crash-*` and prints a hex dump in the step summary.
  - It records its outcome the way the existing job does, and joins whatever aggregation
    `nightly.yml` uses.
  - A crash it finds becomes a `regress-` seed in the PR that fixes it.

### Claude's Discretion
- The exact file split between the adapter and the tests, helper names, and the frame
  counts within the stated budgets.
- Whether the run-ahead depths and scripts share one CTest or several.
- Whether the state-session keys go into `test.cfg.in` globally or into a second template.

### Deferred Ideas (OUT OF SCOPE)
- Running two dlopened copies of the adapter for a second-instance run-ahead test. Do it only
  if the adapter gains state it keeps per instance.
- RetroArch e2e variant with `run_ahead_secondary_instance = "true"` (copies the core to a temp
  dir; needs a `$TMPDIR` snapshot).
- Fuzz seeds from real test-ROM states, if nightly coverage of mapper paths is low.
- A longer fuzz frame mode (10+ frames) for counter-wrap bugs.
- A cross-platform fuzz replay digest.
- Run-ahead cost row in the speed report (Phase 15).
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| LR-01 | `retro_serialize_size`/`retro_serialize`/`retro_unserialize` over the library state | The library already provides the whole contract (`include/nesturbator.h:435-501`, `src/state.c`); the adapter is three one-line delegations. Spike (patched copy, not in repo) ran save, load, rewind and run-ahead in RetroArch 1.22.2 with exactly that delegation. Return-value mapping and edge cases: "Adapter" below. |
| LR-02 | `.info` claims `serialized` then `deterministic`, each backed by tests on the same tree | Value set and RetroArch's real gating measured (flag 1). The claim is only exercised by RetroArch if the core sits in `libretro_directory` (flag 1, critical). `if(TEST name)` inside `cmake_language(DEFER CALL ...)` verified for the D-18 guard. |
| LR-03 | `retroarch-e2e` saves, loads, rewinds and runs ahead through network commands | Command semantics measured (flags 3a-3g). Two deterministic lanes found that need no wall clock. |
| LR-04 | Nightly libFuzzer target over the state loader, corpus replayed in `ci` | Max state 45,869 bytes, no canonicalisation, no long-frame field (flags 4-6); hygiene manifest DOES cover `corpus_state/` (flag 6). |
</phase_requirements>

## Summary

All seven research flags are settled. Most of the plan in CONTEXT.md stands. Six places need changing before planning, found by running the released RetroArch 1.22.2 against a throwaway copy of the adapter with three delegating serialize functions and the `.info` claim edited (the copy lives in the scratchpad, never in the repository):

1. **The current e2e script never exercises the `.info` file.** RetroArch applies the `.info` capability level only when the core file sits in `libretro_directory` (`RA_DIR/cores`) beside the info. `run_retroarch.cmake` passes `-L` to the build tree, and in that setup `savestate = "false"`, `basic`, `serialized` and `deterministic` all behaved identically (rewind and run-ahead ran). With the core copied into `RA_DIR/cores`: `false` and `basic` made no serialize call, `serialized` serialized (rewind) but never unserialized (no run-ahead), `deterministic` ran run-ahead. A missing `savestate_features` key with `savestate = "true"` behaved as `deterministic`. So the e2e must copy the core into `RA_DIR/cores` and `-L` that copy, and `libretro.info_claims` must require the key to be present.
2. **`LOAD_STATE` is ignored while RetroArch is paused; `LOAD_STATE_SLOT 0` works while paused.** A paused load draws nothing, so a screenshot right after it shows the old frame. The first `FRAMEADVANCE` afterwards shows exactly saved-frame+1.
3. **While run-ahead is active and the content runs, RetroArch drops `PAUSE_TOGGLE`, `SCREENSHOT`, `RUNAHEAD_TOGGLE` and `QUIT`.** Only `VERSION` and `GET_CONFIG_PARAM` (the replying commands) still answered. Turn run-ahead on only while paused, never unpause afterwards, and send `QUIT` while paused. Turning `run_ahead_enabled` on in the config has the same effect from boot.
4. **nesteroids frames are not pairwise distinct from frame 1.** 1344 distinct hashes in 1500 frames; the longest all-distinct window is 109 frames (starting at frame 32). Every duplicate is an adjacent pair (one pair 3 apart). The D-23 assertion "frames 1..W are pairwise distinct" fails as written; use set-valued matching.
5. **`nc -u -w1` costs 1 s per command** even when no reply comes. Use `-w0` for fire-and-forget and `-w1` only for `GET_CONFIG_PARAM`/`VERSION`. Commands are level flags, not a queue: 8 back-to-back `FRAMEADVANCE` advanced 2-3 frames, and a fence reply does not prove earlier commands ran. Pace commands (50-100 ms) and confirm with screenshots.
6. **Two deterministic lanes need no network commands and no wall clock**: `--max-frames=N` gives exactly runner frame N, `N+k` with run-ahead depth k (k=1,2,3 measured), and `savestate_auto_save`/`savestate_auto_load` across two `--max-frames` sessions gives exactly runner frame S+M-1. They sit beside the network lane, which Success Criterion 3 requires.

**Primary recommendation:** Implement the adapter as three delegations. Make the e2e copy the core into `RA_DIR/cores`. Pin run-ahead and save/load offsets from the deterministic `--max-frames` lane, and use the network lane only for what only it can do (SAVE_STATE, paused LOAD_STATE_SLOT, REWIND, RUNAHEAD_TOGGLE, QUIT), identified with set-valued frame matching.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| State bytes, validation, all-or-nothing load | Library (`src/state.c`) | — | Done in Phase 12; the adapter adds nothing. |
| Mapping to libretro return values | Adapter (`libretro/libretro.c`) | — | `size_t`/`bool` mapping only; no state of its own. |
| `.info` claim | Packaging file (`libretro/nesturbator_libretro.info`) | Test guard (`tests/CMakeLists.txt`) | RetroArch gates rewind and run-ahead on it; the tree guard keeps claim and proof together. |
| Run-ahead loop, rewind buffer, slot files | Host (RetroArch / simulated host) | — | CONTEXT scope: no loops in the core. |
| Run-ahead proof | Simulated host (`libretro_host.c`) + library model (`test_state_runahead.c`) | RetroArch e2e | Hash equality is exact only in-process; RetroArch proves integration. |
| State-loader robustness | Fuzz harness (`tests/fuzz/state_loader.c`) | `ci` replay | Oracles live in the harness; library gets no fuzz-only flag. |

## Standard Stack

No new packages and no new dependencies (owner rule).

### Core
| Tool | Version | Purpose | Why |
|------|---------|---------|-----|
| C17 adapter, library state API | repo | serialize entry points | Already built in Phase 12. |
| CMake / CTest | 3.25 minimum; 4.4.3 local | tests, guard | `cmake_language(DEFER)` (3.19) and `if(TEST name)` (3.4) verified here. |
| RetroArch | 1.22.2 pinned DMG (SHA-256 in `ci.yml`) | e2e | Local app is the same version: `Version: 1.22.2 (Git 69a4f0ea) Nov 20 2025`. |
| `/usr/bin/nc` (macOS BSD) | system | UDP network commands | Present on the hosted runner (macOS image); no owned sender. |
| Clang libFuzzer | Linux, nightly | state fuzz target | Same recipe as `rom-loader-fuzz`. |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| `nc` per command | an owned UDP sender in C | New code for no gain; rejected by owner preference. |
| set-valued frame identify via PPM dumps | `frame_index` tool linking the library | See "Frame identification". Dumps for W=900 are about 166 MB. |

**Installation:** none.

## Package Legitimacy Audit

No external packages are installed in this phase. `libretro.h` is already vendored and untouched. Nothing to audit.

**Packages removed due to [SLOP] verdict:** none
**Packages flagged as suspicious [SUS]:** none

## Settled research flags

### Flag 1 - `savestate_features` values and RetroArch gating
**Answer.** Spelling is `basic`, `serialized`, `deterministic`, written as `savestate = "true"` plus `savestate_features = "<level>"`. Source comment, verbatim: `# If true, how complete is the savestate support? basic, serialized (rewind), deterministic (netplay/runahead)` with the example line `savestate_features = "serialized"` [CITED: raw.githubusercontent.com/libretro/libretro-core-info/master/00_example_libretro.info]. docs.libretro.com has no core-info page (the URL returns 404), so this example file is the primary source. A real shipped file uses `savestate_features = "deterministic"` (gpSP) [CITED: raw.githubusercontent.com/libretro/libretro-super/master/dist/info/gpsp_libretro.info].

**Measured gating in RetroArch 1.22.2** (adapter copy with delegating serialize; run-ahead enabled in config; core copied to `RA_DIR/cores`; counts of core entry calls over about 3 s) [VERIFIED: local run of RetroArch 1.22.2, this session]:

| `.info` claim | `retro_serialize` calls | `retro_unserialize` calls | Meaning |
|---------------|------------------------|---------------------------|---------|
| `savestate = "false"` | none | none | rewind and run-ahead both off |
| `savestate_features = "basic"` | none | none | rewind and run-ahead both off |
| `serialized` | 384 | 0 | rewind buffer fills; run-ahead off |
| `deterministic` | 674 | 337 | run-ahead on (1 serialize + 1 unserialize per displayed frame, plus rewind's push) |
| `savestate = "true"`, key absent | 734 | 367 | acts as `deterministic` |

With the core outside `RA_DIR/cores` (what `run_retroarch.cmake` does now) the same four claims all produced run-ahead calls: the info was not applied. [VERIFIED: local run]. Confidence: HIGH for 1.22.2 on this machine. Whether the hosted macos-15 runner behaves the same is expected (same binary) and is asserted by the e2e outcome check below.

**Planning consequences.**
- e2e must `file(COPY ...)` the core into `${RA_DIR}/cores/` and pass that path to `-L`. `test.cfg.in` already sets `libretro_path` and `libretro_directory` to `@RA_DIR@/cores`.
- Add an outcome assertion in the state session that the claimed level took effect, not just trusting the file: serialized-first commit -> the session expects rewind to work and (optionally) run-ahead toggle to NOT call unserialize; final commit -> run-ahead works. A cheap, strong check is the deterministic lane: with `deterministic` and `run_ahead_enabled`, `--max-frames=N` yields runner frame N+1; with `serialized` it yields frame N (run-ahead off). That makes the claim level itself observable from a screenshot.
- `libretro.info_claims` must assert `savestate_features` present whenever `savestate = "true"`, because absence silently means `deterministic`.
- Capability key allowlist for D-19 (from the example file, verbatim key names): `savestate`, `savestate_features`, `cheats`, `input_descriptors`, `memory_descriptors`, `libretro_saves`, `core_options`, `core_options_version`, `load_subsystem`, `supports_no_game`, `single_purpose`, `database`, `database_match_archive_member`, `hw_render`, `required_hw_api`, `needs_fullpath`, `disk_control`, `is_experimental`, `description`. The current file sets `supports_no_game`, `savestate`, `libretro_saves`, `memory_descriptors`, `core_options`, `disk_control`, `is_experimental`, `database`, `description` [VERIFIED: `libretro/nesturbator_libretro.info` read this session]. The allowlist is a design choice; keep it to the keys this project may ever set.

### Flag 2 - RetroArch single-instance run-ahead order
**Answer (docs).** Single instance: (1) disable audio and video, run a frame, save state; (2) if running more than one frame ahead, run the extra frames with audio and video still disabled; (3) enable audio and video and run the frame to be seen; (4) load the saved state [CITED: docs.libretro.com/guides/runahead/]. The page does not say whose audio is kept. Two-instance mode exists because many cores leave audio in an unclean state after a load [CITED: same page].

**Answer (measured).** The visible frame leads the real frame by exactly `run_ahead_frames`: with `--max-frames=120` the screenshot equals runner frame 120 off, 121 for depth 1, 122 for depth 2, 123 for depth 3 [VERIFIED: local run]. Over the network: state saved with frame 449 on screen, `RUNAHEAD_TOGGLE`, `LOAD_STATE_SLOT 0`, one `FRAMEADVANCE` shows frame 451 = 449+1+1 [VERIFIED: local run]. With run-ahead on and rewind off the core saw one serialize and one unserialize per displayed frame (rewind adds one serialize per frame) [VERIFIED: serialize count about 2x unserialize in the table above].

**Plan.** D-10's order (real frame, serialize, N speculative frames, show last, unserialize) matches the docs and the measured call counts. Which frame's audio RetroArch keeps is not documented. Make it one named constant in the host AND assert both candidate streams: concatenated real-frame audio equals the uninterrupted run, and concatenated last-speculative-frame audio equals the uninterrupted run shifted by N frames (cheap hash equalities; the library is a pure function of state and input, so both hold when load is exact). Confidence: order MEDIUM (docs), lead HIGH (measured).

### Flag 3 - RetroArch spike (all measured on 1.22.2, content `nesteroids.nes`, isolated HOME as in the existing script)
Method: throwaway adapter copy with three delegations; a PNG decoder in the scratchpad hashed the screenshot RGB and matched it to the runner's `--dump-frame` PPM payload for frames 1..4000. Screenshot RGB equals the runner payload exactly, so no colour conversion is needed to identify a frame [VERIFIED: local run].

**3a. REWIND.** Works paused and running. `rewind_enable = "true"` with `rewind_granularity = "1"` logs `[Rewind] Initializing rewind buffer with size: 20 MB`, so the old "Failed to initialize rewind buffer" is gone and `rewind_buffer_size` need not be set. Paused, 40 `REWIND` datagrams 50 ms apart moved the screen from frame 390 to 354 (net -36, about one step per command); 100 back-to-back moved it -32 (datagrams coalesce); 5 spaced 0.1 s moved -1. After every rewind, successive `FRAMEADVANCE` + screenshot gave idx+1 each time (354, 355, 356, 357; and 325, 326) [VERIFIED: local run]. That consecutive-frame result is the real proof that the rewound state loaded exactly. Recommendation: pause, send 40 `REWIND` at 50 ms spacing, assert the new frame is at most (old - 20), then 3 x (`FRAMEADVANCE`, screenshot) must be new+1, +2, +3. Confidence HIGH on behaviour, MEDIUM on the exact -36 (use the loose bound).

**3b. LOAD_STATE and redraw.** `LOAD_STATE` while paused did nothing: zero `retro_unserialize` calls in 3 of 3 attempts, even after unpausing for 0.4 s. While running, `LOAD_STATE` worked and logged `[State] Loading state "...", 4920 bytes.` then `Saving state "RAM", 4920 bytes.` (an undo buffer). `LOAD_STATE_SLOT 0` worked while paused (the UDP reply echoes the command) and called `retro_unserialize` once [VERIFIED: local run]. A screenshot taken right after a paused load still showed the pre-load frame (604/605), i.e. no redraw; the first `FRAMEADVANCE` afterwards showed saved+1 (state saved after frame 363, then 364/365; the pair is a duplicate-hash pair) [VERIFIED: local run]. Offset after load: zero, i.e. displayed = saved + number of frames advanced. Use `LOAD_STATE_SLOT 0`, never bare `LOAD_STATE`, in the paused sequence.

**3c. Run-ahead display offset and the command hazard.** Offset k = `run_ahead_frames` (1, 2, 3 measured; see flag 2). Hazard: with run-ahead active and the content running, `PAUSE_TOGGLE` (6 tries), `SCREENSHOT`, `RUNAHEAD_TOGGLE` and `QUIT` (3 tries) had no effect; the process had to be killed. `VERSION` and `GET_CONFIG_PARAM` still answered [VERIFIED: local run]. The same happens when `run_ahead_enabled = "true"` is in the config from boot. All run-ahead network steps worked while paused (toggle, load, frame advance, screenshot, quit). Required sequence: be paused, `RUNAHEAD_TOGGLE`, `LOAD_STATE_SLOT 0`, `FRAMEADVANCE`, screenshot (expect saved+1+k), `QUIT` while still paused. Add a hard `TIMEOUT` and a kill in the driver. Do not put `run_ahead_enabled = "true"` in a config used by the network session.

**3d. State file container and size.** Path is `states/nesturbator/game.state` (a per-core subdirectory), so the assertion must glob recursively. With `savestate_file_compression = "false"` the layout is: bytes 0-7 `RASTATE` + `0x01`; `MEM ` + u32 little-endian size (= `nesturbator_state_size`, 0x131f = 4895); the core's state starts at offset 16 and begins with the library magic `89 4E 54 53 0D 0A 1A 0A`; the payload is padded to 8 bytes; then `END ` + 4 zero bytes. File = 16 + roundup8(S) + 8 = 4920 for S = 4895, matching RetroArch's own log line (`4920 bytes`) [VERIFIED: hexdump of the saved file]. Assert: magic at offset 16, `MEM ` size field == S, file size in [S+8, S+32]. The container was observed from the binary's output file, not from its source. Auto-save file name is `game.state.auto` in the same directory.

**3e. Datagram ordering and the fence.** Not a FIFO of effects. Commands act as per-loop-iteration flags: 8 back-to-back `FRAMEADVANCE` advanced 2-3 frames, and a following `GET_CONFIG_PARAM` reply did not mean they had all run [VERIFIED: local run]. `GET_CONFIG_PARAM savestate_directory` replies (`GET_CONFIG_PARAM savestate_directory <path>`); per CONTEXT D-24 `GET_STATUS` crashed 1.22.2 and stays banned; `LOAD_STATE_SLOT`'s reply is an echo. Consequence: replace "fence on order" with "confirm by outcome": send one command, pace 50-100 ms, then poll a screenshot until the frame index changes or a deadline passes. Only `VERSION`/`GET_CONFIG_PARAM` replies are usable as readiness probes (the `[NetCMD]` log line is the other). `nc -u -w1` blocks 1 s even for non-replying commands; `-w0` returns in about 7 ms and still delivers [VERIFIED: timing, local run].

**3f. QUIT.** With `quit_press_twice = "false"` one `QUIT` exited cleanly in clean sessions (paused or running without run-ahead). It does nothing under active run-ahead (3c) [VERIFIED: local run].

**3g. Wall clock.** Displayed rate was not 60 fps in this environment (about 40-130 frames per second in different stretches), so only relative indices can be asserted, as D-23 says.

**3h. Logs.** Output redirected to a file is block-buffered: lines lag and are lost on `kill -9`; read the log after a clean exit. The log contains `[mvk-warn] VK_ERROR_FEATURE_NOT_PRESENT...` lines and the benign `[ERROR] failed_to_start_audio_driver`; D-28's failure patterns must be anchored at line start (`^\[(WARN|ERROR)\] \[(Rewind|Run-Ahead)\]`), not a bare "ERROR". Failure strings like "Failed to save state" were not seen; the outcome checks (frames, file) carry the proof.

### Flag 4 - does load canonicalise masked bits?
**Answer: no.** `nesturbator_load_state` copies every in-range field verbatim (the `nesturbator__sio_*` primitives in `src/state.h` return the stream value and `APPLY` stores it unchanged) and rejects out-of-range values. The only bytes `save(load(S))` rewrites: bytes 16-19 (behaviour revision, `H_BEHAVIOUR`, written as the current `NESTURBATOR_BEHAVIOUR_REVISION`, currently 6 [VERIFIED: `include/nesturbator.h:56`]) and, as a consequence, the CRC at 28-31. Empirical probe (scratchpad, not in repo): 8 synthetic carts across the 6 boards, random in-range patches over a saved baseline (about 1/3 of bounded non-cross fields per iteration plus random byte flips in half the iterations), CRC recomputed, 3000 iterations per cart; about 17,000 accepted loads (8 synthetic carts), 0 differences in `save(load(S))` versus `S` outside bytes 16-19 and 28-31 [VERIFIED: local probe]. Confidence: HIGH for the rule from reading the field primitives, MEDIUM-HIGH for "no field is silently adjusted" (random, not exhaustive; cross-field and signed fields were not mutated).

**Plan.** Assert in the harness: `expected = S` with bytes 16-19 set to the current revision and the CRC recomputed, then `save(load(S)) == expected`. Do not drop the oracle.

### Flag 5 - can a field other than ticks make one frame run long?
**Answer: no other loop is driven by state.** The only unbounded-looking loops are `while (inst->ticks < target && inst->cpu.jammed == 0u)` (`src/frame.c:55`) and `while (nes->ppu.ppu_ticks + 8u <= ticks)` (`src/ppu.c:472`). Both are bounded by header ticks (capped at 2^62) and by the checked `ppu_ticks` gap (less than one frame). The PCM drain loop is bounded by the ring count. Probe: the same accepted random states each ran 2 frames; the worst time was 13.4 ms per frame (MMC1 cart, first iterations include a cold start) [VERIFIED: local probe, `-O2`]. Confidence: MEDIUM-HIGH. Safe default: `-timeout=5` as in D-36, abort on any unexpected status.

### Flag 6 - maximum state size, and hygiene
**Answer.** Largest state for any accepted cart: 45,869 bytes (MMC1, 512 KiB PRG, CHR RAM, 32 KiB PRG RAM from iNES 1 defaults at `src/cartridge.c:205`) [VERIFIED: `nesturbator_state_size` on a synthetic cart, local]. Small boards: NROM 4,895; MMC1/MMC3/UxROM/AxROM with 8 KiB work RAM about 13,100. 65,536 is a safe `-max_len`; the existing `rom-loader-fuzz` already uses 65536. D-36's 32768 would be too small if any input is a whole state; with D-31's patch layout the inputs are small, so the bound is a free choice. Use 65536 for parity with the sibling job.

**Hygiene.** `scripts/hygiene.sh` rejects any file git treats as binary unless the path is the first field of a line in `tests/roms/manifest.txt` (`scripts/hygiene.sh:63-69`); the existing 60 `tests/fuzz/corpus/*` seeds are all listed, five tab-separated fields (path, source, pin, licence, sha256), and `manifest.sha256` verifies the hash (`tests/CMakeLists.txt:1280-1288`). So: every `tests/fuzz/corpus_state/*` seed needs a manifest line, `--write-seeds` must also print those lines, and any regenerated seed changes the hash (regenerate the lines together). The history scan also fails for a binary that was ever committed without a manifest line (`hygiene.sh:176`), so never commit a seed under one name and rename it. Do not give a seed a name ending `.nes .fds .unf .unif .nsf .nsfe .sav .srm .state` (`ROM_NAME` at `hygiene.sh:40`). A state begins `89 4E 54 53`, not the iNES magic, so the content scan is not triggered, but the NUL bytes make it binary. Existing manifest lines pin a commit SHA in the third field; use the same `source`/`pin`/`MIT` form.

### Flag 7 - is `CI required` required with up-to-date branches?
**Answer.** `CI required` is the single required status check (integration id 15368), squash-only merges, linear history, no bypass actors. **Strict (up-to-date) enforcement is OFF**: `strict_required_status_checks_policy: false`, in `.github/rulesets/main.json` and in the live ruleset (`gh api repos/szTheory/nesturbator/rulesets/24400039`) [VERIFIED: both read this session]. Classic branch protection is not set (404). `CI required` depends on `[hygiene, title, build, asan, nofp, hash-equality, retroarch-e2e]` (`.github/workflows/ci.yml`, last job). Consequence for D-18: the tree guard is the guarantee, as CONTEXT says, and nothing stops a stale-branch merge. Before merging, update the branch from `main` if it moved, and rerun checks on the final head. Do not rely on up-to-date enforcement.

## Architecture Patterns

### System Architecture Diagram

```
 RetroArch / simulated host
   |  retro_serialize_size / retro_serialize / retro_unserialize
   v
 libretro/libretro.c  (static inst; no state of its own for these calls)
   |  nesturbator_state_size / save_state / load_state
   v
 src/state.c  (SIZE -> WRITE; or header checks -> VALIDATE -> APPLY; all-or-nothing)

 Proof paths
  libretro.host --serialize   -> real entry points, size/limit/reject table      (libretro.serialize)
  libretro.host --runahead    -> real entry points, N=1..3, oracle = in-process   (libretro.runahead)
                                 uninterrupted run; controls must FAIL
  test_state_runahead.c       -> library two-instance model (RetroArch 2nd inst)  (core.state_runahead)
  libretro.info_claims        -> parse .info, exact key set                       (CTest)
  tests/CMakeLists.txt guard  -> DEFER: claimed level => proving tests registered
  retroarch-e2e (macOS)       -> copy core to RA_DIR/cores, nc -u commands,
                                 screenshots -> frame identify -> assertions
  state_loader.c              -> libFuzzer (nightly) + replay of corpus_state (ci)
```

### Adapter (LR-01): exact mapping
```c
size_t retro_serialize_size(void) { return nesturbator_state_size(inst); }
bool retro_serialize(void *data, size_t len) {
    return nesturbator_save_state(inst, data, len) == NESTURBATOR_OK;
}
bool retro_unserialize(const void *data, size_t len) {
    return nesturbator_load_state(inst, data, len) == NESTURBATOR_OK;
}
```
This is the delegation the spike ran inside RetroArch (save, load, rewind, run-ahead all worked). Why it satisfies D-01..D-05 by construction, from `src/state.c` read this session: `nesturbator_state_size` returns 0 for NULL instance or no cartridge (`state.c:380-384`); save returns `ERR_ARGUMENT` for NULL buf and `ERR_STATE_CARTRIDGE` with no cartridge before touching the buffer (`state.c:414-420`); `capacity < size` gives `BUFFER_TOO_SMALL` and writes nothing; load needs `size == nesturbator_state_size` (`state.c:469`), so `size+1`, `2*size` and 0 are refused, and every check precedes the first write (`state.c:482-488` VALIDATE then APPLY). The test card and unload give size 0 because the cartridge is released (`nesturbator_unload_cartridge`). RetroArch passes exactly the saved length on load (`retro_unserialize len=4895` logged) [VERIFIED: local run]. `inst` can be NULL before `retro_init`; both library functions accept NULL.

Add the Phase-13 hardware-free comment style used in the file: cite `libretro.h` lines for `retro_serialize_size` (the stubs cite `L7707`). Do not call `RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS` (D-06, `libretro.h:1696`); do not query `GET_SAVESTATE_CONTEXT`.

A note on `GET_SAVESTATE_CONTEXT`: `libretro.h:5695-5707` says the `ROLLBACK_NETPLAY` context should omit unneeded data and store integers big-endian. The state format is little-endian by design (`include/nesturbator.h` layout comment). D-07 (adapter ignores the context) stands; rollback netplay is out of scope. Document in the README that the `deterministic` level is claimed for run-ahead and that netplay is not tested, so the `.info` comment "(netplay/runahead)" is not read as a netplay promise.

### Simulated host (libretro.host)
`tests/libretro/libretro_host.c` already resolves `p_serialize_size`/`p_serialize`/`p_unserialize` (lines 91-93), includes `../../src/internal.h`, `../check.h`, `../ines.h`, and defines `fnv_add` (line 688). `state_test.h` includes the same headers, so it is reusable by the host. The host's `env` handles only `SET_SUPPORT_NO_GAME` and `SET_PIXEL_FORMAT` (lines 166-176); add `GET_SAVESTATE_CONTEXT` (`72 | RETRO_ENVIRONMENT_EXPERIMENTAL`) and `GET_AUDIO_VIDEO_ENABLE` (`47 | RETRO_ENVIRONMENT_EXPERIMENTAL`), comparing against the macro values, not 72/47. Separate CTests share one executable by mode, as `--rom-parity` does. Add modes `--serialize`, `--runahead`, `--info-claims`.

### Frame identification for the RetroArch session
Runner `--hash-frame` is the SHA-256 of the native 16-bit buffer, while a screenshot is RGB: the two do not hash the same bytes (the runner printed `6d7bd1fa...` for frame 3, whereas the PPM payload hashes to `84c53527...`) [VERIFIED: local run]. So D-27's "provided both hash the same bytes" is answered: they do not. Options:
1. (Recommended) A small test tool `frame_index` linking the library and `nesturbator_host_convert`: it runs frames 1..W of the ROM once and prints every frame index whose converted RGB equals a given BMP/PNG's pixels. No dumps, no hashing library, no file blow-up. Output is a set (usually 1 index, 2 for adjacent duplicates).
2. `--dump-frame` of 1..W as PPM and compare each: works (the scratchpad did it) but 166 MB for W=900 and slow in CMake.

Set-valued assertions (because of nesteroids duplicates): treat each screenshot as a set `{i}` or `{i, i+1}`. A relation like "m0+5" holds if some pair satisfies it; to be exact, prefer sequences: after each `FRAMEADVANCE` take a screenshot and require the sets to chain (`next` contains `prev+1` for some choice). Windows of 3+ consecutive screenshots are unique except inside duplicate pairs. For stronger identity pick the start inside a window the runner shows has no duplicates within the next 8 frames, retrying `FRAMEADVANCE`/pause if not. Replace D-23's pairwise-distinct assertion with: "every frame in 1..W has a duplicate partner at most 3 frames away, and no hash recurs after a gap larger than 3" (checked from the runner at test time). Measured on nesturbator: 3637 distinct of 4000 frames; late-run duplicates (after frame 300) are still adjacent pairs.

### Deterministic lanes (no wall clock; recommended, additive to D-25)
Config in the session template: `savestate_auto_save`, `savestate_auto_load` (both already named in `test.cfg.in`, currently "false"), `run_ahead_enabled`, `run_ahead_frames`. Each lane uses `--max-frames=N --max-frames-ss --max-frames-ss-path=...` exactly as `ra_session` does today.

| Lane | Sessions | Observed screenshot |
|------|----------|---------------------|
| Baseline | `--max-frames=120` | runner frame 120 |
| Run-ahead depth k | `run_ahead_enabled=true`, `run_ahead_frames=k`, `--max-frames=120` | runner frame 120+k (k=1: 121, k=2: 122, k=3: 123) |
| Save/load round trip | S1: auto-save, `--max-frames=150`; S2: auto-load, `--max-frames=30` | S1: frame 150; state `game.state.auto` with `frame_number` 150 at core offset 32; S2: frame 179 = 150 + 30 - 1 |
| Load + run-ahead | S2 with `run_ahead_enabled=true` | frame 180 |

[All VERIFIED: local runs, repeated with two different frame counts for the round trip.] The "-1" in S2 is RetroArch's, deterministic, to be pinned with a comment. Compare each screenshot to `runner --dump-frame` of that exact frame number with the existing `compare_frame`; no identification needed. These lanes prove LR-03's save, load and run-ahead through RetroArch deterministically; the network lane then adds `SAVE_STATE`, `LOAD_STATE_SLOT`, `REWIND`, `RUNAHEAD_TOGGLE` as the roadmap's Success Criterion 3 requires.

### Launching RetroArch in the background from CMake
`execute_process` blocks. The script is already macOS-only (`sips`, PlistBuddy), so a small POSIX `sh` helper that starts RetroArch with `&`, polls for the `[NetCMD]` log line (0.25 s steps), runs the command sequence, and always kills the child on exit is the minimal route; the spike used exactly that. Keep the ordering `trap 'kill ...' EXIT`, and give every wait a deadline. Isolated HOME/XDG variables as in `ra_session`.

### Recommended network-lane sequence (replaces D-25 steps 2-8 where noted)
1. Boot with the core in `RA_DIR/cores`, no `--max-frames`, `rewind_enable = "true"`, `rewind_granularity = "1"`, run-ahead OFF in config. Wait for `[NetCMD]`; probe with `GET_CONFIG_PARAM savestate_directory` (`-w1`) which must reply.
2. Let it run about 3 s, `PAUSE_TOGGLE`, two screenshots 1 s apart must be identical (pause proof); identify as set M0.
3. `SAVE_STATE`; poll `states/**/game.state` to exist; assert container (3d).
4. 5 x (`FRAMEADVANCE`, 100 ms, screenshot): chained +1 each.
5. `LOAD_STATE_SLOT 0`; `FRAMEADVANCE`; screenshot must be M0+1; two more advances chain.
6. Rewind (3a): 40 x `REWIND` at 50 ms while paused; frame at most old-20; 3 chained advances.
7. Run-ahead: `RUNAHEAD_TOGGLE` (paused), `LOAD_STATE_SLOT 0`, `FRAMEADVANCE`, screenshot = M0+1+k with k = `run_ahead_frames` (the config value, pinned by the deterministic lane). Do NOT unpause.
8. `QUIT` while paused (run-ahead on). Assert exit within a deadline; exit code 0; the user-directory snapshot unchanged (existing check).
Total command time at `-w0` pacing is a few seconds, not minutes. The sequence never unpauses after step 7 begins.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| UDP command sender | a C sender | `printf %s CMD \| nc -u -w0 127.0.0.1 55355` | Owner preference; measured working. |
| Registered-test guard | parsing CTest files | `cmake_language(DEFER CALL guard)` + `if(TEST name)` | Verified in CMake 4.4.3; both exist since 3.19 and 3.4. |
| `.info` parser | a general INI parser | one loop over `key = "value"` lines in the host/test | The file is simple; negative self-test per D-19. |
| CRC-32 in the fuzz harness | linking `src/crc32.c` internals | a local copy (D-32) | Keeps the harness an independent oracle; also `nesturbator__crc32` is internal. |
| Run-ahead model for RetroArch's second instance | two dlopened adapters | library-level two-instance test (D-12) | Same property, no extra process state. |
| Seeds | hand-written binary files | `--write-seeds` of the replay build | Prevents drift (D-34). |
| Rewind/run-ahead loops | anything in the core | RetroArch's own | Out of scope. |

**Key insight:** every step that needs RetroArch's behaviour has a measured answer now; plan around the measurements, not around RetroArch's source (clean room).

## Common Pitfalls

### Pitfall 1: `.info` not applied because the core is loaded by absolute path
**What goes wrong:** the `.info` claim is edited, RetroArch behaves identically, and the claim is "proved" by nothing. **Why:** capability lookup keys on the core file in `libretro_directory`. **Avoid:** copy to `RA_DIR/cores`, `-L` that copy, and assert the observable effect of the claimed level (deterministic lane). **Warning sign:** `savestate = "false"` still lets rewind run.

### Pitfall 2: `LOAD_STATE` while paused
Silently ignored. Use `LOAD_STATE_SLOT 0`; verify with the next `FRAMEADVANCE`, not the screenshot taken immediately after the load.

### Pitfall 3: unpausing with run-ahead active
Commands are dropped, including `QUIT`; the process runs until killed. Never unpause after `RUNAHEAD_TOGGLE`; add a deadline and kill; do not enable run-ahead in the network session's config.

### Pitfall 4: counting commands
Commands are flags, not a queue. Pace, then confirm with a screenshot; never assert "N commands = N frames".

### Pitfall 5: duplicate frames
Adjacent nesteroids frames can be identical; identity by a single hash is ambiguous by one. Use sets and chains.

### Pitfall 6: `nc -w1` latency
1 s per command. Use `-w0` except for replying commands.

### Pitfall 7: log matching
Block-buffered file output, `[mvk-warn] VK_ERROR_...` noise, and the benign audio-driver `[ERROR]` line. Anchor patterns; read logs after a clean exit.

### Pitfall 8: manifest and history for fuzz seeds
Binary seeds need manifest lines with exact SHA-256; regenerating a seed changes its hash; a seed ever committed unlisted fails the history scan. Generate seeds and manifest lines together and commit once.

### Pitfall 9: the Phase 12 lessons
GCC 14 `-Werror`, clang-format 18.1.3 vs 18.1.8, nofp float scan (from CONTEXT D-22). Run the `hygiene` preset before each push; compile changed C with `gcc:14` in Docker.

### Pitfall 10: stale build artifacts
`build/dev/libnesturbator.a` in the working tree predates the state API (the link failed with undefined `nesturbator_save_state`). Rebuild before trusting any local binary.

## Code Examples

### D-18 guard skeleton (verified to run under CMake 4.4.3)
```cmake
# tests/CMakeLists.txt, near the end. One table of claim -> proving tests.
function(nesturbator_check_info_claims)
  file(STRINGS "${PROJECT_SOURCE_DIR}/libretro/nesturbator_libretro.info" lines)
  set(level "none")
  foreach(l IN LISTS lines)
    if(l MATCHES "^savestate_features = \"(serialized|deterministic)\"$")
      set(level "${CMAKE_MATCH_1}")
    endif()
  endforeach()
  set(need_serialized libretro.serialize)
  set(need_deterministic libretro.serialize libretro.runahead core.state_runahead)
  if(level STREQUAL "serialized" OR level STREQUAL "deterministic")
    foreach(t IN LISTS need_${level})
      if(NOT TEST ${t})
        message(FATAL_ERROR ".info claims ${level} but test ${t} is not registered")
      endif()
    endforeach()
  endif()
endfunction()
cmake_language(DEFER CALL nesturbator_check_info_claims)
```
[VERIFIED pattern: scratchpad run printed "late.test registered" for a test added after the DEFER call and "missing.test NOT registered".] Note a `deterministic` level also needs the `serialized` list to be satisfied (the table already includes it).

### Session command helper (shell form used in the spike)
```sh
cmd() { printf %s "$1" | nc -u -w0 127.0.0.1 55355; }          # fire and forget
probe() { printf %s 'GET_CONFIG_PARAM savestate_directory' | nc -u -w1 127.0.0.1 55355; }
# RetroArch launched in the background with the same HOME/XDG isolation as ra_session:
# RetroArch -c "$RA_DIR/test.cfg" -L "$RA_DIR/cores/nesturbator_libretro.dylib" "$content" &
```

### Config keys for the network session (all appended to a state-session template)
```
network_cmd_enable = "true"
network_cmd_port = "55355"
user_language = "0"
rewind_enable = "true"
rewind_granularity = "1"
run_ahead_enabled = "false"       # toggled by RUNAHEAD_TOGGLE while paused
run_ahead_frames = "1"
run_ahead_secondary_instance = "false"
savestate_file_compression = "false"
quit_press_twice = "false"
savestate_thumbnail_enable = "false"
```
Outcome checks that confirm the otherwise-unverifiable keys (RetroArch ignores unknown keys): `savestate_file_compression` - core magic at file offset 16; `quit_press_twice` - one `QUIT` exits; `rewind_granularity` - about one step back per `REWIND`; `savestate_thumbnail_enable` - file size within S+32; `run_ahead_secondary_instance` - no `$TMPDIR` writes (the user-dir snapshot is kept). `rewind_buffer_size` is unnecessary (default 20 MB buffer initialised). [Keys verified to take effect: each outcome above was observed.]

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Frontend ignored `.info` savestate level | RetroArch gates rewind and run-ahead on it (when the core is in the cores dir) | prep doc cites 1.10.1; measured on 1.22.2 here | Claims are meaningful and testable. |
| Core uses quirk flags | None; no `SET_SERIALIZATION_QUIRKS` | D-06 | Level stays the whole signal. |

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | The hosted macos-15 runner behaves like this machine (core-in-cores gating, command-drop under run-ahead, per-core `states/nesturbator/` subdir, `-w0` delivery). | Flags 1, 3 | The hosted e2e could pass locally and fail in CI; first push of the e2e step is the check. Keep bounds loose and read the artifacts. |
| A2 | RetroArch's single-instance order and which frame's audio is kept beyond what docs state. | Flag 2 | Resolved by asserting both audio mappings in the host. |
| A3 | Exact REWIND step rate (-36 for 40 commands at 50 ms) holds on CI. | Flag 3a | Use the loose bound "at most old-20". |
| A4 | The `.info` allowlist for D-19 is a design choice built from the libretro-core-info example. | Flag 1 | Low; extend the list if a legitimate key is added later. |
| A5 | The "no canonicalisation" result is empirical for mutated non-cross unsigned fields. | Flag 4 | An assert would trip in the nightly; downgrade the oracle to ignore the named bytes then. |
| A6 | Linux libFuzzer build of the new target mirrors `rom-loader-fuzz`. | LR-04 | Not run here (Apple clang has no libFuzzer; Docker has `gcc:14` only). Verify in a `ubuntu:24.04` container with `apt install clang` before the first push, or accept the first nightly as the check. |

## Open Questions

1. **Does the hosted runner reproduce the paused-load / run-ahead command-drop behaviour?** Known: same RetroArch binary. Unknown: runner display timing. Recommendation: the first PR run of the new step is the check; upload screenshots, state file and verbose log on failure (D-29).
2. **Which audio stream should the production run-ahead host treat as visible?** Recommendation: assert both (flag 2).
3. **Fuzz `-timeout`:** D-36 says 5; the sibling job uses 2. Recommendation: 5 (instance creation plus 2 frames costs about 15 ms worst measured).

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| RetroArch app | e2e spike and local verification | yes | 1.22.2 (Git 69a4f0ea) | none needed |
| `/usr/bin/nc` | UDP commands | yes | macOS BSD nc (`-w0`, `-u`) | none |
| `sips` | existing PNG to BMP step | yes | system | none |
| CMake / Ninja | all tests | yes | 4.4.3 / present | none |
| Docker | GCC 14 compile, Linux fuzz check | yes | 29.5.2 | none |
| clang with libFuzzer (local macOS) | local fuzz run | no (Apple clang lacks the runtime) | system clang | run inside an `ubuntu:24.04` container after `apt install clang` |
| `gh` | ruleset read | yes (authenticated) | — | none |

**Missing dependencies with no fallback:** none.

## Validation Architecture

Skipped: `workflow.nyquist_validation` is `false` in `.planning/config.json`. The project's own verification is the CTest list in CONTEXT D-17 and D-35.

## Security Domain

Skipped: `workflow.security_enforcement` is `false`. For awareness: the state loader consumes hostile bytes; its protection is the all-or-nothing validation in `src/state.c` plus the new fuzz target.

## Sources

### Primary (HIGH confidence)
- Local runs of RetroArch 1.22.2 (Git 69a4f0ea) against a throwaway adapter copy; spike scripts and logs are in the session scratchpad, not the repository.
- `src/state.c`, `src/state.h`, `src/cartridge.c`, `include/nesturbator.h:56,435-501`, `libretro/libretro.h:1696,1767,3226-3265,5649-5714`, `libretro/libretro.c`, `tests/CMakeLists.txt`, `tests/retroarch/*`, `tests/fuzz/rom_loader.c`, `.github/workflows/ci.yml`, `.github/workflows/nightly.yml`, `.github/rulesets/main.json`, `scripts/hygiene.sh`, `tests/roms/manifest.txt` (all read this session).
- Live ruleset via `gh api repos/szTheory/nesturbator/rulesets/24400039`.

### Secondary (MEDIUM confidence)
- docs.libretro.com/guides/runahead/ (single- and two-instance order).
- docs.libretro.com/development/retroarch/network-control-interface/ (command list; `LOAD_STATE_SLOT`, `RUNAHEAD_TOGGLE`, `REWIND`, `FRAMEADVANCE` "while paused"; UDP only; no ordering statement).
- raw.githubusercontent.com/libretro/libretro-core-info/master/00_example_libretro.info (capability keys and the savestate_features comment).
- raw.githubusercontent.com/libretro/libretro-super/master/dist/info/gpsp_libretro.info (an info file with `deterministic`).

### Tertiary (LOW confidence)
- Forum and wiki results about run-ahead needing deterministic save states; consistent with the measurements and not relied on.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH - no new dependencies; versions read from the repo and machine.
- Architecture (adapter, host, guard): HIGH - library contract read in full; guard verified.
- RetroArch behaviour: HIGH on this machine, MEDIUM for the hosted runner (A1).
- Fuzz (flags 4-6): MEDIUM-HIGH - empirical plus code reading.
- Pitfalls: HIGH - each observed.

**Research date:** 2026-10-12
**Valid until:** 2026-11-12 (RetroArch is pinned at 1.22.2; repo facts change with commits)
