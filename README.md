# nesturbator

A NES emulator core in C: a library you can embed, a headless runner for
automation, and a libretro adapter.

**Status: v3.** The core plays mappers 0, 1, 2, 3, 4 and 7 with battery saves and the soft reset
(`nesturbator_reset()`, which RetroArch's Reset button runs). v3 adds save states in the library
and the runner (`nesturbator_save_state`, `--state-at-frame`, `--load-state`, `--hash-state`),
rewind and run-ahead in RetroArch (the core declares the `deterministic` save state level), a
runner stop contract (SIGINT and SIGTERM, Ctrl+C and Ctrl+Break finish the frame and the save,
and an exit table), frame timing (`--bench`, `--bench-state`) and a `perf` gate that fails a pull
request whose per-frame instruction count rises by more than 1%. A policy still forbids any
registered test to skip. The 6502 core matches the public
65x02 test vectors on every opcode and bus cycle. The library, runner and
libretro core accept bounded iNES 1.0 and NES 2.0 images for mapper 0 (NROM,
16 or 32 KiB PRG), mapper 1 (MMC1, with battery saves; see below) and mapper 2 (UxROM), with 8 KiB CHR ROM or declared CHR RAM, mapper 3 (CNROM, 8 to 32 KiB CHR ROM), mapper 4 (MMC3; see below) and mapper 7 (AxROM, CHR RAM); the PPU renders backgrounds
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
board), with NROM, MMC1, UxROM, CNROM, MMC3 and AxROM as the boards so far. The behaviour revision is 6: the PPUCTRL NMI edge
changed the frames of games that rewrite `$2000` inside their NMI handler
(revision 5's PPU fetch pipeline changed the frames of games that write the
scroll or PPUCTRL while rendering). Mid-frame scroll writes render as on
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
It also runs Holy Mapperel's mapper 1, 2, 3, 4 and 7 test ROMs (zlib), among them the
mapper 4 ROMs `M4_P128K_CR8K` (TNROM-like) and `M4_P256K_C256K` (TxROM-like),
and requires each to report the result code 0000 with `PRG RAM MISSING`. Two derived
copies of `M4_P128K_CR8K` (header byte 10 patched, plus the battery bit for the second)
prove the MMC3 `$A001` PRG RAM enable and write protect: they report `8K PRG RAM OK`,
the battery copy also `+ BATTERY` on its second run. The derived files are built at
build time from the committed ROM after its sha256 is checked, and are never committed. Their boot hashes and scripted DABG two-port movie hashes are checked against
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
flag. A `$2000` write raises an NMI only when bit 7 changes from 0 to 1 while
the vblank flag is set; rewriting it with bit 7 still set does not, and
clearing bit 7 suppresses a pending NMI. Odd NTSC frames skip pre-render dot 340 when rendering is enabled.
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
tick of the dot, including `v` itself while rendering is off; on a visible
rendering line dot 0 shows the first tile's pattern address, and on the
pre-render line it shows `v`, so with the background at `$1000` the first
rise there is at dot 5. Sprites are evaluated into secondary OAM
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
cmake --workflow --preset mmc3-oracle  # blargg's MMC3 test ROMs, fetched at their pin (network)
```

On Windows with MSVC, use `ci-msvc` from a developer command prompt.

## Checks

Each lane is one command, `cmake --workflow --preset <lane>`.

| Lane | What it proves |
|---|---|
| `ci` | Release build with warnings as errors; every test passes, the library installs and builds a separate consumer, and the three release archives are written |
| `ci-msvc` | The same as `ci`, built with MSVC on Windows |
| `asan` | The same tests pass under AddressSanitizer and UBSan, with any report fatal, except that `NESTURBATOR_SHORT_RUNS` leaves `runner.write_hashes`, `runner.boot_to_play` and the two `vectors.registration_policy` tests to the `ci` lanes, sweeps fewer reset points in `core.reset`, and compares the non-save Holy Mapperel parity runs longer than 100 frames at frame 100 |
| `nofp` | The core builds with `-mgeneral-regs-only` and passes the tests labelled `abi` |
| `hygiene` | The tree holds no personal data and no unlisted ROM or binary file, every GitHub Action is pinned to a commit, and the C sources are formatted |
| `vectors-full` | The full 65x02 vector set, fetched by git at the commit in `tests/vectors/pins.txt` and checked file by file, matches the CPU on every test; needs the network and fails without it |
| `mmc3-oracle` | blargg's `mmc3_test_2` and `mmc3_irq_tests` ROMs, fetched by git at the commit in `tests/mmc3/pins.txt` and checked by SHA-256, give the result `tests/mmc3/oracle.txt` records for each; needs the network and fails without it |

Tests never skip: a test that cannot run somewhere is not registered there, and
`policy.no-skip` fails when any registered test could report itself skipped or
is disabled.

`fuzz.regress` replays checked-in malformed cartridge seeds through the public
cartridge load/unload lifecycle on every CI platform. The Linux nightly builds
that same entry point with Clang libFuzzer and ASan/UBSan, then runs it for a
bounded minute. The corpus is manifest-listed and local; CI does not download
ROMs or fuzz seeds.

`fuzz.state.regress` does the same for the save-state loader: it replays the
committed seeds in `tests/fuzz/corpus_state` through `nesturbator_load_state` on
every CI platform. Each seed is a few bytes, not a state. The harness picks one
of twelve synthetic carts (six boards, each with its smallest and largest PRG and
CHR) built from an integer generator, so the corpus holds no ROM bytes, saves a
state, patches it, repairs the fields a load checks first and its checksum, and
loads the result. A refused load must leave the instance unchanged; an accepted
one must run, save and reload, give the same frames and audio in two instances,
and save back to the loaded bytes with the library's behaviour revision.
`fuzz.state` requires every committed seed to equal what the harness generates
(`fuzz.state_loader --write-seeds DIR` writes them and prints their manifest
lines) and every `valid-*` seed to load. The same source builds as the libFuzzer
target `fuzz.state.libfuzzer` when `NESTURBATOR_LIBFUZZER` is on.

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
`runner.write_hashes` runs each pinned game (Nesteroids, DABG, RHDE and
nes-runner), the three scripted DABG two-port movies, the scripted nes-runner
play movie and every Holy Mapperel ROM. It writes ordered native hashes at
frames 1, 30, 60, 120 and 180 plus transition and PCM hashes for each game's
boot run, nes-runner play frames 1, 30, 60, 120, 180, 300, 450 and 600 plus
their audio hashes, and for each Holy Mapperel ROM the result-screen frame and
both audio hashes from the same run. It fails if any requested frame or audio
hash is missing or duplicated, and unless the keys it generated equal the
expected list exactly, naming the missing and the extra keys.
`runner.write_hashes.content` requires all 114 sorted keys to equal
`tests/runner/hashes.txt` byte for byte, with LF line endings only.
The 114 keys are 28 game boot keys (four games, five frames and two audio keys
each), 15 DABG movie keys, 10 nes-runner play keys, and for Holy Mapperel one
`holymapperel/<key>/frame N` key plus two `holymapperel/<key>/audio/...` keys
per ROM (twelve) and per second run (`<key>.saved` for `m1sxrom`, `m4tkrom`),
which is 14 frame keys and 28 audio keys, plus 19 state keys: the
`state_format` row, and for `dabg`, `m1sxrom`, `m2`, `m3`, `m4tkrom` and `m7`
the state hash at frames 0 and 60 (`state/<board> N ticks T sha256 H`, from
`--hash-state`) and the state size (`state/<board>/size S`). The state pins
make a change to the saved state's bytes, size or format version show as a
changed row. `state.format_pin` fails unless the `state_format` row equals
`NESTURBATOR_STATE_FORMAT_VERSION` in `include/nesturbator.h`, and
`runner.state.every_frame` saves a state after every frame from 0 to 180 of
`dabg` and requires frames 1, 30, 60, 120 and 180 to keep their pinned hashes.
The list is written once, in
`tests/cmake/hash_keys.cmake`, which the writer, the CI hash-equality
validator and its self-test all read, so no row count is spelled out anywhere.
`runner.write_hashes` hashes frame N and frame 2N of each Holy Mapperel ROM and
fails if they differ, so a pinned result screen is known to be static.
`policy.rom_hash_coverage` fails when a committed ROM under `tests/roms` or
`tests/roms/hm` is neither hashed nor exempt, or when a ROM glob or list is
empty, and `policy.rom_hash_coverage.self_test` proves both rejections.
AccuracyCoin is the one exemption: its NROM board is hashed through three
games and its scoreboard runs on every leg.
Every other committed ROM (Holy Mapperel's ten files and two derived copies,
Nesteroids, DABG, RHDE and nes-runner) has frame and audio rows that must be
byte-identical on six platforms. `libretro.parity.<key>` runs each of them
through the libretro adapter and requires its last frame to equal the runner's,
for nes-runner both the boot and its played movie. For the two battery boards,
`libretro.parity.m1sxrom.run1`, `.run2`, `libretro.parity.m4tkrom.run1` and
`.run2` also round-trip the save: run 1's save span must equal the runner's
`.sav`, and run 2 loads that file into the span before its first frame and
must end with the span the runner's second run saved. The fuzz seeds are exempt
because they are crash inputs, not pinned pictures, and AccuracyCoin is exempt
as above. Six-platform equality proves the platforms agree with each other, not
that the emulation is right; correctness comes from Holy Mapperel's result
codes, the AccuracyCoin scoreboard and the nightly blargg lane.
`runner.boot_to_play` is the boot-to-play proof for nes-runner v1 (MIT,
mapper 1). It replays generated NMOVIE1 movies through the library, reads
`$3E` (game state) and the score bytes `$5D` to `$60` with
`nesturbator_peek_cpu_ram`, and judges three predicates: RAM (the game is in
state 2, the score rises and reaches 36, and RAM first differs from a no-input
run no earlier than the first input frame and at most one frame after it),
scene (changed pixels and at least half the picture differ from the no-input
run) and audio (every window is loud and differs from the no-input run). The
no-input, menu-navigation, failed-load and Start-only controls must each fail
it with an exact mask. Synthetic mutants (silence, constant tone, a click then
silence, a static picture with a ticking RAM counter, a rainbow-only title, an
NMI-alive hang, early death, input latched late) and boundary traces on both
sides of every threshold are judged without running the emulator and must each
return their expected mask. The check also reads its save: the battery span
starts `5A 6F` and the save generation advances.
The `holymapperel.*` tests read Holy Mapperel's result from the frame, not
from RAM. `holymapperel.<key>.dump` runs a ROM to its frame N and writes the
frame with `--dump-frame`; `holymapperel.<key>.decode` runs the test-only
`holymapperel-decode` on that image and passes only on the line
`holymapperel: code 0000 (`. The decoder exits 0 for the code 0000, 1 for any
other code (it prints what each digit means: WRAM, PRG window bitmask, IRQ,
CHR; C0DE means the driver never finished) and 2 when the image has no result
screen, a digit cannot be read, or the file is not a 256x240 P6 image.
`holymapperel.decode.unit` paints frames to test every decoder branch,
`holymapperel.glyphs` checks the decoder's font against the M3 ROM's CHR, and
`holymapperel.decode.testcard`, `.early` and `.badsize` check that the test
card, frame 1 and a wrong-size file exit 2.
The five MMC1 ROMs are `M1_P128K_CR8K` (SGROM, `PRG RAM MISSING`),
`M1_P128K_C32K_W8K` (SJROM) and `M1_P128K_C128K_S8K` (SKROM) and
`M1_P512K_CR8K_S8K` (SUROM), all `8K PRG RAM OK`, and `M1_P512K_CR8K_S32K`
(SXROM). Each ROM also has a `holymapperel.<key>.prgram` test: the decoder's
`--prg-ram` mode reads the PRG RAM row of the result screen and the test
passes only on its exact text, with the closing quote as the anchor. The SXROM
ROM runs a two-run chain on one save directory with `nesturbator-run
--save-dir`: run 1 empties the directory before it starts, then reads
`0000` and `32K PRG RAM OK` without `+ BATTERY`, `run1.sav` checks the 32,768
byte save for Holy Mapperel's `SAVEDATA` text at offset 0x100
(`tests/cmake/check_sav.cmake`), and run 2 reads `0000` and `32K PRG RAM OK +
BATTERY`, which shows the board loaded its own save. The two runs' screens hash
differently and both are pinned. A new board ROM is one entry and one manifest
line in `tests/holymapperel/roms.cmake`, whose entry is
`<key>:<file>:<N>:<PRG RAM text>` with an optional `:save` field for the
two-run chain, then an optional save span size (default 32768) and an optional
`<offset>=<hex>` that adds a `run1.sav_byte` check on the first run's save;
a `<file>` written `derived/<name>` names a copy that `holymapperel-derive`
builds at build time from a committed ROM listed in
`NESTURBATOR_HOLYMAPPEREL_DERIVED`, with `tests/cmake/hm_derive.cmake`
refusing a base whose sha256 differs from the manifest
(`holymapperel.derive.refuse`). `m4tkrom` is the battery copy: its 8,192 byte
save must hold the write-protected byte 0xB6 at offset 0 (an emulator that
ignored write protect would leave 0x6B) and `SAVEDATA` at 0x100; the tests, `tests/cmake/write_hashes.cmake` and
`tests/cmake/hash_inventory.cmake` all read it.
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
macros and the libretro `.info` file's `display_version`. `runner.version`
runs `nesturbator-run --version` and requires stdout to be exactly
`nesturbator-run` and the text of `version.txt` (read at test time), empty
stderr and exit 0. `core.boundary` checks, after every frame, that the
instance holds no video or audio pointer of the caller and no buffered PCM,
over test-card frames, 60 frames with sound, a JAM, a too-small buffer and an
unload and reload.
`cmake.script_policy` checks that every script in `tests/cmake` sets
`cmake_minimum_required(VERSION 3.25)`, so each CMake release reads it the
same way. `libretro.vendored`
checks that `libretro/libretro.h` is byte for byte the pinned upstream copy.
`libretro.info_claims` parses `libretro/nesturbator_libretro.info` and requires
the exact capability keys and pinned values, a `savestate_features` whenever
`savestate = "true"`, and no duplicate or unknown key; its self-test feeds it
bad files. `libretro.info_claims.self_test` also configures a scratch project to
show the guard in `tests/CMakeLists.txt`: configuring fails when the `.info`
claims a save-state level whose proving tests are not registered.
`libretro.no_quirks` is described with the libretro core above.
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
`retroarch.frame_index` dumps frame 37 of Nesteroids with the runner and requires
`frame_index`, the tool that finds a screenshot among the runner's frames 1..W
by RGB, to report 37; `retroarch.frame_index.dups` requires every repeated
frame in 1..1200 to have its partner at most 3 frames before it.
`retroarch.log_check` and its `.serialized`, `.unlisted_error`, `.rewind_warn`,
`.level_serialized`, `.level`, `.no_netcmd`, `.failed_load` and `.uncommented`
variants run `tests/retroarch/log_check.cmake` on fixture logs: a clean log
passes, and an unlisted `[ERROR]` line, a `[Rewind]` warning, a line allowlisted
only for `serialized` when checked at `deterministic`, a missing `[NetCMD]` line,
a failed state load and an allowlist entry without its comment all fail.
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
The `mmc3-oracle` lane registers its tests only when the CMake option
`NESTURBATOR_MMC3_ORACLE` is on, which its preset sets, so `ci` never touches
the network. It uses blargg's MMC3 ROMs from
`https://github.com/christopherpow/nes-test-roms` at commit
`95d8f621ae55cee0d09b91519a8989ae0e64753b` (`95d8f62`). That repository states
no licence, so the ROMs are fetched and never committed, and no text of theirs
is in the tree. `mmc3.oracle.fetch` runs `tests/cmake/fetch_blargg_mmc3.cmake`:
a sparse, blobless, depth-1 fetch of `mmc3_test_2` and `mmc3_irq_tests`, then a
size and SHA-256 check of the 12 ROMs listed in `tests/mmc3/pins.txt` and a
check that no other `.nes` file is there. It deletes only a directory it
created, which holds the marker `.nesturbator-mmc3`; `mmc3.fetch.guard`, in
`ci` and offline, shows it refuses a foreign directory, a symlink into the
source tree and an upper-case spelling of it. The test-only program
`mmc3-oracle` (`tests/mmc3/oracle.c`) judges a ROM by one of blargg's two
result protocols. `mmc3_test_2` writes the signature `DE B0 61` at
`$6001-$6003`, `$80` at `$6000` while it runs, then a result there (`0` is a
pass) and text from `$6004`; the oracle passes it only after it has seen `$80`,
read `0` and found `Passed` in the text. `mmc3_irq_tests` leave `1` in zero page
`$F8` when they pass; the oracle requires `1` at the frame budget and again 60
frames later. Its exit status is 0 for a pass, the reported value for a
failure, 120 for a ROM that never reported, 121 for no signature, 122 for an
image that does not load and 123 for bad usage; it never exits 77.
`tests/mmc3/oracle.txt` has one tab-separated row per ROM,
`key status code frames hash`: the key is the suite and ROM name such as
`mmc3_test_2/1-clocking`, the status is `pass` or `unsupported`, the code is
the expected exit status as `0xNN`, and frames is the budget; the hash is `-`.
Each row becomes the test `mmc3.oracle.<key>`, which passes when the exit
status equals the code, and an `unsupported` row that exits 0 fails until the
row is edited. These rows are not part of `tests/accuracy/scoreboard.txt`.
The table holds the 12 ROMs, 10 `pass` and two `unsupported`. All six
`mmc3_test_2` ROMs and the four Sharp-revision `mmc3_irq_tests` ROMs pass: `1-clocking`,
`2-details`, `3-A12_clocking`, `4-scanline_timing`, `5-MMC3`, and `1.Clocking`, `2.Details`,
`3.A12_clocking`, `4.Scanline_timing`, `6.MMC3_rev_B`. `mmc3_test_2/6-MMC3_alt` (exit
`0x02`) and `mmc3_irq_tests/5.MMC3_rev_A` (exit `0x03`) are recorded `unsupported`: they test
the NEC revision, and the core models the Sharp revision unless the cartridge header says
submapper 4. The board adds no IRQ delay beyond the A12 edge. In `ci`, offline,
`mmc3.pins` checks `tests/mmc3/pins.txt` (a header with a 40-hex commit and `none-stated`, then 12
lines of a 64-hex hash, path and decimal size, LF only), `mmc3.oracle.inventory` checks that
`oracle.txt` holds exactly the 12 pinned keys in the five-field form with exactly the two
NEC-revision keys `unsupported`, and `mmc3.oracle.unit` runs the oracle's judgement on
synthetic memory (a pass, a timeout, no signature, a result with `$80` never seen, text
without `Passed`, and a `$F8` that changes within 60 frames).
Run it with `cmake --workflow --preset mmc3-oracle`.
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
a different frame, or a different state, fails the run. It also requires six nonempty artifacts with
the exact 114-key inventory, state pins included, from `tests/cmake/hash_keys.cmake`, checking the key
set of every artifact, and rejecting duplicates, missing keys, extra keys and
malformed hashes. The branch rules require one check,
`CI required`, which passes only when every required job succeeded. Every
action is pinned to a commit SHA, and Dependabot proposes updates weekly.

The `dev`, `ci`, `ci-msvc` and `asan` test presets run CTest four tests at a
time (`execution.jobs`). The three long tests, `vectors.registration_policy`,
its self test and `runner.write_hashes`, carry a CTest `COST`, so a cold build
with no timing history starts them first. The `asan` lane does not register them
(`NESTURBATOR_SHORT_RUNS`, which only that preset sets).

The `perf` job runs on every pull request on ubuntu-24.04. It builds the pull request's merge
commit and its first parent (the tip of `main`, which ENGINEERING section 5 calls the merge
base) with the `ci` preset, runs each under Cachegrind for 60 and 360 frames on nesteroids
(NROM), runner (MMC1) and M4_P256K_C256K (MMC3), and reports instructions per frame as
(Ir(360) - Ir(60)) / 300 in the job summary and the job log. The job is part of CI required and fails a
pull request when any ROM's instructions per frame rise by more than 1.00%, unless
tests/perf/allow.txt carries a valid line for that ROM; an unstable base (two base measurements
differ) always fails; a base that cannot be built or run shows `n/a` and does not fail the pull
request. Wall time is never measured or gated. The `perf.roms_render` test shows that
all three ROMs draw a non-blank frame 60, so the job never measures an idle PPU.

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
the job, and its outcome is reported in the same `nightly` issue.

The same workflow runs the `mmc3-oracle` lane (`cmake --workflow --preset
mmc3-oracle`) as a second job, built like `vectors-full`: `contents: read`
only, pinned actions, no cache and no skip code. It saves its CTest inventory
first, runs the 12 blargg MMC3 rows, uploads the inventory and its JUnit file as
`mmc3-oracle-evidence-<run ID>`, and feeds the same `nightly` issue, whose body
lists its failing keys (`mmc3_test_2/<name>`, `mmc3_irq_tests/<name>`,
`mmc3.oracle.fetch`). Pull requests that change `tests/mmc3/`, its fetch, run
and inventory scripts, `src/mapper_mmc3.c`, `src/mapper.h`, `src/ppu.c`,
`src/cartridge.c` or `src/bus.c` run it too. The hygiene test
`hygiene.nightly_workflow_policy` checks each lane on its own job text, and its
self test mutates both lanes. The nightly `state-fuzz` job runs
`fuzz.state.libfuzzer` for 60 s on the committed `tests/fuzz/corpus_state` and
reports in the rolling nightly issue; a crash it finds becomes a `regress-`
seed in the fixing PR. The ROM
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


### Performance allow file

`tests/perf/allow.txt` lists reviewed exceptions to the performance gate, which fails a ROM whose
instructions per frame grow by more than 1.00%. A line reads
`<rom> <max_percent_x100> <expires YYYY-MM-DD> <reason...>`, for example
`nesteroids 350 2026-12-31 PPU timing fix, see #NN`. The cap is a percent times 100 (350 is
3.50%), and the expiry is at most 60 days after the day the check runs; an expired line, a
malformed line, a duplicate, an unknown ROM and an empty reason each fail with their own message.
A line is reviewed in the pull request that adds it and is removed after that pull request
merges. The summary table prints the ROM, the cap and the expiry of an applied line, never the
reason. A base measurement that differs between its two runs prints `UNSTABLE` and fails whatever
the allow file says. The `perf.gate_selftest.*` tests run the comparison on synthetic data and
show that a slowdown above 1% fails without a line and passes with one.

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

The public functions are:

- `nesturbator_create` and `nesturbator_destroy` make and free an instance.
- `nesturbator_get_version` writes the library's version; a host compares it
  with `NESTURBATOR_VERSION_*` when the library may differ from the header.
  `nesturbator-run` calls it for `--version`.
- `nesturbator_get_info` writes the video and audio format.
- `nesturbator_get_palette` writes the palette as XRGB8888.
- `nesturbator_load_cartridge` loads an iNES image;
  `nesturbator_unload_cartridge` removes it and restores the state of a new
  instance. The libretro adapter calls the unload.
- `nesturbator_set_input` sets the next frame's controller state and
  `nesturbator_run_frame` runs one frame.
- `nesturbator_reset` is the soft reset below.
- `nesturbator_get_memory` and `nesturbator_save_generation` give the
  battery span and its write counter (Battery saves, below).
- `nesturbator_peek_cpu_ram` reads CPU RAM for test harnesses.
- `nesturbator_state_size`, `nesturbator_save_state` and
  `nesturbator_load_state` save and restore the whole machine (Save states,
  below).

Soft reset: `nesturbator_reset()` is the console's Reset button. Call it
between frames. It keeps CPU RAM and cartridge RAM (PRG RAM and CHR RAM), so
saves survive, and keeps A, X, Y and the host's input state. The controller
strobe and shift registers and a pending OAM DMA are cleared. Mapper registers
are kept, so the reset vector is read through the board's current banks. The
CPU sets the I flag, lowers S by 3 and takes the reset vector in 7 CPU cycles, which count
in `ticks`. The PPU restarts at the top of the picture and ignores writes to
`$2000`, `$2001`, `$2005` and `$2006` until the end of the next vblank, 29,667
CPU cycles from the reset (the NESdev Wiki documents about 29,658); it keeps
`v`, the status flags, the OAM address and video memory. The APU is silenced as
by a write of 0 to `$4015`, its IRQs are cleared and the last `$4017` mode is
re-applied. Load is unchanged: there is no write-ignore window and no startup
sequence at power-on, so the soft-reset work leaves frame and audio hashes from
load unchanged. With no cartridge it does nothing and returns
`NESTURBATOR_OK`.

### Battery saves

`nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &data, &size)`
writes a pointer to the cartridge's battery RAM and its size. The core does no
file I/O: the host keeps the save file. A NULL `inst`, `data` or `size`, or any
other kind, returns `NESTURBATOR_ERR_ARGUMENT` and writes nothing. With no
cartridge, or a cartridge without battery RAM, it returns `NESTURBATOR_OK` with
`NULL` and 0. The pointer stays valid from a successful load until unload, the
next successful load or destroy; `nesturbator_reset()` and a refused load keep
it, as does `nesturbator_load_state`, which copies into the same allocation.
Copy a saved file in after load and before the first frame; read the span
whenever you are between calls.

`nesturbator_save_generation(inst)` counts CPU bus writes that reached the
span while the board had that RAM enabled and writable, whether or not the
byte changed. A read-modify-write counts both of its writes. It runs from
`nesturbator_create()` and never resets or decreases, so compare it with `!=`
and write the file when it moves. Load (including trainer bytes), reset,
unload and writes through the pointer do not count. A state load counts once
when it changes the battery bytes and not at all when it leaves them
unchanged; a refused load never counts (Save states, below). `NULL` gives 0.

PRG RAM is one block laid out `[V work][N NVRAM]`, work RAM first, and the
save span is the N battery bytes (hardware: NESdev Wiki "MMC1", SOROM). The
header gives the sizes:

| Header | V (work) | N (battery) | Span |
|---|---|---|---|
| iNES 1, mapper 1, PRG up to 256 KiB, no battery | 8 KiB | 0 | none |
| iNES 1, mapper 1, PRG up to 256 KiB, battery | 0 | 8 KiB | 8 KiB |
| iNES 1, mapper 1, PRG above 256 KiB, no battery | 32 KiB | 0 | none |
| iNES 1, mapper 1, PRG above 256 KiB, battery | 0 | 32 KiB | 32 KiB |
| iNES 1, mapper 4, no battery | 8 KiB | 0 | none |
| iNES 1, mapper 4, battery | 0 | 8 KiB | 8 KiB |
| NES 2.0 | byte 10 low nibble | byte 10 high nibble | N bytes at offset V |

With V = N = 0, `$6000-$7FFF` reads as open bus. A trainer on a mapper 1
image with no declared RAM maps its 8 KiB as work RAM (the trainer at `$7000`)
and has no span.

Known gap: an iNES 1 SOROM dump gets one 8 KiB battery block, because that
header cannot say that half of the RAM is volatile; use a NES 2.0 header
(V = 8 KiB, N = 8 KiB) for a true SOROM. An iNES 1 SUROM save is 32 KiB.

Mapper 4 (MMC3) takes the same layout with one rule: V and N are each 0 or
8 KiB, never both (NES 2.0: none, 8 KiB work, or 8 KiB NVRAM). Known gap: an
iNES 1 MMC3 image always gets 8 KiB of RAM, so games that expect no RAM at
all, such as Low G Man, need a NES 2.0 header with V = N = 0.

### Save states

A state is an exact snapshot of the whole machine at a frame boundary. It is
valid for the same format version, ROM and board, and it is byte-identical on
every platform. The library does no file I/O and never allocates for a state;
the host owns the buffer and decides where states live.

1. `nesturbator_state_size(inst)` returns the exact length of a state of the
   loaded cartridge. It is 0 for a NULL instance or with no cartridge, and the
   same value from a successful load until unload.
2. `nesturbator_save_state(inst, buf, capacity)` writes exactly that many
   bytes. A larger buffer is fine and the bytes past the size are not touched;
   a smaller one returns `NESTURBATOR_ERR_BUFFER_TOO_SMALL` and writes nothing.
   A NULL `buf` is never a size query. Saving changes nothing in the instance,
   and saving a jammed instance succeeds.
3. `nesturbator_load_state(inst, buf, size)` needs `size` to equal the state
   size exactly. A load is all or nothing: every refusal leaves the instance
   byte for byte as it was. Loading a state saved after a JAM opcode makes the
   next frame return `NESTURBATOR_STOP_JAM`; loading a clean state clears the
   latch.

Load checks in this order and reports the first failure: arguments, a length
below the 64-byte header, the magic and format version, the cartridge identity
(mapper, submapper, region and ROM CRC-32), the length against the state size
and the header's total size, the CRC-32, and then the contents. A header size
other than 64, a reserved byte other than zero and any field out of range are
content checks and report `NESTURBATOR_ERR_STATE_INVALID`. So does a PPU
position ahead of the header's tick count or one whole frame or more behind it,
and so does a tick count above 2^62 (counters near 2^64 would wrap and never finish),
which keeps the first frame after a load to about one frame of work.

The header is 64 bytes, every multi-byte field little-endian:

| Offset | Bytes | Field |
|---|---|---|
| 0 | 8 | magic `89 4E 54 53 0D 0A 1A 0A` |
| 8 | 2 | format version |
| 10 | 2 | header size, 64 |
| 12 | 4 | total size |
| 16 | 4 | behaviour revision of the saving library |
| 20 | 2 | mapper id |
| 22 | 1 | submapper |
| 23 | 1 | region, 0 for NTSC |
| 24 | 4 | ROM CRC-32 of the 16 header bytes, PRG ROM and CHR ROM |
| 28 | 4 | state CRC-32 of every byte except these four |
| 32 | 8 | frame number |
| 40 | 8 | ticks |
| 48 | 16 | reserved, zero |

The machine fields, the whole PRG RAM (battery RAM included) and the 8 KiB CHR
RAM (when the board has it) follow. The frame buffer and the audio are not in a state. Frame numbers
and ticks continue from the saved point: the first frame after a load reports
the number and ticks of the uninterrupted run.

Battery RAM and the save generation: a state includes the battery RAM. Loading
a state whose battery bytes differ from the instance's raises `nesturbator_save_generation`
once, so the host writes the `.sav`. Loading a state whose battery bytes match,
however many times, never moves it, and neither does a refused load, so
repeated loads such as rewind never trigger a write.

Another format version is refused, never converted. Keep rewind buffers in
memory, and expect user-kept states to need re-creating after a release that
raises the format version. The behaviour revision is recorded and not
enforced: after an emulation fix an old state still loads, but its later frames
may differ from the run that saved it.

| Format version | Introduced |
|---|---|
| 1 | first release that contains save states |

A format bump ships under a `feat:` title with a changelog line saying older
states are refused. It does not change `NESTURBATOR_ABI_VERSION`, because the
C interface is the same.

Battery RAM is part of the PRG RAM, so a load replaces it. Rewinding past an in-game save reverts the save RAM.

The `core.state_fork.*` tests (one per committed game, plus
`core.state_fork.stress`) fork games on all six boards and a DMA and interrupt
stress program (on NROM and on MMC3) at many frames, including frame 0. Each fork loads into an instance that was pre-filled with other state and
then poisoned, and it requires every later frame, sample, tick count and the
final state to match an uninterrupted run. It also shows that saving every
frame changes nothing.

The `core.state_reject` test shows the refusals. It loads each game's state into
every other game, flips every byte of a state and expects the status of the
first failing check, truncates the state to every shorter length, and sets every
bounded field just past each of its bounds (with the CRC-32 recomputed) and at
the type's limits, expecting `NESTURBATOR_ERR_STATE_INVALID`. Fields at their
bounds are loaded and run for a frame. After every refusal the instance and
`nesturbator_save_generation` are unchanged. The same test runs under the
address and undefined-behaviour sanitizers in the `asan` preset.

### Status codes

Every call returns one of these. An error changes no state.

| Value | Status | Meaning |
|---|---|---|
| 0 | `NESTURBATOR_OK` | The call succeeded. |
| 1 | `NESTURBATOR_ERR_ARGUMENT` | A required pointer is NULL, or an argument is out of range. |
| 2 | `NESTURBATOR_ERR_STRUCT_SIZE` | A struct's size field is 0, too small, or larger with a non-zero tail. |
| 3 | `NESTURBATOR_ERR_ABI` | The config's abi field differs from `NESTURBATOR_ABI_VERSION`. |
| 4 | `NESTURBATOR_ERR_NO_MEMORY` | The allocator returned NULL. |
| 5 | `NESTURBATOR_ERR_BUFFER_TOO_SMALL` | A caller-owned buffer is too small for the output. |
| 6 | `NESTURBATOR_STOP_JAM` | The cartridge executed a JAM opcode; the instance is latched. |
| 7 | `NESTURBATOR_ERR_CARTRIDGE` | Cartridge bytes are malformed or outside a supported board. |
| 8 | `NESTURBATOR_ERR_STATE_FORMAT` | The magic is wrong or the format version is not the library's. |
| 9 | `NESTURBATOR_ERR_STATE_CARTRIDGE` | No cartridge is loaded, or the state belongs to another game or board. |
| 10 | `NESTURBATOR_ERR_STATE_SIZE` | The length is wrong, including any truncation. |
| 11 | `NESTURBATOR_ERR_STATE_CHECKSUM` | The state's CRC-32 does not match: the bytes were damaged. |
| 12 | `NESTURBATOR_ERR_STATE_INVALID` | The CRC-32 matches but a field is out of range or a reserved byte is set. |

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

Release tooling changes only the numbers in the public header's three version
macros. A narrow `clang-format off/on` region keeps their inline release-please
markers stable when a number gains a digit, such as patch 9 becoming 10.
`release.version_format` checks the guard, markers and numeric-only mutations
in CI; `hygiene.release_version_format` also checks the full mutated headers
with clang-format 18. Formatting resumes before the ABI macro.

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

`nesturbator-run` runs the core without a window and accepts a mapper 0, 1, 2, 3, 4 or 7 image
with `--rom FILE`. The loader validates the entire image before allocating
cartridge state. It rejects unsupported mapper, console, region, RAM and ROM
geometries, truncation, trailing bytes, and images larger than 64 MiB; the
runner prints a diagnostic on stderr and exits 1 for rejected content. It names the
reason: `nesturbator-run: four-screen cartridges are not supported`,
`nesturbator-run: MMC6 (mapper 4, submapper 1) is not supported`, or
`nesturbator-run: mapper N is not supported; supported mappers are 0, 1, 2, 3, 4 and 7`.
Any other refusal (a supported mapper with a refused size, or a file shorter than the
16-byte header) gets the generic `malformed or unsupported cartridge` line naming the same six
mappers.

```sh
nesturbator-run --frames N [--rom FILE] [--hash-frame N]... [--hash-audio] [--dump-frame N:FILE]...
                [--save-dir DIR [--save-interval N]]
                [--state-at-frame N:FILE]... [--hash-state N]... [--load-state FILE]
                [--raise-after N[:SIG[:K]]]
nesturbator-run --version
```

- `--frames N` runs N frames (N is 1 or more). One of `--frames`, `--movie` or `--bench` is
  required; without one the runner prints its usage and exits 2.
- `--bench N` runs frames 1 to N (N from 1 to 1000000) with `--rom`, times each frame, and prints
  frame-time statistics to stderr. Standard output and every hash are exactly as with
  `--frames N` (each hash line is flushed as it is printed), so `--hash-frame K` (1 to N) and `--hash-audio` work the same, and a test shows
  the output is byte-identical. Only the `nesturbator_run_frame` call sits between the two clock
  readings (a monotonic clock: `CLOCK_MONOTONIC`, or `QueryPerformanceCounter` on Windows), so the
  timer's own overhead is included in each sample. There is no warm-up: the first frames show in
  `max`. `--bench` cannot be combined with `--frames` or `--movie` and needs `--rom`; any other
  misuse (0, above 1000000, given twice) exits 2. It also refuses `--accuracycoin-page`,
  `--raise-after`, `--dump-frame`, `--state-at-frame`, `--hash-state`, `--save-dir`,
  `--save-interval` and `--load-state`, each with exit 2 and its own message, because file I/O and
  loaded starts would pollute the timing. Example stderr:

  ```
  nesturbator-run: bench frames=600 time_ms=701.367 fps=855.47
  nesturbator-run: bench frame_us p50=1163.000 p90=1235.000 p99=1339.000 p99.9=1616.000 max=1616.000
  ```

  Every value is a measured frame time in microseconds, truncated from nanoseconds, by the
  nearest-rank method with no interpolation; p99.9 equals max below 1000 frames, and `fps` is
  frames over the summed frame time. A signal stop prints the statistics over the frames
  finished, then the stop line; a stop before the first frame prints only
  `nesturbator-run: bench frames=0`. Wall time is never gated in CI: the tests check the shape of
  these lines, never a number.
- `--bench-state` needs `--bench`. After every timed frame it saves the state and loads it back,
  timing the save and the load separately from the frame, so `frame_us` stays comparable. It adds
  the `save_us` and `load_us` lines after `frame_us`. A save or load failure exits 1. Standard
  output is unchanged.
- `--raise-after N[:SIG[:K]]` is a test aid: after frame N the runner signals its own
  process K times (K defaults to 1) with SIG, so a stop lands at a known frame. SIG is `int`
  or `term` on POSIX (default `term`) and `int` or `break` on Windows (default `break`). It
  cannot be combined with `--accuracycoin-page`.
- `--movie FILE` replays a validated two-port movie (see above) instead of running `--frames N`,
  and prints a SHA-256 line for every frame.
- `--help` prints the usage and the exit statuses to stdout and exits 0.
- `--accuracycoin-page N` runs AccuracyCoin page N (2, 14 or 17) for 1800 frames and compares its
  RAM results to the scoreboard. It needs `--rom` and `--scoreboard` and installs no stop
  handlers.
- `--scoreboard FILE` names the scoreboard that `--accuracycoin-page` compares against; it is
  an error without `--accuracycoin-page`.
- `--version` prints one line to stdout, `nesturbator-run X.Y.Z`, formatted at
  run time from `nesturbator_get_version`, and exits 0. It needs no other
  option and nothing else is printed. It is handled where the option loop
  reaches it, so options before it are still checked:
  `nesturbator-run --version --bogus` prints the version, while
  `nesturbator-run --bogus --version` is a usage error with exit 2. It prints
  no ABI or behaviour revision.
- `--rom FILE` loads a bounded iNES 1.0 or NES 2.0 image. Accepted
  geometry is 8 KiB CHR ROM or declared 8 KiB CHR RAM with 16 or 32 KiB PRG
  for mapper 0 (NROM), or PRG in 16 KiB banks up to 4 MiB for mapper 2
  (UxROM, submappers 0 to 2), or 16 or 32 KiB PRG with 8, 16 or 32 KiB CHR ROM
  for mapper 3 (CNROM, submappers 0 to 2), or 32 to 256 KiB PRG in 32 KiB banks
  with 8 KiB declared CHR RAM for mapper 7 (AxROM, submappers 0 to 2), or the
  mapper 1 and mapper 4 shapes below; optional trainers are included in the validated
  file length. UxROM writes to `$8000-$FFFF` select the bank at `$8000`; the
  last bank stays at `$C000`. Submappers 0 and 2 AND the written value with
  the ROM byte under the write (submapper 0 is the project default), and
  submapper 1 takes it raw. CNROM writes to `$8000-$FFFF` select the 8 KiB
  CHR bank with the same bus-conflict rule (submappers 0 and 2 AND, submapper
  1 raw); writes to CHR ROM are ignored. AxROM writes to `$8000-$FFFF` select the
  32 KiB PRG bank (bits 0 to 2) and the single-screen nametable page (bit 4);
  the header mirroring bit is ignored, the reset vector comes from bank 0 at power-on (a soft reset
  keeps the bank register, so it then comes from the selected bank), and
  only submapper 2 ANDs the value with the ROM byte under the write (submapper
  0 has no bus conflict).
- Mapper 1 (MMC1) is accepted with submapper 0, or submapper 5 with 32 KiB
  PRG. PRG is a power of two from 32 to 512 KiB, and 512 KiB only with 8 KiB
  CHR. CHR is 8 to 128 KiB of ROM or 8 KiB of RAM. PRG RAM is 0, 8, 16 or 32
  KiB in total (work plus battery), and more than 8 KiB only with 8 KiB CHR.
  The battery bit must be set exactly when battery RAM is declared. CHR NVRAM,
  other submappers, mapper 155 and a 24 KiB or 64 KiB total are refused.
  Mappers 0, 2, 3 and 7 still refuse the battery bit and every RAM size.
  The board is the MMC1B chip: five writes to `$8000-$FFFF` fill the serial
  port, and a write on the CPU cycle right after another write is ignored
  unless its bit 7 is set, which always resets the port (this is what makes
  `INC $8000` shift one bit, not two). PRG RAM is enabled at power-on and bit 4
  of the PRG register disables it. The SNROM, SOROM, SUROM and SXROM variants
  follow from the header sizes (hardware: NESdev Wiki "MMC1" and "SxROM").
  `nesturbator_reset` keeps the MMC1 registers and the battery RAM, because
  the cartridge sees no reset line.
- Mapper 4 (MMC3) is accepted with submapper 0 (the Sharp revision, also used
  for every iNES 1 image) or submapper 4 (the NEC revision). PRG is a power of
  two from 16 to 512 KiB, CHR is a power of two from 8 to 256 KiB of ROM or
  8 KiB of RAM, and PRG RAM is none, 8 KiB of work RAM or 8 KiB of battery
  RAM (the battery bit must match). Refused: MMC6 (submapper 1), submappers
  2, 3, 5 and 6 to 15, four-screen images, CHR NVRAM, other RAM sizes, and
  mappers 118, 119, 206 and 249. Registers decode by `addr & $E001`: `$8000`
  selects a bank register, the PRG mode and the CHR inversion, `$8001`
  writes it, `$A000` picks the mirroring (bit 0 clear is vertical), and
  `$A001` enables (bit 7) and write-protects (bit 6) the RAM at
  `$6000-$7FFF`, which reads as open bus when disabled. At power-on the RAM
  is enabled and writable. Banks wrap modulo the file size, so 16 KiB of PRG
  repeats. `nesturbator_reset` keeps the registers and the battery RAM
  (hardware: NESdev Wiki "MMC3").
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
- `--save-dir DIR` keeps the battery RAM of a mapper 1 cartridge in
  `DIR/<name>.sav`. It needs `--rom` and cannot be combined with
  `--accuracycoin-page`. `<name>` is the last component of the `--rom` path
  (split at `/` or `\` on every platform) without its last extension, so
  `roms/game.v1.nes` uses `DIR/game.v1.sav`. After the cartridge loads and
  before the first frame the runner reads that file into the battery RAM; with
  no file the RAM starts zeroed. When the run ends, including after a JAM, it
  writes the RAM back if the CPU wrote to it and the bytes changed. The write
  goes to `DIR/<name>.sav.tmp`, is synced (with `F_FULLFSYNC` on macOS), and is renamed over the save, so the
  save is the old one or the new one and never torn. A failed write removes its
  `.tmp`, prints `nesturbator-run: cannot save <path> (<reason>); the previous
  save is kept`, and the runner keeps the newest bytes in memory for the next
  attempt. A kill during a write can leave a stale `.tmp` (a regular file or a
  symlink, even a dangling one), which the next write clears without touching a
  link's target; a directory at that name is never removed. There is no directory
  sync, so a power cut just after the rename may still show the previous save. A run that writes
  the same bytes leaves the file and its modification time alone. A `.sav` is
  the RAM's raw bytes with no header, the same bytes as RetroArch's `.srm`. A
  cartridge without battery RAM reads and writes nothing, and without
  `--save-dir` no save file is read or written anywhere. A save
  failure, or a `DIR` that does not exist, exits 1. A `.sav` whose length is
  not the battery RAM's size (a 0-byte file included) is never padded,
  truncated or rewritten: the runner prints
  `<path> is <a> bytes; the cartridge's battery RAM is <b> bytes`, runs no
  frames and exits 4. The runner takes no lock: two runners on one directory,
  or two ROMs with the same name, share one save file, and the size check
  catches a clash of different sizes.
- `--save-interval N` (N of 1 or more, needs `--save-dir`) also writes a
  changed save after every Nth frame, counted from 1, so a runner stopped
  after a flush leaves the RAM as it was then. A failed interval write ends
  the run with exit 1, and the runner tries the write once more before it
  exits. With `--frames 3
  --save-interval 5` no interval write falls inside the run, and the write at
  exit still happens. `--save-interval 0`, a repeated or empty `--save-dir`,
  and `--save-dir` without `--rom` exit 2.

- `--state-at-frame N:FILE` (repeatable; N is 0 or more) writes the save state
  of the machine after frame N to FILE, split at the first colon. N of 0 is
  the state at power-on, or the loaded state itself when `--load-state` is
  given. The file is written through the same atomic replace as a `.sav`, so
  FILE holds the old file or one complete new one; the runner writes no other
  path and leaves no `.tmp` after a successful write. Running the same command
  twice replaces the file with identical bytes. Two runners writing one path
  at the same time are not supported. A write failure exits 1. The same N
  given twice exits 2.
- `--hash-state N` (repeatable; N is 0 or more) prints one line after frame N,
  and for the loaded frame before the first frame runs:
  `state <N> ticks <T> size <S> sha256 <64 lowercase hex digits>`. The hash is the SHA-256 of
  the bytes `nesturbator_save_state` writes, `S` is their length and `T` the
  emulated time after frame N (0 at power-on). Per frame the order is the
  `frame` line, then the `state` line, then the dump files. A run that saves at
  60 and a run that loads that state print the same `state 120` line. The same
  N given twice exits 2.
- `--load-state FILE` (once; a second one exits 2) resumes from a save state
  after the cartridge, and the battery save when `--save-dir` is given, are
  loaded. Frame numbers stay absolute from power-on: a state saved after frame
  K (read from the state's header) makes the run continue at frame K + 1 up to
  `--frames`, and `--hash-frame` and `--state-at-frame` use the same numbers,
  so a run that saves at 60 and a run that loads that state print the same
  line for frame 120. A rejected state exits 5 and prints one line,
  `nesturbator-run: cannot load state <path> (<word>)`, where the word is
  `another format version`, `wrong game`, `wrong length`, `damaged` or
  `invalid contents`, for `NESTURBATOR_ERR_STATE_FORMAT`, `_CARTRIDGE`, `_SIZE`,
  `_CHECKSUM` and `_INVALID`. A file that is missing, is a directory or cannot
  be read exits 1 with `nesturbator-run: cannot read state <path> (<reason>)`.
  With `--save-dir`, a state whose battery bytes differ from the loaded `.sav`
  counts as a change, so the write at exit puts the state's battery bytes in
  the `.sav`.

  After a load at frame K, `--frames N` is the last frame to run (an end frame,
  not a count of further frames), so the run computes frames K + 1 to N. The
  ranges are checked after the load and before any frame runs, and a breach
  exits 2 with a message that names K: `--frames` must be at least K;
  `--hash-frame` and `--dump-frame` need K < N <= `--frames`;
  `--state-at-frame` and `--hash-state` need K <= N <= `--frames`. Without a
  load K is 0. `--movie` cannot be combined with `--load-state` (exit 2), and
  any of `--state-at-frame`, `--load-state` and `--hash-state` without `--rom`,
  or with `--accuracycoin-page`, exits 2.

The save writer is tested under failure. `runner.save_replace` runs the shared
replace helper against a directory at the save path, a missing parent, a
regular file as a parent, an empty and a non-empty directory at `.tmp`, a stale
`.tmp`, and (Linux and macOS) a read-only directory and a symlink (also a dangling one) at `.tmp`;
each leaves the old save intact and no `.tmp` behind. `runner.save_fsize`
(Linux and macOS) repeats that under a file size limit.
`runner.save.dir_at_tmp` runs the runner itself with a directory at
`battery.sav.tmp` and expects exit 1, the `cannot save` message and the
directory left alone.

Exit status: see [Exit status](#exit-status).

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

Exit status: see [Exit status](#exit-status).

### Stopping the runner

On SIGTERM or SIGINT the runner finishes the current frame, writes the battery save and exits 0, or 1 if the save could not be written. It prints one stop line on standard error and adds nothing to standard output. The stop is seen between frames, also before the first frame when the request arrives while the cartridge loads; a load error keeps its own exit status. The save goes through the same temp-and-rename write as any other, so it is never torn. A second signal at any time, including during the flush, ends the runner at once. A signal the runner inherited as ignored stays ignored: a runner started as a shell background job (`cmd &` in a non-interactive shell) or under `nohup` does not stop on SIGINT or SIGTERM, so a launcher that needs the stop starts it with those signals at their default. The `runner.signal.*` and `runner.stop.*` tests need the same, and fail or run to their timeout when ctest itself is started with SIGINT or SIGTERM ignored. A launcher on Linux or macOS must keep reading the runner's standard output, or redirect it to a file, because a full pipe holds the runner in a write and delays the stop until the write returns.

On Linux and macOS the `runner.signal.*` tests send real SIGTERM and SIGINT from another process and require exit 0 within 2 s with the whole save and no temp file, and a second SIGTERM to end a runner that is blocked writing to a full pipe. They also show that a signal during loading leaves a load error's status in charge, and that a second SIGTERM ends a runner held inside its exit flush.

A stopped run exits 1 when something requested was not produced: a `--hash-frame`, `--hash-state`, `--state-at-frame` or `--dump-frame` beyond the last finished frame, `--hash-audio`, or a movie that did not reach its end. The line after the stop line names the first such option on the command line, for example `nesturbator-run: not reached: --hash-frame 6`, and standard output gains nothing. A stopped run prints no audio hash, because it would cover a truncated run. A `--frames` count not reached is not a failure, so a launcher that runs open-ended (`--frames 4294967295 --save-dir DIR`, no hash or dump options) and stops it gets 0. A signal that arrives after the last frame changes nothing: the run is complete, exits 0 and prints its audio hash.

On Windows the runner handles Ctrl+C, Ctrl+Break and console close the same way. SIGTERM is not a Windows mechanism. A forced termination (TerminateProcess, taskkill /F, power loss) cannot be handled; the last committed .sav survives it. Logoff and shutdown are not handled. `--accuracycoin-page` installs no stop handlers. On Windows `runner.stop.win_handler` calls the console handler directly (Ctrl+C and Ctrl+Break handled once and declined after, logoff and shutdown declined, a close event held until the exit sequence is done or its cap passes), and `runner.stop.win_console` sends a real Ctrl+Break through the runner's own console and requires exit 0 with a whole save and no temp file.

A Windows launcher stops the runner with a console event, in one of two cases.

- The launcher shares the runner's console. It starts the runner with `CREATE_NEW_PROCESS_GROUP` (and without `CREATE_NEW_CONSOLE`, `DETACHED_PROCESS` or `CREATE_NO_WINDOW`) and calls `GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, pid)`, the runner's process id being its group id, so only the runner's group gets the event.
- The launcher has no console or a different one. It starts the runner with `CREATE_NEW_CONSOLE`, detaches from its own console (`FreeConsole`), attaches to the runner's (`AttachConsole` with the runner's process id), registers its own handler with `SetConsoleCtrlHandler`, and only then sends `GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0)`. Group 0 reaches every process on that console, the launcher included. A launcher that sends the event to group 0 first registers its own handler that returns TRUE for CTRL_BREAK_EVENT, after attaching and before sending, or the event ends the launcher too. Attaching resets the handler list, so the handler goes in after it.

Never send CTRL_C_EVENT to a process group. No event reaches a runner started with `DETACHED_PROCESS` or `CREATE_NO_WINDOW`. The launcher must read or discard the runner's standard output, because the runner flushes it before the save.

### Exit status

`nesturbator-run --help` prints the usage text and this table to standard output and
exits 0. A bad command line prints the same text to standard error and exits 2.

| Status | Meaning | When |
|---|---|---|
| 0 | done | The run finished, or `--version` or `--help` printed its text. |
| 1 | failure | A file cannot be read or written, a save fails, a stop whose save could not be written, a stopped run that missed a requested output, or a frame fails or hits a JAM. |
| 2 | usage error | The command line is wrong. |
| 4 | .sav size mismatch | The `.sav` file's length differs from the cartridge's battery RAM. |
| 5 | state rejected | `--load-state` refused the file. |

3 and 77 are reserved; the runner never returns them.

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

One library instance lives from `retro_init` to `retro_deinit`. Each
`retro_load_game` first calls `nesturbator_unload_cartridge`, and
`retro_unload_game` calls it, so a reloaded game starts exactly as a fresh one.
`libretro.host` loads, unloads and reloads content without `retro_deinit` and
requires the same frames and audio each time, the test card after an unload,
and a NULL save span after a double unload or a rejected image. The
`core.cartridge` check compares an unloaded instance with a fresh one, field by
field and frame by frame.

RetroArch's Reset (`retro_reset`) runs `nesturbator_reset()`, the console's
soft reset, which keeps RAM and cartridge RAM; it does not unload or reload the
game. With no content it changes nothing. `libretro.host` compares the frames
after a reset with those of a direct-API instance given the same frames, reset
and frames.

`retro_get_memory_data` and `retro_get_memory_size` return the battery span
for `RETRO_MEMORY_SAVE_RAM`, the same span as
`nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, ...)`; every other
id, no content and a cartridge without battery RAM give NULL and 0. The adapter
fetches the span at each successful `retro_load_game`, drops it at unload (NULL
and 0 afterwards) and does no file I/O: the host
writes the `.srm` and copies it back in after `retro_load_game` and before the
first frame. The `.srm` and the runner's `.sav` are the same raw bytes, so a
save moves between RetroArch and `nesturbator-run --save-dir` by renaming it.
`libretro.host` checks the span's size, that its address survives `retro_run`
and `retro_reset`, and that its bytes equal those of a direct-API instance run
for the same frames.

`retro_serialize_size`, `retro_serialize` and `retro_unserialize` use the
library's save state (`nesturbator_state_size`, `nesturbator_save_state`,
`nesturbator_load_state`). The size is 0 with no content and for the test card,
and otherwise the fixed size of the loaded board. Only a buffer of exactly that
size loads, and a refused load changes nothing: the next frame and the next
state equal those of a run that never made the call. The core sets no
serialization quirks, ignores the savestate context and the audio/video enable
flags, and keeps no state of its own for these calls, so the bytes are the same
whatever the frontend reports. `libretro.serialize` checks this through the
built module on five ROMs: the sizes, the exact write length, thirteen refused
buffers, a load straight after `retro_load_game` and after `retro_reset`,
every savestate context, and a load every frame on a battery ROM. A load
counts once in `nesturbator_save_generation` when it changes the battery bytes
and never when loading the same state again. `libretro.no_quirks` fails if
`libretro/libretro.c` names the quirks command or the context query.
`libretro/nesturbator_libretro.info` declares `savestate = "true"` with
`savestate_features = "deterministic"`, the highest of RetroArch's levels
(`basic`, `serialized` for rewind, `deterministic` for run-ahead). RetroArch
applies it only when the core sits in its `cores` directory beside the info
file. The level is backed by `libretro.serialize`, `libretro.runahead` and
`core.state_runahead`, and configuring the tests fails if the info file claims a
level whose tests are not registered. RetroArch uses the deterministic level for
run-ahead and netplay. Run-ahead is proven by the simulated host and the
RetroArch end-to-end run. The state format is fixed little-endian and
byte-identical on all six platforms. Netplay itself has no end-to-end test here.

`libretro.runahead` runs the built module as a run-ahead host: per displayed
frame it runs the real frame, saves, runs one to three frames ahead, shows the
last and loads the saved state, through the real `retro_*` entry points on seven
ROMs and the two synthetic stress programs. Two fixed-seed input scripts change
the buttons on every alignment of the cycle, including the frame right after a
load. The shown frames, the real frames' audio, the last speculative frame's
audio, the state bytes at every frame boundary and the final state must equal an
uninterrupted run, and a repeat mode that gives the speculative frames the
current input, as RetroArch does, must equal a two-instance library model. Four
broken hosts (no load, a stale load, input one frame late, speculative audio in
the real stream) run through the same comparison and each must differ.
The registered `libretro.serialize` and `libretro.runahead` tests first copy each
special ROM into their build directory, then compare that actual supplied file's
SHA-256 with its exact row in `tests/roms/manifest.txt` before starting the host.
A mismatch, missing row or duplicate row fails before host launch. The host assigns
the battery and run-ahead roles only to the case-sensitive final filenames
`M1_P128K_C128K_S8K.nes` and `nesteroids.nes`; `/` separates path components on all
platforms, and `\` does so only on Windows. Parent directory text and similar
filenames select no role. This identity check belongs to the registered CMake tests;
a direct `libretro.host` call uses exact names but does not authenticate file bytes.
The marker fixture retains its separate generator identity.
`core.state_runahead` checks RetroArch's second-instance mode as a library
property: one instance runs the real timeline, a second loads its state each
frame and runs ahead, and the shown frames equal a single instance that saves,
runs ahead and loads. Together with `libretro.serialize` they back the
deterministic claim.

`libretro/nesturbator_libretro.info` is the core information file. It goes in
RetroArch's `info` directory beside the core in `cores`, and declares
`supports_no_game = "true"`, which lets RetroArch start the core without
content, and `libretro_saves = "true"`.

`libretro/libretro.h` is the libretro API header, copied unchanged from
RetroArch; its source and licence are in
[THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).

## Install in RetroArch (Apple Silicon)

With RetroArch installed (see below), these two lines put the released core
in RetroArch's `cores` directory and its information file in `info`. The
first sets the version; the second downloads and extracts:

<!-- x-release-please-start-version -->
```sh
NESTURBATOR_VERSION=0.1.10
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
screenshot with the runner's frame; this is where RetroArch is checked. A
second step runs two RetroArch sessions of the Holy Mapperel SXROM ROM: the
first writes `saves/M1_P512K_CR8K_S32K.srm` (32,768 bytes, `SAVEDATA` at 0x100)
when RetroArch unloads the content, and the second loads it back. Each
screenshot must equal the runner's frame for the same save state (the runner
reads the first session's `.srm` as `.sav`), the two screenshots must differ,
and exactly one `.srm` must exist at the end. A third step does the same for
the MMC3 battery ROM `m4tkrom`, derived at build time from a manifest-listed
Holy Mapperel ROM (MMC3 with battery and 8 KiB of save RAM): two sessions to
frame 600, an 8192-byte `.srm` with `SAVEDATA` at 0x100, screenshots equal to
the runner's frames and different from each other. `run_retroarch.cmake` takes
`-DSAVE_SIZE=<bytes>` for the exact `.srm` size (default 32768); anything but
a positive integer is an error.

`run_retroarch.cmake` also has a state mode (`-DSTATE_ROM`, with `-DFRAME_INDEX`
and `-DSTATE_TEMPLATE`) for the rewind and run-ahead claim. It copies the core
into RetroArch's cores directory beside the `.info`, because RetroArch applies
the `.info` savestate level only to a core found there, and loads that copy. The
expected run-ahead lead comes from the `.info` under test (the
`run_ahead_frames` value for `deterministic`, 0 for `serialized`), and the
screenshot must show it: lane `state-base` shows the runner's frame 120, lane
`state-ahead` shows frame 120 plus the lead. `frame_index` identifies each
screenshot among the runner's frames. After every lane the mode checks that no
thumbnail was written and that no second core instance was set up, and it fails
on any RetroArch `[ERROR]` line, or any rewind or run-ahead warning, that is not
in the measured allowlist `tests/retroarch/log_allowlist.txt`. Each allowlist
entry names the savestate level it is valid for and carries a comment saying
where it was measured on RetroArch 1.22.2.

The official RetroArch v1.22.2 macOS release is
[`RetroArch_Metal.dmg`](https://buildbot.libretro.com/stable/1.22.2/apple/osx/universal/RetroArch_Metal.dmg),
universal for arm64 and x86_64, with measured SHA-256
`81b79121ba26d539064ae13b4d0419a120c3d165afbe656cf5f5412b15fdb434`. The
required `retroarch-e2e` job downloads this asset on `macos-15`, verifies its
checksum and exact version, loads Nesteroids, and compares the nonempty frame-60
screenshot pixel for pixel with the runner's output. It isolates RetroArch's
first-run home under the build tree and retains the asset evidence, screenshot,
and runner frame, and both save round trips' screenshots, runner frames and
`.srm` files (`M1_P512K_CR8K_S32K.srm` and `m4tkrom.srm`), in the `retroarch-e2e-frames` artifact. Missing assets, changes
to the real home directory, startup errors, and frame mismatches fail the
required CI check. No manual screenshot or gameplay check is needed.

The job's state step retains the existing auto-save/auto-load, game, battery,
portable state and audio evidence. Its separate marker network session first
retains two independently published, marker-validated startup captures and
requires the second frame to advance strictly beyond the first. This proves the
host is running before the one-shot pause toggle; a host that starts paused or
stalls fails closed. The next two captures must then show the same frame before
the original native anchor is saved. These four startup/pause screenshots are
followed by the sixteen canonical displayed-state boundaries: five exact
advances, one state-load-plus-advance probe, two post-load advances, resume until
forward progress is observed and pause, forty individually paced rewind sends,
three rewound advances, then a paused run-ahead toggle and load-plus-advance
probe. The two startup captures are retained separately and are not progression
boundaries; the full session sends 18 one-shot SCREENSHOT requests and records
79 one-shot sends overall. Each of the sixteen canonical captures is decoded
within absolute marker frames 4..10000 and checked by the same production prefix
verdict; only a clean host exit after all boundaries earns the complete verdict.
Original and rewound native state containers are retained and validated before
their slots are reused.

Each attempt has a unique owned tree and exclusive local command-port lock. The
only retryable request is the read-only readiness query with its exact isolated
save-directory reply; a complete exact reply is accepted after the owned receive
child exits or is reaped. The 1-second `nc` timeout sits inside a 5-second child
supervision slice, further capped by live operation and attempt time. Empty reads
may retry within the 30-second readiness deadline; every action, state save/load,
screenshot and QUIT request is one-shot. Monotonic timers bound the 30-second
readiness and operation windows, 5-second artifact publication window,
120-second attempt and 5-second cleanup reserve. On the portable synthetic
host, the full sixteen-boundary session took 53.2 seconds, including every
intermediate production verdict. The POSIX control runs the actual shell driver
through the successful full sequence
and reason-specific loss, duplication, stale/partial artifact, wrong-origin,
host-death, stalled-child and deadline cases. Its stalled-read check measures
elapsed time and confirms that the blocked child and owned host are reaped.
The journal records operation time through terminal observation and attempt
time through bounded child/host cleanup, along with request counts,
expected/observed relations and relative artifact identities. Generated config
and host logs are retained only in sanitized form. A local UDP send is not proof
of host receipt; the retained images and native anchors are checked by the
shared progression verdict. `PREFIX_OK` qualifies a declared continuation
only; complete success requires all sixteen boundaries and terminal exit zero.
Fault injection wraps only prescribed one-shot mutations; readiness reads use
the real local `nc`. The wrapper supervises and reaps its own send child on
interruption. This is bounded evidence for the pinned local-host run, not a
guarantee of UDP delivery.

The portable `retroarch.marker.qualify` control qualifies an owned generated NROM
counter. It
runs every absolute library frame 4 through 10000 (9997 frames), checks every
pixel of all counter cells and complementary guards, and measures the guest
counter's mapping to the absolute frame, including startup latency. The interval
is the permitted absolute capture, rewind and display-lead excursion; wall time
alone cannot bound a host's frame rate. Evidence outside the qualified interval
fails. The marker is near the
bottom of the picture so instruction-boundary drift cannot split it in this
window. Generated ROMs and calibration records stay in the build tree.

The required pinned-host `HOSTED_MARKER=ON` invocation gives generated marker
content its own state/network session and identity after checking the released
RetroArch asset pin. It repeats qualification on that content, then runs the
production session driver and one final production verdict over all sixteen
boundaries. The separate Nesteroids game, deterministic auto-state and battery
lanes remain in the hosted job; exact command progression no longer comes from
set-valued screenshot chains.

The host qualification checks distinct base/run-ahead and auto-save/load pictures,
measures the display lead, and retains the native origin at frame 150. Its narrow
reader accepts only the measured uncompressed RetroArch 1.22.2 `RASTATE` version
1 envelope: one `MEM ` chunk of the exact native size, zero padding to eight
bytes and one zero-length `END ` chunk, with no trailer. The public loader on an
isolated same-content instance validates the payload; the oracle additionally
binds current format, behavior revision, absolute origin and the next marker.
SHA-256 identities bind supplied/generated content, copied core, metadata and
each host configuration. A missing image/state, crash or failed launch fails
the host gate. This bounds displayed progression and native origins; it does not
prove every committed machine field or speculative audio. Portable whole-state
and audio tests retain those separate proofs.

`frame_index --marker-verdict-prefix ROM CALIBRATION EVIDENCE` applies the
same versioned, bounded relation verdict to a declared prefix of decoded host
images. It binds the record to the exact marker ROM and measured calibration,
checks each image hash and marker, and validates its retained native anchor on
an isolated same-content instance. `PREFIX_OK` means only that this observed
prefix is consistent with the declared continuation; it is not a complete
session verdict. `--marker-verdict` requires all prescribed boundaries and a
clean terminal host exit. The evidence format preserves paths with spaces and
rejects ambiguous frame candidates instead of selecting whichever candidate
makes a relation fit. Its 16 boundaries keep one original anchor across two
pause captures, five advances and the load/post-load/resume branch, and one
rewound anchor across the coarse rewind, three advances and run-ahead probe.
Load-plus-probe records two sends; the rewind records forty paced sends and
requires at least twenty frames of backward movement. The run-ahead display
lead is recorded separately. `retroarch.progression.controls` checks this
complete path and 40 independent reason-specific controls, including the
same-image A,A,B ambiguity case. `--marker-selftest WORK --fixtures` leaves
build-owned marker images, calibration and native anchors for offline controls.

`retroarch.marker.controls` runs 36 independently specified cases: first/last
frames, startup and end plus one, counter carry, zero/one/two steps and duplicate
identity, invalid colors and partial cells/guards, image geometry, bounded
container length/header/padding/end checks, wrong native content/format/checksum,
and native-valid stale revision and same-ROM wrong origin. Every negative must
reach its named terminal reason. CRC-valid semantic negatives first pass the
public loader, so a parse error or crash cannot masquerade as their rejection.
Unrelated background RGB, top-down 24-bit BMP and bottom-up 32-bit BMP with
unused alpha preserve marker identity. The required hosted
progression gate retains its full qualification and owned attempt tree on success
and failure for seven days in `retroarch-marker-progression`, alongside the
unchanged game, battery and deterministic auto-state artifacts. Dropped and
doubled real FRAMEADVANCE sends must reach the production `PROGRESSION` verdict;
a branch bound to the other native-valid origin must reach `ORIGIN`. Each
negative follows valid screenshot and native-anchor prerequisites, so a host
launch or parsing failure cannot count as semantic evidence.

## What it is

- An accuracy-class, deterministic NES core with no GUI of its own.
- Three deliverables: a C library, a command-line runner, and a libretro core
  for frontends such as RetroArch.
- All emulation code written here, under the MIT licence.

## ROMs

This repository contains no commercial ROM or BIOS data and never will. You
supply your own legally obtained game images. See
[ASSET_POLICY.md](ASSET_POLICY.md).

Holy Mapperel's mapper 1, 2, 3, 4 and 7 test ROMs under `tests/roms/hm/` are zlib
licensed, byte-identical to the v0.02 release, and listed in
`tests/roms/manifest.txt`; their licence is in
[THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).

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
