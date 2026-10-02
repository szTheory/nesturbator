---
title: "libretro adapter, RetroArch on macOS, and the headless runner contract"
summary: "What libretro requires of a core, how a local core reaches RetroArch 1.22.2 on an Apple Silicon Mac, and what Playstead needs from an emulator process."
read_when: "Building the libretro adapter, packaging a release, or designing the runner's command line."
updated: 2026-10-02
---

# libretro adapter, RetroArch on macOS, and the headless runner contract

## Why this file exists
The adapter, the release package and the runner's command line rest on facts held elsewhere: libretro.h, RetroArch's macOS build, Apple's rules for downloaded code, and Playstead's process handling. "Lnnn" is a libretro.h line at the pin in [LR.01]; "(tested)" means run on the project's Mac, macOS 26.6.2 [LR.15]; "(untested)" means not run.

## 1. Minimum libretro surface for "playable in RetroArch"
A core is a dynamic library exporting all 25 functions in libretro.h (L7511-7840) [LR.06]; the header is MIT (L10-27), and the copy at this pin is the one to vendor. It may stay loaded between sessions, so `retro_deinit` resets globals (L7594). `retro_api_version` returns 1.

| Function (line) | Contract |
|---|---|
| `retro_set_environment` (7511), five other `retro_set_*` | called before `retro_init`; the others before `retro_run` |
| `retro_get_system_info` (7627) | static strings; extensions "nes"; `need_fullpath` false lets the frontend load, unzip and patch (L6008) |
| `retro_load_game` (7761), `retro_get_system_av_info` (7642) | ROM bytes in `retro_game_info` (L6833); geometry and timing after a successful load |
| `retro_run` (7694) | one frame: input poll at least once (L7685), video callback once [LR.06], audio |
| `retro_reset`, `retro_get_region` (7812) | soft reset; 0 NTSC, 1 PAL |
| serialize calls (7707-7730) | size 0 means no states [LR.06] |
| `retro_get_memory_data` and `_size` (7826) | NULL and 0 allowed (L498) |
| controller-port, cheat and `retro_load_game_special` calls | may do nothing |

- Pixels: `SET_PIXEL_FORMAT` (10) in `retro_load_game` (L861) picks XRGB8888 (1) or RGB565 (2); the default 0RGB1555 is deprecated (L5620-5647).
- Geometry and timing: unsigned sizes, float `aspect_ratio`, double `fps` and `sample_rate` (L6247-6305).
- Audio: `retro_audio_sample_batch_t` takes interleaved left/right int16 frames (L7465); a core uses it or the per-sample callback, not both (L7453).
- Input: `retro_input_state_t` returns 0 or 1 per RetroPad id (L7499): B 0, Y 1, Select 2, Start 3, Up 4, Down 5, Left 6, Right 7, A 8 (L320-344).
- Battery saves: RetroArch stores `RETRO_MEMORY_SAVE_RAM` (id 0, L509) as `.srm` in `savefile_directory` [LR.03]; the adapter writes no file.
- No content: a core that sets `SET_SUPPORT_NO_GAME` (18) in `retro_set_environment` is given `retro_load_game(NULL)` (L1040-1051). RetroArch starts it without a content argument, or through "Start Core", when the `.info` file has `supports_no_game = "true"` [LR.07]. The built-in test frame relies on this.

## 2. Beyond the minimum
**Serialization.** Between load and unload the size never grows (L7699) [LR.06]. `SET_SERIALIZATION_QUIRKS` (44) bits: 0 incomplete, 1 needs warm-up, 2 size varies, 4 single session, 5 endian-dependent, 6 platform-dependent (L3448-3477); a fixed-size state without pointers (L5666) in a fixed byte order sets none. Netplay compares core version and per-frame state hashes [LR.08], so `library_version` and state bytes equal across platforms matter.

**Memory and RetroAchievements.** `SET_MEMORY_MAPS` (36) takes descriptors whose `ptr` stays valid and unmoved all session (L3591); bank switching cannot be expressed (L3701). rcheevos 12.1.0 in RetroArch models the NES as its CPU address space, $0000-$07FF system RAM and $6000-$7FFF save RAM. It resolves these through descriptors when given; otherwise it reads `RETRO_MEMORY_SYSTEM_RAM` (id 2) from $0000 and `RETRO_MEMORY_SAVE_RAM` for $6000-$7FFF [LR.05]. Recommendation: descriptors, which expose volatile work RAM at $6000 without declaring it save RAM. Alternative: the two regions only. Word from RetroAchievements that NES sets read nothing else would change this.

