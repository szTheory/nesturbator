# nesturbator

A NES emulator core in C: a library you can embed, a headless runner for
automation, and a libretro adapter.

**Status: Phase 1, a test frame.** The library, the runner and the libretro
core build and run.
With no cartridge loaded, the core outputs a fixed test card and silence. CPU,
PPU, APU and ROM loading come in later phases. The plan lives in
[`.planning/`](.planning/).

## Building

You need CMake 3.25 or newer, Ninja and a C17 compiler.

```sh
cmake --workflow --preset ci    # Release, warnings as errors; builds, runs every test, writes the archives
cmake --workflow --preset dev   # Debug build and the same tests
cmake --workflow --preset asan  # the same tests under AddressSanitizer and UBSan
cmake --workflow --preset nofp  # core built with -mgeneral-regs-only; the abi checks
cmake --workflow --preset hygiene  # tree contents, action pins and formatting
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

The `abi` tests hold the core to integer arithmetic and the C memory
functions: a text scan of `src/` and `include/` for `float`, `double` and
floating literals; an `nm` check that the library needs no symbol beyond
`memcpy`, `memmove`, `memset`, `memcmp`, `malloc`, `free` and the
toolchain's fortify and stack-protector helpers (plus `bzero` on macOS); an
`nm` check that it defines no writable data; and a fixture with a `double`
multiply that these checks must reject.

The `hygiene` tests: `hygiene.tree` runs `scripts/hygiene.sh --tree`, which
rejects home-directory paths, email addresses other than GitHub noreply, ROM
and save files, and any file git treats as binary unless
`tests/roms/manifest.txt` lists it. `hygiene.action_pins` checks that every
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
`host.convert` checks the colour conversion the runner and the libretro
adapter share (`host/convert.c`): each native pixel's low 9 bits pick the
table entry, and the input and output row pitches are honoured.
`header.c` and `header.cxx` compile the public header alone as C17 and as
C++ with warnings as errors and check its struct layout. `runner.sha256`
checks the runner's SHA-256 against the FIPS 180-4 example digests, and
`version.consistency` checks that `version.txt` matches the header's version
macros and the libretro `.info` file's `display_version`. `libretro.vendored`
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
`install.stage` installs the library into `build/ci/stage`, and
`install.consumer` builds `tests/consumer`, a separate project that includes
only `<nesturbator.h>`, against that install with `find_package`, then runs
one frame with it.

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
passes. `.github/workflows/release.yml` then runs the full CI on that exact
commit and publishes the release with 18 archives (library, runner and
libretro for six platforms) and `SHA256SUMS`. `SHA256SUMS` carries a
build-provenance attestation that covers every archive. To check a download:

```sh
gh attestation verify FILE --repo <owner>/nesturbator
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

- `--frames N` runs N frames (N is 1 or more).
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
`build/ci/retroarch` and turns off content history. RetroArch runs the core
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

## How the code is written

From hardware documentation and public test ROMs, with AI coding assistants,
and without copying from other emulators. See [PROVENANCE.md](PROVENANCE.md).

## Licence

MIT. See [LICENSE](LICENSE).

nesturbator is not affiliated with or endorsed by Nintendo. The names "Nintendo
Entertainment System", "NES" and "Famicom" are used here only to say what the
core emulates.
