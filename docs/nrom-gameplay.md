# Qualified NROM gameplay consumer

This guide records the upstream C package scenario used by `install.gameplay`.
It proves a bounded DABG transition through a newly configured consumer after
the installed prefix has been moved. The package-only reset requalified this scenario on the fixed NTSC base
(source version 0.1.12, ABI 1, behavior revision 9, state format 3). The consumer
port source is recorded in its plan summary; this remains unpublished candidate
evidence for this source tree; it does not identify a published package or establish a LibFuturo host
result.

## Selected ROM and source-to-RAM proof

The selected image is `tests/roms/dabg.nes`, already admitted by
[`tests/roms/manifest.txt`](../tests/roms/manifest.txt): source commit
`5ecc60b6af3f726851bfeb1c2555388c5953e0c0`, Zlib license, SHA-256
`eca79b9d0b546e96c1bc73099e239539fb9c7773ff10a431bb3a9cd6763208ca`. The
driver verifies that digest before invoking the consumer. The image is an
NROM-256 with CHR RAM; its pinned source and linker configuration are available
at [DABG](https://github.com/NovaSquirrel/DABG/tree/5ecc60b6af3f726851bfeb1c2555388c5953e0c0).

Nesteroids was investigated first because it was the preferred MIT candidate.
Its pinned source is `ae65d78048af68f3e735c49d77a7f37e976a7afe` (source archive
SHA-256 `a5010fbaf75071041543aa28b974c4ab3f9251e29f35ef4f04f1544e76076142`).
The environment had no compatible `ca65` or `ld65`, so the official cc65
`V2.19` source tag was built in isolated scratch: commit
`555282497c3ecf8b313d87d5973093af19c35bd5`, tree
`752d8e4c5492cd6c77cb5ddec0561ace5ead72dc`, source archive SHA-256
`3d51ab140c1f15e06c3ae5c49934a4c291c3314e3e440ef42a99bbe9a1f73d02`, Zlib
license. The resulting tools identify themselves as `V2.18 - Git 5552824`.
The tool source and failed preferred-game route can be reconstructed with
these bounded, one-time commands. Keep all checkouts and generated files in
an external or ignored scratch directory:

```sh
SCRATCH=/path/to/build-owned-scratch
cd "$SCRATCH"
git clone --no-checkout https://github.com/cc65/cc65.git cc65
git -C cc65 checkout 555282497c3ecf8b313d87d5973093af19c35bd5
git -C cc65 archive --format=tar 555282497c3ecf8b313d87d5973093af19c35bd5 | shasum -a 256
make -C cc65/src ca65 ld65
cc65/bin/ca65 --version
cc65/bin/ld65 --version

# Nesteroids compile.bat recipe, repeated from a second fresh source copy.
git clone --no-checkout https://github.com/battlelinegames/nesteroids.git nesteroids
git -C nesteroids checkout ae65d78048af68f3e735c49d77a7f37e976a7afe
git -C nesteroids archive --format=tar ae65d78048af68f3e735c49d77a7f37e976a7afe | shasum -a 256
cd "$SCRATCH/nesteroids"
"$SCRATCH/cc65/bin/ca65" ./src/nesteroids.asm -g -o nesteroids.o
"$SCRATCH/cc65/bin/ld65" -o nesteroids.nes -C nesteroids.cfg nesteroids.o \
  -m nesteroids.map.txt -Ln nesteroids.labels.txt --dbgfile nesteroids.nes.dbg
```

Both ROMs reproduced each other, but their SHA-256
`6c9c2e3a587b1ed81c3448dea52048a5bdbbb4641a066fe01b959ae7dba28259` does not
match the manifest image (`a30dcfb12c1447dc121619c93b4083d2b83c7405a883ed490c3a59e52ce74f36`).
The checked-in symbol files also disagree: `game_screen` is reported at
`$A962` and `$A64D`, while `player_shot_count` is reported at `$0594` and
`$0595`. No Nesteroids address was used as an oracle.

The DABG source was then rebuilt twice from the pinned tree with the identified
cc65 tools. The source recipe is
[`mk.bat`](https://github.com/NovaSquirrel/DABG/blob/5ecc60b6af3f726851bfeb1c2555388c5953e0c0/mk.bat);
the bounded mapping builds added debug output:

```sh
SCRATCH=/path/to/build-owned-scratch
cd "$SCRATCH"
git clone --no-checkout https://github.com/NovaSquirrel/DABG.git dabg
git -C dabg checkout 5ecc60b6af3f726851bfeb1c2555388c5953e0c0
git -C dabg archive --format=tar 5ecc60b6af3f726851bfeb1c2555388c5953e0c0 | shasum -a 256
cd "$SCRATCH/dabg"
"$SCRATCH/cc65/bin/ca65" -g dabg.s -o dabg.o -l dabg.lst
"$SCRATCH/cc65/bin/ld65" -C nrom256.x dabg.o -o dabg.nes \
  -m dabg.map.txt -Ln dabg.labels.txt --dbgfile dabg.nes.dbg
```

Both builds produced identical ROM, map and debug files. The rebuilt ROM digest
matches the permitted fixture exactly. The linker map places `ZEROPAGE` at
`$0010..$00FA` and `BSS` at `$0300..$04F8`; the debug labels put
`AttractMode=$0011`, `PlayerEnabled=$0044` (four entries in this build),
`PlayerPX=$0048` (four entries), `PlayerPYH=$004C` (four entries), and
`LevelNumber=$00DD`. These values match the allocation order in pinned
[`memory.s`](https://github.com/NovaSquirrel/DABG/blob/5ecc60b6af3f726851bfeb1c2555388c5953e0c0/memory.s#L1-L34)
and its later `LevelNumber` declaration. The assembled bytes at offsets `$07AA`
and `$07DB` (after the 16-byte iNES header) are `95 48`, the zero-page indexed
`STA $48,X` operands used by the source's
[`RunPlayer`](https://github.com/NovaSquirrel/DABG/blob/5ecc60b6af3f726851bfeb1c2555388c5953e0c0/player.s#L98-L145)
right-movement update. Its
[`DispPlayer`](https://github.com/NovaSquirrel/DABG/blob/5ecc60b6af3f726851bfeb1c2555388c5953e0c0/player.s#L777-L879)
also derives sprite X positions from `PlayerPX` and Y positions from
`PlayerPYH`. The scenario reads only those mapped bytes, `AttractMode`, and
`LevelNumber` through `nesturbator_peek_cpu_ram`.

`LevelNumber` alone is not a gameplay oracle: the title/menu code initializes
it and level setup increments it. The accepted state requires all of
`AttractMode == 0`, `PlayerEnabled[0] == 1`, and `LevelNumber == 1`, followed
by a right-input-caused `PlayerPX[0]` increase. That final requirement makes a
startup counter change insufficient.

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
and state-format values, the `ntsc-only-base-api` route and rates, exact frame count, RAM
predicates, movement, selected pixel region and PCM summary. It labels this
source build `unpublished`.

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