**Core options.** `SET_CORE_OPTIONS_V2` (67) when `GET_CORE_OPTIONS_VERSION` (52) is 2 or more (L2289). Few is better: the header asks for "the number of options and values as low as possible" (L2319); each option that alters emulation enters state, replay and netplay compatibility; rcheevos bars option values in hardcore mode, including forced PAL or Dendy on two NES cores [LR.05].

**`.info` file.** `nesturbator_libretro.info` holds `key = "value"` lines; the DMG build reads `~/Library/Application Support/RetroArch/info` [LR.02].

| Key | Value, after the NES entries in [LR.09] |
|---|---|
| `display_name` | "Nintendo - NES / Famicom (nesturbator)" |
| `systemname`, `systemid`, `database` | "Nintendo Entertainment System", "nes", "Nintendo - Nintendo Entertainment System" |
| true/false flags | `supports_no_game`, `savestate`, `libretro_saves`, `memory_descriptors`, `core_options`, `disk_control`, `is_experimental` |
| `savestate_features` | "basic", "serialized" (adds rewind), "deterministic" (adds run-ahead, netplay) |

RetroArch has disabled features above the declared level since 1.10.1 [LR.03]. Key or file missing: (untested).

**FDS (later).** `SET_DISK_CONTROL_EXT_INTERFACE` (58) is registered in `retro_init` (L2023); existing NES entries set `disk_control = "false"` and list `disksys.rom` as optional firmware [LR.09].

**Buildbot (later).** An info file goes to libretro-super `dist/info`, a `.gitlab-ci.yml` to the repository root [LR.06]. The macOS CMake template builds target `${CORENAME}_libretro` into `${CORENAME}_libretro.dylib` [LR.10]; a CMake target `nesturbator_libretro` without the `lib` prefix fits.

## 3. RetroArch on an Apple Silicon Mac
1.22.2 (2025-11-20) is the current release [LR.01][LR.12].

| Build | Core directory | Local core |
|---|---|---|
| DMG `RetroArch_Metal.dmg`, universal; also Homebrew `retroarch-metal` [LR.07][LR.12] | `~/Library/Application Support/RetroArch/cores` [LR.02] | copy in, or `-L` |
| Homebrew `retroarch`: Intel-only DMG [LR.08][LR.12] | same | needs x86_64 code: Rosetta cannot mix architectures in a process [LR.13] |
| Mac App Store (sandboxed); Steam [LR.02][LR.08] | in the app, `Contents/Frameworks`; `cores` next to the app [LR.02] | none known; `-L` (both untested) |

Recommendation: the DMG. It keeps `info` beside `cores`, and saves and states under `~/Documents/RetroArch` [LR.02].

**Loading.** Menu: Load Core lists the core directory [LR.07]. Command line: `/Applications/RetroArch.app/Contents/MacOS/RetroArch -L CORE ROM`; CORE is a path or the short name `nesturbator` [LR.03][LR.07]. With no `.info`, Load Core worked but Load Content said "no core available" (Windows, 1.15.0); deleting `info/core_info.cache` forces a re-read [LR.16].

**Signing and quarantine.** The direct, buildbot and App Store entitlement files carry `com.apple.security.cs.disable-library-validation` [LR.02]; without it the hardened runtime loads only code signed by Apple or the same Team ID [LR.13]. Apple's linker ad-hoc-signs arm64 output [LR.14]: `clang -dynamiclib` gave `flags=0x20002(adhoc,linker-signed)` (tested). Since macOS 10.15 an app loads a quarantined plug-in (from the internet or AirDrop) only if it is notarized or the user approves it in System Settings; notarization needs a Developer ID, and a ticket cannot be stapled to a bare binary [LR.13][LR.14].

