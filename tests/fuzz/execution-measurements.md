# Execution fuzz prototype measurement

Measured 2026-10-24 in finite replay mode, before choosing campaign budgets. This is one pass over each target's tracked corpus, with no mutation or retry. The three executables use Apple Clang 21 on arm64, Release configuration with `-O1`, ASan/UBSan, and LLVM source coverage (`-fprofile-instr-generate -fcoverage-mapping`). One instrumented CMake build supplies all three executables and the same core objects. The replay-only counters are enabled by `NESTURBATOR_FUZZ_METRICS=ON`; ordinary CI and libFuzzer builds do not enable them.

## Reproduction and identity

From the repository root, the measured driver command was:

```sh
cmake -DSOURCE_DIR=. -DOUTPUT_DIR=build/execution-measure -DMODE=replay \
  -P tests/cmake/execution_measure.cmake
```

The driver configures with `-DCMAKE_BUILD_TYPE=Release`, `-DCMAKE_COMPILE_WARNING_AS_ERROR=ON`, `-DCMAKE_C_FLAGS='-O1 -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -fprofile-instr-generate -fcoverage-mapping'`, `-DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined -fprofile-instr-generate'`, `-DNESTURBATOR_FUZZ_METRICS=ON`, and `-DNESTURBATOR_LIBFUZZER=OFF`; it builds `fuzz.rom_loader`, `fuzz.state_loader`, and `fuzz.execution` in that same build tree. Coverage tools are resolved with `xcrun --find llvm-profdata` and `xcrun --find llvm-cov`. The driver replays each complete corpus once, writes raw profiles and exported JSON, and compares functions whose source paths are under `src/`, excluding harness and loader files.

The three replay invocations were `cmake -E env LLVM_PROFILE_FILE=build/execution-measure/profiles/rom-loader.profraw /usr/bin/time -p build/execution-measure/build/tests/fuzz.rom_loader tests/fuzz/corpus/*`, `cmake -E env LLVM_PROFILE_FILE=build/execution-measure/profiles/state-loader.profraw /usr/bin/time -p build/execution-measure/build/tests/fuzz.state_loader tests/fuzz/corpus_state/*`, and `cmake -E env LLVM_PROFILE_FILE=build/execution-measure/profiles/execution.profraw /usr/bin/time -p build/execution-measure/build/tests/fuzz.execution --measure 1 tests/fuzz/corpus_execution/nrom-deep`. The driver expands corpus paths itself and records the resolved commands in its ignored `build/execution-measure/logs/commands.txt` artifact.

For each target, the driver runs `xcrun llvm-profdata merge -sparse build/execution-measure/profiles/<target>.profraw -o build/execution-measure/coverage/<target>.profdata` and `xcrun llvm-cov export -format=text -instr-profile=build/execution-measure/coverage/<target>.profdata build/execution-measure/build/tests/<target-binary> > build/execution-measure/coverage/<target>.json`. The individual export files and their hashes are listed below.

The source revision was `1d5c9db76cb037fe7464949ad5f4fcf71fad64b2`, with `source_dirty=1` while the Plan 18-01 measurement edits were uncommitted. The measurement overlay SHA-256 is `defd3b08be5c0699df75f1c453ec9cb49a125d94417809c95f252646a2393309`; its six file hashes are recorded in `build/execution-measure/measurement-inputs.tsv`. The corpus inventory is in `build/execution-measure/corpora.tsv` (SHA-256 `7535da7db6634be92217a472c6c13e65586f9da35ce0f81aa868a788f84355be`). Per-target corpus identities:

| Target | Files | Corpus manifest SHA-256 |
| --- | ---: | --- |
| `fuzz.rom_loader` | 60 | `781e99135484cc24af5d01555b399377a13a6734c818d3b6cb34487dd7825668` |
| `fuzz.state_loader` | 17 | `971757ba4a4d3f34e6a5d9af95d387a71d8b0570de20f5ef79eba6a582391dcc` |
| `fuzz.execution` | 1 | `05b4765ecac08c3643fe99551848ba4a1f2fa45f835aa7ab110eb26ebf88fd75` |

The target corpus hashes cover sorted relative file paths and each file's SHA-256. The execution corpus hash identifies the 16-byte `nrom-deep` descriptor.

