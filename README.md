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

DMC transfers now overlap OAM halt/dummy/alignment cycles. DMC sample reads
take priority, and OAM resumes on GET without skipping or duplicating a byte.
Interior overlaps add two cycles; final windows can add one or three. This
was introduced in behavior revision 11.

Behavior revision 12 separates internal CPU data from the external bus. A DMC
sample ending in $15-$17 can select status or a controller while the CPU is
parked in $4000-$401F. Status reads acknowledge the frame IRQ without replacing
the external sample byte; controller reads retain external upper bits. These
owned checks preserve the existing per-cycle controller-clock profile. Pinned
AccuracyCoin ROM acceptance remains pending.

Behavior revision 13 preserves a one-cycle aborted DMA when explicit disable
or a non-looping last byte stops the reader two or three clocks before reload.
A write suppresses that halt; normal sample requests still retry on writes.
The selected early RP2A03G model omits the later-G/H unexpected extra reload.
Both revision-dependent outcomes remain accepted by the pinned ROM contract;
owned checks do not establish fresh ROM acceptance.

Behavior revision 14 completes armed or imminent explicit refills after
reader disable, and undriven reads reconnect to external data after $4015
selection ends. A just-fetched sample waits past an immediately adjacent
output boundary. This last ordering is an inferred compatibility model for
the pinned early-G looping sweep; owned timing, audio and native checks do
not establish universal silicon behavior. Native format 6 and ABI 1 remain
unchanged. Fresh full-ROM acceptance of this revision is still pending.

With no cartridge, the fixed test card and silence remain available. The
checked MMC3 support clause is:

Supported NTSC MMC3: mapper 4, submappers 0/4; MMC6=no; four-screen=no.

These support details describe the current source tree. A released or locally
installed binary must be identified from its own artifact and provenance.

## Install and identify the core

Install the libretro archive for the operating system and architecture of the
RetroArch executable you actually launch. Choose the host executable's
architecture, not the computer's processor; an x64 RetroArch process needs the
x64 core even on an arm64 machine.

| RetroArch platform | x64 archive | arm64 archive |
|---|---|---|
| Linux | `nesturbator-VERSION-libretro-linux-x64.zip` | `nesturbator-VERSION-libretro-linux-arm64.zip` |
| macOS | `nesturbator-VERSION-libretro-macos-x64.zip` | `nesturbator-VERSION-libretro-macos-arm64.zip` |
| Windows | `nesturbator-VERSION-libretro-windows-x64.zip` | `nesturbator-VERSION-libretro-windows-arm64.zip` |

Download the selected archive from the project's [published releases](https://github.com/szTheory/nesturbator/releases).
Each archive has `cores/` and `info/` directories. Extract both into the
locations shown by RetroArch's `Settings` → `Directory` → `Cores` and
`Core Info` settings, preserving the folders and filenames. The module is
`nesturbator_libretro.so` on Linux, `.dylib` on macOS and `.dll` on Windows;
the matching `nesturbator_libretro.info` file belongs in `info/`. Make sure
the frontend can read both files and that its configured directories are
writable for installation. Libretro's [manual install guide](https://docs.libretro.com/guides/download-cores/)
documents these directories and module extensions. The [release guide](docs/releases.md)
explains archive verification and exact loaded-build identity. Guides live in
the source repository and are not inside the downloaded archives.

On Apple Silicon, these two lines use RetroArch's default macOS data directory,
with the core in `cores` and its information file in `info`. If your configured
directories differ, use the manual extraction instructions above with those
destinations. The first line sets the version; the second downloads and extracts:

<!-- x-release-please-start-version -->
```sh
NESTURBATOR_VERSION=0.1.11
mkdir -p ~/Library/Application\ Support/RetroArch && curl -fsSL "https://github.com/szTheory/nesturbator/releases/download/v$NESTURBATOR_VERSION/nesturbator-$NESTURBATOR_VERSION-libretro-macos-arm64.zip" | tar -xf - -C ~/Library/Application\ Support/RetroArch cores info
```
<!-- x-release-please-end -->

Naming only cores and info leaves LICENSE and THIRD-PARTY-NOTICES.md outside
RetroArch directories. The [release guide](docs/releases.md) documents how the
marked version line and source-build extraction check are maintained.

