# Project Research Summary

**Project:** nesturbator, milestone v3 "Save states and rewind"
**Domain:** deterministic NES emulator core in C17: fixed-size save states, libretro serialize/rewind/run-ahead, headless runner process contract (signals, exit statuses, state files), speed measurement
**Researched:** 2026-10-11
**Confidence:** MEDIUM-HIGH overall (HIGH for state design and libretro facts; MEDIUM for Windows signal delivery and hosted Valgrind; LOW where marked)

## Executive Summary

v3 adds no dependency of any kind. The validated stack stays as it is. The work is one new core module that serializes the whole machine field by field into a fixed-size, little-endian format, plus the libretro and runner code that calls it. Save-state implementations usually fail the same few ways: they dump structs, let size drift with cartridge or options, miss a latch, or half-apply a load. This design avoids each by construction. One field list per struct drives size, write, validate and apply, so they cannot drift apart. Validation completes before any byte is applied, so a rejected load leaves the instance unchanged. Size is a pure function of the loaded cartridge, so libretro's "never grows between load and unload" holds.

Five phases, numbered 11 to 15. Phase 11 clears tune-up debt and adds the shared atomic-write helper. Phase 12 builds the format in the library and proves it with a fork test (run, save, continue; compare against a fresh instance poisoned by another game) on all six boards, with state hashes pinned on the six runners. Phase 13 wires libretro and raises `.info` claims (`serialized` for rewind, then `deterministic` for run-ahead and netplay) only when the tests backing each are green. Phase 14 gives the runner a signal-safe flush (flag in the handler, write from the frame loop) and a documented exit table. Phase 15 adds a never-gated frame-time report and a Cachegrind instruction comparison against the merge base, which becomes a gate only after its noise is measured.

Main risks: hidden state left out (shows up only frames later, first in the audio hash); a half-applied load; the battery save-generation counter (bump only when battery bytes change, or run-ahead rewrites `.sav` every frame); Windows signal delivery, best-effort and needing its own research. Each risk has a named test in the phase that owns it.

## Decisions Where the Research Disagreed

