# Execution fuzz measurement record

The first section records Plan 18-01's NROM-only baseline. The Plan 18-02 section below records the matched six-family update. Both are finite replay measurements from 2026-10-24, before final campaign budgets; neither includes libFuzzer mutation or retries. The three executables use Apple Clang 21 on arm64, Release configuration with `-O1`, ASan/UBSan, and LLVM source coverage (`-fprofile-instr-generate -fcoverage-mapping`). One instrumented CMake build supplies all three executables and the same core objects. The replay-only counters are enabled by `NESTURBATOR_FUZZ_METRICS=ON`; ordinary CI and libFuzzer builds do not enable them.

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

This established useful added NROM reach for the prototype. It did not measure the six required execution families or choose final mutation budgets. The next section adds the required family evidence; final phase requirements remain pending final verification.

## Plan 18-02 six-family matched replay

Measured 2026-10-24 using the reproduction command above, with one replay pass over each of the three exact corpora. The comparison reuses the same Release Clang 21 arm64 ASan/UBSan/LLVM coverage build for ROM loading, state loading, and execution. It additionally replays each execution seed in its own instrumented process and records per-family core function reach. The updated driver rejects any execution corpus other than the six required seeds. Each deep replay now includes a different-board interleaved frame between the saved origin and continuation, so it completes four frames and records 55 public API operations.

### Family results

Each descriptor is 16 bytes and generates its accepted iNES/NES 2.0 image in memory. All six loaded through the public loader, completed four `NESTURBATOR_OK` frame calls, recorded 55 public API operations including load, input, save/restore, a different-board interleave and unload, and stopped with status 0. No qualified family stopped with JAM. Bank-effect bits are PRG `1`, CHR `2`, and AxROM single-screen page `4`.

| Seed | Representative profile | Accepted | Frames (OK/JAM) | API operations | Generated image | Guest ticks | Effects | Core functions hit / linked | Elapsed | Inputs/s | Peak RSS |
| --- | --- | ---: | ---: | ---: | ---: | ---: | --- | ---: | ---: | ---: | ---: |
| `nrom-deep` | mapper 0, iNES 1, 16 KiB PRG, 8 KiB CHR ROM | 1/1 | 4 (4/0) | 55 | 24,592 B | 2,859,144 | none | 118 / 202 | 0.06 s | 16.67 | 18,579,456 B |
| `mmc1-deep` | mapper 1 submapper 0, 32 KiB PRG, 32 KiB CHR ROM, 8 KiB battery RAM | 1/1 | 4 (4/0) | 55 | 65,552 B | 2,859,000 | PRG + CHR | 118 / 202 | 0.06 s | 16.67 | 18,448,384 B |
| `uxrom-deep` | mapper 2 submapper 1, 64 KiB PRG, 8 KiB CHR RAM | 1/1 | 4 (4/0) | 55 | 65,552 B | 2,859,072 | PRG | 119 / 202 | 0.06 s | 16.67 | 18,563,072 B |
| `cnrom-deep` | mapper 3 submapper 1, 32 KiB PRG, 32 KiB CHR ROM | 1/1 | 4 (4/0) | 55 | 65,552 B | 2,859,144 | CHR | 119 / 202 | 0.06 s | 16.67 | 18,415,616 B |
| `mmc3-deep` | mapper 4 submapper 0, 64 KiB PRG, 32 KiB CHR ROM, 8 KiB battery RAM | 1/1 | 4 (4/0) | 55 | 98,320 B | 2,859,120 | PRG + CHR | 123 / 202 | 0.06 s | 16.67 | 18,743,296 B |
| `axrom-deep` | mapper 7 submapper 1, 64 KiB PRG, 8 KiB CHR RAM | 1/1 | 4 (4/0) | 55 | 65,552 B | 2,859,048 | PRG + single-screen | 119 / 202 | 0.06 s | 16.67 | 18,612,224 B |

The MMC1 guest places serial writes on separate CPU cycles; the discrete bank tests use no-conflict submapper 1; MMC3 selects its fixed-and-switchable PRG windows and CHR register while reading through the PPU; AxROM reads back separate nametable pages after switching bank and single-screen page. The examples are representative accepted profiles, not coverage claims for every admitted submapper, RAM size, or maximum image size.

### Matched target totals

