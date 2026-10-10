# Technology Stack - Milestone v2 (mappers, battery saves, CI tune-up, boot-to-play)

**Project:** nesturbator
**Researched:** 2026-10-09
**Scope:** only what is NEW for v2. The v1 stack (C17, CMake 3.25+ presets, Ninja, CTest with `check.h`, GitHub Actions on six runners, release-please v5, libFuzzer nightly, vendored `libretro.h`) is validated and unchanged; v1 text is in git history.
**Overall confidence:** HIGH for licences, hashes and API facts (fetched and measured today); MEDIUM for CI tuning payoff (compile/test split not yet measured); LOW where marked.

## Headline

**No new dependency of any kind.** No library, no vendored file, no new GitHub Action, no new toolchain. The additions are (a) committed test ROMs with explicit licences, (b) two nightly-fetched blargg suites that cannot be committed, (c) three small API/behaviour contracts (save RAM in the C API, libretro and the runner), and (d) CI configuration changes inside files that already exist.

## Core Stack Changes

| Technology | Version | Purpose | Why |
|------------|---------|---------|-----|
| C / CMake / CTest / `check.h` | unchanged | Mapper code, tests | Mappers are page-table setups plus register handlers; nothing here needs a new tool. |
| In-repo synthetic ROM builder (test code, no files) | new, ~100 lines in `tests/` | UxROM/CNROM/AxROM bus-conflict and banking tests; MMC1 serial-port and MMC3 IRQ unit tests | No open-licence UxROM or AxROM game was found (see below), and no public ROM tests bus conflicts directly. A test builds an iNES image in memory (header + hand-assembled 6502 bytes). This is owned code, adds no binary to the tree, and stays clear of `hygiene.sh`. Follows the ladder: reuse (the fuzz corpus already builds `valid-nrom` images) before adding a cc65/ca65 toolchain. |
| `libretro.h` | unchanged pin (RetroArch v1.22.2 is still the latest release, published 2025-11-20) | `RETRO_MEMORY_SAVE_RAM` | Already vendored; no bump needed. |

## Test ROMs

### Committable (explicit licence, verified from the repository `LICENSE`/API today)

Add each to `tests/roms/manifest.txt` with path, source, pin, SPDX licence and SHA-256. A fetch-time hash is the only identity: the upstream `.nes` files are release assets, not tracked files.

| ROM | Source and pin | Licence | Mapper / role | SHA-256 |
|-----|----------------|---------|---------------|---------|
| Holy Mapperel v0.02 (subset, see below) | `pinobatch/holy-mapperel`, tag v0.02, commit `c022622274ca8b83d214dea97e4388a6b0e92d8a`; asset `holy-mapperel-bin-0.02.7z`, archive SHA-256 `70f85671e21f293599baebb662faeb06a4c04e9c9ceb283d96d4197f09e4ce7a` | Zlib (GitHub API `spdx_id` = Zlib) | Banking, CHR RAM/ROM, WRAM, battery, gross IRQ for m1, m2, m3/66, m4, m7 | per file, below |
| nes-runner v1 (`runner.nes`) | `zorchenhimer/nes-runner`, tag v1, commit `47537be0ccb8baeb8462547a1c08888ee2a6cfac` | MIT (`LICENSE.md`) | MMC1, NES 2.0, 64 KiB PRG, 16 KiB CHR ROM, 8 KiB PRG-NVRAM (header byte 10 = `$70`). The boot-to-play game for the MMC1 phase. | `1a67bfabdde2d96eacb04eb7f9d17e8da6c082d4976c318315fe31e2d19b4384` |
| Stallar 0.1.0 (`stallar.nes`) | `wendelscardua/stallar`, tag 0.1.0, commit `0188c66b8aac4c76b3a86b3ca58e3ad3fe52ab52` | MIT (repo); README states the music and SFX are public domain | MMC3, iNES 1, battery flag, 64 KiB PRG, 8 KiB CHR; the source has an IRQ buffer, so it uses the scanline counter. MMC3 "real game" check. | `b5584e142db178fc356b70d635cb290f250a85569461b5915969617740c551f0` |