| Question | What the files said | Decision | Why |
|----------|---------------------|----------|-----|
| Exit status numbering | STACK and ARCHITECTURE: 4 shipped (wrong-length battery file), 5 for rejected state. PITFALLS: save failure after SIGTERM exits 4. | Keep 0, 1, 2, 4 as shipped. **4 = battery save file has wrong length only.** **5 = state rejected** (any of the three library state statuses; stderr names which). Failed flush after a signal exits **1**. | 4 already has a shipped meaning; reusing it for write failures changes a contract the launcher reads. One state code keeps the launcher contract small; the library keeps distinct statuses for hosts. |
| `--expect` (exit 3) and `--capabilities` (exit 77) in v3? | FEATURES marks exit 3 "Done". ARCHITECTURE read `runner/main.c` and found only 0, 1, 2, 4. STACK lists both as v2 recommendations. | **Out of v3.** Reserve 3 and 77, never emitted in v3. Phase 11 greps to confirm 3 is absent; if present, it stays as shipped. | `--hash-state N` plus six-leg hash pins already prove byte identity in CI. `--capabilities` would freeze a JSON contract before the format has a second version. Reserving numbers makes deferral free. Revisit at milestone close if launcher preflight becomes a requirement. |
| Cachegrind starting threshold | STACK: 1%, tighten to 0.5. ARCHITECTURE: +3%. PITFALLS: 5% per benchmark, 2% total. FEATURES: "generous". | **Report-only first, then gate at +1% per-frame Ir per workload**, tightened to 0.5 once stable. Intended slowdowns pass via a reviewed line in `tests/perf/baseline-allowed.txt`. | The measure is deterministic on one binary, so the threshold only has to exceed real code change. A 3-5% gate lets a real several-percent slowdown per PR pass unseen. Gate arms only after report-only runs show zero spread on identical binaries. |
| Runner flag names | STACK: `--save-state`, `--load-state`, `--state-at-frame N:FILE`. ARCHITECTURE: `--state-save N:FILE`, `--state-load`. FEATURES: `--save-state` plus `--save-state-at-frame N`. | **`--state-at-frame N:FILE`** (write after frame N), **`--load-state FILE`** (read before frame 1), **`--hash-state N`**. | `--dump-frame N:FILE` already uses frame-colon-file form. A bare `--save-state` would suggest an end-of-run save. `--load-state` keeps the noun-first form. |
| Signal API | ARCHITECTURE and PITFALLS: `sigaction`, `signal(SIGBREAK)` on Windows. STACK: C17 `signal()` only. | **C17 `signal()`**, handler only sets `volatile sig_atomic_t`. Second signal resets to `SIG_DFL` so a stuck flush can be killed. Windows: `SetConsoleCtrlHandler` for Ctrl+Break and close only. | `sigaction` is POSIX, not C17; the runner has no blocking call to interrupt, so one portable path suffices. Windows never generates SIGTERM. |
| State size | ARCHITECTURE: pure function of cartridge, 0 with no cartridge. FEATURES and PITFALLS: one maximum for all boards. | **Per cartridge**: function of loaded layout, fixed from load to unload, 0 with no cartridge. `nesturbator_state_size(const nesturbator *)`. | libretro needs constancy within a session, not one global size. A maximum pads a typical 13-21 KiB state to about 45 KiB, multiplying rewind memory. |
| Checksum and fingerprint | STACK: no per-state checksum in core; CRC-32 or FNV-1a 64 for identity. ARCHITECTURE and FEATURES: CRC-32 in header. PITFALLS: 64/128-bit fingerprint. | **One CRC-32 table.** Header CRC-32 over the whole state (computed on save, checked on load). ROM identity = CRC-32 over header bytes 0-15, PRG ROM and CHR ROM, computed once at load. No SHA in the core; no trailer in the runner file. | Corruption must be caught cheaply; range checks give memory safety. Identity guards against wrong-game loads, not attackers, so 32 bits suffice. Cost measured in Phase 15; same-instance fast path is the fallback. |
| Where a frame can end | FEATURES: a frame may end mid-instruction; codec must carry it. ARCHITECTURE: run calls stop at instruction boundaries. | **Always serialize instruction and micro-step position fields**, validated to values the core can produce. API stays frame-boundary only. | Correct under either reading for a few bytes. Still checked in the Phase 12 spike. |
| Adapter `retro_unserialize` length | STACK: reject `len != size`. ARCHITECTURE: clamp a larger buffer. PITFALLS: accept larger only if tail is zero. | **Exact length only.** Anything else returns false, instance unchanged. | Size is fixed per cartridge, so another length means another cartridge or format. Ignored trailing bytes would hide that. If RetroArch pads buffers, the Phase 13 e2e shows it; fallback is to accept a zero-filled tail. |
| `.info` levels | STACK: both keys at once. ARCHITECTURE: `deterministic` only once fork and run-ahead tests pass. | **Staged.** `savestate = "true"` with `savestate_features = "serialized"` after serialize round trip is green on six legs. `deterministic` only in the last commit of Phase 13, after fork and run-ahead tests are green on six legs. | Rewind and run-ahead are separate claims with separate tests. |
| Fuzzing the state loader | FEATURES: defer. STACK and ARCHITECTURE: libFuzzer target in Phase 13. | **Table-driven reject tests and flip-every-byte sweep in Phase 12. `fuzz.state_loader` in Phase 13**, nightly, committed corpus replayed as `fuzz.regress.state` in `ci`. | State loader is the first new path taking untrusted bytes (state files, rewind, netplay); fuzz infrastructure exists, so cost is nightly only. |
| Movie plus state | FEATURES: in MVP. ARCHITECTURE: absent. | **Defer.** | Needs a movie frame counter that starts from a state; Phase 12 is already largest. Resume is served by `--load-state` with `--frames`. |
| Perf job and CI required roll-up | STACK: join roll-up like any job. PITFALLS: advisory job must not break roll-up; skipped required job stays pending. | **Join the roll-up from day one in report mode.** Report mode always exits 0 after writing the table to the job summary. Arming the gate is a one-line change. | Nothing skipped or pending, and nothing fails the roll-up until the gate is armed. |
| Runner contract timing | FEATURES: parallel with libretro. ARCHITECTURE: sequential. | **Sequential**: libretro (Phase 13) then runner (Phase 14). | One GSD step per command (project rule 7); each phase ends in a downloadable release. |
| Speed mode name and percentiles | ARCHITECTURE: `--fps`, p50/p95/p99. STACK: `--bench N`, `--bench-state`, p50/p90/p99/p99.9/max. | **`--bench N`, optional `--bench-state`**; p50, p90, p99, p99.9, max; integer milli-fps. | Name says what is measured; `--bench-state` puts rewind cost in the same mode. |
| Signal test mechanism | ARCHITECTURE: wait for flush file, `--raise-signal-after` fallback. PITFALLS: wait for ready line. STACK: `kill` or `GenerateConsoleCtrlEvent`. | **Deterministic in-process raise at frame N** through a hook compiled only into the test build, registered on all six legs. External-signal test on POSIX only. No sleeps. | Frame-indexed delivery needs no timing. Windows gets a test that runs, not a skip. Hook stays out of the shipped runner. |
| Golden state fixture | FEATURES: committed v1 golden state. PITFALLS: fixture per format version. STACK: pins in `hashes.txt`. | **Hash pins only** in v3, in `tests/runner/hashes.txt`. No committed binary state. | Pins already catch layout drift and cross-platform differences. A committed state holds RAM bytes needing hygiene review for no added detection. |

