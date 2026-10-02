# nesturbator

A NES emulator core in C: a library you can embed, a headless runner for
automation, and a libretro adapter.

**Status: Phase 1, a test frame.** The library and the runner build and run.
With no cartridge loaded, the core outputs a fixed test card and silence. CPU,
PPU, APU and ROM loading come in later phases. The plan lives in
[`.planning/`](.planning/).

## Building

You need CMake 3.25 or newer, Ninja and a C17 compiler.

```sh
cmake --workflow --preset ci    # Release, warnings as errors; builds and runs every test
cmake --workflow --preset dev   # Debug build and the same tests
```

On Windows with MSVC, use `ci-msvc` from a developer command prompt.

The `ci` build puts the library at `build/ci/libnesturbator.a` and the runner
at `build/ci/runner/nesturbator-run`. The public header is
`include/nesturbator.h`.

## The runner

`nesturbator-run` runs the core without a window. In this phase it takes no
cartridge, so every frame is the built-in test card.

```sh
nesturbator-run --frames N [--hash-frame N]...
```

- `--frames N` runs N frames (N is 1 or more).
- `--hash-frame N` prints a line after frame N has run. N must be between 1
  and the `--frames` value. The option can be repeated.

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

Exit status: 0 done, 1 failure, 2 usage error.

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
