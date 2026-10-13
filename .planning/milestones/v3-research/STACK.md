# Technology Stack - Milestone v3 (save states, rewind, runner contract, speed)

**Project:** nesturbator
**Researched:** 2026-10-11
**Scope:** only what is NEW for v3. The validated stack (C17 with extensions off, CMake 3.25+ presets, Ninja, CTest with `check.h`, GitHub Actions on six runners, release-please v5, libFuzzer nightly, vendored `libretro.h` at the RetroArch v1.22.2 pin) is unchanged. The v2 research is in git history.
**Overall confidence:** HIGH for libretro.h facts (read in the vendored header), Windows signal and console-handler facts (Microsoft Learn, fetched today) and Cachegrind semantics (Valgrind manual, fetched today); MEDIUM for hosted-runner behaviour of Valgrind on arm64 and for RetroArch's internal rewind buffer (not verifiable without GPL source); LOW where marked.

## Headline

**No new dependency of any kind.** No library, no vendored file, no new GitHub Action, no new language. Three items need an installed tool, and only one is outside the image: `valgrind` from `apt` on `ubuntu-24.04` (not preinstalled; image 20261004.327.1 lists neither Valgrind nor perf). Everything else is C17 plus platform calls inside the runner (never the core), `.info` and adapter changes, and shell in existing workflows.

## 1. Save states in the library and the libretro adapter

### libretro.h surface (all already vendored; no header bump)

| Item | Decision | Why |
|------|----------|-----|
| `retro_serialize_size` / `retro_serialize` / `retro_unserialize` (stubs in `libretro/libretro.c` return 0/false today) | Return the library's fixed state size after a successful `retro_load_game`; the same value for the whole session, and also in the no-cartridge test-frame mode. Before load: 0. | Header (L7699): between load and unload the size must never grow, so a frontend allocates once. A fixed size is already the architecture's rule. |
| `retro_unserialize(data, len)` | Reject `len != size` (not "less than"), validate the whole buffer before touching the instance, return false on any failure with the instance unchanged. | Rewind and run-ahead feed it arbitrary old buffers; netplay feeds it remote bytes. Matches ARCHITECTURE section 7 ("validated in full before any of it is applied"). |
| `RETRO_ENVIRONMENT_SET_SERIALIZATION_QUIRKS` (44) | **Do not call it.** Zero quirks is the default and is the truthful value for a fixed-size, pointer-free, fixed-byte-order state. | Each quirk bit (INCOMPLETE 1<<0, MUST_INITIALIZE 1<<1, VARIABLE_SIZE 1<<2, SINGLE_SESSION 1<<4, ENDIAN_DEPENDENT 1<<5, PLATFORM_DEPENDENT 1<<6) is a way for a frontend to switch features off. INCOMPLETE in particular tells frontends not to use the state for netplay or rerecording. Setting none is the claim "complete". |
| `RETRO_ENVIRONMENT_GET_SAVESTATE_CONTEXT` (72, experimental) | **Do not depend on it.** The core produces byte-identical state in all four contexts (NORMAL 0, RUNAHEAD_SAME_INSTANCE 1, RUNAHEAD_SAME_BINARY 2, ROLLBACK_NETPLAY 3). | The contexts exist so cores may use pointers (context 1) or omit data (context 3). A pointer-free full state satisfies every context at once, which keeps one format, one test and one hash. The header says the netplay context wants integers in big-endian; ours are written byte-wise in one fixed order regardless of host, which gives the property that matters (same bytes on every platform). Document the choice (little-endian, byte-wise) in the public header; do not branch on context. |
| `RETRO_ENVIRONMENT_GET_AUDIO_VIDEO_ENABLE` (47, experimental; bit `RETRO_AV_ENABLE_FAST_SAVESTATES` 1<<2) | **Defer.** Do not read it in v3. | It lets a frontend ask for skipped video/audio during run-ahead. Skipping output is an optimisation that must provably not change state; measure first (section 3), then decide. Reading it changes nothing about correctness. |
| `SET_CORE_OPTIONS_V2` | Still none. | Options enter state, replay and netplay compatibility (existing decision). |