Tested: files fetched with `curl` or `gh release download` carry no `com.apple.quarantine`; `unzip`, `ditto` and `tar` copy it from a marked zip to each extracted file; `xattr -d` removes it. A browser download and RetroArch loading a marked core: (untested). Install recipe for a zip holding `cores/`, `info/` and `LICENSE` (tested with stand-in zips; naming the two folders keeps `LICENSE` out of RetroArch's directory): `curl -fsSL URL | tar -xf - -C ~/Library/Application\ Support/RetroArch cores info`.

| Automation flag [LR.03][LR.08] | Effect |
|---|---|
| `--max-frames=N`, `--max-frames-ss`, `--max-frames-ss-path=FILE` | exit after N frames; screenshot there |
| `-R FILE`, `-P FILE`, `--eof-exit` | record or play a replay; exit at its end |
| `-c FILE`, `--appendconfig=FILE` | config: `savefile_directory`, `libretro_info_path`, `autosave_interval` |
| `--command CMD` | UDP: `QUIT`, `SAVE_FILES`, `READ_CORE_MEMORY` |

## 4. What the adapter needs from the native API
| Native API provides | Because |
|---|---|
| Frame and sample rates as fractions | doubles (L6302); the adapter divides once (NTSC 39375000/655171) |
| Palette lookup, native pixel to XRGB8888 | RGB video callback (L7436); hashes use native pixels |
| Mono int16 samples per frame | duplicated to left/right, one batch call per `retro_run` (L7465) |
| RAM pointers and sizes fixed from load to unload | descriptor `ptr` cannot move (L3591) |
| Fixed state size; caller's buffer; fixed byte order | L7699 |
| Load copies the ROM | data is "valid only until retro_load_game() returns" (L6129). Recommendation: copy. Alternative: `persistent_data` via `SET_CONTENT_INFO_OVERRIDE` (65). Memory pressure on a small target would change this |

## 5. The headless runner's process contract
Playstead [LR.17][LR.18] starts one pinned emulator from an argument template with `{romPath}` and `{saveDir}`, watches `{saveDir}/{romBaseName}.sav`, reads status 0 as clean, and on quit sends SIGTERM, then SIGKILL after 2 s. Its mGBA pin flushes every 24 s, never on SIGTERM. Its installer takes a disk image holding one `.app`, checks the archive SHA-256 and never removes quarantine. Its continuation check allows no network and writes in one directory only, uses exit 0 pass, 77 capability refusal, 1 failure, and is blocked for lack of deterministic input replay and a repeatable visible-state check. Proposal:

| Item | Proposal |
|---|---|
| Invocation | `nesturbator-run [options] [ROM]`; with no ROM the core runs without a cartridge and outputs its test frame; no config file, environment, stdin or network |
| Saves | `--save-dir DIR` writes `DIR/<rom basename>.sav`, the bytes the adapter exposes as save RAM; nothing beside the ROM |
| Flush | temp file and rename, within `--save-interval` frames of a change, on SIGUSR1, on exit |
| Signals | SIGTERM, SIGINT: flush, exit 0 inside 2 s; crash signals unhandled |
| Exit status | 0 done; 1 failure; 2 usage; 3 hash mismatch; 77 unsupported capability |
| Replay and checks | `--movie FILE`, `--frames N`; `--hash-frame N` and `--hash-audio` print SHA-256; `--dump-frame N:FILE`; `--expect FILE` compares the printed hashes with a recorded file and exits 3 on a difference |
| Preflight | `--capabilities`: JSON with version, mappers, save media, state fingerprint (state format plus ROM hash); writes nothing |

Choices. SIGTERM exits 0, since Playstead knows it sent the signal; alternative, a dedicated status; Playstead needing to tell them apart by status would change this. Movies use FM2's text frame lines [LR.04]; alternative, a private binary format; foreign movies failing to sync would change this. The runner has no window: it serves Playstead's replay check, not play. RetroArch offers `autosave_interval` and a `SAVE_FILES` command [LR.03][LR.08].

## 6. Release artifacts
Core file names [LR.11]: `nesturbator_libretro.dylib` (macOS, one per architecture), `.so` (Linux), `.dll` (Windows), `nesturbator_libretro_android.so`, `nesturbator_libretro_ios.dylib`. Suggested set: `nesturbator-VERSION-libretro-OS-ARCH.zip` holding `cores/`, `info/` and `LICENSE`; runner and library archives; `SHA256SUMS` in `shasum -a 256` format (`-c` check tested). Playstead pins URL and archive SHA-256 [LR.17], so replacing an asset breaks the pin. Only Developer ID signing with notarization passes the quarantine check unaided [LR.13].

## 7. The future shared host
- One C ABI for all cores: same entry points, a version call, a per-system descriptor (geometry, timing, input ports, save media, memory regions, state size).
- The libretro adapter is then one source file compiled against each core.
- Loading. Recommendation: cores linked into the host or signed with it, so no library-validation exception [LR.13]. Alternative: `dlopen` with the entitlement. Cores shipping separately would change this.

## Open questions
- Does RetroArch refuse a quarantined ad-hoc core? Browser-download the zip, extract, run with `-v`, read the log.
- Do shipped entitlements match [LR.02]? `codesign -d --entitlements - RetroArch.app`.
- Missing `.info` or `savestate_features`, and RetroArch on SIGTERM (flush, exit status): one session with the DMG build.
- RetroAchievements and banked $6000-$7FFF RAM: its developers.
- Owner: the process for NES play in Playstead before the shared host; Developer ID and notarization; installer archives other than a disk image.

## Sources
Accessed 2026-10-02. RA = https://github.com/libretro/RetroArch/tree/69a4f0ea1e8aaf442ae4858f2e7f2b31a1776576 (tag v1.22.2). DOCS = https://github.com/libretro/docs/tree/36e9222824863d7e84754a19b065837e6d77bc44/docs. AD = https://developer.apple.com/documentation.

| ID | Tier | Source | Pin or access date | Supports |
|---|---|---|---|---|
| LR.01 | T1 | RA `libretro-common/include/libretro.h`; https://github.com/libretro/RetroArch/releases | v1.22.2 | Lnnn, release |
| LR.02 | T1 | RA `pkg/apple/*.entitlements`, `*.xcconfig`; `frontend/drivers/platform_darwin.m` L359-436 | v1.22.2 | macOS build |
| LR.03 | T1 | RA `retroarch.c` help text L6486-6728; `retroarch.cfg`; `CHANGES.md` | v1.22.2 | flags, config, history |
| LR.04 | T3 | https://fceux.com/web/FM2.html | 2026-10-02 | FM2 |
| LR.05 | T1 | RA `deps/rcheevos/src/`: `rcheevos/consoleinfo.c` L706-723, `rc_libretro.c` L80-107, L490-732 (MIT) | v1.22.2 | achievements |
| LR.06 | T1 | DOCS `development/cores/developing-cores.md` | 36e9222 | core contract |
| LR.07 | T1 | DOCS `guides/install-macos.md`, `cli-intro.md`, `starting-a-game.md` | 36e9222 | install, launch |
| LR.08 | T1 | DOCS `development/retroarch/network-control-interface.md`, `netplay.md`, `compilation/osx.md` | 36e9222 | commands, netplay, Steam |
| LR.09 | T1 | https://github.com/libretro/libretro-super/tree/a7054054afbd97173d48e5d7619cff412c237d06/dist/info: example and three NES entries | a705405 | info keys |
| LR.10 | T1 | https://git.libretro.com/libretro-infrastructure/ci-templates/-/tree/96f603ee450eff3e9ad2baeac75b300aa83c1c9e `osx-cmake-*.yml` | 96f603e | CI |
| LR.11 | T1 | https://buildbot.libretro.com/nightly/ listings | 2026-10-02 | file names |
| LR.12 | T2 | https://formulae.brew.sh/api/cask/retroarch-metal.json, `retroarch.json` | 2026-10-02 | casks |
| LR.13 | T1 | AD `security/notarizing-macos-software-before-distribution`, `security/customizing-the-notarization-workflow`, `bundleresources/entitlements/com.apple.security.cs.disable-library-validation`, `apple-silicon/about-the-rosetta-translation-environment` | 2026-10-02 | plug-ins |
| LR.14 | T1 | https://support.apple.com/102445; local `man ld` (ld-1267) | 2026-10-02 | approval, linker |
| LR.15 | T3 | Local tests: Apple clang 21.0.0, curl 8.7.1, gh 2.101.0; quarantine attribute set by hand | 2026-10-02 | "(tested)" |
| LR.16 | T4 | https://github.com/libretro/RetroArch/issues/15553 | 2026-10-02 | no info file |
| LR.17 | T1 | Playstead `playstead-mac/Playstead/Adapter/`: `AdapterPin.json`, `AdapterHost.swift`, `AdapterInstaller.swift` | 1de3e95 | launch, install |
| LR.18 | T1 | Playstead `playstead-mac/docs/CONTINUATION-PROTOCOL.md` | 1de3e95 | continuation check |
