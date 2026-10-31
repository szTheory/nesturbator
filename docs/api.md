# Library API and native state

This guide describes the public C API, battery-RAM view, status values, and native save-state buffer. It documents the current source tree; identify a published binary from its own release artifacts.

## Emulated video and mapper interface

The public frame output is a 256×240 raster in physical visible scanline order. Row 0 follows the pre-render scanline; an OAM Y value of `$FF` can wrap a sprite onto it. Each native `uint16_t` pixel holds a palette entry in bits 0–5 and the three PPUMASK emphasis bits in bits 6–8; bits 9–15 are zero. Background output follows nametable mirroring, pattern and attribute data, scrolling, grayscale and emphasis. Sprite output follows OAM order, transparency, palette, horizontal and vertical flips, 8×8 or 8×16 pattern selection and priority. `$2001` controls left-edge clipping. Native pixels are the canonical frame-hash input; display-palette conversion is separate. [HWP.02][HWP.08]

Sprite overflow follows the 2C02 diagonal OAM scan: after eight sprites are selected, a tile, attribute or X byte can set overflow, while an in-range Y skipped by the scan does not. Overflow remains in PPU status until pre-render dot 1. The first eight selected sprites are copied one byte per odd/even OAM pair; their final X bytes are copied before the ninth Y comparison at dot 130, so `$2002` reads before that comparison still see overflow clear. Pre-render evaluation includes OAM Y=`$FF` sprites on visible row 0. [HWP.06]

Cartridges use a per-board mapper interface with page tables, a four-entry nametable map, CPU-cycle-stamped writes, a mapper IRQ combined with the APU IRQ, and PPU A12 edges reported to the board. The current supported board profiles are summarized in the README and specified by `nesturbator_load_cartridge`. [HWP.12]

## Behavior revision history

The current public header reports behavior revision 10. These revision facts describe this source tree; a published core must be identified from its own release or CI artifact.