### `.info` file (`libretro/nesturbator_libretro.info`)

Change two keys, in one commit with the adapter change (CLAUDE.md rule 6):

```
savestate = "true"
savestate_features = "deterministic"
```

`savestate_features` allowed values (libretro-super `00_example_libretro.info`): `basic`, `serialized` (adds rewind), `deterministic` (adds run-ahead and netplay). RetroArch disables features above the declared level since 1.10.1 (LIBRETRO-AND-RUNNER section 2), so `deterministic` is what turns on run-ahead and netplay for users; it is a claim the CI must back, which is why the determinism tests below are not optional. `memory_descriptors` stays as is (separate decision). Do not add `cheats` or `input_descriptors` in this milestone.

Ship-order rule: the `.info` file must not say `deterministic` in any release whose determinism tests are not green on all six runners. Make the info key flip the last commit of the state phase.

### Netplay and run-ahead contract (what "deterministic" must mean here)

- Netplay compares core version and per-frame state hashes, so `library_version` (from `version.txt`) and state bytes must be equal across platforms. This is already the project's hash-pinning claim; v3 extends it to state bytes.
- Run-ahead in RetroArch (single instance) saves a state, runs N frames, loads the state back every frame; "second instance" mode loads the core twice. Both require no mutable globals (already enforced by the `nm` check) and `retro_run` free of side effects outside the instance.
- Tests that back the claim (all run by `cmake --workflow --preset ci`, no framework): (a) save at frame N, run M frames, hash; load, run M frames, hash; hashes equal; (b) N then M frames equals N+M; (c) two instances in one process, interleaved, produce unchanged hashes; (d) the saved bytes themselves are hashed and pinned in `hashes.txt`, so `hash-equality` compares them across the six legs; (e) the same state loaded after `reset` and after a different ROM is rejected with no change to the instance; (f) a libFuzzer target for the state loader (already planned as PERF-01 in ENGINEERING section 4) with a committed corpus replayed as `fuzz.regress.state` on legs without libFuzzer.
- Rewind efficiency (MEDIUM, from RetroArch behaviour I could not verify in source): RetroArch rewinds by taking a state every frame and delta-compressing it. Keep the state a multiple of 4 bytes, put fast-changing fields (CPU/PPU registers, counters) in fixed positions, zero all padding explicitly and never store anything that changes without game-visible meaning (host timestamps, pointers, allocation addresses). Then consecutive states differ in few bytes. A state of roughly 50 KB (RAM 2 KB, VRAM 2 KB, OAM 256 B, palette 32 B, CHR RAM up to 32 KB, PRG RAM up to 8 KB, registers) is cheap at 60 Hz.

### Format (library, not libretro)

| Item | Decision | Why |
|------|----------|-----|
| Header | magic, state-format version, behaviour revision, body length, ROM fingerprint | Versions are separate fields (ARCHITECTURE section 7). The fingerprint is how the runner and `--capabilities` tell a state belongs to a ROM (LIBRETRO-AND-RUNNER section 5). |
| Fingerprint | A small owned hash computed once at `load_cartridge` (CRC-32 or FNV-1a 64 over PRG+CHR), not SHA-256 | It guards against loading a state into the wrong game, not against an attacker. SHA-256 exists in the runner only; the core links only C memory functions and should not grow a crypto module for this. |
| Integrity | Structural validation (ranges of every index, mapper register bounds, enum values), **no** per-state checksum in the core | A checksum over 50 KB per frame is a cost on every rewind step and does not make a load safe; range checks do. The runner's state file may add a SHA-256 trailer, since the runner is not on the per-frame path. |
| Byte order | Byte-wise little-endian helpers; no struct dumps, no packed structs, no bit-fields | ENGINEERING section 1 rule; makes the bytes platform-independent. |
| Battery RAM | Included in the state, and also exposed separately as save RAM | A state is complete; loading one over a different `.srm` is the user's choice. `retro_unserialize` must not mark the battery as clean-synced; bump the dirty generation so the host flushes. |

