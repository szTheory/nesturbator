# nesturbator

A NES emulator core in C: a library you can embed, a headless runner for
automation, and a libretro adapter.

**Status: v1 shipped; milestone v2 starts with a tune-up.** v1 plays
NROM games with picture and sound; milestone v2 adds UxROM (mapper 2). Phase 5 adds parallel CI, a policy that
no registered test may skip, and the soft reset (`nesturbator_reset()`, which
RetroArch's Reset button runs). The 6502 core matches the public
65x02 test vectors on every opcode and bus cycle. The library, runner and
libretro core accept bounded iNES 1.0 and NES 2.0 images for mapper 0 (NROM,
16 or 32 KiB PRG) and mapper 2 (UxROM), with 8 KiB CHR ROM or declared CHR RAM, mapper 3 (CNROM, 8 to 32 KiB CHR ROM) and mapper 7 (AxROM, CHR RAM); the PPU renders backgrounds
and evaluated sprites, including palette priority, flips, 8x16 selection,
clipping, sprite-zero hit and the eight-sprite limit. Pre-render evaluation
includes OAM Y=$FF sprites on visible framebuffer row 0. After eight sprites
are selected, the 2C02's diagonal OAM scan can set sprite overflow from a
tile, attribute or X byte; an in-range Y skipped by that scan does not set it.
Overflow remains in PPU status until pre-render dot 1. The first eight
sprites are copied one byte per odd/even OAM pair, so their final X bytes are
copied before the ninth Y comparison at dot 130; `$2002` reads before that
comparison still see overflow clear. This is
not full game compatibility: cartridges load through a per-board mapper
interface (page tables, a four-entry nametable map, a CPU-cycle-stamped write
hook, a mapper IRQ ORed with the APU's, and PPU A12 edges reported to the
board), with NROM, UxROM, CNROM and AxROM as the boards so far. The behaviour revision is 5: the PPU
fetch pipeline changed the frames of games that write the scroll or PPUCTRL
while rendering. Mid-frame scroll writes render as on
the console, which a split-scroll test shows scanline by scanline.
With no cartridge, the fixed test card and silence remain available. The plan lives in [`.planning/`](.planning/).

The runner accepts content with `--rom FILE`, for example:

A trainer-bearing NROM, UxROM, CNROM or AxROM image copies its 512 trainer bytes into writable
instance-owned PRG RAM at CPU `$7000-$71FF` before the reset vector is used.
The full `$6000-$7FFF` 8 KiB window is writable and starts at zero outside the
trainer span. Trainerless images keep `$6000-$7FFF` unmapped. Invalid images
are rejected before cartridge allocation, and a failed reload leaves the
previous cartridge usable.

```sh
nesturbator-run --frames 1 --rom game.nes --hash-frame 1 --dump-frame 1:frame.ppm
```

The CI suite pins three redistributable mapper-0 games: MIT-licensed
Nesteroids, zlib-licensed Double Action Blaster Guys, and all-permissive RHDE.
Their boot hashes and scripted DABG two-port movie hashes are checked against
`tests/runner/hashes.txt` on every platform; the hashes use native pixels
before display-palette conversion. RHDE's iNES header declares zero CHR-ROM
banks and uses the 8 KiB CHR RAM it fills during startup.

The pinned MIT AccuracyCoin test ROM is also included for conformance checks.
The CI runner drives one menu page at a time, reads result bytes from CPU RAM
through `nesturbator_peek_cpu_ram`, and compares the exact names, order, status,
and result codes on pages 2 and 17 with `tests/accuracy/scoreboard.txt`:

```sh
nesturbator-run --accuracycoin-page 2 --rom tests/roms/accuracycoin.nes \
  --scoreboard tests/accuracy/scoreboard.txt
```

Before the workflow preset, each of the six CI build lanes fetches protected
`refs/heads/main` with tags disabled. A cross-platform CMake script exports the
exact protected-main scoreboard bytes through
`NESTURBATOR_SCOREBOARD_BASELINE`; fetch, path inspection, content retrieval,
and environment handoff errors fail the job. If a successful fetch confirms
that main has no scoreboard yet, CI supplies an empty baseline for the first
merge. The scoreboard test requires that CI-provided path and never falls back
to the candidate snapshot. Detached local runs use the committed
`tests/accuracy/scoreboard-main.txt` snapshot. In both cases a prior `pass` row
must remain present and passing; AccuracyCoin results continue to be checked
against live emulated RAM. Page 14 adds the six required APU results: Length
Counter, Length Table, Frame Counter IRQ, Frame Counter 4-step, Frame Counter
5-step, and Delta Modulation Channel.

The CPU RAM inspection function is read-only, accepts the `$0000-$1FFF` RAM
mirrors, and rejects other bus addresses without side effects. It is intended
for conformance and debugger integrations.

It can also replay an owned, versioned two-port input movie:

```sh
nesturbator-run --movie input.nmovie --rom game.nes
```

The binary movie begins with the eight bytes `NMOVIE1` and a zero byte, then
little-endian `uint32_t` version `1` and frame count. Each frame stores two
little-endian `uint16_t` button masks, port 0 then port 1; values must fit the
eight standard NES button bits. The count is limited to 1,000,000 frames, and
the file must contain exactly the declared records. Empty movies are valid and
run zero frames. Invalid, truncated, extra, unsupported-version, or oversized
movies fail before a frame runs. Replay prints one native-pixel SHA-256 line
per frame in ascending frame order; a JAM stop reports its frame and exits
nonzero.

Frame time advances in 24-tick CPU cycles. A frame request runs complete
instructions through the requested boundary and reports the actual tick count,
retaining any overshoot for the next request. Audio is mono signed 16-bit PCM at
48 kHz; each frame returns 798 or 799 samples with the fraction carried
forward. With no cartridge loaded, samples are zero. Mapper-0 pulse register
writes and the `$4015` enable gate share the CPU bus timeline; pulse timer,
duty, length and DAC gating follow NTSC RP2A03 documentation in
`.planning/preparation/NES-HARDWARE-CPU-APU.md` section 4 (HWC.08). Both pulse
units use independent duty phases, timers and length gates; triangle follows
its 32-step DAC sequence and linear/length gates; noise uses the 15-bit LFSR,
period table and mode tap; and DMC uses its timer, sample fetch, shift register
and 7-bit DAC. RP2A03G noise starts at measured LFSR state `$0000`, with its
first clock shifting in 1 (HWC.05); the APU_Noise overview's “loads 1” wording
describes the operational initialization model (HWC.08). These channel
sequences use integer state and bus-cycle ordering. The 4-step and 5-step frame
sequencers clock envelopes, linear and length counters, and maintain a frame
IRQ independent from the DMC IRQ; `$4015` reports and clears the frame source.
The CPU accepts an enabled IRQ from its instruction-end polling sample and
enters the IRQ vector with the normal seven-cycle stack sequence (HWC.02).
Writes to `$4017` take effect after the parity-dependent three or four CPU
cycles. DMC sample fetches halt CPU reads at the bus seam, repeat the parked
read, and preserve the independent IRQ source. These timing rules follow
NES-HARDWARE-CPU-APU sections 2 and 4 (HWC.01, HWC.23) and are exercised by
`core.apu` and the six AccuracyCoin page-14 results. The nonlinear pulse and TND
mixer uses checked-in integer pulse and 16×16×128 TND tables from HWC.11.
Mixed-level changes feed a per-instance, fixed-point band-limited synthesizer.
Its 16-tap, 32-phase Q15 kernel preserves each level transition exactly, then
applies the NES 90 Hz and 440 Hz high-pass filters and 14 kHz low-pass filter
before returning signed 16-bit mono PCM. The bounded staging ring carries
kernel and filter history across caller frame boundaries. `runner.spectral`
checks pulse periods 100, 40, 12 and 8, and triangle period 1 using a 4096
sample filter warm-up and a 32768-sample periodic-Hann spectrum. It excludes
one bin on either side of the first 512 integer harmonic orders after folding
them into the positive FFT spectrum; the triangle's fundamental bin is folded
at Nyquist. Each tone uses the nearest coherent bin to its NTSC timer
frequency. The largest remaining peak below 16 kHz must be below -80 dB
relative to the fundamental. These analysis choices are test rules; the
synthesizer itself uses integer arithmetic and checked-in coefficient tables
only. A private per-instance transition observer runs before synthesis consumes
each changed mixed level and retains no event log. `nesturbator-run --hash-audio`
prints separate SHA-256 hashes for 12-byte transition records
(`uint64` CPU cycle little-endian, then `int32` level little-endian) and signed
PCM samples (`int16` little-endian). Both encodings are host-endian independent.
If a cartridge executes JAM, the frame call returns `NESTURBATOR_STOP_JAM`; the
CPU stays latched until the cartridge is unloaded or reloaded.

The PPU sets vblank at scanline 241 dot 1 and clears it at scanline 261 dot 1.
A `$2002` read immediately before the vblank start dot suppresses the flag and
NMI edge for that frame; reads on or after the start dot observe and clear the
flag. Odd NTSC frames skip pre-render dot 340 when rendering is enabled.
Nametable accesses use the cartridge's horizontal or vertical
mirroring bit. Register accesses retain the CPU open-bus value in un-driven
bits, and `$2007` reads are buffered outside palette space.
Visible native pixels are written to the caller's frame buffer as PPU dots advance. The
background is fetched from `v` two dots per access (nametable, attribute, pattern
low and high) on the documented dots 1-256 and 321-336, through pattern and
attribute shifters, so the selected pattern table, nametable attributes,
fine X and the universal backdrop colour apply as on the console. Scroll
writes take effect at the dot-257 horizontal copy and the pre-render dots
280-304 vertical copies, not at the pixel being drawn, and a `$2007` access
while rendering increments coarse X and Y together. The PPU address bus and
its A12 line are reported to the cartridge board on every change, with the
tick of the dot, including `v` itself while rendering is off. Sprites are evaluated into secondary OAM
and fetched for the following scanline, one slot at a time on dots 257-320
(two garbage nametable accesses, then the pattern low and high bytes), with
an empty slot fetching tile `$FF` as the console does; transparent pixels reveal the
background, and the priority bit selects which opaque layer appears in front.
`$2001` grayscale and emphasis remain in the native pixel value; host palette
conversion is separate and does not affect frame hashes.
Writing a page number to `$4014` queues an OAM DMA; the next CPU read is halted
while 256 bytes transfer through the CPU bus into OAM. The transfer wraps from
the current `$2003` address and stalls the CPU for 513 or 514 cycles according
to cycle parity. The PPU keeps advancing during the transfer.

The public `nesturbator_set_input` API accepts a size-tagged pair of standard
controller button masks. Port 0 is `$4016`; port 1 is `$4017`. Each port shifts
A, B, Select, Start, Up, Down, Left, Right, least-significant bit first. A
write with bit 0 high makes reads report the current A button; the high-to-low
transition latches both masks. After eight reads, D0 returns 1. D6 reads high,
while D5 and D7 retain the CPU bus open-bus value. Refused input calls leave
the instance unchanged. Input is snapshotted at the beginning of each
successful frame call. An instruction that crosses the requested frame
boundary completes with that frame's snapshot; the next mask begins on the
next `nesturbator_run_frame` call. The libretro adapter polls both standard
joypads once per frame and maps the host's NES button IDs to these masks.

## Building

You need CMake 3.25 or newer, Ninja and a C17 compiler.

```sh
cmake --workflow --preset ci    # Release, warnings as errors; builds, runs every test, writes the archives
cmake --workflow --preset dev   # Debug build and the same tests
cmake --workflow --preset asan  # the same tests under AddressSanitizer and UBSan
cmake --workflow --preset nofp  # core built with -mgeneral-regs-only; the abi checks
cmake --workflow --preset hygiene  # tree contents, action pins and formatting
cmake --workflow --preset vectors-full  # the full 65x02 vector set, fetched at its pin (network)
```

On Windows with MSVC, use `ci-msvc` from a developer command prompt.

## Checks

Each lane is one command, `cmake --workflow --preset <lane>`.

| Lane | What it proves |
|---|---|
| `ci` | Release build with warnings as errors; every test passes, the library installs and builds a separate consumer, and the three release archives are written |
| `ci-msvc` | The same as `ci`, built with MSVC on Windows |
| `asan` | Every test passes under AddressSanitizer and UBSan, with any report fatal |
| `nofp` | The core builds with `-mgeneral-regs-only` and passes the tests labelled `abi` |
| `hygiene` | The tree holds no personal data and no unlisted ROM or binary file, every GitHub Action is pinned to a commit, and the C sources are formatted |
| `vectors-full` | The full 65x02 vector set, fetched by git at the commit in `tests/vectors/pins.txt` and checked file by file, matches the CPU on every test; needs the network and fails without it |

Tests never skip: a test that cannot run somewhere is not registered there, and
`policy.no-skip` fails when any registered test could report itself skipped or
is disabled.

`fuzz.regress` replays checked-in malformed cartridge seeds through the public
cartridge load/unload lifecycle on every CI platform. The Linux nightly builds
that same entry point with Clang libFuzzer and ASan/UBSan, then runs it for a
bounded minute. The corpus is manifest-listed and local; CI does not download
ROMs or fuzz seeds.

The `abi` tests hold the core to integer arithmetic and the C memory
functions: a text scan of `src/` and `include/` for `float`, `double` and
decimal or hex floating literals, with a self-test that it finds `1.5`,
`0x1p3` and `0x1.8p1`; an `nm` check that the library needs no symbol beyond
`memcpy`, `memmove`, `memset`, `memcmp`, `malloc`, `free` and the
toolchain's fortify and stack-protector helpers (plus `bzero` on macOS); an
`nm` check that it defines no writable data; and a fixture with a `double`
multiply that these checks must reject.

The `hygiene` tests: `hygiene.tree` runs `scripts/hygiene.sh --tree`, which
rejects home-directory paths, email addresses other than GitHub noreply, ROM
and save files, and any file git treats as binary unless
`tests/roms/manifest.txt` lists it. It reads file names unquoted, so a name
with non-ASCII bytes is scanned, and it reports a file it cannot read rather
than passing it. It runs in the C locale, so text that is not UTF-8 is still
scanned. The pre-push hook runs `scripts/hygiene.sh --history`, which also
scans each commit's message; there the noreply address of the AI co-author
trailer is allowed as well. `hygiene.scan_selftest` builds a scratch
repository and, under a UTF-8 locale, requires the scan to find a home path
and an address in a file with a non-ASCII name and in a Latin-1 file, and to
report a file whose name holds a newline as unreadable. It then requires
`--history` to find a home path and an address in a commit message and to
pass a clean message that carries the trailer.
`hygiene.action_pins` checks that every
`uses:` key under `.github` names a full 40-digit commit SHA or a local
path; `hygiene.action_pins.bad` and `.good` show it rejects a tag and accepts
a SHA. `hygiene.format` runs clang-format over every tracked C source except
the vendored `libretro/libretro.h` and fails on any change it would make. The
lane needs clang-format 18, the version on CI's Ubuntu 24.04 image, and stops
at configure if it finds another; on macOS, `brew install llvm@18` provides
it. To format before committing:

```sh
/opt/homebrew/opt/llvm@18/bin/clang-format -i $(git ls-files '*.c' '*.h' '*.cpp' | grep -v '^libretro/libretro.h$')
```

The tests are plain C programs under `tests/` that use the macros in
`tests/check.h`; CTest runs them. `core.api` checks every status code of
`nesturbator_create` and that a custom allocator gets back each block it
handed out. `core.frame` checks the buffer rules of `nesturbator_run_frame`,
that a refused call changes nothing, the audio sample count over a whole
period, the test card's edge pixels, and that two instances run apart.
`core.palette` checks how `nesturbator_get_palette` copies and the colour
table's invariants: `$20` and `$30` are white, `$xE` and `$xF` are black under
every emphasis, an emphasis bit raises no colour channel but its own, and
brightness never falls down a column.
`runner.write_hashes` runs each pinned game and the three scripted DABG
two-port movies. It writes ordered native hashes at frames 1, 30, 60, 120 and
180, plus transition and PCM hashes for each game's boot run. It fails if any
requested frame or audio hash is missing or duplicated;
`runner.write_hashes.content` requires all 36 sorted keys to equal
`tests/runner/hashes.txt` byte for byte, with LF line endings only.
`runner.dump` runs the command above and checks the image's size, header and
pixels; `runner.usage.dump*` and `runner.dump.unwritable` check its errors.
`runner.usage.noargs` checks that a run without `--frames` is a usage error.
`host.convert` checks the colour conversion the runner and the libretro
adapter share (`host/convert.c`): each native pixel's low 9 bits pick the
table entry, and the input and output row pitches are honoured.
`header.c` and `header.cxx` compile the public header alone as C17 and as
C++ with warnings as errors and check its struct layout. `runner.sha256`
checks the runner's SHA-256 against the FIPS 180-4 example digests, and
`version.consistency` checks that `version.txt` matches the header's version
macros and the libretro `.info` file's `display_version`.
`cmake.script_policy` checks that every script in `tests/cmake` sets
`cmake_minimum_required(VERSION 3.25)`, so each CMake release reads it the
same way. `libretro.vendored`
checks that `libretro/libretro.h` is byte for byte the pinned upstream copy.
`libretro.host` loads the built libretro core at run time, calls it in the
order RetroArch does, and checks that the frame it receives equals the
runner's dumped image pixel for pixel, plus four colours written into the
test. `retroarch.compare` checks `compare_frame`, the tool the hosted `retroarch-e2e` job
uses to compare a screenshot with the runner's frame. It writes one picture as
a P6 image and as BMPs in each layout `sips` can produce (40, 108 and
124-byte headers, 24 and 32 bits per pixel, either row order), and requires
each to read back equal. It also requires a 257x240 or 256x239 image to fail
on size, one colour channel of pixel (17,200) off by one to be reported as
`first difference at 17,200`, and truncated or malformed files to be
rejected. `retroarch.compare.cli` runs `compare_frame` on an equal pair.
`palette.regen` rebuilds the colour table with `tools/palgen` and
checks that it matches the checked-in `src/palette_ntsc.c` byte for byte.
`vecconv.a9` converts the first three upstream `a9` (LDA immediate) tests in
`tests/vectors/fixtures` to the binary N65V form with `tools/vecconv` and
reads them back through the shared reader, which must decode exactly what was
written. It then runs them through the CPU with `cpu.vectors`, which links
the CPU object the library ships against a test bus of flat RAM that logs
every cycle, and requires each test's registers, RAM and every bus cycle
(address, value, read or write) to match: `65x02/a9: 0 of 3 vectors failed`.
`vectors.n65v` reads the committed sample `tests/vectors/65x02-sample.n65v`
through the same reader and requires 256 chunks of 100 tests, chunk k holding
opcode k, indexes 0 to 99 in each chunk, and nothing after the last chunk.
`cpu.vectors.<xx>` runs opcode xx's 100 sample tests through the CPU and
compares registers, RAM and every bus cycle, dummy reads included; it passes
only on exit status 0 and the exact line `65x02/<xx>: 0 of 100 vectors
failed`. A failure names the upstream test as `xx.json[i]` and the first field
that differs. Each test starts from zeroed RAM: after a pass the harness
zeroes the addresses the test listed, and after a failure all 64 KiB, since a
failing CPU may have written anywhere. `cpu.vectors.stray-write` runs two INC
tests that do not list their target address, the first with a wrong expected
A, and requires `65x02/e6: 1 of 2 vectors failed`: the first test's write
must not reach the second. The label `vectors` selects all 256 opcodes, `cpu.vectors.00`
to `cpu.vectors.ff`, with no opcode or vector skipped: the 151 official ones
in every addressing mode, the stable unofficial ones (NOP variants, LAX, SAX,
SLO, RLA, SRE, RRA, DCP, ISC, ANC, ALR, ARR, SBX and SBC `eb`), the twelve
JAM opcodes, and the unstable ANE, LXA, LAS, SHY, SHX, SHA and TAS. The CPU
has no decimal mode, as on the 2A03: SED and CLD set and clear D, PHP, PLP and
RTI keep it, and ADC and SBC stay binary. A JAM opcode makes eleven reads and
no write, and leaves the CPU jammed. ANE and LXA use a constant of `0xEE`,
the NTSC RP2A03G value, which the instance holds as part of its machine
profile.
`cpu.unit` covers what the sample cannot show, on the same CPU object and
test bus. A JSR at `0x017B` with S at `0x7D` pushes PCL over its own
high-address operand and must jump to the pushed byte: exactly six cycles,
ending at `0x0155` with S at `0x7B` (SingleStepTests 65x02 issue 18). After a
JAM opcode, each later step must be one read of `0xFFFF` with no register,
flag or PC change, and a JAM at `0xFFFF` must leave PC at `0x0000`.
`core.profile` creates an instance and requires both profile constants to be
`0xEE`.
`vectors.n65v.crafted` encodes a valid two-test buffer byte by byte and
requires the reader to reject, naming the byte offset at fault, every
truncation of it, bad magic, version 2, a cycle kind of 2, the wrong opcode, a
header count that disagrees with the tests present, a repeated index, and one
trailing byte. `vecconv.02` converts the first three tests of `02.json`,
written in the compact layout, and runs them through the CPU:
`65x02/02: 0 of 3 vectors failed`. `vecconv.a9_tail` converts three tests from
a prefix of `a9.json` cut inside the fourth; `vecconv.a9_tail.cut` asks the
same prefix for four and requires vecconv to fail. The fourteen
`vecconv.reject.*` tests each edit the `a9` fixture into one malformed input
(a fraction, a sign, an exponent, a leading zero, an address of 65536, a byte of 256, an
unknown cycle kind, an unknown or missing key, 256 cycles, an empty file, an
empty array or a short closed array without `--first`, a cut inside the third
test) and require vecconv to exit 1 with the byte offset.
`manifest.sha256` checks every line of `tests/roms/manifest.txt`: five
tab-separated fields, a pin that is a 40-digit commit or a release tag, a
licence, and a file whose SHA-256 equals the line's.
`manifest.sha256.selftest` lists a scratch file with a wrong hash and passes
only if the check reports it.
The `vectors-full` lane registers its tests only when the CMake option
`NESTURBATOR_VECTORS_FULL` is on, which its preset sets, so `ci` never touches
the network. `cpu.vectors-full.fetch` runs `tests/cmake/fetch_vectors.cmake`:
a sparse, blobless, depth-1 git fetch of upstream `nes6502/v1` at the commit
on the first line of `tests/vectors/pins.txt` (about 190 MB, a minute or two),
then a size and SHA-256 check of all 256 files against that file's lines. A
missing, extra or altered file fails with its path and both hashes, and so
does a run without the network; nothing is skipped. When every file already
verifies it fetches nothing. `cpu.vectors-full.00` to `cpu.vectors-full.ff`
each convert one whole file with `vecconv`, which then requires exactly 10000
tests, and run it through the CPU; each passes only on the line
`65x02/<xx>: 0 of 10000 vectors failed` and prints its failing-vector count
otherwise. That is all 2,560,000 upstream tests. `cpu.vectors-full.sample-match`
converts the first 100 tests of every fetched file and requires them to equal
the committed sample chunk by chunk, byte for byte, which proves where the
sample came from; a difference names the chunk and the blob offset.
`vectors.pins`, in `ci` and offline, checks `tests/vectors/pins.txt`: its pin
line, 256 lines in order from `00.json` to `ff.json`, sizes summing to
1,081,529,097 bytes, and a commit equal to the sample's manifest pin.
`vectors.pins.selftest` swaps two lines in a copy and passes only if the check
reports them as not sorted.
`vectors.fetch.guard` checks, offline, that the fetch deletes only what it
created: a `65x02-src` directory without the fetch's marker file is refused
and left in place, and a directory that reaches the source tree through a
symbolic link, or through a different letter case on macOS and Windows, is
refused as inside the source tree; the fetch compares paths after resolving
them to the links' targets and the case stored on disk. A refused directory
that does not exist yet is not created.
`bus.unit` checks the library's bus (`src/bus.c`) on its own: each read or
write advances time by 24 ticks, a byte written at `0x0001` reads back at
`0x0801`, `0x1001` and `0x1801`, a read outside RAM returns the last value on
the bus, and a write outside RAM changes no RAM byte.
`install.stage` installs the library into `build/ci/stage`, and
`install.consumer` builds `tests/consumer`, a separate project that includes
only `<nesturbator.h>`, against that install with `find_package`, then runs
one frame with it. `embed.subdirectory` builds `tests/embed`, a C-only
project that adds the source tree with `add_subdirectory`; it requires that
embedding enables no C++, adds no runner, libretro, `palgen`, `vecconv` or
host-helper target, adds exactly the targets `nesturbator_cpu` and
`nesturbator` and the helpers listed under "Using the library", and writes no
CPack configuration, then runs one frame.

## Continuous integration

`.github/workflows/ci.yml` runs on every pull request, every push to `main`
and on demand, with the same preset commands as above. The `build` job runs
`ci` on Linux and macOS and `ci-msvc` on Windows, each on x64 and arm64: six
platforms. Each leg also checks its three archives and writes native-frame
hashes for each pinned game's boot and the DABG port-0, port-1 and combined
input scripts:

```sh
cmake -DBUILD=build/ci -DOUT=hashes.txt \
      -DMOVIE_WRITER=build/ci/tests/runner.game_movie \
      -P tests/cmake/write_hashes.cmake
```

The `hygiene` job runs the `hygiene` lane, `asan` runs `asan` with Clang 18,
and `nofp` runs `nofp` with GCC 14 on Linux x64 and arm64. `title` requires
the pull-request title to be a Conventional Commit. `hash-equality` requires
the six `hashes.txt` files to be byte-identical, so a platform that computes
a different frame fails the run. It also requires six nonempty artifacts with
the exact 30-key game and movie inventory, rejecting duplicates, missing keys,
extra keys and malformed hashes. The branch rules require one check,
`CI required`, which passes only when every required job succeeded. Every
action is pinned to a commit SHA, and Dependabot proposes updates weekly.

The `dev`, `ci`, `ci-msvc` and `asan` test presets run CTest four tests at a
time (`execution.jobs`). The three long tests, `vectors.registration_policy`,
its self test and `runner.write_hashes`, carry a CTest `COST`, so a cold build
with no timing history starts them first.

`.github/workflows/nightly.yml` runs the `vectors-full` lane every night at
04:17 UTC on Ubuntu 24.04, on demand, on every push to `main`, and on pull
requests that change a file that can change what its tests run: the workflow,
the root `CMakeLists.txt`, `CMakePresets.json`, `tests/CMakeLists.txt`,
`tests/vectors/`, `tests/cpu/`, `tools/vecconv/`, `src/cpu.c`, `src/internal.h`,
`include/nesturbator.h` and the scripts `fetch_vectors.cmake`,
`vectors_full_run.cmake`, `vectors_sample_match.cmake` and
`vector_registration_policy.cmake` in `tests/cmake/`.
It is outside `CI required`, so a red nightly blocks no merge. Before the
full run, it saves the CTest JSON inventory and checks all 258 full-tier tests,
including their labels, fixtures and no-skip properties. The workflow then
runs the lane once and saves its JUnit result. Both files are uploaded together
as `vectors-full-evidence-<run ID>`; the job summary records the event, head
SHA, run URL and artifact name. This makes each main commit's exact run and
test evidence queryable by its run ID. The workflow fetches the full set at
its pin on every run, with no cache, because a cache used every night would
stop the fetch from ever being tested; offline, or with any file differing
from `tests/vectors/pins.txt`, it fails rather than skips. Scheduled and
main-push runs share one open issue labelled `nightly`: a failure opens it,
or updates it with the event, head SHA, run URL and failing keys
(`65x02/<xx>`, `fetch`, `sample-match`), and the next passing run closes it.
The `suite-flake` job builds the `ci` preset on Ubuntu 24.04 and runs its
suite three times in random order with
`ctest --preset ci --repeat until-fail:3 --schedule-random`; any failure fails
the job, and its outcome is reported in the same `nightly` issue. The ROM
loader fuzz outcome is recorded in the job summary and included in
scheduled and main-push failure issues. It uses GitHub's per-job token: the
full-run job has only `contents: read`, and only the report job has
`issues: write`. Checkout credentials are not persisted and the workflow
makes no commits.

After the Phase 2 merge, collect release, exact-commit main-push and first
post-merge scheduled vector evidence with this read-only command (replace the
SHA and tag with the merge commit and expected release tag):

```sh
scripts/phase2_outcomes.sh <phase-2-merge-sha> <release-tag>
```

It reports external events that have not happened yet as `PENDING` (exit 2);
completed failures return exit 1, and a fully passing report returns exit 0.
Only a published release and successful runs with matching commits and all
258 completed vector results are reported as `PASS`. It requires authenticated
`gh`, `jq` and CMake. Run `scripts/phase2_outcomes.sh --self-test` to exercise
the evidence parser without querying GitHub.

Locally, the lane keeps the fetched files in `build/vectors-full/vectors-full`.
To keep them somewhere that survives a clean build, set the cache variable
`NESTURBATOR_VECTORS_DIR` in an untracked `CMakeUserPresets.json`, for example
a configure preset that inherits `vectors-full`. The files go in its
`65x02-src` subdirectory, which the fetch replaces only when it holds the
marker file `.nesturbator-vectors` that the fetch wrote; any other
`65x02-src` there fails the fetch. Files fetched before the move to
`65x02-src` remain in `<dir>/src`, about 1 GB that the fetch never deletes;
delete that directory by hand. The fetch refuses a directory inside the
source tree other than under `build/`, after resolving symbolic links. To
move to a new upstream commit, regenerate `pins.txt` and the vector sample in one change. The
command below prints the new total size, which replaces 1,081,529,097 in
`tests/cmake/pins_check.cmake`:

```sh
cmake -DDIR=build/pins -DSOURCE_DIR=. -DGIT=git -DCOMMIT=<40-digit upstream commit> \
      -DWRITE_PINS=tests/vectors/pins.txt -P tests/cmake/fetch_vectors.cmake
```

## Using the library

Install the library, its header and its package files:

```sh
cmake --install build/ci --component library --prefix <dir>
```

This writes `include/nesturbator.h`, the static library under `lib`, a CMake
package under `lib/cmake/nesturbator` and `lib/pkgconfig/nesturbator.pc`.
Both package files find their paths relative to themselves, so the directory
can be moved. From CMake:

```cmake
find_package(nesturbator CONFIG REQUIRED)   # with CMAKE_PREFIX_PATH=<dir>
target_link_libraries(app PRIVATE nesturbator::nesturbator)
```

With pkg-config, set `PKG_CONFIG_PATH=<dir>/lib/pkgconfig` and use
`pkg-config --cflags --libs nesturbator`. The components `runner` and
`libretro` install `bin/nesturbator-run` and `cores/` with `info/`.

The source tree can also be embedded in another CMake project, which needs
only a C compiler:

```cmake
add_subdirectory(nesturbator)               # or FetchContent_MakeAvailable
target_link_libraries(app PRIVATE nesturbator::nesturbator)
```

An embedded build adds only the library and its `library` install rules,
plus four build helpers with the `nesturbator` prefix: the CPU object library
`nesturbator_cpu` that the library folds in, the CMake functions
`nesturbator_core_flags` and `nesturbator_warnings`, and the option
`NESTURBATOR_NOFP`. The runner, the libretro core, `palgen`, `vecconv`, the tests and the release
packaging are built only when nesturbator is the top-level project. On a system or
processor other than the six release targets, a top-level build still
configures and names its archives after what CMake reports, with a warning.

Every struct passed to or from the library starts with a `size` field. A
caller zeroes the struct with `memset`, then sets `size` to its `sizeof` and
fills the other fields. For `nesturbator_config` that is `size` and `abi`
(`NESTURBATOR_ABI_VERSION`); the zeroed allocator means `malloc` and `free`.
Zeroing first keeps padding and the bytes a newer header appends at zero, so
a host built against a newer header still runs with an older library.

Soft reset: `nesturbator_reset()` is the console's Reset button. Call it
between frames. It keeps CPU RAM and cartridge RAM (PRG RAM and CHR RAM), so
saves survive, and keeps A, X, Y and the host's input state. The controller
strobe and shift registers and a pending OAM DMA are cleared. The CPU sets the
I flag, lowers S by 3 and takes the reset vector in 7 CPU cycles, which count
in `ticks`. The PPU restarts at the top of the picture and ignores writes to
`$2000`, `$2001`, `$2005` and `$2006` until the end of the next vblank, 29,667
CPU cycles from the reset (the NESdev Wiki documents about 29,658); it keeps
`v`, the status flags, the OAM address and video memory. The APU is silenced as
by a write of 0 to `$4015`, its IRQs are cleared and the last `$4017` mode is
re-applied. Load is unchanged: there is no write-ignore window and no startup
sequence at power-on, so the soft-reset work leaves frame and audio hashes from
load unchanged. With no cartridge it does nothing and returns
`NESTURBATOR_OK`.

## Downloads and archives

Each release has three zip archives per platform, named
`nesturbator-VERSION-COMPONENT-OS-ARCH.zip`, where OS is `linux`, `macos` or
`windows` and ARCH is `x64` or `arm64`. Files sit at the zip root, with no
enclosing directory:

| Archive | Contents |
|---|---|
| `nesturbator-VERSION-libretro-OS-ARCH.zip` | `cores/nesturbator_libretro.{so,dylib,dll}`, `info/nesturbator_libretro.info`, `LICENSE`, `THIRD-PARTY-NOTICES.md` |
| `nesturbator-VERSION-runner-OS-ARCH.zip` | `bin/nesturbator-run` (`.exe` on Windows), `LICENSE` |
| `nesturbator-VERSION-library-OS-ARCH.zip` | `include/nesturbator.h`, the static library and package files under `lib/`, `LICENSE` |

Unzip the libretro archive into RetroArch's directory to put the core in
`cores` and its information file in `info`.

`cmake --workflow --preset ci` ends by writing the three archives for the
build machine to `build/ci/packages/`. This checks their names and contents:

```sh
cmake -DDIR=build/ci -P tests/cmake/check_archives.cmake
```

## Releases

Merging a behaviour-changing pull request publishes a GitHub release.
release-please reads the Conventional Commit titles on `main`, opens a release
pull request that sets the version and the changelog, and merges it once CI
passes. The version comes from the Conventional Commit titles alone: before
1.0 a `feat:` raises the patch number, and `release.no_release_as` keeps a
one-time `release-as` pin from staying behind in
`release-please-config.json`. `.github/workflows/release.yml` then runs the full CI on that exact
commit and publishes the release with 18 archives (library, runner and
libretro for six platforms) and `SHA256SUMS`. `SHA256SUMS` carries a
build-provenance attestation that covers every archive. To check a download:

```sh
gh attestation verify FILE --repo szTheory/nesturbator
```

The `release.nonbehavioral_policy` check requires the `needs:` and `if:` lines
of the publish and ci jobs to equal fixed strings, so an added condition such
as `|| always()` fails the suite.

Before publishing, the workflow requires exactly the 18 expected archive
names, verifies each archive this way, and confirms that a copy with one byte
changed fails. Pull-request rules are in [CONTRIBUTING.md](CONTRIBUTING.md);
security reports go through [SECURITY.md](SECURITY.md).

## The colour table

`src/palette_ntsc.c` maps each native pixel value to calibrated sRGB in
XRGB8888. The existing offline `tools/palgen` host tool decodes the NTSC signal
levels, phases and emphasis rules from the cited NESdev Wiki revisions, applies
the 2.4 transfer curve and 525-line BT.601 primaries, converts linear RGB to
sRGB/D65, clips out-of-gamut channels and rounds to 8 bits. The BT.601
chromaticities and transfer curve are from the [ICC BT.601 registry](https://registry.color.org/rgb-registry/bt601);
the sRGB primaries and transfer curve follow the [W3C sRGB specification](https://www.w3.org/Graphics/Color/srgb).
`nesturbator_get_palette` returns this display table; it never affects native
frame hashes. Do not edit the table by hand; change palgen and regenerate:

```sh
build/ci/tools/palgen/palgen src/palette_ntsc.c
```

The `ci` build puts the library at `build/ci/libnesturbator.a` and the runner
at `build/ci/runner/nesturbator-run`. The public header is
`include/nesturbator.h`.

## The runner

`nesturbator-run` runs the core without a window and accepts a mapper 0, 2, 3 or 7 image
with `--rom FILE`. The loader validates the entire image before allocating
cartridge state. It rejects unsupported mapper, console, region, RAM and ROM
geometries, truncation, trailing bytes, and images larger than 64 MiB; the
runner prints a diagnostic and exits nonzero for rejected content.

```sh
nesturbator-run --frames N [--rom FILE] [--hash-frame N]... [--hash-audio] [--dump-frame N:FILE]...
```

- `--frames N` runs N frames (N is 1 or more). It is required; without it
  the runner prints its usage and exits 2.
- `--rom FILE` loads a bounded iNES 1.0 or NES 2.0 image. Accepted
  geometry is 8 KiB CHR ROM or declared 8 KiB CHR RAM with 16 or 32 KiB PRG
  for mapper 0 (NROM), or PRG in 16 KiB banks up to 4 MiB for mapper 2
  (UxROM, submappers 0 to 2), or 16 or 32 KiB PRG with 8, 16 or 32 KiB CHR ROM
  for mapper 3 (CNROM, submappers 0 to 2), or 32 to 256 KiB PRG in 32 KiB banks
  with 8 KiB declared CHR RAM for mapper 7 (AxROM, submappers 0 to 2); optional trainers are included in the validated
  file length. UxROM writes to `$8000-$FFFF` select the bank at `$8000`; the
  last bank stays at `$C000`. Submappers 0 and 2 AND the written value with
  the ROM byte under the write (submapper 0 is the project default), and
  submapper 1 takes it raw. CNROM writes to `$8000-$FFFF` select the 8 KiB
  CHR bank with the same bus-conflict rule (submappers 0 and 2 AND, submapper
  1 raw); writes to CHR ROM are ignored. AxROM writes to `$8000-$FFFF` select the
  32 KiB PRG bank (bits 0 to 2) and the single-screen nametable page (bit 4);
  the header mirroring bit is ignored, the reset vector comes from bank 0, and
  only submapper 2 ANDs the value with the ROM byte under the write (submapper
  0 has no bus conflict).
- `--hash-frame N` prints a line after frame N has run. N must be between 1
  and the `--frames` value. The option can be repeated.
- `--hash-audio` prints one hash for all mixed-level transitions and one for
  all signed 16-bit PCM samples emitted by the requested run. It also works
  with `--movie` and AccuracyCoin page mode; a no-cartridge run hashes an empty
  transition stream and its silent PCM bytes.
- `--dump-frame N:FILE` writes frame N to FILE as a binary PPM (P6), 256x240,
  in the RGB of the colour table. N follows the `--hash-frame` rules, and the
  option can be repeated. The image is converted by `host/convert.c`, the same
  loop the libretro adapter uses, so both show the same colours.

Each hashed frame prints one line:

```
frame <N> ticks <ticks> sha256 <64 lowercase hex digits>
```

`ticks` counts emulated time in half master-clock periods, 714732 per NTSC
frame. The hash is SHA-256 over the 256x240 native pixels, each a 16-bit
value written low byte first, row by row from the top row. The same inputs
give the same hash on every platform. For the test card:

```
$ nesturbator-run --frames 1 --hash-frame 1
frame 1 ticks 714732 sha256 b49e9be44573a4de82d845179d4389a0a6e28e934bd9ab31516a40db2c0b0453
```

Audio hashes can be checked without an audio device:

```
$ nesturbator-run --frames 1 --hash-audio
audio transitions sha256 e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
audio pcm sha256 bfd6d535131a45e8a31340082147c4928edd44f2e05fe127081a1900b3cc0edc
```

The hash is always over the native pixels, never over a dumped image:

```
$ nesturbator-run --frames 1 --dump-frame 1:frame1.ppm --hash-frame 1
frame 1 ticks 714732 sha256 b49e9be44573a4de82d845179d4389a0a6e28e934bd9ab31516a40db2c0b0453
```

writes `frame1.ppm` (184335 bytes) and prints the same line as before.

Exit status: 0 done, 1 failure (including a FILE that cannot be written),
2 usage error.

## The libretro core

`libretro/` builds the core that libretro frontends such as RetroArch load.
The file is named `nesturbator_libretro` with no `lib` prefix:

| OS | File |
|---|---|
| macOS | `nesturbator_libretro.dylib` |
| Linux | `nesturbator_libretro.so` |
| Windows | `nesturbator_libretro.dll` |

The `ci` build puts it at `build/ci/libretro/`. It exports only the 25
`retro_*` functions of `libretro.h`.

With no content, the core shows the test card and sends silence: in RetroArch,
use "Start Core", or launch it with `-L` and no content path. The public core
returns mono signed 16-bit PCM. The libretro adapter duplicates each sample to
left and right at 48000 Hz, preferring one batch callback per frame and using
the single-sample callback only when no batch callback is registered. When both
callbacks are registered, only the batch callback receives audio.
`libretro.host` checks both callback paths against direct core PCM without an
audio device.

RetroArch's Reset (`retro_reset`) runs `nesturbator_reset()`, the console's
soft reset, which keeps RAM and cartridge RAM; it does not unload or reload the
game. With no content it changes nothing. `libretro.host` compares the frames
after a reset with those of a direct-API instance given the same frames, reset
and frames.

`libretro/nesturbator_libretro.info` is the core information file. It goes in
RetroArch's `info` directory beside the core in `cores`, and declares
`supports_no_game = "true"`, which lets RetroArch start the core without
content.

`libretro/libretro.h` is the libretro API header, copied unchanged from
RetroArch; its source and licence are in
[THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).

## Install in RetroArch (Apple Silicon)

With RetroArch installed (see below), these two lines put the released core
in RetroArch's `cores` directory and its information file in `info`. The
first sets the version; the second downloads and extracts:

<!-- x-release-please-start-version -->
```sh
NESTURBATOR_VERSION=0.1.7
mkdir -p ~/Library/Application\ Support/RetroArch && curl -fsSL "https://github.com/szTheory/nesturbator/releases/download/v$NESTURBATOR_VERSION/nesturbator-$NESTURBATOR_VERSION-libretro-macos-arm64.zip" | tar -xf - -C ~/Library/Application\ Support/RetroArch cores info
```
<!-- x-release-please-end -->

Then start RetroArch, choose Load Core → nesturbator, then Start Core: the
test card appears.

Each release pull request updates the version on the first line; the first
release is v0.1.0. The version sits on a line of its own because
release-please changes only one version on each marked line. Naming `cores info` leaves the archive's `LICENSE` and
`THIRD-PARTY-NOTICES.md` out of RetroArch's directory. Files fetched with
`curl` carry no quarantine flag, so macOS loads the core without a prompt.
This checks the lines' extraction against the archive a build writes:

```sh
cmake -DREADME=README.md -DPACKAGES=build/ci/packages -DOUT=build/ci/install-line -P tests/cmake/check_install_line.cmake
```

It takes the `tar` arguments from the second line above, pipes the zip through them
on standard input as `curl` would, and requires exactly
`cores/nesturbator_libretro.dylib` and `info/nesturbator_libretro.info`. It
also requires the `NESTURBATOR_VERSION` line to equal `version.txt`, and the
URL to take both the release tag and the file name from
`$NESTURBATOR_VERSION`. It runs on macOS,
whose `tar` reads a zip from standard input, and on the macOS arm64 leg of
continuous integration. On every platform, the `ci` preset's
`release.one_version_per_line` test fails, naming file and line, if any line
inside an `x-release-please` block of a file release-please updates holds
more than one version.

## Try it in RetroArch (from a build)

On an Apple Silicon Mac, install RetroArch from `RetroArch_Metal.dmg` on the
RetroArch website, or with `brew install --cask retroarch-metal`. The
`retroarch` cask is the Intel-only build, which cannot load an arm64 core.
After `cmake --workflow --preset ci`, start the core with no content to see
the test card:

```sh
/Applications/RetroArch.app/Contents/MacOS/RetroArch -L build/ci/libretro/nesturbator_libretro.dylib
```

RetroArch's picture is checked by the hosted `retroarch-e2e` CI job, not by a
local test. It installs the pinned RetroArch 1.22.2 on a macOS runner, runs the
core on the manifest-listed Nesteroids image, and compares RetroArch's
screenshot with the runner's frame; this is where RetroArch is checked.

The official RetroArch v1.22.2 macOS release is
[`RetroArch_Metal.dmg`](https://buildbot.libretro.com/stable/1.22.2/apple/osx/universal/RetroArch_Metal.dmg),
universal for arm64 and x86_64, with measured SHA-256
`81b79121ba26d539064ae13b4d0419a120c3d165afbe656cf5f5412b15fdb434`. The
required `retroarch-e2e` job downloads this asset on `macos-15`, verifies its
checksum and exact version, loads Nesteroids, and compares the nonempty frame-60
screenshot pixel for pixel with the runner's output. It isolates RetroArch's
first-run home under the build tree and retains the asset evidence, screenshot,
and runner frame in the `retroarch-e2e-frames` artifact. Missing assets, changes
to the real home directory, startup errors, and frame mismatches fail the
required CI check. No manual screenshot or gameplay check is needed.

## What it will be

- An accuracy-class, deterministic NES core with no GUI of its own.
- Three deliverables: a C library, a command-line runner, and a libretro core
  for frontends such as RetroArch.
- All emulation code written here, under the MIT licence.

## ROMs

This repository contains no commercial ROM or BIOS data and never will. You
supply your own legally obtained game images. See
[ASSET_POLICY.md](ASSET_POLICY.md).

The CPU test data under `tests/vectors/` is MIT data from
[SingleStepTests 65x02](https://github.com/SingleStepTests/65x02): the sample
`65x02-sample.n65v` and three small JSON fixtures. Each is listed in
`tests/roms/manifest.txt` with its source, pin, licence and SHA-256, and its
licence notice is in [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).

### Regenerating the vector sample

The sample holds the first 100 tests of each of the 256 upstream files. To
rebuild it at an upstream commit, from a configured `ci` build:

```sh
cmake -DCOMMIT=<40-digit upstream commit> -DVECCONV=build/ci/tools/vecconv/vecconv \
      -DWORK=build/ci/vectors-regen -DOUT=tests/vectors/65x02-sample.n65v \
      -P tests/cmake/vectors_regen.cmake
```

It downloads the first 64 KiB of each file (about 16 MB), converts 100 tests
from each with `vecconv`, joins the chunks in opcode order, and prints the
file's size, its SHA-256 and the line for `tests/roms/manifest.txt`. Moving to
a new upstream commit is a change of its own, with the new manifest line.

## How the code is written

From hardware documentation and public test ROMs, with AI coding assistants,
and without copying from other emulators. See [PROVENANCE.md](PROVENANCE.md).

## Licence

MIT. See [LICENSE](LICENSE).

nesturbator is not affiliated with or endorsed by Nintendo. The names "Nintendo
Entertainment System", "NES" and "Famicom" are used here only to say what the
core emulates.