The README's earlier remark that nes-runner is already a candidate stands; it is not yet in the manifest.

**Holy Mapperel subset to commit** (15 files, 2.44 MB with the 512 KiB one; 1.92 MB without). The full 40 ROMs unpack to over 8 MB, which is not worth committing. SHA-256s verified today for the ones marked (h); compute the rest at commit time.

| File | Size | Covers | SHA-256 |
|------|------|--------|---------|
| `M1_P128K_C32K_S8K.nes` | 163,856 | MMC1 with 8 KiB battery RAM (header byte 10 = `$70`); the battery-save test: the ROM writes `SAVEDATA` at `$6100`, so a run, `.sav` round trip, second run reports `has_savedata` | `c6101b411ee99f6b0c8e6ec0abe32b9fcfa5c32cef8017dd728bb3c992d16efd` (h) |
| `M1_P128K_C32K_W8K.nes` | 163,856 | MMC1 with volatile 8 KiB WRAM (byte 10 = `$07`); must not appear as save RAM | compute |
| `M1_P128K_CR8K.nes`, `M1_P128K_C128K.nes`, `M1_P128K.nes` | 131,088 / 262,160 / 131,088 | MMC1 CHR RAM and CHR ROM sizes | compute |
| `M2_P128K_V.nes`, `M2_P128K_CR8K_V.nes` | 131,088 each | UxROM (7432 variant, vertical mirroring) | `M2_P128K_CR8K_V` = `c7e83755bd9adbb7c705ea9f29535442af7390444632f9f824fcf4c00632069b` (h); `M2_P128K_V` compute |
| `M3_P32K_C32K_H.nes` | 65,552 | CNROM | `499891c6d8c7a1e7631bdc601d9d624938842735f92fe1fda7b19ef9fae514b7` (h) |
| `M66_P64K_C16K_V.nes` | 81,936 | Detected as mapper 66; Holy Mapperel's README says NROM and CNROM report as 66 when sizes match. Keep as the GxROM case, not a CNROM proof. | `e62c4e897063fbf71e5bd3901dfb39fe04082150d0fb9291cc6756e08a688496` (h) |
| `M4_P128K.nes`, `M4_P128K_CR8K.nes`, `M4_P128K_CR32K.nes` | 131,088 each | MMC3: banking, CHR RAM, gross IRQ, WRAM protect | `M4_P128K` = `b145558f6d301fd294e3e7641a7a1c414d9f743b1f89aace324c61b72c082e36` (h); others compute |
| `M7_P128K.nes`, `M7_P128K_CR8K.nes` | 131,088 each | AxROM (single-screen select) | `M7_P128K_CR8K` = `4aa0050f36ae17e17701506821e5147df5b843bcdf2827468fa9c66e4b7ac1ba` (h); other compute |
| `M4_P256K_C256K.nes` | 524,304 | MMC3 at maximum PRG/CHR; optional | compute |
| Leave out | | m0 (already covered), m9, m10, m11, m28, m34, m69, m78.3, m118, m180, and the 512 KiB SxROM ROMs (`M1_P512K_*`) for this milestone | |

Notes that change implementation:

- **Result protocol.** No memory channel; the result is a 4-digit code on screen (WRAM, PRG, IRQ, CHR; `0000` is normal) plus beeps. The per-frame text must be read from the nametable (tile = ASCII per the existing blargg screen-text approach) or by frame hash. Result addresses are not documented (CONFORMANCE gap). Recommended: frame hash of the final screen per ROM, recorded in the scoreboard after the first correct run, plus a decoded-text assertion once the nametable layout is read from the running ROM. Frames needed to reach the result: not measured (CHR-RAM tests run 8 passes while buzzing); measure before setting `timeout` values.
- **Expected non-zero codes.** README documents `1xxx` / `4xxx` for MMC1 WRAM-disable rules and `2xxx` for MMC3 read-only mode; the MMC3 test warns about missing WRAM write-protect on iNES 1 environments. Decide per ROM whether `0000` is required or an accepted code is recorded.
- **Header dependence.** The ROMs are NES 2.0 and carry PRG-RAM, PRG-NVRAM and CHR-RAM sizes in header bytes 10 and 11. The NES 2.0 parser must therefore honour those (v1 loader accepts NES 2.0; confirm bytes 10/11 are read, not only `flags 6`).
- **Extraction.** The archive is `.7z`. macOS/Linux `bsdtar -xf` (libarchive) extracted it with no extra install (tested); `p7zip-full` is on `ubuntu-24.04`; 7-Zip 26.03 is on the Windows image. Extraction is a one-time maintainer step, not a CI step, because the files are committed. Record the archive hash in the manifest `pin` field.
- **Save fixtures.** The archive also holds `2k.sav`, `8k.sav`, `32k.sav` (all 0xFF-filled, tested). Do not commit them: `hygiene.sh` rejects save files and the tests can generate them.

### Not committable, fetch nightly at a pin (no licence)

| Suite | Pin | Why | Use |
|-------|-----|-----|-----|
| `mmc3_test_2/rom_singles/*.nes` (blargg) | `christopherpow/nes-test-roms` @ `95d8f621ae55cee0d09b91519a8989ae0e64753b` (the pin CF.06 already uses) | The repository and the tests state no licence (CONFORMANCE: 589 files checked); cannot be redistributed. Same rule as the existing 65x02-style fetch. | `$6000` result protocol; RAM at `$6000` required. SHA-256 (fetched today): `1-clocking` `b06d8a97f0ca672be92c841d6af7d1e650696e86e9cc0cf6eeb90d67a6ab499b`; `2-details` `e7af16c764b119e60effb7b1cfeec3dd8e2e657041283693cdbbeedb4081f1e3`; `3-A12_clocking` `b375f15b9f9d372c8084b9c50928be9e41a3ac48be831ce82d203c18891433ad`; `4-scanline_timing` `14a220b9d1272acc7a820ab38e9762a7cdf2d54c65e753be87f23dfcaf1bb845`; `5-MMC3` `e0824123d60b83868dac1189b28250f8e10376a01be468a5a74aa59937cb32ca`; `6-MMC3_alt` `56698b6918453d161a8d4e51f66e363d6966b054939c8176c53c401a6b55269b`. All 40,976 bytes, mapper 4, header battery 0. |
| `mmc3_irq_tests/*.nes` (blargg) | same pin | Same | Screen text, result in `$F8`. Files 1-6 are 16,400 bytes each. |

Rules for these two: pick **MMC3 revision B behaviour** (the SMB3 / Mega Man 3 chip) as the single emulated behaviour; `5-MMC3` must pass and `6-MMC3_alt` (revision A) is recorded `unsupported` in the scoreboard. This follows the readme (the last two ROMs target different chip revisions) and avoids a core option, which would enter state, replay and netplay compatibility (LIBRETRO-AND-RUNNER section 2). Whether NES 2.0 submapper numbers should select the revision: not researched; LOW, leave to the MMC3 phase.

### Games that exist but are excluded