## Key Findings

### Recommended Stack

No new dependency, vendored file, GitHub Action, or language. Everything is C17 in the runner (never the core), `.info`/adapter changes, and shell in existing workflows. The one new tool is `valgrind` 3.22.0 from `apt` on `ubuntu-24.04` (not preinstalled), used only by the perf job.

- `retro_serialize_size`/`retro_serialize`/`retro_unserialize`: thin wrappers over the library. No quirks set. `SET_SERIALIZATION_QUIRKS`, `GET_SAVESTATE_CONTEXT`, `GET_AUDIO_VIDEO_ENABLE` not called.
- C17 `signal()` + `volatile sig_atomic_t` for stop/flush flags; `SetConsoleCtrlHandler` only for Windows Ctrl+Break and close.
- `clock_gettime(CLOCK_MONOTONIC)` / `QueryPerformanceCounter` in the runner only.
- Cachegrind (`valgrind --tool=cachegrind --cache-sim=no`) is the per-PR gate. Chosen over `perf stat` (no PMU on hosted VMs) and wall time (several-percent noise).

### Expected Features

**Table stakes:** library state API (size; save into caller buffer, no allocation; all-or-nothing load with distinct statuses); own format (magic, format version 1, header size, total size, behaviour revision recorded not enforced, mapper id, submapper, region, ROM identity, section sizes, CRC, reserved zero); complete machine state with battery RAM included and generation bumped only when battery bytes change; byte-identical state across six platforms pinned in CI; libretro serialize with staged `.info`; runner `--state-at-frame`, `--load-state`, `--hash-state`; flag-only signal handling, flush from frame loop, exit 0 within 2 s of SIGTERM/SIGINT after clean flush; documented exit table with a test per status; no stale `.tmp`; frame-time report (integer percentiles); Cachegrind comparison against merge base.

**Differentiators:** one size per cartridge stated in README; cheap save/load measured as share of frame; format-version policy with clean refusal (migrator deferred); `--bench-state` shows run-ahead cost.

**Deferred:** movie with embedded or anchored start state; `--expect` (3) and `--capabilities` (77), numbers reserved; compression or deltas (host job); memory descriptors beyond keeping RAM buffers stable; migration code; Windows signal parity beyond Ctrl+Break and close; arm64 perf leg.

**Anti-features:** raw struct `memcpy`; size varying with options or content after load; partial load; quirks used to excuse gaps; mid-instruction save API; in-core rewind/run-ahead/netplay loops; compression or encryption in core; signal handlers doing I/O; battery `.sav` replaced by states; options altering emulation; wall-clock thresholds in CI.

