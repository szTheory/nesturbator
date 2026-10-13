# Feature Landscape: v3 Save states and rewind

**Domain:** Save states, rewind, run-ahead and netplay for a libretro NES core, plus the process contract of a launcher-driven headless runner and a speed report
**Researched:** 2026-10-11
**Overall confidence:** MEDIUM-HIGH. The libretro rules and the user complaints come from the project's own sourced preparation files (LIBRETRO-AND-RUNNER sections 2 and 5, NES-ECOSYSTEM sections 4 and 5, SEED-002, SEED-004). RetroArch feature behaviour (rewind per-frame serialization, run-ahead second instance, netplay state hashing) is documented behaviour, not re-verified against a live build this session. No GPL or LGPL emulator source was opened.

Scope: only what the new features need. Mappers 0/1/2/3/4/7, battery saves, movie replay, hash pins and the adapter already exist.

## How the hosts use a state (what drives every requirement)

| Host feature | What it does with the core | Hard requirement on the core |
|--------------|---------------------------|------------------------------|
| Save/load state (slot, auto) | `retro_serialize_size`, `retro_serialize`, `retro_unserialize` between `retro_run` calls | Size known after load; buffer supplied by caller; load rejects bad data without side effects |
| Rewind | Serializes every frame (or every N) into a ring buffer, restores backwards; compresses/diffs states | Declared `serialized`; small and fast state; stable size so the ring can be preallocated |
| Run-ahead (single instance) | Each frame: save, run N frames with audio/video muted, load back, then run one real frame | Declared `deterministic`; serialize+unserialize+run costs well under a frame; load must be exact (no warm-up frames) |
| Run-ahead (second instance) | Second copy of the core runs ahead and receives state each frame | State is self-contained: no pointers, no globals, no dependence on what instance wrote it |
| Netplay (rollback) | Peers compare core version and per-frame state hashes, roll back with states | Bytes equal across platforms and architectures; fixed byte order; no uninitialized padding |
| Achievements hardcore | Often forbids loading states | Nothing for the core to do |

## Table Stakes

Missing any of these = RetroArch disables the feature, or users lose saves.

