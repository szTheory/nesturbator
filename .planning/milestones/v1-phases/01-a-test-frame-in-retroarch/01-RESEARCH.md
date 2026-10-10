# Phase 1: A test frame in RetroArch - Research

**Researched:** 2026-10-02
**Domain:** C17 library/runner/libretro build, CMake presets and CPack, GitHub Actions CI and release-please, NTSC palette generation, RetroArch automation on macOS
**Confidence:** HIGH for build/packaging/palette facts (measured or read this session); MEDIUM for RetroArch runtime behaviour (no RetroArch on this machine)

## Summary

The preparation files settle almost every choice for this phase: presets, CI matrix, release flow, names, header conventions, test-card layout, palette approach and the RetroArch comparison. This research fills the gaps they leave and checks their assumptions against measurements. Five findings change what the planner must write:

1. **One D-14 invariant is false.** A direct YUV decode of the wiki's levels makes red emphasis raise R in 33 of 64 entries and blue emphasis raise B in 31 (for example `$00`: `626262` becomes `674837`). "Emphasis never raises a channel" would fail on every runner. The invariant that holds across all 512 entries is: **an emphasis bit never raises a channel other than the one it names** (red emphasis may raise R, blue may raise B, green raises none). Measured with a prototype of the D-13 algorithm in this session.
2. **The undefined-symbol allowlist must include the default allocator and the toolchain's hardening symbols.** The default allocator in D-08 puts `malloc` and `free` in the core. A `-O2` Apple clang build of a core-like file also leaves `___memcpy_chk`, `___stack_chk_fail` and `___stack_chk_guard` undefined. All five were measured with `nm -u`, and they appear with or without `-mgeneral-regs-only`.
3. **CMake `MODULE` libraries on macOS get a `.so` suffix.** Measured: `nesturbator_libretro.so`. The target needs `SUFFIX ".dylib"` on Apple, or RetroArch and the archive layout get the wrong file name.
4. **Apple clang contracts `a*b+c` into `fmadd` by default on arm64.** Measured. `tools/palgen` must build with `-ffp-contract=off` (GCC/Clang). MSVC since VS 2022 generates no contractions under `/fp:precise` (cited). Without this, the "regenerate and byte-compare" test can differ between runners.
5. **RetroArch pauses when its window loses focus** (`pause_nonactive = true` by default, read from the pinned `retroarch.cfg`). A ctest-launched window may never reach `--max-frames`. `test.cfg` must set `pause_nonactive = "false"`, which D-18 does not list.

The phase also depends on things that do not yet exist. There is **no git remote**, and RetroArch and clang-format are **not installed** on this machine. FRAME-07 (PR archives, release) cannot be shown until the owner creates the GitHub repository, the release credential, auto-merge and the ruleset. FRAME-05 skips locally until RetroArch is installed. The GSD-HANDOFF already lists these as owner tasks "before executing phase 1".

**Primary recommendation:** Build bottom-up in this order: header, core and `check.h` tests under `dev`/`ci` on the Mac; then palgen and the table; the runner; the libretro adapter with a `dlopen` host test; install/consumer/CPack; the `asan`/`nofp`/`hygiene` presets; the RetroArch test; and last the CI and release workflows. Each step should end with `cmake --workflow --preset ci` green locally.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Test card pixels, frame metadata, audio sample count | Core library (`src/`) | — | Deterministic, integer-only, all state in the instance (DEC.12, D-01, D-09) |
| Native-to-XRGB8888 table | Core library (copy-out `nesturbator_get_palette`) | Offline tool `tools/palgen` | The table is integer data in the core; it is generated offline with floating point outside the core (D-12, D-13) |
| SHA-256, PPM writer, option parsing, exit codes | Runner (`runner/`) | — | Hashing and files are host concerns; the core links only memory functions |
| Pixel conversion, mono to stereo, fps/sample rate as double, no-game | libretro adapter (`libretro/`) | — | `libretro.h` has `double fps`, so conversion happens at the boundary ([ENG.18]) |
| Equality of runner and adapter frames | Tests (`tests/`) | Runner output file as fixture | Compare actual outputs, not re-derived ones |
| RetroArch screenshot comparison | Tests (`tests/retroarch/`, macOS only) | Platform tool `sips` | D-17 |
| Build lanes, archives | CMake presets + CPack | GitHub Actions calls the same presets | DEC.28 |
| Version, changelog, release | release-please + `release.yml` | `version.txt` read by CMake | DEC.30 |

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

Everything in `.planning/preparation/DECISIONS.md` (DEC.01–DEC.39) is settled and not repeated here. The decisions below fill the gaps that remained for Phase 1.

#### Test frame content
- **D-01:** The no-cartridge frame is a **static test card computed from coordinates**: a pure integer function `test_pixel(x, y)` returning a native 16-bit pixel (`entry | emphasis << 6`), called for every pixel on every `run_frame` while no cartridge is loaded. No table, no font, no per-instance state for the pattern. Frame N equals frame 1.
- **D-02:** Layout (0-based rows/columns; first matching rule wins):
  1. Corner block: rows 0–7, columns 0–7 → `$30`, emphasis 0. Only at top-left, so any mirror shows.
  2. Border: row 0, row 239, column 0, column 255 → `$30`.
  3. Top overscan band rows 1–7 → `$16` (red); bottom overscan band rows 232–238 → `$12` (blue). Different colours so a vertical flip shows.
  4. Chart: rows 8–231 in 8 bands of 28 rows. Band `b = (y-8)/28` is the emphasis value 0–7; inside a band `r = ((y-8)%28)/7` is the high nibble 0–3; column `c = x/16` is the low nibble. Pixel `= (r<<4 | c) | b<<6`. Every one of the 512 native values appears in its own 16×7 cell.
- **D-03:** No version, ABI or text in the pixels. Version information is reported by the version call, `retro_get_system_info`/`.info` and the runner; putting it in pixels would change the recorded hash on every release.
- **D-04:** Frame advance is shown by frame metadata, not pixels: each `run_frame` advances `frame_number` by 1 and `ticks` by one NTSC frame. A runner test checks that `--hash-frame 3` equals `--hash-frame 1` and that metadata advanced.
- **D-05:** The libretro test program also checks about four pixels against XRGB8888 values hard-coded in the test (for example (0,0) = white `$30`; one band-0 cell; the same cell in band 1; a `$0D` cell). This catches an indexing bug in the shared palette path that would otherwise cancel out in the runner-vs-adapter comparison.