## 2. Runner (`nesturbator-run`) signals, exit statuses, state files

### Signal handling: use C17 `signal()` and a `volatile sig_atomic_t`

| Item | Decision | Why |
|------|----------|-----|
| API | `signal(SIGINT, h)` and `signal(SIGTERM, h)` from `<signal.h>`, handler only stores to `static volatile sig_atomic_t g_stop` | This is the whole of what C17 allows a handler to do (access `volatile sig_atomic_t`, lock-free atomics, call `abort`, `_Exit`, `quick_exit`, `signal`). `fwrite`, `rename`, `malloc` and `printf` are forbidden inside a handler; Microsoft's page lists the same restrictions. The main loop polls the flag once per frame (about a millisecond of work), flushes saves and states through the existing temp-file-and-rename path, and exits. That gives the 2 s budget Playstead allows (SIGTERM, then SIGKILL after 2 s) a large margin. |
| `sigaction` | **Not needed.** Skip it. | `sigaction` is POSIX, not C17, and with `-std=c17` the headers hide it unless `_POSIX_C_SOURCE` is defined (`runner/save.c` already does this for `fsync`). The flag-only handler does not need `SA_RESTART` control because the runner makes no blocking calls between frames. One portable code path beats two. |
| Re-arming | In the handler, call `signal(sig, SIG_DFL)` first | A second SIGTERM or Ctrl+C then kills a runner stuck in a flush. It is also required on Windows: Microsoft documents that "before the specified function is executed, the value of func is set to SIG_DFL". On POSIX BSD semantics (glibc, macOS) keep the handler installed; setting SIG_DFL deliberately gives the same behaviour on both. |
| Windows | `signal(SIGINT, h)` works for Ctrl+C in a console. `SIGTERM` is **never generated** by Windows (Microsoft: SIGILL and SIGTERM "aren't generated under Windows"; only `raise` delivers them). For console close use `SetConsoleCtrlHandler` behind `#ifdef _WIN32` for `CTRL_BREAK_EVENT` and `CTRL_CLOSE_EVENT` only. | The handler runs on a **new thread** the system creates (HandlerRoutine docs), so write a `volatile LONG` or use `InterlockedExchange`, never touch the instance. `CTRL_CLOSE_EVENT` gives about 5 s (`SPI_GETHUNGAPPTIMEOUT`, 5000 ms); Ctrl+C and Ctrl+Break have no timeout. Logoff and shutdown events reach services only; ignore them. `windows.h` is already used in `runner/save.c`. Playstead is a macOS app, so this is best-effort and tested by `GenerateConsoleCtrlEvent` only if cheap; do not spend a phase on it. |
| `SIGUSR1` flush | `#ifdef SIGUSR1` (POSIX only); same flag pattern with a second flag `g_flush` | Existing proposal. Not defined on MSVC, so the guard keeps the file portable. |
| Crash signals | Unhandled (as proposed) | A handler for SIGSEGV cannot safely flush and would hide bugs from ASan. |
| Exit status after SIGTERM/SIGINT | 0 after a successful flush; 1 if the flush failed | Playstead reads 0 as clean and sent the signal itself. 128+n is the shell convention for death by signal, which a handled signal is not. |
| Exit status table | 0 done; 1 failure; 2 usage; 3 hash mismatch; 4 save file of the wrong length (v2 recommendation; check it shipped); 5 state rejected (wrong version, wrong ROM, bad length); 77 unsupported capability | Keep one `enum` in `runner/` and print the table in `--help` and the README. A test per status. Do not renumber existing ones. |

Test it with a CTest case that starts the runner on a long `--frames`, sends the signal through a small helper (`kill` from a test process on POSIX; `GenerateConsoleCtrlEvent` or `raise` via a test-only flag on Windows), and checks: exit status 0 inside 2 s, the save file is whole, and no `.tmp` remains. `tests/test_process.h` is the existing seam.