## Measured replay

| Target | Inputs accepted / total | Frame calls (OK / JAM) | Public API operations | Guest ticks | Generated image / max state / custom tracked allocation peak | Elapsed; inputs/s; accepted/s | Peak RSS | Core functions hit / linked |
| --- | ---: | ---: | ---: | ---: | --- | --- | ---: | ---: |
| ROM loader | 11 / 60 | 0 (0 / 0) | 240 | 0 | n/a | 0.06 s; 1,000.00; 183.33 | 32,620,544 B | 39 / 180 |
| State loader | 12 / 17 | 48 (24 / 24) | 446 | 25,733,664 | n/a | 0.14 s; 121.43; 85.71 | 48,398,336 B | 158 / 202 |
| Execution | 1 / 1 | 3 (3 / 0) | 30 | 2,144,280 | 24,592 B / 4,895 B / 224,670 B | 0.06 s; 16.67; 16.67 | 18,006,016 B | 108 / 202 |

The ROM-loader target makes four public calls per input and never runs a frame. The state-loader corpus ran 48 frames across its 12 accepted state inputs; 24 returned `NESTURBATOR_OK` and 24 returned `NESTURBATOR_STOP_JAM`. The generated NROM recipe was accepted and completed all three calls with `NESTURBATOR_OK` (status value 0), with no JAM stops. Its process-level guest work was 2,144,280 ticks. The state-loader's 25,733,664 ticks came from a different corpus and twice as many non-JAM frames; these totals describe workload, not relative speed.

Elapsed time is the wall time from `/usr/bin/time -p` for a single instrumented replay process, so startup and coverage overhead are included. Throughput is that elapsed time divided into the exact corpus count and accepted count; these short single-pass rates are exploratory, not campaign performance budgets. Peak RSS is `getrusage(RUSAGE_SELF).ru_maxrss`, reported in bytes on macOS and normalized to bytes on Linux. The execution-only generated-image/state/allocator values come from the harness; the other targets do not expose those custom counters (`n/a` in the table, zero in the TSV).

The initial `/usr/bin/time -l` attempt failed because its `sysctl kern.clockrate` query is denied in this environment. The driver uses POSIX `time -p` for elapsed time and test-side `getrusage` for peak RSS instead; neither changes the emulator library or public API.

## Added core reach

LLVM coverage counted functions compiled into each executable and compared function identities by `src/` file and name. The execution replay reached 20 source functions not reached by either existing corpus. They are the new PPU/APU paths plus the generated program's RAM marker and audio synthesis work:

```text
src/apu.c::apu_write_register
src/apu.c::nesturbator__apu_write
src/apu.c::pulse_volume
src/instance.c::nesturbator_peek_cpu_ram
src/ppu.c::address_dot
src/ppu.c::at_address
src/ppu.c::background_access
src/ppu.c::bg_pattern_address
src/ppu.c::copy_horizontal
src/ppu.c::copy_vertical
src/ppu.c::inc_coarse_x
src/ppu.c::inc_y
src/ppu.c::nt_address
src/ppu.c::read_dot
src/ppu.c::reload_shifters
src/ppu.c::shift_background
src/ppu.c::sprite_access
src/ppu.c::sprite_pattern_address
src/synth.c::interpolate_coefficient
src/synth.c::nesturbator__synth_transition
```

The complete list is also saved as `build/execution-measure/execution-added-core-functions.txt`. Coverage exports are retained at `build/execution-measure/coverage/`:

| Export | SHA-256 |
| --- | --- |
| `rom-loader.json` | `29033ff48d9abf4a3732cce9c4699c5ea1ec8cb87ddd36f518fbb8adb1435b5f` |
| `state-loader.json` | `af63e8ed69f69558092baabbcbbc6561baa3ca1be5c6c2ef64d28688a112a3ce` |
| `execution.json` | `8b81527be6e5a7868d186ff21f29b6e7d41c89e5c16d858050ed5e40c7eab4fc` |

This establishes useful added NROM reach for the prototype. It does not measure the six required execution families, choose final mutation budgets, or run libFuzzer mutation on macOS. Those measurements remain for the later Phase 18 plans; ROB-01 remains pending.