#### Public header scope (Phase 1)
- **D-06:** The header ships **only the surface Phase 1 exercises**, with every convention fixed now; later phases only append functions, struct fields at the end, and status codes. No stubs that pretend a feature exists. — **Reversibility:** costly — the conventions (prefix, size tags, ABI number, status-code numbering, allocator signature) are copied by every host and by the sibling core; changing them later breaks every consumer even under 0.x.
- **D-07:** Conventions:
  - Prefix `nesturbator_` for functions and types, `NESTURBATOR_` for macros and constants; no `_t` suffix (POSIX reserves it). Opaque instance `typedef struct nesturbator nesturbator;`. `extern "C"` guards.
  - Every boundary struct starts with `uint32_t size`. Input: reject `size` 0 or below the struct's first released size with `NESTURBATOR_ERR_STRUCT_SIZE`; accept a larger size only if the unknown tail bytes are zero. Output: write only min(size, own sizeof). One internal helper does this before any other field is read (the tag is untrusted).
  - Struct fields are fixed-width integers and pointers only — no enum, `bool` or `size_t` fields (the allocator's `size_t` arguments are the exception).
  - `NESTURBATOR_ABI_VERSION` is an integer starting at 1, raised by hand only in a breaking header change; appending fields or functions does not raise it. `NESTURBATOR_CONFIG_INIT` macro fills `size` and `abi` from the header the host compiled against (zlib `deflateInit` pattern); `create` refuses a mismatch with `NESTURBATOR_ERR_ABI`. A shared library's SOVERSION equals the ABI number.
  - Library version macros `NESTURBATOR_VERSION_MAJOR/_MINOR/_PATCH` carry release-please markers and are separate from the ABI number.
  - `enum nesturbator_status` with explicit values, only appended: `NESTURBATOR_OK = 0`, `ERR_ARGUMENT`, `ERR_STRUCT_SIZE`, `ERR_ABI`, `ERR_NO_MEMORY`, `ERR_BUFFER_TOO_SMALL`. A failing call changes no state.
- **D-08:** Phase 1 functions:
  1. `void nesturbator_get_version(nesturbator_version *out)` — size, major, minor, patch, abi, behaviour_revision (state-format and battery-layout numbers appended in the phases that create them).
  2. `nesturbator_status nesturbator_create(const nesturbator_config *cfg, nesturbator **out)` — config: size, abi, allocator `{ void *(*alloc)(void *user, size_t size); void (*free)(void *user, void *ptr, size_t size); void *user; }`. All-NULL allocator means the C library's malloc/free; partly NULL is `ERR_ARGUMENT`. `free` receives the size so arena allocators work.
  3. `void nesturbator_destroy(nesturbator *)` — accepts NULL.
  4. `void nesturbator_get_info(const nesturbator *, nesturbator_info *out)` — size, width 256, height 240, frame rate 39375000/655171, sample rate 48000/1 (both as num/den). The seed of the future per-system descriptor.
  5. `void nesturbator_get_palette(const nesturbator *, uint32_t *out_xrgb8888, uint32_t count)` — copies up to 512 entries, indexed by native pixel value. A copy-out function, not an exported data symbol (avoids Windows DLL data-import pitfalls and leaves room for per-PPU-revision tables).
  6. `nesturbator_status nesturbator_run_frame(nesturbator *, nesturbator_frame *io)` — in: size, `uint16_t *video`, `uint32_t video_pitch` (pixels), `int16_t *audio`, `uint32_t audio_capacity`; out: `audio_count`, `uint64_t frame_number`, `uint64_t ticks`. Buffers are checked before anything runs.
- **D-09:** With no cartridge, `run_frame` returns **silence at the true sample count** (accumulate so that 352 samples are produced per 13125 CPU cycles at 48000 Hz), documented as "no cartridge: test pattern and silence". This exercises the capacity check, the adapter's mono-to-stereo batch call and RetroArch's audio sync from day one.
- **D-10:** Left out until the phase that tests them: load/unload and the adapter's content path (Phase 3; `retro_load_game` with content returns false in Phase 1), controller input and memory peek/regions (Phase 3), run_ticks with stop reason (Phase 2 uses internal test hooks), save state and battery RAM (later milestones), a status-string function.
- **D-11:** Tests under `ci`: one per status code; a counting allocator showing every allocation freed; a failing allocator giving `ERR_NO_MEMORY`; struct size too small and too large with a non-zero tail; ABI mismatch; too-small buffer leaves state unchanged; exact timing fractions; audio count summed over a whole period; the header compiles alone with `-std=c17 -pedantic -Werror` and as C++; `_Static_assert` on field offsets; the FRAME-02 consumer program against the installed package.

#### Palette table source
- **D-12:** The 512-entry native→XRGB8888 table is produced by **our own offline C generator** `tools/palgen/palgen.c`, built by CMake but not linked into the core, and the output `src/…/palette_ntsc.c` (`const uint32_t[512]`, integers only) is checked in with a "generated by tools/palgen — do not edit" header naming its parameters and wiki revisions. This refines DEC.13 / NES-HARDWARE-PPU-CARTRIDGE §4 ("from pally or a host file"): pally is not vendored; the same pull request updates that preparation file. — **Reversibility:** reversible — swapping the table changes no hash.
- **D-13:** Generator inputs are cited NESdev Wiki facts only (NTSC video, PPU palettes, Colour emphasis): the signal levels for `$0D`–`$20`/`$30`, 12-phase square waves with colour `$xY` on wave Y, `$xE/$xF` at the black level, emphasis attenuating the voltage on active phases (native bit 6 = PPUMASK bit 5 → wave C, native bit 7 = PPUMASK bit 6 → wave 4, native bit 8 = PPUMASK bit 7 → wave 8, `$xE/$xF` excepted; verify against the wiki revision cited), and the wiki's YUV→RGB matrix. A plain direct R'G'B' decode — no colorimetry (`pow`, `cos`) in Phase 1. The generator uses only `+ − × ÷` and `sqrt`, built with floating-point contraction off, so its output is bit-identical on every runner.
- **D-14:** One `ci` test regenerates the table and byte-compares it with the checked-in file (`cmake -E compare_files`) on all six runners. Invariant tests: `$20` equals `$30` and is white; `$xE/$xF` are black and unaffected by emphasis; emphasis 0 is the base palette; emphasis never raises a channel; luma is monotonic down each column.
- **D-15:** The runner and the libretro adapter both get colours from `nesturbator_get_palette` and convert with the same loop, so the RetroArch comparison is like for like (D-05 guards the shared path).

#### Frame image and RetroArch comparison
- **D-16:** `nesturbator-run --dump-frame N:FILE` writes a **binary PPM (P6)**, 256×240, in palette RGB from `nesturbator_get_palette`. The SHA-256 from `--hash-frame` stays over native 16-bit pixels (little-endian bytes), never the image. No PNG writer.
- **D-17:** `ctest -L retroarch` (macOS only): a `cmake -P` driver launches RetroArch, converts its PNG screenshot to BMP with macOS `sips -s format bmp`, and an in-repo C comparator (`tests/retroarch/compare_frame.c`) reads the runner's P6 and the BMP (24 or 32 bpp, BI_RGB or BI_BITFIELDS, either row order, bounds-checked) and requires **exact** RGB equality at exactly 256×240. On mismatch it reports the first differing pixel. No inflate code, no reference image committed.
- **D-18:** The RetroArch run is isolated with `-c <build>/retroarch/test.cfg` (generated from a template) so the user's config is never read: `config_save_on_exit = "false"`, `video_gpu_screenshot = "false"` (raw core frame, no scaling/shader/aspect), `libretro_info_path`, `system_directory`, `savefile_directory`, `savestate_directory`, `screenshot_directory` all under the build directory, `audio_driver = "null"`, `menu_show_start_screen = "false"`, `video_fullscreen = "false"`. Launched with `-L <absolute path to built dylib>` and no content (relies on `SET_SUPPORT_NO_GAME`), `--max-frames=N --max-frames-ss --max-frames-ss-path=<build>/retroarch/shot.png`.
- **D-19:** Driver order: exit 77 if the RetroArch binary is missing (check a `NESTURBATOR_RETROARCH` override, then `/Applications/RetroArch.app/Contents/MacOS/RetroArch`); fail if RetroArch exits non-zero or the screenshot is missing; fail if the image is not 256×240. CTest properties: `SKIP_RETURN_CODE 77`, `TIMEOUT 60`, `RUN_SERIAL`, label `retroarch`. Needs a logged-in GUI session.
- **D-20:** If RetroArch's PNG carries a non-sRGB `gAMA` or an `iCCP` chunk, `sips` colour-converts (measured: `gAMA 1.0` shifted colours; plain, `gAMA 1/2.2` and `sRGB` were exact). Fallback: `sips --deleteColorManagementProperties` before converting.

### Claude's Discretion
- Exact source layout below `src/` (e.g. `src/video/` vs flat), file names other than those named above, and the internal structure of the instance.
- The libretro test program's shape (a host stub that `dlopen`s the core and drives it the way RetroArch does), within FRAME-04.
- Runner option parsing and exit codes, following LIBRETRO-AND-RUNNER §5; only `--frames`, `--hash-frame`, `--dump-frame` are needed in Phase 1.
- The value of N for `--max-frames` (the card is static, so any N ≥ 1 works).

### Deferred Ideas (OUT OF SCOPE)
- Colorimetry in the palette generator (SMPTE-C→sRGB, CRT gamma, hue tweak) — Phase 3, when real games make colour quality matter; keep the regeneration check exact.
- Loading a host `.pal` file (192 or 1536 bytes) as a display-only libretro core option — Phase 3.
- Calibrating RetroArch's frame index against the runner's (`--max-frames` off-by-one) with a moving NROM frame — Phase 3; the static card cannot reveal it.
- Running the RetroArch test off macOS (would need ImageMagick or a decoder) — not planned.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| FRAME-01 | `cmake --workflow --preset ci` (`ci-msvc` on Windows) builds library, runner, libretro core and runs every test on all six platforms; CI runs the same commands | Preset skeleton (Code Examples); runner-image tool table (Environment); MSVC env step; `CC=gcc-14` note; Ninja on all images except an unconfirmed `ubuntu-24.04-arm` |
| FRAME-02 | A C program using only the public header creates, runs one frame, destroys; `ci` builds and runs it against the installed package | `ctest --build-and-test` fixture pattern; component install; `NESTURBATOR_CONFIG_INIT` C/C++ pitfalls (Pitfalls 6, 7) |
| FRAME-03 | No cartridge → fixed test frame; `nesturbator-run --frames 1 --dump-frame 1:FILE --hash-frame 1` writes the image and prints a SHA-256 equal on every platform | Independent expected hash `b49e9be4…0453`; SHA-256 FIPS vectors; ticks/audio arithmetic; CRLF pitfall in `hashes.txt` |
| FRAME-04 | libretro core loads in a test program calling it as RetroArch does; frame equals the runner's | Pinned `libretro.h` SHA-256; `RETRO_API` export macro; `dlsym` via `memcpy`; call order; `.dylib` suffix; hidden visibility |
| FRAME-05 | README one-line install on Apple Silicon; `ctest -L retroarch` launches RetroArch unattended, compares screenshot; skipped where RetroArch absent | Measured `sips` BMP layouts; `pause_nonactive`; `audio_enable`; asan exclusion; README version marker |
| FRAME-06 | `asan`, `nofp`, `hygiene` presets pass | Measured undefined-symbol set; float-scan comment pitfall; clang-format version pin; `uses:` SHA check; Ubuntu hardening defaults |
| FRAME-07 | Every PR builds the archives per platform; a behaviour-changing merge publishes them with `SHA256SUMS` as a GitHub release with no manual step | CPack component ZIP variables (read from local CMake 4.4.3 docs); action SHAs; attest `subject-checksums`; ruleset required for auto-merge; owner tasks |
</phase_requirements>

## Project Constraints (from CLAUDE.md)

- **Own code:** all emulation and test code written here; `libretro.h` is the only vendored file; the core links only the C memory functions. No test framework, no PNG or zip library, no third-party CMake modules.
- **Three deliverables:** library, runner, libretro adapter. No window, audio-device or input-device code. The RetroArch test drives an external binary; it adds no GUI code.
- **Clean tree:** no ROM/BIOS bytes except entries in `tests/roms/manifest.txt`. No personal paths, email addresses or names. Commits use the GitHub noreply identity. `scripts/hygiene.sh` runs on commit and push. A tracked binary file needs a manifest entry (ENGINEERING §7 adds this rule this phase).
- **Clean room:** implement from hardware documentation. Do not open GPL/LGPL emulator source. The NESdev wiki's palette example code is CC BY-SA (see Pitfall 1): use its facts, not its code.
- **Deterministic core:** integer-only, all state in the instance, the same hashes on every platform.
- **Tested and documented together:** every behaviour change has a test run by `cmake --workflow --preset ci`. The README and the public header comments are updated in the same change. No check waits on a person.
- **One step at a time; phase branch; PR with a Conventional Commit title.**
- **Style:** small modules, plain control flow, fixed-width integer types. Hardware comments cite their source. Follow the ponytail ladder (remove the need → reuse → standard library → platform → minimum new code).
- **Owner preference (CONTEXT specifics):** "another copy and paste is better than another dep". Prefer a few owned lines or a platform tool (`sips`, `cmake -E`) over any library or third-party action.

## Standard Stack

### Core
| Tool | Version | Purpose | Why Standard |
|------|---------|---------|--------------|
| C | C17, `CMAKE_C_EXTENSIONS OFF` | All code | DEC.27 [CITED: ENGINEERING §1] |
| CMake | ≥ 3.25 (presets schema 6); local 4.4.3 | Build, presets, CTest, CPack | [VERIFIED: `cmake --version` → 4.4.3] |
| Ninja | 1.13.2 local and on five of six images | Generator | [VERIFIED: runner-images readmes; local `ninja --version`] |
| `libretro.h` | RetroArch v1.22.2, commit `69a4f0ea1e8aaf442ae4858f2e7f2b31a1776576`, SHA-256 `bd3398d29c3763d18617087020ff56b5450a48a63123403a4b674f8e79947acb`, 7846 lines | The only vendored file | [VERIFIED: fetched from raw.githubusercontent at that commit and hashed with `shasum -a 256` this session] |
| CPack ZIP generator | built into CMake | Release archives | No external zip tool; libarchive is inside CMake [CITED: cmake `cpack-generators(7)`, local 4.4.3] |

### CI actions (pin by full SHA; DEC.29)
| Action | Tag | Commit SHA | Use |
|--------|-----|-----------|-----|
| actions/checkout | v7.0.1 | `3d3c42e5aac5ba805825da76410c181273ba90b1` | every job |
| actions/upload-artifact | v7.0.1 | `043fb46d1a93c77aae656e7c1c64a875d1fc6a0a` | archives, `hashes.txt` |
| actions/download-artifact | v8.0.1 | `3e5f45b2cfb9172054b4087a40e8e0b5a5461e7c` | `hash-equality`, publish |
| googleapis/release-please-action | v5.0.0 | `45996ed1f6d02564a971a2fa1b5860e934307cf7` | release PR / release |
| actions/create-github-app-token | v3.2.0 | `bcd2ba49218906704ab6c1aa796996da409d3eb1` | release credential (DEC.31) |
| actions/attest | v4.2.2 | `1e69f48acb82d1966a394da916b4c1698aa569d6` | provenance on `SHA256SUMS` |

[VERIFIED: `gh api repos/<repo>/releases/latest` and `gh api repos/<repo>/commits/<tag>` this session.] The executor should re-resolve the SHAs at execution time if a newer patch release exists. Dependabot (`github-actions` only) keeps them current afterwards.

**No MSVC setup action.** Load the developer environment with ~6 lines of PowerShell (`vswhere` + `vcvarsall.bat` → `$GITHUB_ENV`). This follows the owner's "own lines over a dependency" rule. See Code Examples.

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| own vcvars step | `ilammy/msvc-dev-cmd` | One less script, but a third-party action holding the job token |
| CPack component ZIPs | `cmake -E tar cf x.zip --format=zip` in a script | Works, but the `package` workflow step expects CPack. Use CPack. |
| `ctest --build-and-test` for the consumer | ExternalProject | `--build-and-test` is CTest's built-in mode for exactly this [CITED: `ctest(1)` Build and Test Mode] |
| `dlopen` host test | Linking the adapter statically into the test | A static link would not show the 25 `retro_*` exports exist in the shipped module |

**Installation:** none. No package manager installs anything in this phase. On CI, `ubuntu-24.04-arm` may need `sudo apt-get install -y ninja-build` (see Environment).

## Package Legitimacy Audit

No npm, PyPI or crates packages are installed in this phase. External code consists of the vendored `libretro.h` (pinned and hashed above) and the six GitHub Actions above. They come from official `actions/*`, `googleapis/*` repositories and are pinned by commit SHA.

| Package | Registry | Age | Downloads | Source Repo | Verdict | Disposition |
|---------|----------|-----|-----------|-------------|---------|-------------|
| (none) | — | — | — | — | — | — |

**Packages removed due to [SLOP] verdict:** none
**Packages flagged as suspicious [SUS]:** none

## Architecture Patterns

### System Architecture Diagram

```
                    nesturbator_create(cfg) ──► instance {alloc, frame_number, ticks, audio_rem}
                                                     │
 host buffers ──► nesturbator_run_frame(io) ──► validate size tag ─► validate buffers ─┐ (fail: status, no state change)
                                                     │                                  │
                                                     ▼                                  │
                                    test_pixel(x,y) → video[y*pitch+x] (uint16 native)  │
                                    audio: n = (rem + 714732*352)/315000; zeros          │
                                    frame_number += 1; ticks += 714732 ◄────────────────┘
                                                     │
            ┌────────────────────────────────────────┼─────────────────────────────────────┐
            ▼                                        ▼                                     ▼
   runner (nesturbator-run)               libretro adapter (MODULE)                 consumer test
   SHA-256 over LE uint16 ─► stdout       palette[512] (get_palette) ─► XRGB8888    create/run/destroy
   palette ─► P6 PPM file ─┐              mono ─► stereo ─► audio_batch_cb          via installed pkg
                           │              video_cb(256,240,1024)
                           │                         │
                           │          ┌──────────────┴──────────────┐
                           │          ▼                             ▼
                           │   libretro host test (dlopen)    RetroArch (-L, -c test.cfg,
                           │   captures video_cb buffer        --max-frames-ss) ─► PNG
                           │          │                             │ sips -s format bmp
                           └────► compare (exact RGB) ◄────────────-┘ compare_frame.c
```

### Recommended Project Structure
```
include/nesturbator.h          # the one public header (version macros carry release-please markers)
src/
├── instance.c                 # create/destroy/get_version/get_info/get_palette, size-tag helper
├── frame.c                    # run_frame: buffer checks, test card, audio count, metadata
├── testcard.c                 # test_pixel(x, y) (pure function)
├── palette_ntsc.c             # GENERATED by tools/palgen — do not edit
└── internal.h
runner/  main.c  sha256.c sha256.h  ppm.c ppm.h
libretro/ libretro.c  libretro.h (vendored, unmodified)  nesturbator_libretro.info
tools/palgen/palgen.c          # host tool, not linked into anything shipped
tests/
├── check.h                    # comparison macros
├── core/*.c                   # D-11 unit tests (status codes, allocators, size tags, audio period…)
├── header_c.c header_cxx.cpp  # header compiles alone, as C and as C++
├── consumer/CMakeLists.txt main.c   # FRAME-02, built by ctest --build-and-test
├── libretro_host.c            # FRAME-04 dlopen host
├── retroarch/compare_frame.c run_retroarch.cmake test.cfg.in   # FRAME-05
└── cmake/undefined_symbols.cmake float_scan.cmake action_pins.cmake palette_regen.cmake
cmake/nesturbatorConfig.cmake.in  nesturbator.pc.in
CMakeLists.txt  CMakePresets.json  version.txt (0.0.0)
release-please-config.json  .release-please-manifest.json
.github/workflows/ci.yml release.yml   .github/dependabot.yml   .github/rulesets/main.json
.clang-format  SECURITY.md  CONTRIBUTING.md  THIRD-PARTY-NOTICES.md
```

### Pattern 1: Size-tagged struct check (one helper, before any field is read)
**What:** validate `size` against `[first_released_size, ∞)`. If larger than our `sizeof`, every byte past our `sizeof` must be zero. Output structs: write `min(size, sizeof)` bytes from a fully built local copy using `memcpy`.
**When:** every public function taking a struct pointer.
```c
/* Returns NESTURBATOR_OK or NESTURBATOR_ERR_STRUCT_SIZE. 'first' is the size
   of the struct as first released; 'ours' is sizeof in this build. */
static nesturbator_status check_size_in(const void *s, uint32_t first, uint32_t ours)
{
    uint32_t size;
    memcpy(&size, s, sizeof size);           /* the tag is untrusted */
    if (size == 0u || size < first) return NESTURBATOR_ERR_STRUCT_SIZE;
    for (uint32_t i = ours; i < size; i++)
        if (((const uint8_t *)s)[i] != 0u) return NESTURBATOR_ERR_STRUCT_SIZE;
    return NESTURBATOR_OK;
}
```

### Pattern 2: Exact audio sample count (D-09) in ticks
NTSC: 24 ticks per CPU cycle [CITED: ARCHITECTURE §2], so "352 samples per 13125 CPU cycles" is **352 samples per 315000 ticks**. One average NTSC frame is 89341.5 dots × 8 = **714732 ticks**. That matches the reported rate exactly: tick rate 472500000/11 Hz ÷ 714732 = 39375000/655171 (derived; 472500000/12 = 39375000 and 7862052/12 = 655171).
```c
uint64_t acc = inst->audio_rem + 714732u * 352u;   /* fits easily in 64 bits */
uint32_t n   = (uint32_t)(acc / 315000u);           /* 798 or 799 */
/* check n <= io->audio_capacity BEFORE mutating anything */
inst->audio_rem = acc % 315000u;
```
Whole-period test (D-11): over **13125 frames** the sum of `audio_count` is exactly **10482736** (= 29780.5 × 352; 714732 × 13125 / 315000 = 29780.5). Derived arithmetic. The adapter's stereo buffer needs ≥ 799 frames; use a fixed 1024.

### Pattern 3: libretro host test, the way RetroArch calls a core
Order: load the module; resolve all 25 `retro_*` symbols (a missing one fails). Then `retro_api_version()==1` → `retro_set_environment(env)` (record `SET_SUPPORT_NO_GAME`=18 true) → `retro_init()` → `retro_get_system_info()` → `retro_set_video_refresh/audio_sample/audio_sample_batch/input_poll/input_state` → `retro_load_game(NULL)` must return true and call `SET_PIXEL_FORMAT`=10 with `RETRO_PIXEL_FORMAT_XRGB8888`=1 → `retro_get_system_av_info()` → `retro_run()` (expect exactly one video callback with 256×240 and pitch ≥ 1024, ≥ 1 input poll, one batch call with 798/799 frames of zeros) → `retro_unload_game()` → `retro_deinit()`. Then compare the captured XRGB buffer with the runner's P6 (CTest fixture) and the four D-05 pixels. Also assert `retro_load_game(&info_with_data)` returns false (D-10). [Constants VERIFIED: pinned libretro.h L865 `#define RETRO_ENVIRONMENT_SET_PIXEL_FORMAT 10`, L1051 `#define RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME 18`, L97 `#define RETRO_API_VERSION         1`]

### Pattern 4: Exports and visibility
`libretro.h` L63-85 defines `RETRO_API` as `__declspec(dllexport)` on `_WIN32` (unless `RETRO_IMPORT_SYMBOLS`) and `__attribute__((__visibility__("default")))` on GCC ≥ 4 [VERIFIED: pinned libretro.h L63-85]. So defining the functions in `libretro.c` with the header's declarations exports them. Set `C_VISIBILITY_PRESET hidden` on both the static core library and the module. Then only `retro_*` leave the module, and `nesturbator_*` from the linked static library do not. Set `POSITION_INDEPENDENT_CODE ON` on the static core library, because it is linked into a shared module on Linux.

### Anti-Patterns to Avoid
- **Transcribing the wiki's `NTSCsignal`/decoder C++ into palgen.** The wiki page says its example sources are CC BY-SA 4.0. Take the facts (levels, phase table, emphasis phases, matrix constants) and write the generator in this project's own structure.
- **Hashing the PPM.** D-16: hash native LE uint16 only.
- **A shared `nesturbator` library in Phase 1 archives.** Shipping one would need an export macro in the public header (`dllexport/dllimport`) and another lane. Ship static. Set `VERSION`/`SOVERSION` properties so a later `BUILD_SHARED_LIBS=ON` honours D-07. This follows the "remove the need" rung.
- **`audio_driver = "null"` as the only audio setting.** That driver name is not confirmed for the macOS build. Add the verified key `audio_enable = "false"` as well.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Archives (zip) | a zip writer | CPack `ZIP` with `CPACK_ARCHIVE_COMPONENT_INSTALL ON` | Built into CMake on all six runners |
| PNG decoding | inflate | `sips -s format bmp` (macOS) | D-17; measured layouts below |
| Consumer configure/build/run | a script | `ctest --build-and-test` | Built into CTest |
| Test runner/selection/timeouts | a framework | CTest + `tests/check.h` | DEC.24 |
| File checksums in release | own hasher | `sha256sum` (Linux job) in `shasum -a 256` format | Format is identical; `-c` check tested in prep |
| Provenance | own signing | `actions/attest` with `subject-checksums: SHA256SUMS` | The input exists in v4.2.2 [VERIFIED: action.yml] |

**Key insight:** the runner's SHA-256 *is* owned code, because the runner must print hashes with no dependency. Write it from FIPS 180-4 and test it with the three standard vectors (see Code Examples).

## Common Pitfalls

### Pitfall 1: Wiki palette code is CC BY-SA
**What goes wrong:** copying the wiki's `NTSCsignal()`/decoder into `tools/palgen` imports share-alike expression into an MIT repo.
**How to avoid:** use only these facts, cited by wiki revision.
- Levels in mV: low `228 312 552 880`, high `616 840 1100 1100`, attenuated low `192 256 448 712`, attenuated high `500 676 896 896`.
- Colour `$xY` is high on phase p when `(Y + p) mod 12 < 6`. Colour 0 is high only; colours 13–15 are low only; `$xE/$xF` force level 1 (= `$1D`, 312 mV).
- Emphasis (PPUMASK bit 5 → wave C, bit 6 → wave 4, bit 7 → wave 8) attenuates on those waves' high phases, except `$xE/$xF`.
- Normalise with black 312 mV and white 1100 mV.
- Demodulate with chroma gain 2 and reference angles 15° + 30°·k: the wiki's "+3 − 0.5" offsets.
- Matrix from 0.299/0.587/0.114 with 0.492111 (B−Y) and 0.877283 (R−Y).
- Clip to [0,1] and round to 8 bits.

[VERIFIED: NESdev wiki "NTSC video" raw source, revision 24244 (2026-09-10)]. The page states "Both sources are licensed under Creative Commons Attribution-ShareAlike 4.0 International." The emphasis-bit mapping in D-13 matches the wiki's table ("Bit 7 Color 8 / Bit 6 Color 4 / Bit 5 Color C"), so D-13's "verify" item is done.
**Sine/cosine without `cos`:** the 12 reference angles are 75°, 105°, …, i.e. 15° + 30°k. Their sines and cosines are ±(√6−√2)/4, ±√2/2 and ±(√6+√2)/4, so `sqrt` alone suffices, as D-13 requires. A prototype found **0** of 512 entries differing between the `sin()` and `sqrt` forms. A colorburst self-check confirms the reference phase: colour `$x8` decodes to V = 0, U < 0 (−U), which is the wiki's statement that colorburst is phase 8.
**Revisions to cite in the generated header:** NTSC video rev 24244 (2026-09-10), PPU palettes rev 24257 (2026-09-13), Colour emphasis rev 23220 (2025-11-03) [VERIFIED: MediaWiki API query this session].

### Pitfall 2: D-14 "emphasis never raises a channel" fails
**What goes wrong:** the invariant fails for 64 entries, because red emphasis raises R and blue emphasis raises B (attenuating one hue increases its complement's chroma).
**How to avoid:** use "an emphasis bit never raises a channel other than its own" (bit 6 → R allowed, bit 7 → none, bit 8 → B allowed). This held for all 7×64 cases. Also checked and holding: `$20 == $30 == FFFFFF`; all `$xE/$xF` under every emphasis `== 000000`; integer luma `299R+587G+114B` non-decreasing down every column for every emphasis (0 violations). "Emphasis never raises integer luma" also fails twice (`$1D` with bits 6 or 8, from clipping), so do not substitute it. This is a CONTEXT decision, so the planner should record the wording change in the same PR, as D-12 does for its refinement.
**Warning signs:** the invariant test fails on the first run on every platform.

### Pitfall 3: FP contraction breaks the regeneration check
**What goes wrong:** Apple clang `-O2` on arm64 emits `fmadd d0, d0, d1, d2` for `a*b+c`; with `-ffp-contract=off` it emits `fmul` + `fadd` (measured). GCC under `-std=c17` (ISO mode) defaults to contraction off. Clang defaults to `on`. MSVC since VS 2022 generates none under `/fp:precise` [CITED: learn.microsoft.com /fp].
**How to avoid:** `target_compile_options(palgen PRIVATE $<$<C_COMPILER_ID:GNU,Clang,AppleClang>:-ffp-contract=off> $<$<C_COMPILER_ID:MSVC>:/fp:precise>)`. Link `m` on non-Windows (`sqrt`). Do not rely on `#pragma STDC FP_CONTRACT`, because GCC ignores it [ASSUMED].

### Pitfall 4: The undefined-symbol allowlist is too small
**What goes wrong:** `nm -u` on a `-O2` static library of a file using `memcpy`, a local array and `malloc`/`free` gave `___memcpy_chk ___stack_chk_fail ___stack_chk_guard _free _malloc` on Apple clang 21, with or without `-mgeneral-regs-only` (measured).
**How to avoid:** allowlist after stripping one leading `_` on Mach-O: `memcpy memmove memset memcmp malloc free` plus the fortify and stack-protector helpers `__memcpy_chk __memmove_chk __memset_chk __stack_chk_fail __stack_chk_guard`. Ubuntu GCC also enables `_FORTIFY_SOURCE` and `-fstack-protector-strong` by default [ASSUMED], so expect the same names on ELF. Parse the last whitespace token of each `nm -u` line; GNU prints `U name`, Apple prints `name`. Use `${CMAKE_NM}`. A float fixture (a `double` multiply) must make the test fail. Prep saw this fail through `__muldf3`.

### Pitfall 5: CMake MODULE suffix on macOS
**What goes wrong:** the build produces `nesturbator_libretro.so` on macOS (measured with CMake 4.4.3). RetroArch's and the release's name is `.dylib` [CITED: LIBRETRO-AND-RUNNER §6].
**How to avoid:** `set_target_properties(nesturbator_libretro PROPERTIES PREFIX "")` and `if(APPLE) set_target_properties(... SUFFIX ".dylib")`. The output is ad-hoc linker-signed, `flags=0x20002(adhoc,linker-signed)` (measured).

### Pitfall 6: `NESTURBATOR_CONFIG_INIT` under C++ and `-Wextra`
**What goes wrong:** `{ sizeof(nesturbator_config), NESTURBATOR_ABI_VERSION }`
- In C++ it is a narrowing error (`size_t` to `uint32_t` in braces) [ASSUMED].
- In C, GCC/Clang `-Wmissing-field-initializers` (in `-Wextra`) warns on a positional partial initializer [ASSUMED].

**How to avoid:** cast and list every member: `{ (uint32_t)sizeof(nesturbator_config), NESTURBATOR_ABI_VERSION, { NULL, NULL, NULL } }`. The header-as-C++ test catches this.

### Pitfall 7: MSVC `/W4 /WX` on runner and tests
**What goes wrong:** `fopen`, `getenv`, `sscanf` raise C4996 "unsafe" → error [ASSUMED]. `<windows.h>` under `/std:c17` (conforming preprocessor) has raised C5105 on older SDKs [ASSUMED].
**How to avoid:** `$<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>` on runner and test targets (never needed by the core). In the libretro host test, include `<windows.h>` only in a `#ifdef _WIN32` block, with `WIN32_LEAN_AND_MEAN`.

### Pitfall 8: `dlsym` result to function pointer
**What goes wrong:** ISO C has no conversion from `void *` to a function pointer. GCC `-Wpedantic` warns [ASSUMED]; Apple clang 21 accepted the cast (measured).
**How to avoid:** `void *p = dlsym(h, name); memcpy(&fn, &p, sizeof fn);`. On Windows, use `GetProcAddress` with the same `memcpy`. Link `${CMAKE_DL_LIBS}`.

### Pitfall 9: CRLF in `hashes.txt` breaks hash equality
**What goes wrong:** the Windows C runtime writes `\n` as `\r\n` on text-mode `stdout`, so a captured `hashes.txt` differs byte-wise from the Linux/macOS ones [ASSUMED].
**How to avoid:** produce `hashes.txt` with a `cmake -P` step that captures runner output and `string(REPLACE "\r" "" ...)` before `file(WRITE ...)`. Or have the `hash-equality` job strip CR before comparing.

### Pitfall 10: RetroArch pauses unfocused, and other config traps
**What goes wrong:** with the default `pause_nonactive = true` [VERIFIED: pinned `retroarch.cfg` L978-979 "# Pause gameplay when window focus is lost. / # pause_nonactive = true"], a window launched behind the terminal stays paused, and the 60 s timeout fires.
**How to avoid:** add `pause_nonactive = "false"` to `test.cfg.in`. Also add `audio_enable = "false"` (key verified at L296). For the start screen, the pinned template names `rgui_show_start_screen` (L74); `menu_show_start_screen` (D-18) is not in it. Write both, since unknown keys are harmless [ASSUMED]. Verified key names in the template: `config_save_on_exit` (L78), `video_fullscreen` (L138), `libretro_info_path` (L850), `system_directory` (L827), `screenshot_directory` (L877), `savefile_directory` (L895), `savestate_directory` (L899), `video_gpu_screenshot` (L995).

### Pitfall 11: ASan core loaded into RetroArch
**What goes wrong:** under the `asan` preset on the Mac, the retroarch test would load an ASan-instrumented dylib into an uninstrumented RetroArch, which aborts [ASSUMED].
**How to avoid:** the `asan` test preset uses `"filter": {"exclude": {"label": "retroarch"}}`.

### Pitfall 12: Float scan trips on the generated table's comments
**What goes wrong:** the generated header comment in `src/palette_ntsc.c` names parameters such as "black 0.312 V", and the text scan of `src/` flags it as a float literal.
**How to avoid:** have palgen print its parameters as integers (mV, wiki revision IDs, ratios such as `877283/1000000`). Or make the scan skip comments; the integer form is simpler.

### Pitfall 13: Compiler selection on Windows and Linux
**What goes wrong:**
- `windows-2025` has `gcc 15.2.0` on PATH and `windows-11-arm` has `gcc 14.2.0` [VERIFIED: image readmes]. A Ninja configure without `CMAKE_C_COMPILER` could pick MinGW GCC instead of `cl` [ASSUMED].
- On `ubuntu-24.04`, the default `gcc` package is 13 (`gcc 4:13.2.0-7ubuntu1`), while ENGINEERING wants GCC 14 [VERIFIED: Ubuntu2404 readme: "GNU C++: 12.4.0, 13.3.0, 14.2.0"].

**How to avoid:** `ci-msvc` inherits `ci` and sets `CMAKE_C_COMPILER`/`CMAKE_CXX_COMPILER` to `cl`. The Linux job sets `CC=gcc-14 CXX=g++-14`; `asan` sets `clang-18`.

### Pitfall 14: README install line needs a versioned URL
**What goes wrong:** asset names carry `VERSION` (`nesturbator-VERSION-libretro-OS-ARCH.zip`), so `releases/latest/download/...` cannot name the file.
**How to avoid:** add `README.md` to release-please `extra-files` and end the install line with `<!-- x-release-please-version -->`. The generic updater replaces the version on that line [CITED: release-please docs/customizing.md "Updating arbitrary files"]. Also, `~/Library/Application Support/RetroArch` only exists after RetroArch's first launch: start the one-liner with `mkdir -p`. The URL contains the GitHub account handle; see Open Questions.

### Pitfall 15: Auto-merge needs a required check
**What goes wrong:** `gh pr merge --auto` on a release PR waits on required checks. Without a ruleset requiring `CI required`, auto-merge cannot wait for CI [ASSUMED].
**How to avoid:** `.github/rulesets/main.json` (ENGINEERING §6) must be applied, and repository "Allow auto-merge" enabled. Both are owner/admin actions; there is no remote yet.

## Code Examples

### CMakePresets.json skeleton (schema 6)
```json
{
  "version": 6,
  "cmakeMinimumRequired": { "major": 3, "minor": 25, "patch": 0 },
  "configurePresets": [
    { "name": "base", "hidden": true, "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/${presetName}" },
    { "name": "dev", "inherits": "base", "cacheVariables": { "CMAKE_BUILD_TYPE": "Debug" } },
    { "name": "ci", "inherits": "base",
      "cacheVariables": { "CMAKE_BUILD_TYPE": "Release", "CMAKE_COMPILE_WARNING_AS_ERROR": "ON" } },
    { "name": "ci-msvc", "inherits": "ci",
      "cacheVariables": { "CMAKE_C_COMPILER": "cl", "CMAKE_CXX_COMPILER": "cl" } },
    { "name": "asan", "inherits": "base",
      "cacheVariables": { "CMAKE_BUILD_TYPE": "Debug", "CMAKE_COMPILE_WARNING_AS_ERROR": "ON",
        "CMAKE_C_FLAGS": "-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer" } },
    { "name": "nofp", "inherits": "ci", "cacheVariables": { "NESTURBATOR_NOFP": "ON" } },
    { "name": "hygiene", "inherits": "base", "cacheVariables": { "NESTURBATOR_HYGIENE": "ON" } }
  ],
  "buildPresets":   [ { "name": "ci", "configurePreset": "ci" } ],
  "testPresets":    [ { "name": "ci", "configurePreset": "ci",
                        "output": { "outputOnFailure": true },
                        "execution": { "noTestsAction": "error" } } ],
  "packagePresets": [ { "name": "ci", "configurePreset": "ci", "generators": [ "ZIP" ] } ],
  "workflowPresets": [ { "name": "ci", "steps": [
      { "type": "configure", "name": "ci" }, { "type": "build", "name": "ci" },
      { "type": "test", "name": "ci" },      { "type": "package", "name": "ci" } ] } ]
}
```
Each lane repeats the build/test(/package) presets. `asan`'s test preset excludes label `retroarch`. `nofp`/`hygiene` test presets filter labels `abi`/`hygiene`. In a workflow preset, every step must use the same configure preset [CITED: cmake-presets(7)]. The `asan` linker flags also need `CMAKE_EXE_LINKER_FLAGS`/`CMAKE_MODULE_LINKER_FLAGS` set to the same `-fsanitize=` value.

### CPack component archives (names per ENGINEERING §6)
```cmake
set(CPACK_GENERATOR ZIP)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)        # zip root holds cores/, info/, LICENSE
set(_base "nesturbator-${PROJECT_VERSION}")
set(CPACK_ARCHIVE_LIBRETRO_FILE_NAME "${_base}-libretro-${NESTURBATOR_OS}-${NESTURBATOR_ARCH}")
set(CPACK_ARCHIVE_RUNNER_FILE_NAME   "${_base}-runner-${NESTURBATOR_OS}-${NESTURBATOR_ARCH}")
set(CPACK_ARCHIVE_LIBRARY_FILE_NAME  "${_base}-library-${NESTURBATOR_OS}-${NESTURBATOR_ARCH}")
include(CPack)
install(TARGETS nesturbator_libretro LIBRARY DESTINATION cores COMPONENT libretro)  # MODULE → LIBRARY
install(FILES libretro/nesturbator_libretro.info DESTINATION info COMPONENT libretro)
install(FILES LICENSE DESTINATION . COMPONENT libretro)   # repeat per component
```
`CPACK_ARCHIVE_<COMPONENT>_FILE_NAME` takes the component name in upper case and no extension [CITED: cpack-generators(7), CMake 4.4.3: ".. variable:: CPACK_ARCHIVE_<component>_FILE_NAME … Note that ``<component>`` is all uppercase in the variable name."]. For `NESTURBATOR_OS`, map `CMAKE_SYSTEM_NAME` Linux/Darwin/Windows → `linux/macos/windows`. For `NESTURBATOR_ARCH`, map `CMAKE_SYSTEM_PROCESSOR` `x86_64|AMD64` → `x64` and `arm64|aarch64|ARM64` → `arm64`. With MSVC, prefer `CMAKE_C_COMPILER_ARCHITECTURE_ID` [ASSUMED]. Ship `THIRD-PARTY-NOTICES.md` in the libretro archive too, since the module is compiled against MIT `libretro.h`.

### FRAME-02 consumer via an install fixture
```cmake
add_test(NAME install.stage COMMAND ${CMAKE_COMMAND} --install ${PROJECT_BINARY_DIR}
         --component library --prefix ${PROJECT_BINARY_DIR}/stage)
set_tests_properties(install.stage PROPERTIES FIXTURES_SETUP staged)
add_test(NAME install.consumer COMMAND ${CMAKE_CTEST_COMMAND} --build-and-test
         ${PROJECT_SOURCE_DIR}/tests/consumer ${PROJECT_BINARY_DIR}/consumer
         --build-generator ${CMAKE_GENERATOR}
         --build-options -DCMAKE_PREFIX_PATH=${PROJECT_BINARY_DIR}/stage
                         -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
         --test-command consumer)
set_tests_properties(install.consumer PROPERTIES FIXTURES_REQUIRED staged)
```

### MSVC developer environment without a third-party action
```yaml
- name: Load MSVC environment
  if: runner.os == 'Windows'
  shell: pwsh
  run: |
    $vs = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -property installationPath
    $arch = if ($env:RUNNER_ARCH -eq 'ARM64') { 'arm64' } else { 'x64' }
    cmd /c "`"$vs\VC\Auxiliary\Build\vcvarsall.bat`" $arch && set" |
      ForEach-Object { if ($_ -match '^([^=]+)=(.*)$') { "$($Matches[1])=$($Matches[2])" >> $env:GITHUB_ENV } }
```
[ASSUMED: `vcvarsall.bat arm64` on an ARM64 host selects native tools; confirm on the first `windows-11-arm` run.] VS 2026 is at `...\Microsoft Visual Studio\18\Enterprise` on `windows-2025` [VERIFIED: image readme]. `vswhere -latest` finds either version.

### BMP reading for compare_frame.c (measured `sips` output, macOS 26.6.2)
| PNG input | BMP header | Height | bpp | Compression | Pixel bytes |
|---|---|---|---|---|---|
| RGB (colour type 2) | 40-byte BITMAPINFOHEADER, data at offset 54 | **−240 (top-down)** | 24 | 0 (BI_RGB) | B,G,R |
| RGBA (colour type 6) | **124-byte V5 header**, data at offset 138 | −240 (top-down) | 32 | 3 (BI_BITFIELDS), masks R `0xff0000` G `0xff00` B `0xff` A `0xff000000` at file offset 54 | B,G,R,A |

So the comparator must accept header sizes 40 and 124 (and 108). It must handle negative height (top-down) and positive height (bottom-up), and read masks from offset 54 when compression is 3. Row stride is `((w*bpp+31)/32)*4`. Bounds-check the data offset + stride × |h| against the file size.

### SHA-256 known-answer vectors (FIPS 180-4 examples)
```
""        e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
"abc"     ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
          248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1
```
[VERIFIED: `shasum -a 256` this session]

### Independent expected values for the test card
- A Python model of D-02 (separate from the C code to be written) gives **SHA-256 of the 256×240 LE uint16 frame = `b49e9be44573a4de82d845179d4389a0a6e28e934bd9ab31516a40db2c0b0453`** and confirms all 512 native values appear. Record it in the runner test as the FRAME-03 expectation. If the C implementation disagrees, one of the two misreads D-02.
- D-05 pixel candidates from the palette prototype: `$30` → `FFFFFF`, `$16` (band 0) → `C23400`, `$56` (`$16` in band 1, red emphasis) → `C52700`, `$0D` → `000000`, `$00` → `626262`, `$2A` → `36F632`. These are provisional until palgen runs. Take the test's hard-coded values from the checked-in table, reviewed against these.

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| MSVC `/fp:precise` contracts to FMA | No contractions under `/fp:precise`; opt in with `/fp:contract` | VS 2022 | palgen is deterministic on MSVC without extra flags [CITED: learn.microsoft.com /fp] |
| CPack archive file name only per component | `CPACK_ARCHIVE_FILE_NAME` also for non-component packages | CMake 4.0 | Irrelevant here (component mode); `CPACK_ARCHIVE_<COMP>_FILE_NAME` exists since 3.9 |
| `windows-2025` with VS 2022 | VS 2026 18.10 image; `windows-11-arm` still VS 2022 17.14 | 2026 | Never name a VS generator in presets; use Ninja + vcvars [VERIFIED: readmes] |

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | GCC ignores `#pragma STDC FP_CONTRACT` | Pitfall 3 | Low: the flag is used anyway |
| A2 | Ubuntu GCC enables `_FORTIFY_SOURCE` and stack protector by default, so ELF shows `__memcpy_chk`, `__stack_chk_fail`, `__stack_chk_guard` | Pitfall 4 | Low: the first `nofp` run shows the exact set; allowlist adjusts |
| A3 | C++ brace narrowing error and `-Wmissing-field-initializers` on partial positional init | Pitfall 6 | Low: the header tests catch it |
| A4 | MSVC C4996 on `fopen`; C5105 with `<windows.h>` | Pitfall 7 | Low: first `ci-msvc` run |
| A5 | GCC `-Wpedantic` warns on `dlsym` cast | Pitfall 8 | None: `memcpy` works either way |
| A6 | Windows text-mode stdout writes CRLF into captured hashes | Pitfall 9 | Medium: `hash-equality` fails until handled |
| A7 | Unknown RetroArch config keys are ignored; `menu_show_start_screen` may be the current key | Pitfall 10 | Low |
| A8 | `video_gpu_screenshot = false` + `--max-frames-ss` writes the raw 256×240 core frame, with no content, under the Metal driver | D-18/D-19 | Medium: the driver checks 256×240 and fails loudly; first local run with RetroArch decides |
| A9 | ASan dylib inside RetroArch aborts | Pitfall 11 | Low: exclusion is cheap anyway |
| A10 | Ninja may pick MinGW GCC on Windows without `CMAKE_C_COMPILER=cl` | Pitfall 13 | Low: preset sets it |
| A11 | release-please's generic updater replaces the version on a Markdown line ending with `<!-- x-release-please-version -->` | Pitfall 14 | Low: the release PR diff shows it |
| A12 | `gh pr merge --auto` needs a required-check rule to wait | Pitfall 15 | Medium: release PR merges before CI, or auto-merge errors |
| A13 | `vcvarsall.bat arm64` on an ARM64 host gives native tools; `CMAKE_C_COMPILER_ARCHITECTURE_ID` is set for MSVC | Code Examples | Low |
| A14 | `ubuntu-24.04-arm` has Ninja and CMake ≥ 3.25: its partner readme lists "CMake" with no version and does not list Ninja. That is silence, not evidence of absence | Environment | Low: add `command -v ninja \|\| sudo apt-get install -y ninja-build` |

## Open Questions (RESOLVED)

All six questions are settled by the phase plans; each answer is recorded below its question.

1. **Ticks per frame: 714732 or 714736?** — RESOLVED: 714732 (plan 01: `NESTURBATOR_TICKS_PER_FRAME 714732u`; the D-04 and D-09 test values in plans 01 and 02 use it).
   - What we know: rendering-off hardware frames are 89342 dots (714736 ticks), which is 29531250/491381 Hz [CITED: NES-HARDWARE-PPU-CARTRIDGE §4]. D-08 reports 39375000/655171, which is exactly 714732 ticks per frame.
   - Recommendation: 714732. It is the rate the core reports and the rate RetroArch syncs to. With no PPU in Phase 1 there is no rendering state to model; Phase 3's PPU replaces it. If the planner disagrees, the audio period and the D-04 test values change accordingly.
2. **`frame_number` after the first `run_frame`:** recommend 1, so `--hash-frame 1` means "after the first call" and N counts completed frames. — RESOLVED: 1 (plan 01, `run_frame` step "frame_number += 1 (equals 1 after the first call)"; tested in plan 02 `core.frame`).
3. **clang-format version.** Not installed locally. ENGINEERING pins version 18 from the Ubuntu image, while Homebrew installs a newer major whose output can differ. — RESOLVED: clang-format 18 is required and checked at configure; Homebrew `llvm@18` locally, CI authoritative (plan 08, objective decision and Task 2).
   - Recommendation: the `hygiene` preset requires clang-format 18 specifically: check `--version` and fail configure otherwise. The owner runs `hygiene` locally only if they install 18; CI is authoritative. The alternative is to defer `.clang-format` to Phase 2.
4. **GitHub repository does not exist yet (no remote).** — RESOLVED: the owner-only steps are the phase's single owner checkpoint (plan 11 Task 1, after every local plan, which also asks the owner to install RetroArch); push, settings, immutable releases, private vulnerability reporting and the ruleset are automated in plan 12 Task 1. FRAME-07 and the CI half of FRAME-01 need the owner to:
   - create the public repo;
   - push `main` and the phase branch;
   - create the App (or fine-grained token) and store its secrets;
   - enable auto-merge, squash-only and `PR_TITLE` squash titles, private vulnerability reporting and immutable releases;
   - apply `.github/rulesets/main.json`.

   The planner should put these in one owner checkpoint before the CI/release plan, matching GSD-HANDOFF §5.
5. **Account handle in README/URLs.** The install URL necessarily contains the GitHub owner's handle. `hygiene.sh` does not check handles; the rule forbids "names". Confirm with the owner that the handle is acceptable. A GitHub organisation is the alternative. — RESOLVED: the owner confirms the handle (or names an organisation) in the plan 11 Task 1 checkpoint; plan 11 Task 3 writes it into the README.
6. **Rulesets drift workflow (scheduled compare).** ENGINEERING §6 lists it, but no FRAME requirement needs it. Recommendation: ship `main.json` (needed for Pitfall 15) and defer the scheduled comparison unless cheap. — RESOLVED: `main.json` ships in plan 10 and is applied in plan 12; the scheduled drift workflow is deferred (plan 10 objective decision).

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| CMake | all | ✓ | 4.4.3 | — |
| Ninja | all presets | ✓ | 1.13.2 | — |
| Apple clang | dev/ci/asan/nofp locally | ✓ | 21.0.0 | — |
| GCC (real) | `-Wduplicated-cond -Wlogical-op`, nofp on Linux | ✗ locally (`gcc` is Apple clang) | — | CI only; add the GCC-only flags behind `$<C_COMPILER_ID:GNU>` |
| `nm` | undefined-symbol test | ✓ | Xcode | — |
| `sips` | retroarch test | ✓ | macOS 26.6.2 | — |
| RetroArch (DMG / `retroarch-metal`) | FRAME-05 | ✗ | — | Test reports skipped (77). Owner installs before verifying FRAME-05 |
| clang-format 18 | hygiene preset | ✗ | — | CI `ubuntu-24.04` has 18.1.3 [VERIFIED: readme] |
| `gh` (authenticated) | workflow and release checks | ✓ | logged in; scopes `repo`, `workflow` | — |
| git remote / GitHub repo | FRAME-07, CI | ✗ | — | none; owner task (Open Question 4) |
| Python 3 | not required | ✓ | 3.14.4 | used only for this research's prototypes |

CI images [VERIFIED: actions/runner-images and partner-runner-images readmes, fetched this session]:
- `ubuntu-24.04`: CMake 3.31.6, Ninja 1.13.2, Clang 18.1.3, clang-format 18.1.3, GCC 14.2.0 as `gcc-14`.
- `macos-15` / arm64: CMake 4.4.2 / 4.4.3, Ninja 1.13.2, default Xcode 16.4 (Apple clang 17).
- `windows-2025`: VS 2026 18.10, CMake 4.4.3, Ninja 1.13.2.
- `windows-11-arm`: VS 2022 17.14, CMake 4.2.1, Ninja 1.13.2.
- `ubuntu-24.04-arm`: readme lists "CMake", "Gcc (12, 13, 14)", "Clang 16, 17, 18", no versions for CMake and no Ninja entry (see A14).

**Missing dependencies with no fallback:** GitHub repository and release credential. They block FRAME-07 verification only.
**Missing dependencies with fallback:** RetroArch (test skips), clang-format 18 (CI runs it), GCC (CI runs it).

## Validation Architecture

Skipped: `workflow.nyquist_validation` is `false` in `.planning/config.json`. The phase's test map is D-11 plus the per-requirement tests named in Phase Requirements. Gate command is `cmake --workflow --preset ci` (config `workflow.test_command`).

## Security Domain

Omitted as an ASVS section: `workflow.security_enforcement` is `false`. Relevant hardening already in scope:
- size-tag validation before reading fields (Pattern 1);
- BMP/PPM parsing bounds checks (D-17);
- SHA-pinned actions;
- App token scoped to release-please and gone at job end;
- `pull_request_target` banned;
- attestation with a tampered-copy negative check.

## Sources

### Primary (HIGH confidence)
- NESdev Wiki "NTSC video" raw wikitext, rev 24244: levels, phase table, emphasis table, decode, normalisation, licence note on example code
- NESdev Wiki MediaWiki API: revisions for NTSC video 24244, PPU palettes 24257, Colour emphasis 23220
- RetroArch v1.22.2 (`69a4f0ea…`) `libretro-common/include/libretro.h`: fetched, hashed, lines 33-97, 865, 1051 read
- RetroArch v1.22.2 `retroarch.cfg`: configuration key names only (no source code opened)
- Local CMake 4.4.3 manuals: `cpack-generators(7)` Archive section, `ctest(1)` Build and Test Mode, `CPACK_INCLUDE_TOPLEVEL_DIRECTORY`
- actions/runner-images and actions/partner-runner-images readmes (main, fetched 2026-10-02)
- GitHub API: release tags and commit SHAs for six actions; release-please-action v5.0.0 `action.yml` and README outputs; actions/attest v4.2.2 `action.yml` and README permissions
- googleapis/release-please `docs/customizing.md`: generic updater markers
- learn.microsoft.com `/fp` reference: contraction defaults since VS 2022
- Local measurements (macOS 26.6.2, Apple clang 21, CMake 4.4.3): `sips` BMP layouts; MODULE `.so` suffix; ad-hoc signature; `fmadd` contraction; `nm -u` symbol set; libretro.h compiling clean under the full warning set; palette prototype invariants; test-card hash

### Secondary (MEDIUM confidence)
- Preparation files (ENGINEERING, LIBRETRO-AND-RUNNER, ARCHITECTURE, DECISIONS, NES-HARDWARE-PPU-CARTRIDGE §4) and their own source tables

### Tertiary (LOW confidence)
- Assumptions A1–A14 above (training knowledge, not run here)

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH. Versions, SHAs and libretro.h hash were resolved by tool this session.
- Architecture: HIGH. It follows locked decisions; the arithmetic is derived and checked.
- Palette: HIGH. Wiki facts were read from the raw source, and a prototype confirmed the invariants (and disproved one).
- RetroArch runtime behaviour: MEDIUM. Config keys were verified, but there is no RetroArch to run here.
- CI/release: MEDIUM. Inputs and outputs were read from the actions. Auto-merge/ruleset interplay and the first release remain open in ENGINEERING too.

**Research date:** 2026-10-02
**Valid until:** 2026-11-01 (action SHAs and runner images move monthly; the rest is stable)