| Game | Reason |
|------|--------|
| `wendelscardua/signos` (MMC3, Unlicense repo) | README: assets (graphics and music) are copyrighted, redistribution needs permission. |
| `captain-http/flappy-paratroopa-nes` (MMC1, MIT) | Appears to reuse a Nintendo franchise's characters; the repository licence cannot grant that. |
| `mhughson/mbh-cart` Minekart Madness (m28 and MMC1 builds, MIT) | Franchise-derived title; provenance not verified. |
| `AnthonyBongers/GhostsAndGraves` (mapper 3, battery flag, Unlicense) | CNROM with a battery flag is not a real board (header: 32 KiB PRG, 40 KiB CHR, battery set). Useful only as a robustness case; asset licence not stated. LOW. |
| Games with `GPL`, `CC BY-NC`, or "do not distribute" terms | Already listed in CONFORMANCE "Excluded". |
| `CleverCatGames/nes-platformer-template` (MMC1, CHR RAM, 128 KiB, battery, BSD-3-Clause) | Licensable, but it is a template; the art/music provenance is not stated. Keep as a fallback second MMC1 game, not a fixture. MEDIUM. |
| `wendelscardua/file-fixers` (MMC3, iNES 1, battery, MIT) | Usable as an extra MMC3 game; asset terms not read. Fallback only. |

**UxROM and AxROM have no open-licence commercial-quality game fixture.** A search of GitHub (licence filter MIT, Zlib, Unlicense, Apache, BSD, CC0, ISC, WTFPL) for homebrew `.nes` release assets found games on mappers 0, 1, 3, 4 and 28 only. Cover UxROM, CNROM and AxROM with Holy Mapperel plus the in-repo synthetic builder, and state it in the roadmap rather than hunting further.

## Libretro Integration (no new API beyond what v1 already vendors)

| Item | Decision | Why |
|------|----------|-----|
| `retro_get_memory_data(RETRO_MEMORY_SAVE_RAM)` / `retro_get_memory_size` | Return the cartridge's battery-backed PRG-NVRAM buffer and its exact byte length; `NULL`/0 when the cartridge has no battery. | `libretro.h` L498-509: save RAM is the battery-backed cartridge RAM; NULL/0 means "does not apply". RetroArch stores it as `<content>.srm` in `savefile_directory` (LR.03). The adapter writes no file (current stub at `libretro/libretro.c` returns NULL/0). |
| Pointer and size stability | Fixed at `retro_load_game`, unchanged until `retro_unload_game`. | The host reads/writes the buffer directly; RetroArch loads the `.srm` into it after load. A fixed size also avoids unverified size-mismatch behaviour (see Gaps). |
| Volatile PRG-RAM (e.g. `M1_P128K_C32K_W8K`) | Never exposed as `SAVE_RAM`. | Would make RetroArch write `.srm` for games with no battery. Expose through `SET_MEMORY_MAPS` for RetroAchievements, per the existing decision (LIBRETRO-AND-RUNNER section 2). |
| Reset and power cycle | `retro_reset` must not clear NVRAM; `retro_deinit`/unload leaves the host buffer valid until unload completes. | The host loads save data once, after content load. |
| Header sizing | Battery size from NES 2.0 byte 10 high nibble (`64 << n` bytes); for iNES 1 with battery and byte 8 = 0, use 8 KiB. Non-battery iNES 1 mappers that tests need at `$6000` (blargg MMC3 suites) also need 8 KiB RAM when byte 8 = 0. | `runner.nes` and the Holy Mapperel S8K ROM declare `$70` = 8 KiB; `stallar.nes` and the platformer template are iNES 1 with battery and zero in byte 8. |
| Autosave | None in the core. RetroArch decides (`autosave_interval`, `SAVE_FILES` UDP command; defaults vary by version and platform, so do not rely on them). | Core stays free of file I/O (rule 2). |
| E2E proof | Extend the existing `retroarch-e2e` job (macos-15, 111 s today, pinned RetroArch 1.22.2, SHA-256 `81b79121...b434`) to run the S8K ROM, quit, assert a non-empty `.srm` of exactly 8192 bytes, then relaunch and compare the screen. | "No check waits on a person"; RetroArch 1.22.2 remains the latest stable. |

## Runner (`nesturbator-run`) save handling

Build on the existing proposal (LIBRETRO-AND-RUNNER section 5) rather than re-deciding it; the research changes only the details below.