| Target | Accepted / inputs | Frames (OK / JAM) | API operations | Guest ticks | Generated bytes / max state / custom allocation peak | Elapsed; inputs/s; accepted/s | Peak RSS | Core functions hit / linked |
| --- | ---: | ---: | ---: | --- | --- | --- | ---: | ---: |
| ROM loader | 11 / 60 | 0 (0 / 0) | 240 | 0 | n/a | 0.10 s; 600.00; 110.00 | 32,653,312 B | 39 / 180 |
| State loader | 12 / 17 | 48 (24 / 24) | 446 | 25,733,664 | n/a | 0.14 s; 121.43; 85.71 | 48,431,104 B | 158 / 202 |
| Execution | 6 / 6 | 24 (24 / 0) | 330 | 17,154,528 | 385,120 B / 13,111 B / 626,054 B | 0.11 s; 54.55; 54.55 | 21,905,408 B | 143 / 202 |

Execution reached 24 `src/` functions not hit by either existing corpus, compared by source file and function name. The difference from the NROM-only baseline is the CNROM write path, MMC3 PPU A12 and counter paths, and the shared PPU write path in addition to the prior APU, rendering, RAM observation and synthesis paths:

```text
src/apu.c::apu_write_register
src/apu.c::nesturbator__apu_write
src/apu.c::pulse_volume
src/instance.c::nesturbator_peek_cpu_ram
src/mapper_cnrom.c::cnrom_cpu_write
src/mapper_mmc3.c::clock_counter
src/mapper_mmc3.c::mmc3_ppu_a12
src/ppu.c::address_dot
src/ppu.c::at_address
src/ppu.c::background_access
src/ppu.c::bg_pattern_address
src/ppu.c::copy_horizontal
src/ppu.c::copy_vertical
src/ppu.c::inc_coarse_x
src/ppu.c::inc_y
src/ppu.c::nesturbator__ppu_write
src/ppu.c::nt_address
src/ppu.c::read_dot
src/ppu.c::reload_shifters
src/ppu.c::shift_background
src/ppu.c::sprite_access
src/ppu.c::sprite_pattern_address
src/synth.c::interpolate_coefficient
src/synth.c::nesturbator__synth_transition
```

The finite comparison shows useful added runtime reach across all six families beyond both loader/state corpora; it does not assert hardware accuracy or quantify raw mutation throughput. The fuzz input accepts arbitrary PRG/CHR payload bytes after the fixed descriptor, but this measurement intentionally contains only the six protected deep descriptors. Throughput is a one-pass exploratory rate that includes process startup and sanitizer/coverage overhead.

### Measurement identity and artifacts

The instrumented source revision was `1d7a010bdf3afdd56f874422f5ba72382a72b02e`, with `source_dirty=1` because Task 3 lifecycle controls and interleaving were measured before their task commit. The final measurement overlay SHA-256 was `4a99c0f01b472192b23ccfa4bcb8bf788d6e6e59f6ea4cab0922dfbb5433a0cf`; the sorted corpus manifest SHA-256 was `1a76c9de3aae9d5f999deeab93db8d8506d65cd9cb9cb7647f524f8496e15dc2`. The six-file execution corpus manifest hash was `bef47c764bef3be36726ffb95f0c4b8f5b3c06a6c0f269e23b6ca2aa7cdc0f88`.

Full command logs, resource counters, corpus inventory and coverage exports are retained under ignored `build/execution-measure/`. `execution-families.tsv` contains the raw per-family result rows, and `execution-added-core-functions.txt` contains the function difference. Export hashes:

| Export | SHA-256 |
| --- | --- |
| `execution.json` | `b85ace0dab512fc2e6feb739a53c9a7cf4a1bdebd9de16b8c258e673b1bfc0e9` |
| `execution-nrom.json` | `446476f7ad32111ba55fa9b777267f12167be92fc819b6d60219f8675f7e81d3` |
| `execution-mmc1.json` | `7344897280ae97884dad2ebbcdc59ccc71f1c75bcaffdbabb8b54ae729df2a1e` |
| `execution-uxrom.json` | `c41ace4ef4fe4162be917696fdfd7d7583447197557ad7e04f3441c7b7e9e888` |
| `execution-cnrom.json` | `6d528eef00e4c20284268e2dbbb20a14ba7014a243f0ad165ce18f230da17f55` |
| `execution-mmc3.json` | `1168baad3bb5f09a7d6779289b61566c55c055bc445763ebfc243bd452f5ea11` |
| `execution-axrom.json` | `2764bcc1c578c0f6278090f180061f379ac90551df6d62481805e3a8f7022598` |

The per-family report row for each seed confirms one accepted image, four non-JAM frames, 55 operations, positive core-function reach, and the expected register/bank effect. The 24 new functions remain reached across all six deep profiles. This measurement is useful reach evidence only: it does not mark ROB-01 or ROB-02 complete before final phase verification.
