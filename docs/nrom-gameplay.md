# Qualified NROM gameplay consumer

This guide records the upstream C package scenario used by `install.gameplay`.
It proves a bounded DABG transition through a newly configured consumer after
the installed prefix has been moved. It is provider-side evidence for ABI 1,
behavior revision 10, native state format 4 and the NTSC route in the
accompanying machine profile. Read the semantic version from the installed
header macros and `lib/pkgconfig/nesturbator.pc`; for a published package,
confirm it against the Release Please tag and the separate provider receipt.
The scenario establishes no LibFuturo host result.

The library archive stores this guide as `docs/nrom-gameplay.md`, the provider
guide and machine profile under `docs/`, the project notice at its root, and
the verbatim license at `docs/licenses/DABG-ZLIB-LICENSE.txt`. In the matching
tagged source, the [ROM manifest](../tests/roms/manifest.txt),
[DABG notice](../THIRD-PARTY-NOTICES.md),
[verbatim license](../tests/roms/licenses/DABG-ZLIB-LICENSE.txt),
[public API guide](api.md), [consumer driver](../tests/cmake/qualified_consumer.cmake)
and [profile checker](../tests/cmake/provider_profile.cmake) provide source
navigation and implementation details. These source-only files and recipes
are not part of the ROM-free binary package; the DABG image remains a
manifest-listed test fixture.

The DABG Zlib license permits use, including commercial use, and
redistribution subject to its conditions: preserve the origin, mark altered
source as altered, and do not remove or change the notice from a source
distribution. The installed notice and verbatim license above carry those
terms with the provider documents.

## Selected ROM and source-to-RAM proof