**State inventory:** CPU registers, PC, interrupt lines and edge detectors (NMI edge, IRQ level and delay latches), DMA phase and offsets (OAM, DMC), open-bus latch, RAM. PPU: all registers, `v`/`t`/`x`/`w`, VRAM, palette, OAM and secondary OAM with counters, sprite-evaluation state, shifters and fetch latches, dot/scanline/odd-frame parity, `$2007` read buffer, open-bus decay, A12 filter. APU: channel state, frame counter phase and delayed-write latch, DMC buffer and shift state, IRQ flags. Synthesis: fixed-point history, transition history, filter history, output cursor (PCM ring drops out only if Phase 11 shows it is always empty at frame boundaries). Controllers: strobe, shift positions, held buttons. Mapper: registers, shift state, MMC3 IRQ and A12, PRG RAM and CHR RAM bytes; derived page tables rebuilt, not saved. Not in state: ROM bytes, frame buffer (regenerated; first frame after load is tested), host pointers, allocator, pending input, transition hook, save generation counter.

### Architecture Approach

One new core module, `src/state.c`, owns the format. Each serialized struct has one field list written as a visitor in four modes: SIZE, WRITE (canonical little-endian), VALIDATE (range-checks every field), APPLY (stores). VALIDATE and APPLY walk identical bytes with identical ranges, so APPLY cannot fail and no staging copy is needed. Each mapper board gains one `state` op for its own registers. Public header gains three functions and three status codes, all appended, no ABI bump. Adapter and runner are thin clients.

Layout: 64-byte header, then nine sections, each with 4-byte tag and 4-byte length, fixed order: core scalars, CPU, bus, APU, synthesis, PPU, mapper (registers padded to 32 bytes), PRG RAM, CHR RAM. Lengths checked against SIZE before any field is parsed.

Load: args and header checks, CRC, VALIDATE every section, APPLY, copy RAM into existing allocation, `mapper_load` to rebuild page tables, bump save generation only if battery span bytes changed (one compare before copy). Every failure returns before APPLY.

Frame-scoped pointers (`video_output`, `sample_output` and counts) are NULL between frames, never saved or loaded; a test asserts this after every frame.

**Components:** (1) `src/state.c` and the visitor: single source of size, bytes, validation, apply. (2) Mapper `state` ops, six boards; missing op fails a table-driven test. (3) Adapter entry points with exact-length checks. (4) Runner state flags and signal flag; frame-loop flush through shared atomic-write helper; one exit-status enum. (5) Perf job: head and merge-base in one job, same preset, fixed workloads, per-frame instruction counts.

### Critical Pitfalls

1. **Hidden state left out.** Divergence appears frames after load, first in audio hash. Avoid: field list per struct, `sizeof` tripwires, fork test with poisoned fresh instance, audio hashes alongside frame hashes, save-every-frame must not change any hash.
2. **Struct dump, padding, endianness.** Avoid: byte-wise LE helpers; pointers derived; state bytes pinned per board on all six legs.
3. **Size drift.** Avoid: size is pure function of cartridge, asserted equal across load, frames, platforms; no option affects it (v3 adds none).
4. **Loading a state into another game.** Bank indices out of range become out-of-bounds page pointers. Avoid: ROM identity in header; range checks before rebuild; 6x7 cross-ROM rejection matrix including no-cartridge instance. Identity is not a security boundary; range checks are.
5. **Save generation and rewind.** Unconditional bump rewrites `.sav` at 60 Hz under run-ahead; no bump leaves file stale. Avoid: counter never serialized, never decreases; bump only when span bytes differ; test 600 no-change loads leave it alone and a changing load bumps exactly once. Document that rewinding past an in-game save reverts save RAM.
6. **Signal handlers and Windows.** Avoid: flag-only handlers; flush in frame loop; `volatile sig_atomic_t` (POSIX), `volatile LONG` + `InterlockedExchange` (Windows); no crash-signal handlers; document that `TerminateProcess` cannot be handled, so `--save-interval` is the Windows guarantee.
7. **Exit-status drift.** Avoid: one table, one header, printed by `--help` and README; test parses README against constants; a test per row; never use 126, 127, 128+, 130, 143.

## Implications for Roadmap

Phases continue from v2 (ended at Phase 10). Each phase ends in a downloadable release.

### Phase 11: Tune-up and state readiness