| Feature | Why Expected | Complexity | Notes / dependency on existing |
|---------|--------------|------------|-------------------------------|
| Library API: `state_size`, `state_save(buf, len)`, `state_load(buf, len)` on the opaque instance | The libretro calls map onto it one for one; Playstead and a future shared host use the same ABI | Med | Extends the public header; README and header comments updated in the same change (project rule 6). Declaration-baseline guard needs a refresh |
| Fixed size for the life of a loaded cartridge, and identical for all options | "State size changes with a sound option" is a recorded FCEUmm complaint; libretro forbids growth between load and unload | Med | There are no core options today, so keep it so. Size may differ per board (MMC1 vs MMC3 registers) only if it is fixed once the cartridge is loaded; the simpler and safer choice is one size across all six boards (union of board registers padded) so the ring buffer and netplay never see a size depend on content. Decide in the architecture phase |
| Own format version and header (magic, format version, ROM/board fingerprint, payload length, checksum) | Cross-version breakage is the loudest user complaint ("known working states from 2016 not loading", puNES, Nestopia 2018). The version is separate from the library version (SEED-002 note) | Low-Med | Fingerprint ties a state to the loaded cartridge so loading a state from another game is refused, not corrupting |
| Validate in full before applying (all-or-nothing load) | A failed load that half-applies state crashes or desyncs the running game | Med | Two-phase: parse and range-check every field into a scratch/temporary, then commit. Reuses the loader's validation habits; every pointer-like index (page tables, nametable map, audio ring cursors) must be bounds-checked |
| Complete machine state in the blob (see state inventory below) | Anything missing shows up as an audio click, a hash change or a desync after load | High | The bulk of the work and the main risk. Proof: save at frame N, load into a fresh instance, run M frames, hashes equal an uninterrupted run |
| Pointer-free canonical layout, explicit little-endian field encoding, no struct memcpy, zeroed padding | Equal bytes on six platforms (netplay compares state hashes); already the project's audio-serialization precedent | Med | Per-module `save/load` functions writing fixed-width fields. Reuse the Phase 4 explicit little-endian helpers |
| Determinism round trip tests | Run-ahead and netplay depend on it; project rule: every behaviour lands with a test | Med | Tests: save-load-continue equals straight run for each board and for every ROM in the hash list; state bytes at frame N pinned by hash on six platforms; save twice without running gives identical bytes; load into a different instance gives identical bytes next save |
| libretro: `retro_serialize_size/serialize/unserialize` | Required for any state feature | Low | Thin over the library API. Keep `retro_run` as the only place frames advance |
| `.info` file: `savestate = "true"`, `savestate_features = "deterministic"`; no serialization quirks set | RetroArch disables rewind below "serialized" and run-ahead/netplay below "deterministic" since 1.10.1 | Low | Existing info file exists; test that the file says it (grep-style CTest) and a hosted retroarch-e2e step proves the feature works, not only that it is declared |
| States taken only at frame boundaries (between `retro_run` calls) | The libretro contract; mid-frame states are impossible for hosts to request | Low | The blob still contains mid-scanline CPU/PPU/APU position, because a frame ends mid-instruction-stream. See mid-frame section |
| Load rejects garbage safely (wrong size, bad magic, wrong version, wrong ROM, truncated, corrupt fields) with distinct error codes | Hosts show "load failed"; users feed old/foreign files | Med | Distinct codes help the runner exit statuses. Malformed-input fuzzing is deferred to a later milestone (SEED-004), but table-driven reject tests for each field class are needed now |
| Battery RAM inside the state, with write-generation handled on load | Loading a state must restore PRG RAM the game saw; the host's `.srm` write is driven by the generation counter | Med | Load must bump the generation counter so the host writes the restored RAM to disk (otherwise the file and memory disagree). Needs a decision: include battery RAM in state (what users expect and what RetroArch rewinds) - recommended yes |
| Runner: `--save-state FILE` / `--load-state FILE` (and at-frame variants) with movie replay | Playstead needs to resume and the replay check needs a known mid-run position | Med | Builds on the existing movie loader and `--frames`; writes by temp+fsync+rename like saves. Exit status for state errors is distinct |
| Runner: flush battery save on SIGTERM/SIGINT, then exit 0 inside 2 s | Playstead sends SIGTERM and SIGKILL after 2 s; its mGBA pin loses up to 24 s of progress today | Med | Signal handler only sets a flag (async-signal-safe); the main loop flushes between frames. Reuses the existing atomic save write. A failed flush must not leave a stale `.tmp` (v2 debt 08 IN-02) |
| Runner: documented exit statuses | Playstead reads 0 as clean; its checker uses 0/77/1 | Low | Table in README and `--help`: 0 done, 1 failure, 2 usage, 3 hash mismatch, 77 unsupported capability, plus a status for state errors if needed. A test per status, run as a child process |
| Runner: SIGUSR1 flush on demand | In the proposed contract; cheap once the flush path is signal-safe | Low | Not available on Windows; document as POSIX only |
| Speed: runner mode printing frames per second and frame-time percentiles (p50, p95, p99, max) | Rewind and run-ahead multiply per-frame cost; accuracy cores publish 250-400 fps | Med | Wall-clock output is informational and never asserted in CI (shared runners are noisy). Excludes hashing/IO from the timed loop; prints ROM, frames, build type |
| Per-PR instruction-count comparison (head vs merge-base) | Wall time is too noisy; instruction counts are stable | Med-High | Linux job with Cachegrind or equivalent; fails above a stated threshold or reports in the PR. Extends CI; needs a fixed ROM and frame count |

## State inventory (what must be in the blob)

Missing any of this is the most common source of "state loads but game glitches / hashes differ".

