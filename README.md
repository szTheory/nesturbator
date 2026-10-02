# nesturbator

A NES emulator core in C: a library you can embed, a headless runner for
automation, and a libretro adapter.

**Status: preparation.** There is no emulator code yet. The plan for the first
releases lives in [`.planning/`](.planning/).

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