**Rationale:** Small, independent, removes debt state work would inherit. The atomic-write helper must exist before state files are written.
**Delivers:** one atomic-write helper (temp, fsync, checked close, rename; unlink temp on failure) used by battery saves, with failure-injection tests (unwritable path, directory at target) asserting no `.tmp`; stale-temp cleanup for this ROM's own names only; delete dead `map.a12`; rename runner `save_state` struct to `battery_save`; white-box assertions: frame-scoped pointers NULL after every frame, `synth.pcm_count` == 0 at every frame boundary; audit no non-const function-local static state; MMC3 `.srm` round trip in pinned RetroArch job over UDP network-control, waiting on a marker, never a sleep; confirm existing `nesturbator_status` numbers (8, 9, 10 assumed) and grep for exit 3; README/header debt (`get_version`, `unload_cartridge`).
**Addresses:** v2 debt 08 IN-02 and 08 WR-03; state readiness.
**Avoids:** Pitfalls 17, 18, and the layout surprise in pitfall 1.
**Ends with:** runner leaves no stale temp after a failed save; RetroArch e2e for MMC3 save round trip.
**Research flag:** standard patterns.

### Phase 12: State format in the library

**Rationale:** Largest phase and the hard prerequisite for everything else.
**Delivers:** `src/state.c` with visitor, 64-byte header, nine tagged sections, CRC-32 table, three public functions (`nesturbator_state_size(const nesturbator *)`, 0 without cartridge; `nesturbator_save_state` on const instance; `nesturbator_load_state` all-or-nothing). `rom_crc32` at load; `state` op on all six boards; `map.a12` removed. Appended header content, three status codes, format version macro, layout and battery-span documentation; declaration-baseline guard refreshed; README "Save states" section in same change. Tests: size shape over 1000 frames; save-load-save idempotence; fork test (run N, save, continue M; poisoned fresh instance must reproduce every frame hash, audio hash, tick count and final state byte); rewind at several N including 0; run-ahead pattern with 2-3 save/load cycles per frame; every board; save-every-frame does not change hashes; cross-ROM matrix; truncation at every length; flip-every-byte; same sweep with CRC recomputed under ASan/UBSan; `sizeof` tripwires; battery-generation rule. Runner: `--state-at-frame`, `--load-state` (after battery load, before frame 1), `--hash-state N`, as test tooling so six-leg pins run here. Rejected state exits 5. Writes use Phase 11 helper.
**Avoids:** Pitfalls 1-5 and 10.
**Ends with:** runner writes and resumes a state on all six platforms.
**Research flag:** short spike: read `src/frame.c` for instruction boundaries; measure save/load cost with placeholder CRC; confirm inventory via fork test on all six boards.

### Phase 13: libretro serialize, rewind, run-ahead

**Rationale:** Depends only on Phase 12. Claims staged so each ships with its own evidence.
**Delivers:** adapter `retro_serialize_size` (instance size, 0 without content); `retro_serialize` (writes first `size` bytes of a buffer at least that large; no allocation); `retro_unserialize` (exact length only; false leaves instance unchanged); no quirks. Staged `.info`: `serialized` after serialize round trip green on six legs; `deterministic` only in last commit, after fork and run-ahead green on six legs. Host tests: size constant across frames; short or long buffer rejected; corrupt buffer returns false and next frame matches; no-content size 0; first frame after load matches. Mock host with N+1 speculative runs and load-back: visible frame and audio hashes equal uninterrupted run; callbacks once per `retro_run`. `fuzz.state_loader` nightly (recomputes CRC), corpus replayed as `fuzz.regress.state` in `ci`. RetroArch e2e (pinned 1.22.2, UDP network-control): save, load, rewind and run-ahead enabled via appended config; assert N frames with no core error and stable screenshot hash. A test that `.info` contains claimed keys.
**Avoids:** Pitfalls 8, 9, 23, 24.
**Ends with:** RetroArch build where rewind and run-ahead work with the core.
**Research flag:** standard; whether RetroArch disables run-ahead for an unverifiable core is LOW confidence, read the `-v` log in first run.

### Phase 14: Runner process contract