| Area | Must include | Easy to miss |
|------|--------------|--------------|
| CPU | A, X, Y, S, P, PC; instruction/micro-step position (a frame can end mid-instruction); pending interrupt lines and edge detectors (NMI edge, IRQ level and delay latches); DMA state (OAM DMA step/phase and offset, DMC DMA stall/alignment); RAM 2 KiB; open-bus latch; cycle counters | NMI edge-detect bit (the v2 `$2000` NMI fix made this matter); halted/DMA phase; half-cycle parity |
| Time | Master/half-clock phase and any "catch-up" cursors between CPU, PPU, APU | The PPU catch-up point at the M2 edges |
| PPU | All registers, `v`/`t`/`x`/`w`, VRAM 2 KiB, palette, OAM and secondary OAM plus the OAM address/counter and diagonal-overflow state, shift registers and latches, fetch pipeline temporaries, dot/scanline/frame parity, vblank/NMI flags, `$2007` read buffer, open-bus decay latch, emphasis, the A12 state and its filter counter | Sprite evaluation latches; odd-frame skip parity; the frame buffer itself (see below); decay counters |
| APU | Pulse, triangle, noise, DMC channel state, length/envelope/sweep units, frame counter phase and its delayed-write latch, DMC sample address/bytes remaining/shift register, IRQ flags | Frame-counter reset delay after `$4017` write; DMC buffer-empty vs shifting state |
| Audio synthesis | Fixed-point synth history, pending transition list, band-limit/filter history, output sample cursor, DC filter state, any queued PCM not yet returned to the host | The project's bounded per-instance transition history. If omitted, audio after a load differs from an uninterrupted run and run-ahead produces clicks every frame |
| Controllers | Latch/strobe state, shift register positions for both ports, the held buttons as seen by the core | Strobe edge; a host that loads a state and polls input immediately |
| Mapper / board | Registers, shift registers (MMC1 serial write count and the consecutive-write guard), derived page tables rebuilt from registers rather than saved (or saved and validated), MMC3 IRQ counter/latch/reload/enable and A12 filter, mirroring, bus-conflict mode is header-derived (not state) | Derived pages: save registers only and recompute, so a corrupt state cannot produce a wild page index. CHR RAM and PRG RAM content |
| Memory | Work RAM (PRG RAM), battery RAM span, CHR RAM, nametable RAM | Battery RAM generation counter handling on load |
| Not in state | ROM bytes (copy lives with the cartridge), palette lookup, config, host callbacks, ROM-derived tables | The state must not contain ROM, partly for size and partly for legal hygiene (no ROM bytes in the tree or in shared state files) |
| Frame buffer | Recommended: not saved if the visible frame is regenerated by the next `retro_run`; for run-ahead and rewind the screen after load is the next frame anyway | A host showing the screen right after load (some do a frame refresh) would show stale pixels; decide and test, because it changes whether load alone yields hash equality |

## Mid-frame vs frame-boundary

| Question | Answer |
|----------|--------|
| Which boundary does libretro use? | Between `retro_run` calls only. A state is requested by the host when no frame is in progress |
| So is mid-frame state needed? | No host-visible mid-frame save point. But "frame boundary" in this core is the point where video/audio for the frame was emitted, which is not a clean CPU instruction boundary or a scanline boundary. The blob must therefore carry mid-instruction CPU position and PPU dot position. Do not add a frame-boundary-only shortcut that assumes the CPU is between instructions |
| Should the library API offer save at arbitrary cycles? | No (anti-feature below). Provide save only between `step_frame`-style calls, and say so in the header |
| Runner states | Frame-boundary only: `--save-state-at-frame N` is exact because the runner stops after whole frames |

## Differentiators