| Item | Decision | Why |
|------|----------|-----|
| Option and file | `--save-dir DIR`; file `DIR/<rom basename>.sav`, raw bytes, exactly the length the core reports. | Matches the proposal and the owner's "raw .sav bytes". A RetroArch `.srm` is the same raw bytes, so a file can be copied between host and runner (LOW until tested together). |
| Load | If the file exists and its length equals the core's size, copy it in before frame 1. Any other length: warn on stderr, start from the power-on fill, and do not overwrite on exit unless the game changed it. | A silent truncation/pad would corrupt a save. Decision for the roadmap: mismatch is an error with a distinct exit status, or a warning; pick one. Recommended: error, status 4, no write. |
| Write | Temp file in the same directory, `rename`, on exit, on SIGTERM, on SIGUSR1, and every `--save-interval` frames when dirty. Dirty detection: a flag set by any CPU write to the NVRAM range. | Atomic replace matches the proposal; a 8 KiB `memcmp` against a snapshot is the simpler alternative and costs one compare per interval. |
| Hashes | Frame/audio hashes must not depend on whether a save file exists unless the test passes one. | Determinism rule 5. |
| Fresh NVRAM fill | Choose one value (suggest `0x00`) and document it in the header. Real boards are unspecified; Holy Mapperel looks for the `SAVEDATA` string, not for a fill. | Same hash on every platform. |
| Tests | New CTest cases: write on exit, reload equals, wrong-length refused, kill-and-recover leaves either old or new file but never a partial one. | Rule 6. |

## CI and Build Tooling Changes

Measured on the latest successful `main` run (job wall seconds): macos-15-intel build 181, asan 171, windows-11-arm 144, macos-15 arm64 113, retroarch-e2e 111, windows-2025 92, ubuntu x64 78, ubuntu arm64 77, nofp 16-21, hygiene 19. Local `ctest -j1` over 357 tests takes 49 s on this Mac. The pipeline is already about three minutes end to end; the CI tune-up is about trimming and hardening, not rescuing.