**Rationale:** Depends on Phase 12 only via exit 5. Sequential after 13 under the one-step rule; may swap with 13 if Playstead timing demands.
**Delivers:** SIGTERM/SIGINT flag (C17 `signal()`), checked at frame boundary; flush via shared helper; exit 0 after clean flush, 1 if flush failed; second signal resets to `SIG_DFL`; signal before loop honoured. SIGUSR1 flush (`#ifdef`, POSIX), continues running. Windows: `SetConsoleCtrlHandler` for Ctrl+Break and close setting `volatile LONG` via `InterlockedExchange`; bounded wait for flush. One exit enum in one header, printed by `--help` and README: 0 done or stopped by signal after clean flush; 1 failure; 2 usage; 3 and 77 reserved; 4 battery wrong length; 5 state rejected. Tests: one per status row; deterministic signal at frame N via test-only hook on all six legs; POSIX external-signal test (`kill`, wait on ready line) asserting exit 0 within 2 s, whole `.sav`, no `.tmp`; failed flush exits 1; state write never changes `.sav` bytes or mtime. State files use `.state` extension and explicit paths; runner writes only the path given.
**Avoids:** Pitfalls 13-16.
**Ends with:** runner the launcher can stop cleanly, on every covered platform.
**Research flag:** deeper research: Windows console-event delivery in hosted session (`GenerateConsoleCtrlEvent`, new process group); fsync latency vs 2 s on hosted disks; close events on `windows-11-arm`. Mark Windows delivery best-effort in README.

### Phase 15: Speed

**Rationale:** Last; measures everything; baseline must include state code.
**Delivers:** runner `--bench N` (optional `--bench-state`): monotonic clock per frame into preallocated `uint64` ns array, sort once, p50/p90/p99/p99.9/max, integer milli-fps; key=value to job summary; never gating; test that hashes are equal with and without bench. Cachegrind perf job on `ubuntu-24.04` x64: worktree at merge base, same preset, binary copied to fixed path, run under `env -i`; three committed ROMs (NROM, MMC1, MMC3) at `--bench N` and N=0; per-frame `(Ir(N) - Ir(0)) / N`; `--cache-sim=no`; table to job summary; `cg_annotate` output as artifact only. Report mode first, in the roll-up; gate at +1% per workload armed only after zero spread on identical binaries; intended slowdowns via reviewed line in `tests/perf/baseline-allowed.txt`. Only then, optimisation accepted only if every frame, audio and state hash is unchanged.
**Avoids:** Pitfalls 19-22.
**Ends with:** speed report and per-PR instruction table on every PR.
**Research flag:** short spike: Valgrind runs the `ci` build under the image's GCC 13; identical builds give identical totals.

### Phase Ordering Rationale

- 11 first: helper and `pcm_count` result are inputs to the format; debt would be inherited otherwise.
- 12 before 13 and 14: both are clients; its six-leg hash pins are the cross-platform proof everything relies on.
- 13 before 14: rewind and run-ahead have their own RetroArch evidence; runner contract is independent.
- 15 last: baseline must include state code; optimisation safe only once hash equality is cheap.
- Staging inside phases (`serialized` before `deterministic`; report before gate) keeps every release's claims backed by a test on the same commit.

### Research Flags

Need deeper research: **Phase 12** (frame.c instruction boundaries; save/load cost); **Phase 14** (Windows console events in CI; fsync vs 2 s; close events on `windows-11-arm`); **Phase 15** (Valgrind with `ci` compiler; count stability).
Standard patterns: **Phase 11** (helper and cleanups; `pcm_count` is a test); **Phase 13** (standard once 12 lands; `.info` keys verified in libretro-super example, HIGH).

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | libretro facts read from vendored header; `.info` levels from libretro-super; Cachegrind method from Valgrind manual. MEDIUM for hosted arm64 and RetroArch internals, neither needed for the gate. |
| Features | MEDIUM-HIGH | From project preparation files and documented libretro behaviour; RetroArch rewind and run-ahead not re-verified against a live build. |
| Architecture | HIGH for inventory and layout (source read); MEDIUM for size figures (estimates until Phase 12 measures). | |
| Pitfalls | MEDIUM-HIGH | Project files and v2 audit primary. Hardware latch details from NESdev knowledge, not re-fetched. Windows console and signal details unverified, flagged for Phase 14. |

