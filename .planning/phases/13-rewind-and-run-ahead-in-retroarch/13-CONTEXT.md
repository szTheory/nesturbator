# Phase 13: Rewind and run-ahead in RetroArch - Context

**Gathered:** 2026-10-12
**Status:** Ready for planning

<domain>
## Phase Boundary

The libretro adapter exposes the Phase 12 library state through `retro_serialize_size`,
`retro_serialize` and `retro_unserialize`, so players can save and load states, rewind and use
run-ahead in RetroArch. The `.info` file claims `serialized` and then `deterministic`, each
backed by a test in the same tree. A hosted `retroarch-e2e` step proves save/load, rewind and
run-ahead in RetroArch 1.22.2. A nightly libFuzzer target feeds the state loader, and its
corpus is replayed in `ci`. Requirements LR-01..LR-04.

Not in this phase: rewind, run-ahead or netplay loops inside the core; serialization quirk
flags; buildbot submission; state migration or compression; memory descriptors.

</domain>

<decisions>
## Implementation Decisions

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
  (Amended at planning, 2026-10-12: M1_P128K_C32K_W8K has no battery bit; M1_P128K_C128K_S8K
  is used, and its 8192-byte save span is asserted non-NULL before the loop.)

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

  (Note at planning, 2026-10-12: the README says the `.info` claims `deterministic`, which
  RetroArch uses for run-ahead and netplay; run-ahead is proven by the simulated host and the
  RetroArch e2e; the state format is fixed little-endian and byte-identical on all six
  platforms; netplay itself has no end-to-end test here. LR-02's "(run-ahead and netplay)"
  describes what the level enables and is unchanged.)
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

  (Amended at planning, 2026-10-12: the adapter requests no log interface, so a core
  `RETRO_LOG_ERROR` line cannot appear. "No core error" is enforced instead by failing every
  state lane on any line-start `[ERROR]` line in the RetroArch log, and on any
  `[WARN]`/`[ERROR]` line tagged `[Rewind]` or `[Run-Ahead]`, unless the line matches an
  explicit allowlist committed in `tests/retroarch/`. Each allowlist entry is measured on
  RetroArch 1.22.2 and carries a comment saying why it is benign. The failed-save and
  failed-load strings and the `[NetCMD]` requirement stay. A CTest feeds the matcher a fixture
  log with an un-allowlisted `[ERROR]` line and requires the check to fail.)
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

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Requirements and scope
- `.planning/ROADMAP.md`: Phase 13 entry, its success criteria and its note on claim order.
- `.planning/REQUIREMENTS.md`: LR-01..LR-04 and the Out of Scope table (no quirks, no in-core
  loops, no buildbot).
- `.planning/PROJECT.md`: constraints, and the Key Decisions on the state format and the 2^62
  ticks cap.