| Change | Decision | Why |
|--------|----------|-----|
| Compiler cache (ccache) | **Do not add in v2 unless a measurement says so.** ccache is on none of the runner images (checked Ubuntu 24.04, macOS 15, Windows 2025 image readmes); `hendrikmuhs/ccache-action` v1.2.24 (2026-08-31) or `actions/cache` v6.1.0 (2026-06-26) would be a new third-party `uses:` for an unknown win. First step of the tune-up phase: print the compile share of the slowest legs (`ninja -d stats` or ctest `Test time` vs build time). Adopt only if compile exceeds about 40% of macos-15-intel or windows-11-arm; if so use `actions/cache` keyed by preset, runner label and compiler version (existing ENGINEERING rule), and SHA-pin it. | Owned-code, minimal-dependency stance; every new action must also pass `sha_pinning_required` and the `hygiene` `uses:` scan. |
| Parallel tests | Add `"execution": { "jobs": 4 }` (CMake 3.25 preset field) to the `ci`, `ci-msvc`, `dev` and `asan` test presets. Check first that no test shares a fixed temp path (runner `.sav` tests will need per-test directories). | Largest cheap lever as ROM tests grow (Holy Mapperel adds about 14 ROM runs). The 357 tests currently run serially. |
| Flake hunt | Nightly job (existing `nightly.yml`): `ctest --repeat until-fail:3 --schedule-random` on the `ci` preset. | Finds order-dependent and shared-path tests that PR runs hide, supporting "a test that failed without a code change is fixed or removed" (SEED-006). |
| Job splitting | Do not split `build`. Consider moving `asan` to start in parallel (already is) and giving `retroarch-e2e` its own build so it does not run the full suite a second time: its "Build and run the local test suite" step is 89 of its 111 seconds and duplicates `build (macos-15)`. Upload the built core/runner as an artifact from `build` and let the e2e job download it. | Saves about 80 s of runner time; the e2e job's critical path is not the wall-clock bottleneck, so this is optional. MEDIUM. |
| Timeouts | Recompute as twice the new cold time, per the existing rule. `retroarch-e2e` is 30 min for a 111 s job. | Keeps stuck jobs cheap. |
| Intel macOS leg | Keep for v2; schedule its removal or demotion to `main`-only. `macos-15-intel` is the last Intel image; GitHub says Intel macOS is unsupported after the macOS 15 image is retired in fall 2027. It is also the slowest leg. | Hash-equality across x64 and arm64 is still covered by Linux and Windows x64/arm64. |
| Action pins | Bump `actions/upload-artifact` v7.0.1 to **v7.0.2** and `actions/download-artifact` v8.0.1 to **v8.0.2** (both released 2026-10-07); `actions/checkout` v7.0.1 is current; `actions/attest` is v4.2.2; `googleapis/release-please-action` v5.0.0 is current. Dependabot (`github-actions`, weekly) will propose the first two; merge them in the tune-up phase. SHAs must be resolved from the tag, not copied from this file. | Pins are one patch behind today. |
| Runner labels | Keep `ubuntu-24.04`, `ubuntu-24.04-arm`, `macos-15`, `macos-15-intel`, `windows-2025`, `windows-11-arm`. `ubuntu-26.04` and `macos-26` exist; do not move in v2 (no feature needs it). `macos-14` is retired on 2026-11-02 and is not used. The `windows-11-arm` label moved to the Visual Studio 2026 image (runner-images issue 14602); the MSVC step uses `vswhere -latest`, so it should hold, and the log of the next run will confirm. | Image churn is a cost without a benefit. |
| AccuracyCoin pin | Re-check upstream HEAD during the tune-up; a pin move is its own change. | SEED-006. |
| New ctest labels | Add `mappers` (Holy Mapperel + unit tests) and `games` (nes-runner, stallar) so the tune-up phase can time them separately and `asan` can exclude slow ones if needed. | Same mechanism as the existing `retroarch`, `abi`, `hygiene` labels. |
| Scoreboard | Add `holymapperel/<file>` and `mmc3/<name>` keys to `tests/accuracy/scoreboard.txt`; blargg MMC3 keys are `unavailable` in PR runs and filled by the nightly. | Existing "may only improve" rule. |
| Boot-to-play check | A movie (existing `--movie` / `--expect` machinery) for `runner.nes`: Start press on the title screen, jumps over several obstacles, frame hash at a fixed frame, plus a save written and reloaded across two runs. | Automated, deterministic, no person in the loop. Whether the movie reaches visible "play" has not been run; LOW until done. |

## Alternatives Considered

| Category | Recommended | Alternative | Why Not |
|----------|-------------|-------------|---------|
| UxROM/AxROM fixtures | Holy Mapperel + in-repo synthetic images | cc65/ca65 toolchain to build homebrew | A toolchain is a new dependency, and building from source reintroduces a "ROM bytes from elsewhere" audit for each build. |
| MMC3 IRQ proof | Nightly fetched blargg suites | Committing blargg ROMs | No licence stated anywhere; cannot redistribute. |
| MMC3 IRQ proof | Nightly fetched blargg suites | `Fixatron/MMC3_Rom`, `Atomfusion1/NES_MMC3_CA65_*` | Banking demos, no stated licence or no IRQ result protocol. |
| Compiler cache | None, measure first | ccache + `ccache-action` | New action, no image support, unknown payoff on a 3-minute pipeline. |
| Battery path | `retro_get_memory_*` | Writing `.srm` from the adapter | Violates rule 2 (no file I/O in the adapter) and fights RetroArch's own save-directory logic. |
| MMC3 revision | One fixed behaviour (rev B), rev A recorded unsupported | Core option | Options alter state, replay and netplay; the header asks for few. |
| Test ROM storage | Commit 15 Holy Mapperel files (about 2 MB) | Fetch at pin each run | A fetch makes PR CI depend on the network; Zlib permits committing. |