### State files in the runner

| Item | Decision | Why |
|------|----------|-----|
| Options | `--save-state FILE`, `--load-state FILE`, plus `--state-at-frame N:FILE` for tests | Mirrors `--dump-frame N:FILE`. Load before frame 1; refuse with status 5 and no file written if validation fails. |
| Write | Same temp file, sync, rename path as `.sav` (`runner/save.c`) | One implementation of atomic replace, already correct on POSIX (`fsync`, `rename`) and Windows (`_commit`, `MoveFileExA` with replace). Fixes nothing new; reuse. |
| `--capabilities` | Add state format version, state size and behaviour revision to the JSON | Already specified: "state fingerprint". |

## 3. Speed: runner timing mode and per-PR instruction counts

### Runner FPS and frame-time percentiles (informational, never a gate)

| Item | Decision | Why |
|------|----------|-----|
| Flag | `--bench N` (frames), optional `--bench-state` (serialize+unserialize each frame, which models rewind cost) | One mode serves both wall-clock and Cachegrind. No hashing, no dumping inside it, because SHA-256 would dominate the Ir. |
| Clock | `clock_gettime(CLOCK_MONOTONIC)` on Linux and macOS (macOS 10.12+), `QueryPerformanceCounter` on Windows, behind a ten-line wrapper in the runner | C17 `timespec_get(TIME_UTC)` is a wall clock and can step; C17 has no monotonic clock. Runner only: the core never reads time, so determinism is untouched. Needs `_POSIX_C_SOURCE 200809L` like `save.c`. |
| Statistics | Preallocate a `uint64_t` nanosecond array of N, `qsort`, nearest-rank p50, p90, p99, p99.9 and max; integer frames-per-second as `frames * 1000000000 / total_ns` (also milli-fps) | `qsort` is standard library, no float is needed, and it keeps the runner as integer-only as the core. Print one stable `key=value` line per statistic so a script can parse it. |
| Use | Print in the `perf` job summary and nightly logs; **never fail on it** | Hosted-runner wall time varies by several percent between runs (Valgrind manual's own comparison), and noisy neighbours add more. |

### Per-PR instruction counts: Cachegrind, `Ir` only, on `ubuntu-24.04` x64

**Recommendation: `valgrind --tool=cachegrind --cache-sim=no`, one job, `ubuntu-24.04` x64, `apt-get install -y --no-install-recommends valgrind`.** Version in the Ubuntu 24.04 repository is 3.22.0-0ubuntu3 (amd64 and arm64 builds exist). It is not preinstalled on the image (checked image 20261004.327.1). It is not a GitHub Action, so no new `uses:` pin and nothing for the `hygiene` scan to resist.

Why this tool:

| Candidate | Verdict | Why |
|-----------|---------|-----|
| **Cachegrind (`Ir`)** | **Use** | Counts executed instructions in user space, needs no hardware counters, runs inside any VM. The manual: instruction counts are "highly reproducible", execution time varies "several percent". Rustls' `ci-bench` and the Rust compiler's builtins repo gate CI on the same method. `--cache-sim=no` is the default and the fastest. |
| `perf stat -e instructions` | Reject | Hosted runners are VMs; `perf` is not on the image, `linux-tools` would need to match the kernel, and the runner-images issue 4974 reports the PMU counters as unsupported on hosted images (2022 data; no current GitHub statement found, so MEDIUM). Even where counters work they vary run to run and by CPU model. |
| Callgrind | Optional drill-down only | Same determinism, 2 to 3 times slower, adds call graphs. Use it by hand when the Cachegrind summary shows a regression; do not run it in CI. |
| `cg_annotate --diff`, `cg_diff` | Artifact only | `cg_annotate --diff a b` computes b minus a (negative means b is cheaper); `cg_diff` is described as deprecated in the manual. Upload both outputs as an artifact for humans. Do not parse them for the gate. |
| arm64 (`ubuntu-24.04-arm`) | Skip for the gate | The Ubuntu package exists for arm64, but no source I found confirms behaviour on the hosted arm runner, and `Ir` is ISA-specific, so one x64 baseline is enough. If wanted later, add a separate leg with its own baseline and expect to debug (LOW). |
| CodSpeed, iai-callgrind, hyperfine | Reject | Hosted service, Rust-only, and wall-clock respectively. |

### How to compare head and merge-base deterministically

Cachegrind totals are exactly repeatable for the same binary, same arguments, same environment and same path. The manual warns that results depend on executable and shared-library sizes, file-name lengths, and ASLR, so isolate those variables:

1. `actions/checkout` with `fetch-depth: 0` (existing pinned action; no new one). Compute `BASE=$(git merge-base origin/main HEAD)` on `pull_request`. Use the merge-base, not the base branch tip, so unrelated `main` commits do not appear as this PR's change. Create it with `git worktree add` into a sibling directory.
2. Build **only the runner target** in both trees with the **same preset** (`ci`, Release) and the runner's default compiler (GCC 13.3 on the image; do not use `-march=native`). Same compiler, same flags, build dirs of equal-length names.
3. **Copy each binary in turn to one fixed path** (for example `$RUNNER_TEMP/perf/bin`) and run it from the same working directory with `env -i` (empty environment). This removes argv[0], path-length and environment-size noise, which is the main way two builds of one program can differ by a few hundred instructions.
4. Run each of a **fixed list of workloads**: `--bench N` on three committed ROMs (an NROM test ROM, an MMC1 game, the MMC3 game) plus one run with `--bench-state`, and the same with N = 0 (`--bench 0` loads, runs no frame). Per-frame cost = Ir(N) minus Ir(0) divided by N, which removes startup and loader cost from the signal.
5. Command: `valgrind --tool=cachegrind --cache-sim=no --cachegrind-out-file=OUT ./bin --bench N --rom ROM`. Read the total from the `summary:` line of `OUT` (format: an `events: Ir` header and a `summary: <number>` line) with `awk`; no Python or `cg_annotate` is involved in the gate. Keep `OUT` files as an artifact and run `cg_annotate --diff` once for the job summary (needs `python3`, present on the image; cg_annotate's language is not stated in the manual, so treat its availability as MEDIUM and make the gate independent of it).
6. Report a table (workload, base Ir/frame, head Ir/frame, change in percent) in `$GITHUB_STEP_SUMMARY`. **No PR comment**: it needs `pull-requests: write`, which a fork's `pull_request` token does not have, and the rule against `pull_request_target` stands (ENGINEERING section 6).
7. Gate: report-only for the first week of runs, to confirm in practice that two runs of the same binary give identical totals (expected: identical; any difference means an uncontrolled input). Then fail the job when any workload's per-frame Ir grows more than **1 percent** (start; tighten to 0.5 once stable). A threshold that low is only possible because the measure is deterministic. An intended slowdown is accepted by editing a tracked `tests/perf/baseline-allowed.txt` line with a reason, in the same PR, so the exception is reviewed and nothing waits on a person.
8. Timeout: twice a measured cold run, per the existing rule; expect a few minutes (two Release builds of one target plus Valgrind at roughly 20 to 50 times slowdown over a fixed, small frame count). Choose N so that the whole job stays near 5 minutes.

Join it to the `CI required` roll-up like any other job (the roll-up accepts only `success`). The ENGINEERING table already has a `perf` row on `ubuntu-24.04`.

Gaps for the speed phase: the compile time of two builds (could reuse the `build` job's artifact for head; measure first); the real Ir repeatability on the hosted runner (step 7 proves it); whether `apt-get update` flakes need a retry wrapper (add one if the first weeks show it).

## 4. Tune-up phase tooling

| Item | Decision | Why |
|------|----------|-----|
| RetroArch e2e for the MMC3 `.srm` round trip, state, rewind and run-ahead | Extend the existing pinned RetroArch 1.22.2 job; drive it over the UDP network-control interface | The interface (`network_cmd_enable`, default port 55355, documented in libretro/docs) offers `SAVE_FILES`, `SAVE_STATE`, `LOAD_STATE`, `LOAD_STATE_SLOT <n>`, `REWIND`, `RUNAHEAD_TOGGLE`, `PAUSE_TOGGLE`, `FRAMEADVANCE`, `READ_CORE_MEMORY`, `GET_STATUS`, `QUIT`. All of it is scriptable with `nc -u` or the shell's `/dev/udp`, so "no check waits on a person" holds. |
| State e2e assertions | After `SAVE_STATE`, assert a state file exists in `savestate_directory`; `LOAD_STATE`, then compare a `--max-frames-ss` screenshot or `READ_CORE_MEMORY` bytes with the pre-save values | RetroArch compresses states by default and wraps them in its own container, so assert existence, non-zero size and behaviour, not the bytes. Byte-level checks belong to the library tests. |
| Rewind and run-ahead e2e | `rewind_enable = "true"` in the appended config; `REWIND` while running; `RUNAHEAD_TOGGLE` with `run_ahead_enabled`; assert the process runs for N frames without a core error and the screenshot hash is stable | Proves the `.info` claim end to end. Whether RetroArch prints a distinct log line when it disables run-ahead for a core is unverified (LOW); read the `-v` log in the first run. |
| Nightly timeout | Set `timeout-minutes` to twice the first measured cold run | Existing rule. |
| Stale `.tmp` after a failed save | Already addressed by the temp-file path; add a failure-injection test (target is a directory, or an unwritable path) that asserts no `.tmp` remains | Pure C test through `tests/test_process.h`; no tooling. |

## Alternatives Considered

| Category | Recommended | Alternative | Why Not |
|----------|-------------|-------------|---------|
| Handler API | C17 `signal` plus `volatile sig_atomic_t` | `sigaction` + `sigaltstack`, `signalfd`, self-pipe | Not C17 / not portable to MSVC and macOS; the runner has no blocking call to interrupt. |
| Windows close | `SetConsoleCtrlHandler` for Break and Close only | Hook everything, services API | Playstead is macOS; Windows gets Ctrl+C and nothing more. |
| State context | One byte-identical state for all contexts | Branch on `GET_SAVESTATE_CONTEXT` (smaller netplay state, pointers for run-ahead) | Four formats to test and hash; the state is about 50 KB, so the saving is negligible. |
| State integrity | Structural validation | CRC or SHA over every state | Per-frame cost for rewind with no safety gain. |
| Quirks | None set | `INCOMPLETE` to be cautious | Turns off netplay and rerecording features users expect from `deterministic`. |
| Instruction counter | Cachegrind `Ir` | `perf stat`, `qemu` plugins, wall-clock `hyperfine` | No PMU in hosted VMs; noise of several percent. |
| Comparison input | Merge-base, same fixed path, empty environment | Base branch tip, or separate paths per tree | Tip adds unrelated changes; path and env differences shift Ir. |
| Windows runner count for perf | One `ubuntu-24.04` x64 leg | Six legs | `Ir` is per-ISA; six legs multiply cost for no new signal. |
| Valgrind source | Ubuntu 3.22.0 via apt | Build 3.26 from source, a Valgrind action | New dependency or a new `uses:` with no need; 3.22 is enough for `Ir`. |

## Installation

Only in the `perf` job:

```bash
sudo apt-get update
sudo apt-get install -y --no-install-recommends valgrind   # 3.22.0 on ubuntu-24.04
```

Nothing to install locally; the dev presets are unchanged. A maintainer on macOS has no usable Valgrind for arm64 macOS; reproduce a perf finding by pushing the branch or in a Linux container.

## What NOT to Add

- No library for compression, hashing or serialization (zlib, protobuf, cereal-style helpers) in the core. States are raw bytes with byte-wise helpers.
- No state checksum in the core's per-frame path.
- No call to `SET_SERIALIZATION_QUIRKS`, no `GET_SAVESTATE_CONTEXT` branching, no variable-size state.
- No signal handler that does I/O; no `sigaction`, threads, `signalfd`; no SIGSEGV/SIGABRT handler.
- No `perf`, `linux-tools`, `hyperfine`, CodSpeed or other perf SaaS; no PR-comment bot; no `pull_request_target`.
- No wall-clock performance gate. Wall-clock output is information only.
- No timing code in the core or the library; time stays in the runner.
- No new GitHub Action for Valgrind; no ccache (v2 decision stands).
- No floating point for percentiles or FPS in the runner; integer nanoseconds.
- No GPL/LGPL emulator source for state-format or rewind ideas; RetroArch behaviour is learned from the pinned released binary and the documentation only.

## Gaps to Address in Phase Research

- Hosted arm64 Valgrind behaviour (only needed if an arm64 perf leg is wanted).
- Whether the RetroArch 1.22.2 binary disables run-ahead or rewind for cores whose `.info` claims `deterministic` but whose behaviour it cannot verify (observe via `-v`).
- RetroArch rewind buffer sizing for a core whose state is not a multiple of 4 (avoided by sizing it as one).
- Exact Ir noise floor across runs on the hosted runner (step 7 above measures it).
- Whether `windows-11-arm`'s console delivers `CTRL_CLOSE_EVENT` in a CI session (best-effort, test only if cheap).
- Playstead's rule for state files (path, naming, when it asks for one); `--state-dir` may be wanted once its contract is known.

## Sources

- `libretro/libretro.h` in this repository (RetroArch v1.22.2 pin): L1683-1696 (`SET_SERIALIZATION_QUIRKS`), L1767 and L3226-3288 (audio/video enable), L2430 and L5649-5713 (`GET_SAVESTATE_CONTEXT`, `retro_savestate_context`), L3437-3470 (quirk bits), L7690-7740 (`retro_serialize_size`, `retro_serialize`, `retro_unserialize`). HIGH.
- libretro-super `dist/info/00_example_libretro.info` (raw, master, fetched 2026-10-11): `savestate`, `savestate_features` values `basic`, `serialized`, `deterministic`. HIGH.
- libretro docs `development/retroarch/network-control-interface.md` (master, fetched 2026-10-11): command list and defaults. HIGH.
- Microsoft Learn, `signal` (CRT) and `HandlerRoutine` callback pages (fetched 2026-10-11): SIGINT thread behaviour, SIGTERM not generated, handler reset to SIG_DFL, forbidden calls, console event timeouts. HIGH.
- Valgrind manual, Cachegrind chapter (valgrind.org, fetched 2026-10-11): `--cache-sim` default, `--cachegrind-out-file`, `cg_annotate --diff`, reproducibility and sensitivity notes. HIGH.
- Ubuntu package page for `valgrind` in noble: 1:3.22.0-0ubuntu3, architectures include amd64 and arm64 (fetched 2026-10-11). HIGH.
- actions/runner-images `Ubuntu2404-Readme.md`, image 20261004.327.1: GCC 12/13/14, Clang 16/17/18, CMake 3.31.6, Ninja 1.13.2, no Valgrind, no perf (fetched 2026-10-11). HIGH.
- actions/runner-images issue 4974 (perf counters unavailable on hosted images, 2022) via search summary. MEDIUM.
- rustls `ci-bench` and rust-lang compiler-builtins CI (Cachegrind on hosted runners via apt), via search summary. MEDIUM.
- Project files: `.planning/preparation/LIBRETRO-AND-RUNNER.md` sections 2, 5, 7; `ENGINEERING.md` section 5; `ARCHITECTURE.md` section 7; `runner/save.c`, `runner/main.c`, `libretro/libretro.c`. HIGH.