The selected image is `tests/roms/dabg.nes`, already admitted by the
[ROM manifest](../tests/roms/manifest.txt): source commit
`5ecc60b6af3f726851bfeb1c2555388c5953e0c0`, Zlib license, SHA-256
`eca79b9d0b546e96c1bc73099e239539fb9c7773ff10a431bb3a9cd6763208ca`. The
consumer verifies that digest before execution. The image is an NROM-256 with
CHR RAM; its pinned [DABG source](https://github.com/NovaSquirrel/DABG/tree/5ecc60b6af3f726851bfeb1c2555388c5953e0c0)
and linker configuration are available at that exact commit.

The bounded source-to-RAM mapping was reproduced from the pinned DABG tree
with cc65 commit `555282497c3ecf8b313d87d5973093af19c35bd5`, tree
`752d8e4c5492cd6c77cb5ddec0561ace5ead72dc`, archive SHA-256
`3d51ab140c1f15e06c3ae5c49934a4c291c3314e3e440ef42a99bbe9a1f73d02`.
Keep tool checkouts and generated files in external or ignored scratch. The
following commands build the tools, verify the DABG source pin and add debug
mapping to its canonical [`mk.bat`](https://github.com/NovaSquirrel/DABG/blob/5ecc60b6af3f726851bfeb1c2555388c5953e0c0/mk.bat)
recipe:

```sh
SCRATCH=/path/to/build-owned-scratch
cd "$SCRATCH"
git clone --no-checkout https://github.com/cc65/cc65.git cc65
git -C cc65 checkout 555282497c3ecf8b313d87d5973093af19c35bd5
git -C cc65 archive --format=tar 555282497c3ecf8b313d87d5973093af19c35bd5 | shasum -a 256
make -C cc65/src ca65 ld65
cc65/bin/ca65 --version
cc65/bin/ld65 --version

git clone --no-checkout https://github.com/NovaSquirrel/DABG.git dabg
git -C dabg checkout 5ecc60b6af3f726851bfeb1c2555388c5953e0c0
git -C dabg archive --format=tar 5ecc60b6af3f726851bfeb1c2555388c5953e0c0 | shasum -a 256
cd "$SCRATCH/dabg"
"$SCRATCH/cc65/bin/ca65" -g dabg.s -o dabg.o -l dabg.lst
"$SCRATCH/cc65/bin/ld65" -C nrom256.x dabg.o -o dabg.nes \
  -m dabg.map.txt -Ln dabg.labels.txt --dbgfile dabg.nes.dbg
```

Two builds produced identical ROM, map and debug files, and the rebuilt ROM
matches the manifest digest above. The linker map places `ZEROPAGE` at
`$0010..$00FA` and `BSS` at `$0300..$04F8`. The debug labels place
`AttractMode=$0011`, `PlayerEnabled=$0044`, `PlayerPX=$0048`,
`PlayerPYH=$004C` (four entries each), and `LevelNumber=$00DD`. The assembled
bytes at offsets `$07AA` and `$07DB` after the 16-byte iNES header are `95 48`,
the zero-page indexed `STA $48,X` operands used by DABG's
[`RunPlayer`](https://github.com/NovaSquirrel/DABG/blob/5ecc60b6af3f726851bfeb1c2555388c5953e0c0/player.s#L98-L145)
right-movement update. Its
[`DispPlayer`](https://github.com/NovaSquirrel/DABG/blob/5ecc60b6af3f726851bfeb1c2555388c5953e0c0/player.s#L777-L879)
derives sprite X positions from `PlayerPX` and Y positions from `PlayerPYH`.
The scenario reads only those mapped bytes, `AttractMode`, and `LevelNumber`
through `nesturbator_peek_cpu_ram`.

`LevelNumber` alone is not a gameplay oracle: the title/menu code initializes
it and level setup increments it. The accepted state requires all of
`AttractMode == 0`, `PlayerEnabled[0] == 1`, and `LevelNumber == 1`, followed
by a right-input-caused increase in `PlayerPX[0]`. That final requirement
makes a startup counter change insufficient.

## Reproducible route and measurements

Frames are one-based. Before every frame, the consumer sets both controller
masks. Port 1 stays neutral. Port 0 uses this fixed sequence:

| Frames | Port 0 mask | Purpose |
|---|---:|---|
| 1–2 | `0x00` | Boot |
| 3, 6, …, 57 | `A` (`0x01`) for one frame | Advance the title and menu choices |
| 58–79 | `0x00` | Let level setup settle |
| 80–110 | `Right` (`0x80`) | Move the enabled player through the first level |

Six exploratory replays were used, all within the plan's 600-frame and
15-second per-run caps. They established the three-frame A pulse cadence and
the first stable movement window without relying on the earlier unverified
60-frame neutral guess. The final 110-frame route is below the cap.

The numeric measurements below are historical observations from the pinned
`bf9aacb3244ea40b2c2afb62651ef063c7452c1f` source. Each current runtime report
supplies the identity and measured predicates for the package it actually
tests; these historical values are explanatory and do not replace that report.

The fresh reset measurement reaches `LevelNumber == 1` at frame 59. At frame 80 the
player is enabled at X=40; with Right held, X advances by two each frame,
reaching X=70 at frame 95. The native-pixel check compares frames 80 and 95 in
the source-derived player sweep rectangle `[38, 206, 48, 20]` and requires at
least one changed native `uint16_t` pixel; the measured count was 238 of 960
pixels. Full-frame FNV-1a hashes are retained as supporting evidence, not as a
pass predicate.

Gameplay PCM is measured only over frames 61–76, after the level transition.
Across those 16 frames the accepted run emitted 12,779 samples (798 or 799 per
frame), with 12,647 nonzero samples and peak absolute amplitude 4,926. The
predicate requires a complete window, at least one nonzero sample, and a
nonzero peak. Rates come from `nesturbator_get_info`: NTSC video
`39375000/655171` frames per second and mono signed PCM `48000/1` samples per
second. The observed native output is 256×240.

The retained runtime JSON records the header and library version/ABI/behavior
and state-format values, the `ntsc-only-base-api` route and rates, exact frame
count, RAM predicates, movement, selected pixel region and PCM summary.

## Negative controls and continued play

Two separate consumer invocations create fresh emulator instances and run the
same 110-frame budget with different input sequences. The no-input route sends zero on both ports for every frame. The
Start-only route pulses Start on frames 3, 6, …, 57, then stays neutral. Each
emulator run exits successfully only after all 110 frames; its JSON verdict
must identify missing `game_state` and `input_caused_progress` predicates.
Neither route reached the mapped first-level/player state in the measured run.

The accepted positive trace is also copied four times in memory and passed
through the same pure verdict function with one field removed at a time. The
report requires separate named rejection of missing game state, input-caused
progress, visible pixels and gameplay PCM. These synthetic traces test the
verdict logic only; they are not emulator gameplay results.

At frame 80, after the player has entered level 1, the consumer makes a copy
of the ROM, corrupts its magic byte and attempts a replacement load. The public
API must return `NESTURBATOR_ERR_CARTRIDGE` (status 7). An untouched twin has
received the same complete input prefix. The consumer checks the base API's video and
audio metadata and the mapped RAM canaries immediately, then sends the same
Right input to both instances for frames 81–110. Every frame must match in
frame number, cumulative ticks, all 61,440 native pixels, `audio_count`, each
valid PCM sample and the mapped state bytes. The refused instance continues
from player X=40 to X=100 across those 30 frames. This proves active-game
preservation for this call; it does not qualify machine snapshots or
persistence.

The buffer check replays an active prefix into a second matched pair and
exercises four refusals. A capacity one below that frame's actual 798/799
sample count and video pitch 255 return `NESTURBATOR_ERR_BUFFER_TOO_SMALL`;
NULL audio with positive capacity returns `NESTURBATOR_ERR_ARGUMENT`; NULL
audio with zero capacity returns `NESTURBATOR_ERR_BUFFER_TOO_SMALL`. Each
refusal leaves output sentinels intact. A retry with pitch 256 and capacity
exactly equal to the reference frame's sample count succeeds and matches the
twin's pixels, valid PCM, frame/tick values and RAM canaries.

The driver also has two package-boundary controls. A deliberately wrong ROM
is rejected by its SHA-256 gate before the consumer is invoked, so content
identity failure cannot count as a semantic gameplay control. A prefix that
contains only the public header fails specifically at `find_package`; it
cannot fall back to the source or build tree. Both control processes have
finite timeouts and retain their named outcomes in build-owned evidence.

## Relocated installed-package check

`tests/cmake/qualified_consumer.cmake` owns a unique scratch directory under
the build tree and marks it before use. It installs the library component (or
copies only the supplied installed prefix), configures and builds the ordinary
consumer, and runs its no-argument smoke. It then moves the prefix to a path
containing spaces, removes the earlier consumer build inside the marked
scratch directory, and verifies both are absent before a fresh configure.

The fresh consumer's package lookup is restricted to the moved prefix, with
CMake package registries and environment package lookup disabled. Its
`find_package` uses `NO_DEFAULT_PATH`. Configure-time checks require every
imported include and library path to resolve below that prefix. The driver
retains the CMake cache, package-resolution properties, compile commands,
linker command evidence, and runtime JSON under the ignored build directory;
it rejects references to source/build-tree headers and requires the moved
library in the actual link description. No shared `install.stage` fixture is
moved or removed.

Run the focused integration check with:

```sh
cmake --preset ci
cmake --build --preset ci --target nesturbator
ctest --test-dir build/ci -R '^install\.gameplay$' --output-on-failure --no-tests=error
```

The positive moved-prefix run took 1.34 seconds including two independent
consumer configures/builds. With semantic controls, the refusal twin, buffer
edges and prefix controls included, the latest `install.gameplay` CTest took
2.06 seconds. The pinned upstream rebuild and mapping are one-time
qualification work; recurring CTest uses the permitted manifest image and
committed source-backed facts, with no network access or toolchain download.
Only the concise guide is tracked. Upstream checkouts, assembler/linker
binaries, rebuilt ROMs, maps, labels, debug output and runtime evidence stay
outside tracked source or in ignored build output.
