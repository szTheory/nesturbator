# nesturbator

A NES emulator core in C: a library you can embed, a headless runner for
automation, and a libretro adapter.

**Current source tree.** The library, headless runner and libretro adapter
support battery-backed RAM, soft reset, native save states, and RetroArch
rewind and run-ahead. The 6502 core matches the public 65x02 vectors on every
opcode and bus cycle.

The library, runner and libretro core accept bounded iNES 1.0 and NES 2.0
images for mapper 0 (NROM, 16 or 32 KiB PRG), mapper 1 (MMC1, with battery
saves), mapper 2 (UxROM), mapper 3 (CNROM, 8 to 32 KiB CHR ROM), mapper 4
(MMC3) and mapper 7 (AxROM, CHR RAM), with 8 KiB CHR ROM or declared CHR RAM
where supported. This is not full game compatibility. The PPU renders
backgrounds and evaluated sprites, including palette priority, flips, 8x16
selection, clipping, sprite-zero hit and the eight-sprite limit.

With no cartridge, the fixed test card and silence remain available. The
[API guide](docs/api.md) owns the native API, battery and state contracts. The
[runner guide](docs/runner.md) documents invocation and options.

## Downloads and archives

Install the libretro archive for the operating system and architecture of the
RetroArch executable you actually launch. Choose the host executable's
architecture, not the computer's processor; an x64 RetroArch process needs the
x64 core even on an arm64 machine.

| RetroArch platform | x64 archive | arm64 archive |
|---|---|---|
| Linux | `nesturbator-VERSION-libretro-linux-x64.zip` | `nesturbator-VERSION-libretro-linux-arm64.zip` |
| macOS | `nesturbator-VERSION-libretro-macos-x64.zip` | `nesturbator-VERSION-libretro-macos-arm64.zip` |
| Windows | `nesturbator-VERSION-libretro-windows-x64.zip` | `nesturbator-VERSION-libretro-windows-arm64.zip` |

Each archive has `cores/` and `info/` directories. Extract both into the
locations shown by RetroArch's `Settings` → `Directory` → `Cores` and `Core
Info` settings, preserving the folders and filenames. The module is
`nesturbator_libretro.so` on Linux, `.dylib` on macOS and `.dll` on Windows;
the matching `nesturbator_libretro.info` file belongs in `info/`. Make sure
the frontend can read both files and that its configured directories are
writable for installation. Libretro's [manual install guide](https://docs.libretro.com/guides/download-cores/)
documents these directories and module extensions. The [release guide](docs/releases.md)
explains how to verify an archive and identify the exact loaded binary.

## Releases

The maintainer release process, version-marker ownership, package inventory
and attestation checks are in the [release guide](docs/releases.md).

## Developer guides

- [API and native state](docs/api.md)
- [Runner options and exit behavior](docs/runner.md)
- [Release archives and process](docs/releases.md)
- [Build, tests, CI and evidence limits](docs/testing.md)

## The runner

See [Runner guide](docs/runner.md) for command options, output examples, movies, stop behavior and exit statuses.

Supported NTSC MMC3: mapper 4, submappers 0/4; MMC6=no; four-screen=no.

## The libretro core

The adapter is the nesturbator_libretro module for a libretro frontend such as
RetroArch. Its checked capabilities and the existing host evidence are
documented in the [testing guide](docs/testing.md#libretro-adapter-checks).
Player installation steps are below.

## Install in RetroArch (Apple Silicon)

With RetroArch installed (see below), these two lines put the released core
in RetroArch's `cores` directory and its information file in `info`. The
first sets the version; the second downloads and extracts:

<!-- x-release-please-start-version -->
```sh
NESTURBATOR_VERSION=0.1.11
mkdir -p ~/Library/Application\ Support/RetroArch && curl -fsSL "https://github.com/szTheory/nesturbator/releases/download/v$NESTURBATOR_VERSION/nesturbator-$NESTURBATOR_VERSION-libretro-macos-arm64.zip" | tar -xf - -C ~/Library/Application\ Support/RetroArch cores info
```
<!-- x-release-please-end -->

Then start RetroArch, choose Load Core → nesturbator, then Start Core: the
test card appears.

Naming only cores and info leaves LICENSE and THIRD-PARTY-NOTICES.md outside
RetroArch directories. The [release guide](docs/releases.md) documents how the
marked version line and source-build extraction check are maintained.

## Protecting progress

RetroArch's battery saves and save states are separate files, and loading an
older state or rewinding past an in-game save can restore older battery RAM.
Back up the newest battery save independently before an upgrade or state load;
keep it outside the active save and state folders. See the [progress guide](docs/progress.md)
for the scratch-only walkthrough, compatibility limits and rollback steps.

## What it is

- An accuracy-class, deterministic NES core with no GUI of its own.
- Three deliverables: a C library, a command-line runner, and a libretro core
  for frontends such as RetroArch.
- All emulation code written here, under the MIT licence.

## ROMs

This repository contains no commercial ROM or BIOS data. Supply your own
legally obtained game images; see [ASSET_POLICY.md](ASSET_POLICY.md). The
permitted test-ROM and vector inventory, licences, hashes and regeneration
procedure are in the [testing guide](docs/testing.md#test-fixtures-and-source-policy).

## How the code is written

From hardware documentation and public test ROMs, with AI coding assistants,
and without copying from other emulators. See [PROVENANCE.md](PROVENANCE.md).

## Licence

MIT. See [LICENSE](LICENSE).

nesturbator is not affiliated with or endorsed by Nintendo. The names "Nintendo
Entertainment System", "NES" and "Famicom" are used here only to say what the
core emulates.