Not expected, valued by the hosts and builders who adopt a core.

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| One state size for all boards, pinned and stated in the README | Rewind ring buffers and netplay never reallocate; a "state size never changes" promise is exactly the complaint other cores earn | Low-Med | Cheap if decided up front; costs a few hundred bytes of padding |
| State format version with a documented compatibility policy and migration table | The top-ranked user grievance in the research. Policy options: (a) never break within a major format version, (b) when a format changes, the loader accepts at least the previous version and converts | High | Recommended: ship format v1 with a header that allows later versions; promise the loader refuses cleanly (distinct error), and add a committed v1 golden state file loaded by every future build (a regression test that fails when someone silently changes layout). A migrator is deferred until a v2 exists |
| Golden state file test across six platforms | Proves the byte-equal claim netplay relies on, and catches layout drift in a diff | Low-Med | The state of a pinned ROM at a pinned frame hashed on six platforms. Test ROM bytes only from the manifest |
| Cheap serialize path for run-ahead (no allocation, no copy of ROM, minimal bytes) | Run-ahead calls it every frame; "clean and fast enough" is the libretro guide's bar | Med | Benchmark in the speed report: save+load cost per call, shown as a share of frame time |
| Speed report includes save/load microbench and run-ahead-equivalent cost | Shows the numbers hosts care about (frames/sec with 1 frame run-ahead) | Low-Med | Same runner mode, extra rows |
| Movie plus state: start a movie from a state ("anchor state"), and state embedded in a movie for resume | Playstead-style resumable replay; shortens long regression runs | Med-High | See state-in-movie below. Recommended as a runner option (`--movie` with `--load-state`), not a movie format change |
| Compressed or delta states in the runner or library | Smaller files; smaller rewind memory | High | Anti-feature at the core: size stability and speed matter more. Compression is a host job (RetroArch compresses rewind itself) |
| Capabilities JSON exposes state format version and fingerprint | Playstead preflight can know whether a stored state is usable before launch | Low | Already proposed in the preparation file (`--capabilities`); wire the real format version in. Writes nothing |
| `--expect`-style state hash (`--hash-state N`) | Lets CI pin state bytes by frame; supports golden tests | Low | Reuses SHA-256 helper already in the runner |
| Instruction-count history per PR comment | Review sees speed drift as a number | Med | Optional on top of the required comparison |
| Memory descriptors for achievements | Already listed in preparation; touches the state only in that pointers must remain stable across loads | Low-Med | Out of core v3 theme unless cheap: state load must write into the same RAM buffers (no reallocation), which also keeps descriptors valid |

## Anti-Features

| Anti-Feature | Why Avoid | What to Do Instead |
|--------------|-----------|-------------------|
| State size that varies with options, board, or content after load | Breaks rewind rings, netplay, and is a recorded complaint in other cores | One fixed size, asserted by a test |
| Raw `memcpy` of internal structs into the state | Pointers, padding and endianness differ across platforms; leaks uninitialized bytes; breaks netplay hashes | Field-by-field canonical little-endian writers |
| Saving ROM bytes, derived tables or host pointers in the state | Bloat, legal hygiene, pointer bugs | Registers only; rebuild derived pages from registers and validate |
| Partial load ("best effort") of a bad state | Half-applied machine is undebuggable and unsafe | Validate everything, then commit; on failure the instance is untouched |
| Setting serialization quirks to excuse gaps (incomplete, needs warm-up, single session, platform-dependent) | Each quirk disables run-ahead or netplay for everyone | Fix the state instead; leave quirks at zero |
| Mid-instruction or arbitrary-cycle save API | No host asks for it; large test surface | Frame-boundary only, documented |
| In-core rewind buffer, run-ahead loop or netplay code | Hosts own them; the core ships the primitives. Duplication causes double rewind | Declare `serialized`/`deterministic` and stay out of it |
| State compression or encryption inside the core | Costs time on the hot path and adds format surface | Hosts compress; core stays raw and fixed |
| Automatic state migration across arbitrary old formats before any second format exists | Speculative, untestable | Version field plus a clean refusal; add migration with the first real format change |
| States as the durable save medium (autosave state replacing `.sav`) | Breaks Playstead's contract that watches `{saveDir}/{rom}.sav`, and states break across versions | Battery saves stay raw `.sav`; states are a convenience |
| Options that alter emulation (overclock, sprite limit) added in this milestone | Each one multiplies state and replay compatibility | None; keep the option count at zero |
| Wall-clock thresholds asserted in CI | Shared runners give 2x noise; produces flaky red builds (the project already fights flake) | Wall time printed only; gating uses instruction counts |
| Speed-for-hash trade (faster PPU that moves a hash) | Violates the determinism core value | Any speed change must leave every hash identical |
| Signal handlers doing file IO directly | Not async-signal-safe; torn saves | Set a flag; flush from the main loop |
| Catching crash signals (SIGSEGV) to flush | Hides bugs, can write a corrupt save | Leave crash signals unhandled; the periodic flush bounds loss |

## State in movie (decision)