**Overall:** MEDIUM-HIGH for library and libretro design; MEDIUM for runner on Windows and perf job.

### Gaps to Address

- **Frame boundary and instruction position:** research disagrees on mid-instruction stops. Serialize position fields either way; confirm in Phase 12 spike.
- **`synth.pcm_count` at frame boundaries:** decides whether the 2 KiB PCM ring is in state. Assert in Phase 11; if not always 0, serialize ring in logical order and reset indices on load.
- **Existing status numbers and exit 3:** 8, 9, 10 assumed to follow existing values; exit 3 assumed absent. Verify in Phase 11 before appending.
- **Launcher (Playstead) state rules:** state-file path, naming, default location, and whether it needs `--capabilities` are not in research. Resolve in Phase 14 requirements.
- **RetroArch padding and run-ahead behaviour:** observe in Phase 13 e2e (`-v` log); strict-length decision has a stated fallback.
- **Windows signal delivery in CI:** deterministic in-process test covers handler and flush on all six legs; external delivery is best-effort.
- **fsync latency vs 2 s:** unmeasured on hosted disks; measure in Phase 14 and document.
- **Cachegrind on the image:** not preinstalled; check with `ci` compiler and count stability in Phase 15 spike.
- **State cost vs frame:** CRC and two-pass load estimated at microseconds for 20 KiB; measure in Phases 12 and 15.
- **Frame buffer after load:** first frame after load must match uninterrupted run; tested in Phase 13.

## Sources

### Primary (HIGH)

- `libretro/libretro.h` (RetroArch v1.22.2 pin): serialization functions, quirk bits, savestate context, audio/video enable.
- libretro-super `dist/info/00_example_libretro.info`: `savestate` and `savestate_features` values.
- libretro docs, network control interface: command list and defaults.
- Microsoft Learn: CRT `signal` (SIGINT on separate thread; SIGTERM not generated on Windows; handler reset to SIG_DFL) and `HandlerRoutine` (console event timeouts).
- Valgrind manual, Cachegrind chapter: `--cache-sim`, `--cachegrind-out-file`, `cg_annotate --diff`, reproducibility notes.
- Ubuntu noble `valgrind` 1:3.22.0-0ubuntu3 (amd64, arm64).
- actions/runner-images `Ubuntu2404-Readme.md`: GCC 12-14, Clang 16-18, CMake 3.31.6, Ninja 1.13.2; no Valgrind, no perf.
- Project source: `include/nesturbator.h`, `src/internal.h`, `src/mapper.h`, `src/instance.c`, `src/frame.c`, `src/cartridge.c`, `libretro/libretro.c`, `libretro/nesturbator_libretro.info`, `runner/main.c`, `runner/save.c`, `runner/save.h`, `tests/CMakeLists.txt`, `tests/libretro/libretro_host.c`, `tests/fuzz/rom_loader.c`.
- Project preparation: `.planning/preparation/LIBRETRO-AND-RUNNER.md` (2, 5, 7); `ARCHITECTURE.md` (2, 6, 7, 8); `ENGINEERING.md` (4-6); `NES-ECOSYSTEM.md` (4, 5).
- Seeds: `.planning/seeds/SEED-002-save-states-and-runner-contract.md`, `SEED-004-robustness-and-speed.md`.

### Secondary (MEDIUM)

- actions/runner-images issue 4974 (hardware counters unavailable on hosted images; 2022 data).
- rustls `ci-bench` and Rust compiler-builtins CI (Cachegrind on hosted runners).
- RetroArch rewind, run-ahead, netplay behaviour from preparation files; not re-verified live.
- NESdev wiki latch behaviour (from knowledge).

### Tertiary (LOW, needs validation)

- Whether RetroArch disables run-ahead for an unverifiable core (Phase 13 log).
- Whether `windows-11-arm` delivers close events in a hosted session (Phase 14).
- Whether Valgrind runs the `ci` build cleanly under the image compiler (Phase 15).

---
*Research completed: 2026-10-11*
*Ready for roadmap: yes*