## Installation

Nothing to install. Maintainer one-time steps:

```bash
# Extract the Holy Mapperel subset (bsdtar reads 7z on macOS and Linux)
bsdtar -xf holy-mapperel-bin-0.02.7z testroms/M1_P128K_C32K_S8K.nes ...
shasum -a 256 testroms/*.nes     # fill the manifest sha256 column
```

## What NOT to Add

- No cc65/ca65, Python or other ROM-building toolchain in CI.
- No `ccache` / third-party cache action before a measurement justifies it.
- No save-file I/O in the library or libretro adapter; only the runner touches the file system.
- No `.sav`/`.srm` files in the tree (hygiene rejects them); generate in tests.
- No GPL or LGPL emulator source or its test-ROM sets (Mesen's, etc.); MesenCE stays a released-binary oracle.
- No copyrighted-asset games (signOS, franchise-derived hacks) even when the code licence is permissive.
- No second RetroArch version in e2e; stay on 1.22.2 until a newer stable exists.

## Gaps to Address in Phase Research

- RetroArch's behaviour when a `.srm` length differs from `retro_get_memory_size` (not verifiable without reading its GPL source; test against the pinned binary in e2e).
- Frames and exact on-screen layout needed for Holy Mapperel results; whether the S8K ROM keeps `SAVEDATA` across a cold start with the 8 KiB `.sav` fixture.
- Whether `runner.nes` really writes its battery RAM during play (header declares it; not run).
- iNES 1 vs NES 2.0 RAM defaults for mapper 4 and mapper 1 on the blargg MMC3 ROMs (they need `$6000` RAM yet declare none).
- MMC3 NES 2.0 submapper handling and which revision the nightly MMC3 suites should expect.
- Compile-versus-test time split on the slowest CI legs.

## Sources

- Holy Mapperel: `github.com/pinobatch/holy-mapperel` (README, LICENSE = Zlib, tag v0.02 at `c022622`, asset hash computed 2026-10-09). HIGH.
- nes-runner: `github.com/zorchenhimer/nes-runner` (MIT, tag v1 at `47537be`, release asset hash computed). HIGH for licence; ROM behaviour not yet run.
- Stallar: `github.com/wendelscardua/stallar` (MIT, README public-domain note, tag 0.1.0 at `0188c66`). MEDIUM for art provenance.
- signOS README (asset restriction): `github.com/wendelscardua/signos`. HIGH.
- blargg MMC3 suites: `github.com/christopherpow/nes-test-roms` @ `95d8f62` (readme in `mmc3_test_2`; file hashes computed; no licence). HIGH on facts, no redistribution.
- `libretro.h` in this repository, L498-520 (memory ids); `libretro/libretro.c` stub; existing `.planning/preparation/LIBRETRO-AND-RUNNER.md` sections 1, 2, 5; CONFORMANCE.md "Classic suites" and "ROMs that can be committed". HIGH.
- Runner image inventories: `github.com/actions/runner-images` (README label table, Ubuntu 24.04, macOS 15, Windows 2025 readmes: no ccache; CMake 3.31.x / 4.x; Ninja 1.13.2). HIGH.
- Action releases fetched via the GitHub API on 2026-10-09: checkout v7.0.1, upload-artifact v7.0.2, download-artifact v8.0.2, cache v6.1.0, attest v4.2.2, release-please-action v5.0.0, ccache-action v1.2.24, RetroArch v1.22.2. HIGH.
- GitHub changelog on macOS 13 closing and Intel runner support ending after the macOS 15 image (fall 2027); macOS 14 retirement notice dated 2026-10-01. MEDIUM (secondhand via search summary).
- Job timings: `actions/runs/38011892333` on `main`; local `ctest -j1` timing on this machine. HIGH.
- RetroArch autosave defaults: forum and wiki posts that disagree by version. LOW; do not depend on a default.