| Option | Verdict |
|--------|---------|
| Movie format with an embedded start state | Do not do now. Couples two formats and puts versioned binary inside text FM2-style lines |
| Runner combines `--load-state FILE` with `--movie FILE` (movie starts at the state's frame) | Recommended differentiator: small, composes existing parts, enables resumable replay. Needs the movie frame counter to be explicit, and the state fingerprint to be checked |
| Movie records a state hash checkpoint every N frames | Optional later; useful for desync detection and mirrors netplay's per-frame hash check. `--hash-state N` supports it |
| Power-on vs reset-anchored movies | Already handled by existing movie support; a state-anchored movie is a third start type and needs a named header field |

## Launcher (Playstead) process expectations

Taken from the preparation file's account of Playstead's current behaviour; consistent with common launcher practice.

| Expectation | Detail | Complexity |
|-------------|--------|------------|
| Arguments from a template: ROM path, save directory | Already supported (`--save-dir`) | Done |
| Watches `{saveDir}/{rom}.sav` | Save file must be raw bytes, atomically replaced (rename) so a watcher never sees a torn file | Done in v2; keep |
| SIGTERM then SIGKILL after 2 s | Flush and exit within 2 s of SIGTERM, with exit 0; SIGINT the same | Med |
| Status 0 clean | A signalled shutdown counts as clean because the launcher sent the signal | Decision recorded in preparation: keep exit 0 |
| Failed ROM, bad movie, bad state | Exit 1 (failure) or a documented distinct status; message on stderr, nothing on stdout that scripts parse | Low |
| Usage error | Exit 2 | Low |
| Hash mismatch | Exit 3, from `--expect` | Done |
| Unsupported capability | Exit 77 (matches the launcher's continuation checker) | Low |
| No network, writes in one directory only | No writes outside `--save-dir` (state files included: default them into the save dir or require an explicit path) | Low; add a test that runs with a read-only cwd |
| Preflight JSON without side effects | `--capabilities` | Low |
| Deterministic replay | Same movie, same hashes on all platforms | Done |
| Periodic flush interval | `--save-interval` frames, and flush when generation changed | Done; verify under the signal path |
| Windows | SIGTERM/SIGUSR1 do not exist the same way; document Ctrl+Break/console control handler, or declare POSIX-only for the signal contract and test it on Linux and macOS | Med; call this out in requirements |
| Exit-status doc and test | A table-driven test that spawns the runner with each failure and checks the number | Low-Med |

## What a speed report should show

| Item | Include? | Notes |
|------|----------|-------|
| Frames per second (mean, steady-state) | Yes | Warm-up frames excluded; frame count and ROM named |
| Frame-time percentiles p50, p90, p99, p99.9, max (microseconds) | Yes | Percentiles over per-frame times show the stalls that a mean hides, which matter for rewind and run-ahead pacing |
| Total frames, total time, build type, compiler, platform | Yes | A report without the build flavour is meaningless; warn when not a release build |
| Excluded from the timing: hashing, IO, state serialization (unless asked) | Yes | A separate row for save and load microseconds |
| Optional row: frames/sec with 1-frame run-ahead cost modelled (save + load + 2 frames) | Differentiator | Honest because it shows the real hot path |
| Machine-readable output (one JSON object or key=value lines) | Yes | The PR job and Playstead can parse it; wall times are not gated |
| Per-PR instruction count (Cachegrind or equivalent), head vs merge-base, same ROM and frame count, percent change, threshold | Yes | The gate. Linux only. Stable enough to fail on a few percent; set the first threshold generously and tighten after observing noise |
| Comparison against published emulator speeds | No | Not measurable on the same machine without running their binaries; keep out |
| A "fast mode" that skips accuracy | No | Out of scope by project decision |

## Feature Dependencies

```
Serialization of each module (CPU, PPU, APU, synth, controllers, board, memory)
  -> Library state API (size/save/load, header, version, fingerprint, validation)
       -> Round-trip determinism tests and golden state file
       -> libretro serialize calls + .info declaration (serialized -> deterministic)
            -> retroarch-e2e proof: save/load state, rewind (needs "serialized"), run-ahead (needs "deterministic")
            -> Netplay readiness (cross-platform byte equality is already proven by the golden test)
       -> Runner --save-state/--load-state
            -> Runner state + movie (anchor start)
            -> --hash-state and --capabilities state fingerprint

Existing atomic save write + generation counter
  -> Signal-safe flush (flag + main-loop flush) -> SIGTERM/SIGINT/SIGUSR1 behaviour
  -> No stale .tmp after a failed save (v2 debt) must land before or with the signal work
  -> Documented exit statuses + child-process status tests

Existing runner + hash helpers
  -> Speed mode (fps + percentiles)
  -> Instruction-count job (needs fixed ROM, fixed frames, deterministic run)
  -> save/load microbench row (needs state API)

Tune-up (MMC3 .srm round trip in retroarch-e2e, README text for get_version / unload_cartridge, measured nightly timeout)
  -> independent; the .srm e2e step should land before state e2e so the hosted job structure is reused
```

Existing dependencies to protect: the declaration-baseline guard (refresh with the new API), the 95-key hash list (a state change must not move any hash; add state-bytes pins as new keys, do not reuse), `policy.no-skip` (every new e2e check must run, not skip), the six-platform matrix (the golden state check belongs in it), and the nofp preset (the serializers must stay integer-only).

## MVP Recommendation

Prioritize:
1. Library state API with a fixed size, header, version and fingerprint, plus all-or-nothing validated load (table stakes core).
2. Complete state inventory with round-trip determinism tests per board and the six-platform golden state pin.
3. libretro serialize and the `.info` declaration at "deterministic", proven in retroarch-e2e for save/load state, rewind and run-ahead.
4. Runner: signal-safe flush, documented exit statuses with tests, `--save-state`/`--load-state`, no stale `.tmp`.
5. Speed mode and the per-PR instruction-count comparison.
6. One differentiator: runner `--load-state` combined with `--movie`, plus `--hash-state` (small, enables Playstead resume and golden pins).

Defer: state migration (no second format yet); compression or deltas; memory descriptors for achievements (unless trivially compatible); movie-embedded states; a run-ahead-cost row in the speed report if time is short; Windows signal parity beyond a documented limitation; fuzzing the state loader (listed for a later milestone; keep reject tests table-driven in v3).

## Suggested ordering signal for the roadmap

1. Tune-up phase first (small, unblocks CI stability for the heavier e2e work).
2. State library phase (largest risk; everything else depends on it).
3. libretro and RetroArch proof phase.
4. Runner contract phase (signals, statuses, state commands); can run in parallel with 3 once the library API exists.
5. Speed phase last: it measures the state costs too, and the instruction-count baseline should be taken after the state code has landed so later PRs compare against a stable base.

## Open questions to settle in requirements or architecture research

- Include battery RAM in the state? Recommended yes, with generation bump on load. Verify the RetroArch behaviour where loading a state while a `.srm` exists does not surprise players.
- One fixed size for all boards, or fixed per cartridge? Recommended one size.
- Frame buffer in the state or regenerated? Recommended regenerated, provided the first frame after load matches an uninterrupted run (test it).
- Windows signal story for the runner.
- Threshold values for the instruction-count gate (set after two or three baseline runs).

## Sources

- `.planning/preparation/LIBRETRO-AND-RUNNER.md` sections 2 and 5 (serialization rules, quirk bits, `.info` levels, Playstead process behaviour, runner contract proposal) - project file, cited sources inside. Confidence: HIGH for facts it cites, MEDIUM where marked untested there.
- `.planning/preparation/NES-ECOSYSTEM.md` sections 4 and 5 (state-size, cross-version and run-ahead complaints in other NES cores' public issue trackers; ranked complaint counts). Confidence: MEDIUM-HIGH (secondary, project-compiled from issue titles).
- `.planning/seeds/SEED-002-save-states-and-runner-contract.md`, `.planning/seeds/SEED-004-robustness-and-speed.md` (scope and intent).
- `.planning/PROJECT.md` (existing capabilities, constraints, decisions such as the explicit little-endian audio serialization and the battery-save generation counter).
- RetroArch feature behaviour for rewind, run-ahead and netplay: general documented behaviour as described in the preparation file; not re-verified against a live build here. Confidence: MEDIUM.