- **Revision 10:** The DMC bit counter continues through eight-clock silent cycles. Its separate silence flag gates DAC changes; re-enabling a sample preserves the cycle position. [HWC.23]
- **Revision 9:** IRQ acceptance is retained at the instruction hardware poll point, using the old I flag for CLI, SEI and PLP. Branches preserve their first poll and add the page-cross poll. Accepted IRQs survive later source clearing until entry arbitration; NMI wins, and a held IRQ may be accepted again after RTI restores I. A post-vector-choice NMI input remains an unrecognized candidate until held through the first handler opening; releasing it before then cancels it. [HWC.02][HWC.17]
- **Revision 8:** CPU bus-clock seams capture rising edges on the derived PPU NMI line; instruction-specific polls decide when a captured request is serviced, and later line deassertion does not revoke it. `$2002` samples vblank bit 7 at M2 rise and sprite bits 6/5 at M2 fall. PPUCTRL disable and pre-render clear lower the live line without clearing an already captured request. IRQ keeps its close-edge level through the instruction poll. [HWC.02][HWP.03]
- **Revision 7:** OAMADDR clears during sprite tile loading on dots 257–320 while rendering, and evaluation starts at OAMADDR; the first sprite found there acts as sprite 0. [HWP.06]
- **Revision 6:** A `$2000` write raises an NMI only when bit 7 changes from 0 to 1 while vblank is set; rewriting it with bit 7 still set does not retrigger NMI. [HWP.03]
- **Revision 5:** The PPU fetch pipeline changed frames for programs that write scroll or PPUCTRL while rendering. Mid-frame scroll writes now render scanline by scanline. [HWP.04][HWP.09]

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
  battery span and its write counter [Battery saves](#battery-saves).
- `nesturbator_peek_cpu_ram` reads CPU RAM for test harnesses.
- `nesturbator_state_size`, `nesturbator_save_state` and
  `nesturbator_load_state` save and restore the whole machine [Save states](#save-states).

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
unchanged; a refused load never counts; see [Save states](#save-states).
`NULL` gives 0.

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

## Save states

A state is an exact snapshot of the whole machine at a frame boundary and is
byte-identical across platforms for the same core build and content. The
supported pairing is the exact core build and exact content file that created
the state. This is support guidance, not an additional loader check: the native
loader checks format 4 and cartridge identity using mapper, submapper, region,
and the ROM CRC-32 of the 16 header bytes, PRG ROM and CHR ROM. The behavior
revision is stored but not enforced, as described below. The core handles only
the native state payload; host files and disk containers are outside this API.
The library does no file I/O and never allocates for a state; the host owns the
buffer and decides where states live.

The current state format is 4 and the behavior revision is 10. Format 4 adds
the DMC silence flag and counts silent output cycles in its saved bit position.
Format-3 states are refused without conversion. It retains the format-3 interrupt
histories:
the sampled NMI line, captured request, last instruction poll, retained
close-edge IRQ level and accepted IRQ decision independently of the live IRQ
line. It also stores an active BRK/IRQ/NMI entry's origin, next unperformed bus
action, selected vector pair and partial low-vector byte. Only reachable
completed-action stages load. The CPU records IRQ acceptance at the
instruction-specific hardware poll point, using old I for CLI, SEI and PLP and
retaining the first branch poll through later cycles (with the page-cross poll
added). A source cleared after acceptance does not revoke the pending IRQ before
entry arbitration. NMI wins that shared arbitration; an IRQ still asserted after
RTI restores I can be accepted again. State distinguishes a recognized NMI
request from an inhibited candidate that still requires the held input. Normal
instruction execution drains a restored entry
before continuing at the next instruction boundary. Public save/load uses
frame-boundary host calls and exposes no exact mid-entry stop; the core remains
synchronous. Exact race and loaded-entry cutpoints are exercised by internal
tests, while core, runner and libretro replay tests cover public frame-boundary
continuation.

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
RAM (when the board has it) follow. The video frame buffer, caller's PCM output
and transient staged PCM are not in a state; APU sample phase and synthesizer
and filter history are machine state and are serialized. This distinction also
holds in an internal snapshot before the first output frame: `core.nmi_state`'s
`test_frame_zero_empty_pcm_preserves_history` saves at frame 0 with no PCM
staged and nonzero audio history, restores it canonically, and checks empty
drain plus future continuation. Frame numbers and ticks continue from the saved
point: the first frame after a load reports the number and ticks of the
uninterrupted run.

Battery RAM and the save generation: a state includes the battery RAM. Loading
a state whose battery bytes differ from the instance's raises `nesturbator_save_generation`
once, so the host writes the `.sav`. Loading a state whose battery bytes match,
however many times, never moves it, and neither does a refused load, so
repeated loads such as rewind never trigger a write.

Format 4 rejects every other format version, including versions 1, 2 and 3;
states are never converted. Keep rewind buffers in
memory, and expect user-kept states to need re-creating after a release that
raises the format version. The behaviour revision is recorded and not
enforced: after an emulation fix an old state still loads, but its later frames
may differ from the run that saved it.

| Format version | Introduced |
|---|---|
| 1 | first release that contains save states |
| 4 | DMC silence flag and continuous bit-cycle position |

A format bump ships under a `feat:` title with a changelog line saying older
states are refused. It does not change `NESTURBATOR_ABI_VERSION`, because the
C interface is the same.

Battery RAM is part of the PRG RAM, so a load replaces it. Rewinding past an in-game save reverts the save RAM.

The `core.state_fork.*` tests (one per committed game, plus
`core.state_fork.stress` and `core.state_fork.nmi`) fork games on all six boards,
a DMA and interrupt stress program (on NROM and on MMC3), and an owned NMI
program at public frame boundaries. The NMI program records a one-based guest
entry sequence, changes the visible backdrop, and keeps pulse audio active.
Each saved fork restores into a differently run and poisoned instance; later
native state, frame, tick, PCM, and independently expected guest events must
match the uninterrupted timeline. The board and DMA/interrupt matrices remain
registered, and the tests also show that saving every frame changes nothing.

The `core.state_reject` test shows the refusals. It loads each game's state into
every other game, flips every byte of a state and expects the status of the
first failing check, truncates the state to every shorter length, and sets every
bounded field just past each of its bounds (with the CRC-32 recomputed) and at
the type's limits, expecting `NESTURBATOR_ERR_STATE_INVALID`. Fields at their
bounds are loaded and run for a frame. After every refusal the instance and
`nesturbator_save_generation` are unchanged. The same test runs under the
address and undefined-behaviour sanitizers in the `asan` preset.

## Status codes

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