The core's displayed name and release version help you check your selection;
the release version does not change emulated output. They do not prove the
exact binary. For exact identity, retain the module path used by
the active launch and compare its digest with the matching release/CI artifact
or pinned local build. If the loaded path or source cannot be established,
report the build as unknown. The [release guide](docs/releases.md#identifying-a-loaded-build)
has the verification steps. macOS may restrict a downloaded quarantined
module; verify its source and digest and follow the current system security
guidance rather than assuming a binary is signed or notarized. See Apple's
[notarization guidance](https://developer.apple.com/documentation/security/notarizing-macos-software-before-distribution).

## Load content

In RetroArch, load `nesturbator` from `Main Menu` → `Load Core` → `Core`, then
choose `Load Content` and select an image supported above. When launching from
an existing ES-DE or Steam library entry, keep that entry and its configured
core association. Use the host's installed UI wording if the menus differ.

To see the no-content test card, load the core and choose `Start Core`. No
commercial game is needed to check that the adapter loads. RetroArch's
[starting a game guide](https://docs.libretro.com/guides/starting-a-game/)
describes the core/content route.

## Controls

RetroArch owns host-device assignment, controller profiles, remaps and
hotkeys. A libretro port is the core's logical player input; **Device Index**
selects which connected host controller feeds that port. In the launch route
you use for play, check `Settings` → `Input` → `RetroPad Binds` →
`Port 1 Controls` → `Device Index`. With one controller, assign it to Port 1 and leave
Port 2 unassigned; do not select the same device for both ports. For two
players, assign distinct intended devices to the two active ports. The unused
choice and menu labels vary by host driver/version. RetroArch documents the
[Device Index controls](https://docs.libretro.com/guides/input-and-controls/).

Check remaps under the active core/content's Quick Menu → Controls and inspect
hotkeys if a button opens a host menu instead of reaching the game. A separate
RetroArch launch can use different settings from ES-DE/Steam, so verify the
same launch route. The existing logical-port tests and one dated actual-launch
readback have limited scope: they do not cover every button, reconnection or
handheld fallback. Changing a port does not diagnose an unclassified graphics
or slowdown report; see [testing evidence](docs/testing.md#disposable-progress-fixtures).

## Protect progress

Native state format 6 adds the expiring DMA abort countdown. It retains separate
internal CPU and external data latches, the DMC silence flag and the continuously
clocked bit position. Formats 1 through 5 are refused; there is no converter.
Battery saves keep their separate format.

RetroArch's battery saves and save states are separate files, and loading an
older state or rewinding past an in-game save can restore older battery RAM.
Back up the newest battery save independently before an upgrade or state load;
keep it outside the active save and state folders. See the [progress guide](docs/progress.md)
for the scratch-only walkthrough, which stops on command failure, compatibility
limits and rollback steps.

## Troubleshooting

[Report a scene or reproduce a permitted fixture](docs/reproduction.md)


- **The core is missing or may be the wrong build:** Confirm the archive OS and
  executable architecture, then check the configured `Cores` and `Core Info`
  directories and select nesturbator explicitly. A displayed name/version is
  only a first check; if the actual loaded module path and digest are unknown,
  keep the identity unknown.
- **Input is ignored or opens a host action:** Check Device Index for both
  ports, the active RetroPad remap and hotkeys through the same launch route.
  Leave Port 2 unassigned with one controller. A port change does not establish
  the cause of a graphics or pacing symptom.
- **A battery save is missing:** Check the active frontend's configured Save
  Files directory and close content cleanly. Paths vary across installs; the
  core does not write the file itself.
- **A snapshot is rejected or loads automatically:** Preserve the newest
  battery save and both backup sets, isolate the questionable state, disable
  automatic loading, and try a fresh launch of an identified core. Native
  refusal does not establish how a host container recovers.
- **Content does not load:** Check the image format, mapper/submapper and
  support limits above. A mapper-family match alone does not promise full game
  compatibility.

## About and developer guides

This project ships three deliverables: an embeddable C library, a headless
command-line runner, and a libretro core for frontends such as RetroArch. It
is an accuracy-class, deterministic NES core with no GUI of its own. All
emulation code is written here from hardware documentation and public test
ROM behavior, with AI coding assistants and without copying other emulators.
See [PROVENANCE.md](PROVENANCE.md). The adapter's checked capabilities and
existing host evidence are documented in the
[testing guide](docs/testing.md#libretro-adapter-checks).

This repository contains no commercial ROM or BIOS data. Supply only legally
obtained game images; see [ASSET_POLICY.md](ASSET_POLICY.md). The permitted
test-ROM and vector inventory, licences, hashes and regeneration procedure
are in the [testing guide](docs/testing.md#test-fixtures-and-source-policy).

- [API, battery and native state contracts](docs/api.md)
- [Runner options, output, stop behavior and exit statuses](docs/runner.md)
- [Release archives, identity and maintainer process](docs/releases.md)
- [Build, tests, CI and evidence limits](docs/testing.md)

For a generated Release Please merge commit, ordinary push CI skips its
redundant runner jobs while the Release workflow runs reusable full CI and
keeps the publication gate; main-push Nightly remains enabled. See the
[testing](docs/testing.md#continuous-integration) and
[release](docs/releases.md#release-commit-ci-ownership) guides.

MIT. See [LICENSE](LICENSE). nesturbator is not affiliated with or endorsed
by Nintendo. The names "Nintendo Entertainment System", "NES" and "Famicom"
are used here only to say what the core emulates.