### libretro rules and earlier research
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` §2 and §5: serialization rules, quirk bits,
  `.info` levels.
- `.planning/research/FEATURES.md`, `.planning/research/ARCHITECTURE.md` (section 7.5,
  run-ahead), `.planning/research/PITFALLS.md`: v3 research.
- `libretro/libretro.h`: the vendored MIT header. Use it for the `GET_SAVESTATE_CONTEXT`,
  `GET_AUDIO_VIDEO_ENABLE` and `SET_SERIALIZATION_QUIRKS` constants.
- docs.libretro.com: the network control interface and run-ahead pages. The rewind page
  returns 404.

### The Phase 12 state contract
- `include/nesturbator.h` (around line 435-501): `nesturbator_state_size`, `save_state`,
  `load_state` and their documented behaviour.
- `src/state.c`: header layout, validation order, CRC and ticks cap.
- `.planning/phases/12-save-states-in-the-library/12-CONTEXT.md` and `12-VERIFICATION.md`.

### Clean room
- `CLAUDE.md` rule 4. RetroArch is GPL: never open its source (`command.c`, `runahead.c`
  and so on). Use only its docs, the cfg template for key names, and observed binary
  behaviour.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `libretro/libretro.c`: the stubs for `retro_serialize_size`, `retro_serialize` and
  `retro_unserialize` (around line 243) are replaced. The adapter keeps static
  `inst`/`video`/`xrgb`/`mono`/`stereo`/`save_data`. Confirm no carried-over cursor or pending
  flag feeds the next frame.
- `tests/libretro/libretro_host.c` (1520 lines): the C frontend test with the `fnv_add`
  hashing, environment and callback plumbing and the parity tests. It gains the serialize
  and run-ahead modes.
- `tests/state_test.h`, `tests/core/test_state_fork.c`, `tests/core/test_state_reject.c`: the
  Phase 12 fork and stress images, the CRC-recompute helper and the reject-table pattern.
- `tests/retroarch/run_retroarch.cmake`, `test.cfg.in`, `test_compare_frame.c`: isolated HOME,
  the user-directory snapshot, the `sips` → BMP conversion and the pixel compare.
- `tests/fuzz/rom_loader.c`, `tests/fuzz/corpus/`, the `fuzz.regress`/`fuzz.registration`
  CTests, `tests/cmake/fuzz_registration.cmake`, the `rom-loader-fuzz` nightly job.
- The runner's `--hash-frame`, `--dump-frame`, `--save-state` and `--load-state`.

### Established Patterns
- Hardware and contract comments cite their sources. The README and the public header
  change in the same commit as the behaviour (rule 6).
- Outcome assertions instead of trusting RetroArch keys, because RetroArch ignores unknown
  keys. The `.srm` precedent is in `test.cfg.in`.
- Tests are table-driven. Hashes are FNV-1a for in-test comparison and pinned keys only for
  six-platform hashes.
- On this macOS host, long ctest runs must be in the foreground: background shells are
  throttled.

### Integration Points
- `tests/CMakeLists.txt` (`libretro.host` around line 1027, the fuzz tests around line 725),
  `.github/workflows/ci.yml` (the `retroarch-e2e` job around line 178),
  `.github/workflows/nightly.yml` (`rom-loader-fuzz` around line 192 and the aggregation job
  around line 267), `libretro/nesturbator_libretro.info`, `README.md`.

</code_context>

<specifics>
## Specific Ideas

- Owner preference: "another copy and paste is better than another dep". Use `nc`, not a
  package; copy the nightly job; keep CRC-32 local to the harness.
- The research probe of RetroArch 1.22.2 (on this machine, current core) observed:
  - **Replies:** only `VERSION`, `GET_CONFIG_PARAM` and `READ_CORE_*` answer; the other
    commands give no reply.
  - **Locale:** the log follows the machine locale unless `user_language = "0"`.
  - **Quit:** a single `QUIT` did not exit.
  - **Log lines:**
    - `[NetCMD] bringing_up_command_interface_at_port 55355`
    - `Run-Ahead has been disabled because this core does not support save states`
    - `Failed to initialize rewind buffer`

    The last two are expected until serialize works.
  - **Probe files:** the scratchpad, not the repository.

</specifics>

<deferred>
## Deferred Ideas

- Running two dlopened copies of the adapter for a second-instance run-ahead test. Do it only
  if the adapter gains state it keeps per instance.
- RetroArch e2e variant with `run_ahead_secondary_instance = "true"` (copies the core to a temp
  dir; needs a `$TMPDIR` snapshot).
- Fuzz seeds from real test-ROM states, if nightly coverage of mapper paths is low.
- A longer fuzz frame mode (10+ frames) for counter-wrap bugs.
- A cross-platform fuzz replay digest.
- Run-ahead cost row in the speed report (Phase 15).

</deferred>

## Research flags for the phase researcher
1. Confirm the `savestate_features` value set and spelling (`basic`/`serialized`/`deterministic`)
   and how RetroArch 1.22.2 gates rewind and run-ahead on it (docs.libretro.com).
2. Confirm RetroArch's single-instance run-ahead order and which frame's audio and video it
   keeps (from the docs). D-10 makes the order one named constant in the host.
3. Spike in RetroArch, once serialize works:
   - REWIND stepping and the burst size.
   - Whether LOAD_STATE redraws while paused, and the frame offset after it.
   - The run-ahead display offset.
   - The state file container or compression and its size bound.
   - Whether datagrams are processed in FIFO order (the fence relies on it).
4. Whether load canonicalises masked bits (decides the `save(load(S)) == S` oracle in D-33).
5. Whether any field other than ticks can make one frame run long (fuzz hang bound).
6. Maximum state size against `-max_len`. Whether the hygiene manifest covers `corpus_state/`.
7. That `CI required` is a required check with up-to-date branches enforced (merge gate for D-18).

---

*Phase: 13-rewind-and-run-ahead-in-retroarch*
