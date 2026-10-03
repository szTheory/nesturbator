# nesturbator

A NES emulator core in C: a library you can embed, a headless runner for
automation, and a libretro adapter.

**Status: Phase 2, the CPU.** The library, the runner and the libretro core
build and run. The 6502 core matches the public 65x02 test vectors on every
opcode and every bus cycle, and does not yet drive frames.
With no cartridge loaded, the core outputs a fixed test card and silence. The
PPU, APU, ROM loading and the CPU running games come in later phases. The plan
lives in [`.planning/`](.planning/).

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
| `asan` | Every test except the RetroArch launch passes under AddressSanitizer and UBSan, with any report fatal |
| `nofp` | The core builds with `-mgeneral-regs-only` and passes the tests labelled `abi` |
| `hygiene` | The tree holds no personal data and no unlisted ROM or binary file, every GitHub Action is pinned to a commit, and the C sources are formatted |
| `vectors-full` | The full 65x02 vector set, fetched by git at the commit in `tests/vectors/pins.txt` and checked file by file, matches the CPU on every test; needs the network and fails without it |

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
`runner.write_hashes` runs `tests/cmake/write_hashes.cmake`, which CI uses
to write each platform's `hashes.txt`, and `runner.write_hashes.content`
requires that file to equal `tests/runner/hashes.txt` byte for byte, with LF
line endings only.
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
test. `retroarch.compare` checks `compare_frame`, the tool the RetroArch test
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
refused as inside the source tree. A refused directory that does not exist
yet is not created.
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
platforms. Each leg also checks its three archives and writes its hashes for
frames 1 and 3:

```sh
cmake -DBUILD=build/ci -DOUT=hashes.txt -P tests/cmake/write_hashes.cmake
```

The `hygiene` job runs the `hygiene` lane, `asan` runs `asan` with Clang 18,
and `nofp` runs `nofp` with GCC 14 on Linux x64 and arm64. `title` requires
the pull-request title to be a Conventional Commit. `hash-equality` requires
the six `hashes.txt` files to be byte-identical, so a platform that computes
a different frame fails the run. The branch rules require one check, `CI
required`, which passes only when every other job succeeded. Every action is
pinned to a commit SHA, and Dependabot proposes updates weekly.

`.github/workflows/nightly.yml` runs the `vectors-full` lane every night at
04:17 UTC on Ubuntu 24.04, on demand, and on pull requests that change a
file that can change what its tests run: the workflow, the root
`CMakeLists.txt`, `CMakePresets.json`, `tests/CMakeLists.txt`,
`tests/vectors/`, `tests/cpu/`, `tools/vecconv/`, `src/cpu.c`,
`src/internal.h`, `include/nesturbator.h` and the scripts `fetch_vectors.cmake`,
`vectors_full_run.cmake` and `vectors_sample_match.cmake` in `tests/cmake/`.
It is outside `CI required`, so a red nightly blocks no merge. It fetches the full set at
its pin on every run, with no cache, because a cache used every night would
stop the fetch from ever being tested; offline, or with any file differing
from `tests/vectors/pins.txt`, it fails rather than skips. A scheduled run
keeps one open issue labelled `nightly`: a failure opens it, or updates it,
with the run's URL and the failing keys (`65x02/<xx>`, `fetch`,
`sample-match`), and the next passing run closes it. It holds no secret and
makes no commit; its token can only read the repository, and only the report
job can write issues.

Locally, the lane keeps the fetched files in `build/vectors-full/vectors-full`.
To keep them somewhere that survives a clean build, set the cache variable
`NESTURBATOR_VECTORS_DIR` in an untracked `CMakeUserPresets.json`, for example
a configure preset that inherits `vectors-full`. The files go in its
`65x02-src` subdirectory, which the fetch replaces only when it holds the
marker file `.nesturbator-vectors` that the fetch wrote; any other
`65x02-src` there fails the fetch. The fetch refuses a directory inside the
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

Before publishing, the workflow requires exactly the 18 expected archive
names, verifies each archive this way, and confirms that a copy with one byte
changed fails. Pull-request rules are in [CONTRIBUTING.md](CONTRIBUTING.md);
security reports go through [SECURITY.md](SECURITY.md).

## The colour table

`src/palette_ntsc.c` maps each native pixel value to an XRGB8888 colour. It is
generated by `tools/palgen`, a host tool built with the project but not
linked into the library, from the NTSC signal levels, phases and emphasis
rules on the NESdev Wiki (the page revisions are named in the file's header).
A host reads it with `nesturbator_get_palette`; it is display data only and
never part of a hash. Do not edit the table by hand; change palgen and
regenerate:

```sh
build/ci/tools/palgen/palgen src/palette_ntsc.c
```

The `ci` build puts the library at `build/ci/libnesturbator.a` and the runner
at `build/ci/runner/nesturbator-run`. The public header is
`include/nesturbator.h`.

## The runner

`nesturbator-run` runs the core without a window. In this phase it takes no
cartridge, so every frame is the built-in test card.

```sh
nesturbator-run --frames N [--hash-frame N]... [--dump-frame N:FILE]...
```

- `--frames N` runs N frames (N is 1 or more). It is required; without it
  the runner prints its usage and exits 2.
- `--hash-frame N` prints a line after frame N has run. N must be between 1
  and the `--frames` value. The option can be repeated.
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

In this phase the core starts with no content and shows the test card, with
silence: in RetroArch, use "Start Core", or launch it with `-L` and no content
path. Loading a game is not supported until the cartridge phase; the core
refuses any content. It sends XRGB8888 frames of 256x240 and one batch of
stereo samples per frame at 48000 Hz.

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
NESTURBATOR_VERSION=0.1.0
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

To check RetroArch's picture without looking at it:

```sh
ctest --preset ci -L retroarch
```

This runs `retroarch.testframe`. It starts RetroArch with the
configuration `build/ci/retroarch/test.cfg`, generated from
`tests/retroarch/test.cfg.in`, so your own RetroArch settings are never read.
That configuration points every directory and file RetroArch uses under
`build/ci/retroarch`, turns off content history, and stops the macOS app
unpacking its bundled assets into your RetroArch directory. RetroArch runs the core
for 5 frames and writes a screenshot of the core's frame. `sips` converts it
to BMP, and `compare_frame` requires it to equal the runner's frame 5 at
exactly 256x240, pixel for pixel. The test also lists RetroArch's directory
in your home folder before and after the run, and fails if anything in it was
created, changed or removed. RetroArch opens a window, so the test needs a
logged-in desktop session. Set `NESTURBATOR_RETROARCH` to use a RetroArch
binary somewhere else. On other systems, or when RetroArch is not installed,
the test reports itself skipped.

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
