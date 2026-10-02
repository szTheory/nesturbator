# Phase 1: A test frame in RetroArch - Context

**Gathered:** 2026-10-02
**Status:** Ready for planning

<domain>
## Phase Boundary

The library, the headless runner and the libretro core build, test and release on six platforms (Linux, macOS, Windows; x64 and arm64). With no cartridge loaded the core outputs a built-in test frame; the runner dumps and hashes it, the libretro test program receives an equal frame, and RetroArch on an Apple Silicon Mac shows it and its screenshot matches. Presets `ci`/`ci-msvc`/`asan`/`nofp`/`hygiene`, GitHub Actions CI and the release-please pipeline land here. No emulation (CPU, PPU, APU, ROM loading) in this phase. Requirements: FRAME-01 to FRAME-07.

</domain>

<decisions>
## Implementation Decisions

Everything in `.planning/preparation/DECISIONS.md` (DEC.01–DEC.39) is settled and not repeated here. The decisions below fill the gaps that remained for Phase 1.

### Test frame content
- **D-01:** The no-cartridge frame is a **static test card computed from coordinates**: a pure integer function `test_pixel(x, y)` returning a native 16-bit pixel (`entry | emphasis << 6`), called for every pixel on every `run_frame` while no cartridge is loaded. No table, no font, no per-instance state for the pattern. Frame N equals frame 1.
- **D-02:** Layout (0-based rows/columns; first matching rule wins):
  1. Corner block: rows 0–7, columns 0–7 → `$30`, emphasis 0. Only at top-left, so any mirror shows.
  2. Border: row 0, row 239, column 0, column 255 → `$30`.
  3. Top overscan band rows 1–7 → `$16` (red); bottom overscan band rows 232–238 → `$12` (blue). Different colours so a vertical flip shows.
  4. Chart: rows 8–231 in 8 bands of 28 rows. Band `b = (y-8)/28` is the emphasis value 0–7; inside a band `r = ((y-8)%28)/7` is the high nibble 0–3; column `c = x/16` is the low nibble. Pixel `= (r<<4 | c) | b<<6`. Every one of the 512 native values appears in its own 16×7 cell.
- **D-03:** No version, ABI or text in the pixels. Version information is reported by the version call, `retro_get_system_info`/`.info` and the runner; putting it in pixels would change the recorded hash on every release.
- **D-04:** Frame advance is shown by frame metadata, not pixels: each `run_frame` advances `frame_number` by 1 and `ticks` by one NTSC frame. A runner test checks that `--hash-frame 3` equals `--hash-frame 1` and that metadata advanced.
- **D-05:** The libretro test program also checks about four pixels against XRGB8888 values hard-coded in the test (for example (0,0) = white `$30`; one band-0 cell; the same cell in band 1; a `$0D` cell). This catches an indexing bug in the shared palette path that would otherwise cancel out in the runner-vs-adapter comparison.

### Public header scope (Phase 1)
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

### Palette table source
- **D-12:** The 512-entry native→XRGB8888 table is produced by **our own offline C generator** `tools/palgen/palgen.c`, built by CMake but not linked into the core, and the output `src/…/palette_ntsc.c` (`const uint32_t[512]`, integers only) is checked in with a "generated by tools/palgen — do not edit" header naming its parameters and wiki revisions. This refines DEC.13 / NES-HARDWARE-PPU-CARTRIDGE §4 ("from pally or a host file"): pally is not vendored; the same pull request updates that preparation file. — **Reversibility:** reversible — swapping the table changes no hash.
- **D-13:** Generator inputs are cited NESdev Wiki facts only (NTSC video, PPU palettes, Colour emphasis): the signal levels for `$0D`–`$20`/`$30`, 12-phase square waves with colour `$xY` on wave Y, `$xE/$xF` at the black level, emphasis attenuating the voltage on active phases (native bit 6 = PPUMASK bit 5 → wave C, native bit 7 = PPUMASK bit 6 → wave 4, native bit 8 = PPUMASK bit 7 → wave 8, `$xE/$xF` excepted; verify against the wiki revision cited), and the wiki's YUV→RGB matrix. A plain direct R'G'B' decode — no colorimetry (`pow`, `cos`) in Phase 1. The generator uses only `+ − × ÷` and `sqrt`, built with floating-point contraction off, so its output is bit-identical on every runner.
- **D-14:** One `ci` test regenerates the table and byte-compares it with the checked-in file (`cmake -E compare_files`) on all six runners. Invariant tests: `$20` equals `$30` and is white; `$xE/$xF` are black and unaffected by emphasis; emphasis 0 is the base palette; an emphasis bit never raises a channel other than its own (red emphasis may raise R, blue may raise B, green raises none; corrected during planning — the original wording fails on 64 entries, see 01-RESEARCH Pitfall 2); integer luma is monotonic down each column.
- **D-15:** The runner and the libretro adapter both get colours from `nesturbator_get_palette` and convert with the same loop, so the RetroArch comparison is like for like (D-05 guards the shared path).

### Frame image and RetroArch comparison
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

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Settled choices
- `.planning/preparation/DECISIONS.md` — DEC.01–DEC.39; read before reopening anything
- `.planning/preparation/README.md` — index of the preparation files and where each question is answered

### Build, tests, CI, release
- `.planning/preparation/ENGINEERING.md` — C rules and warnings (§1), presets (§2), tests and labels (§3), CI jobs and runners (§5), release-please config, token, flow and artifacts (§6), hygiene and legal files (§7)

### Public API, adapter, runner
- `.planning/preparation/ARCHITECTURE.md` §7 (public API principles), §8 (adapters), §4 (video output format)
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` §1–2 (libretro surface, no-game, `.info` keys), §3 (RetroArch on Apple Silicon, install recipe, automation flags), §4 (what the adapter needs), §5 (runner process contract), §6 (release artifact names)

### Video and palette
- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md` §4 — native pixel format and emphasis; update its palette line per D-12
- NESdev Wiki pages "NTSC video", "PPU palettes", "Colour emphasis" (https://www.nesdev.org/wiki/) — generator inputs; cite the revisions used in the generated file header

### Project rules
- `CLAUDE.md`, `AGENTS.md`, `PROVENANCE.md`, `ASSET_POLICY.md` — clean room, owned code, hygiene
- `scripts/hygiene.sh` — rejects home paths, emails, ROM names and magic; tracked binaries need `tests/roms/manifest.txt`

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `scripts/hygiene.sh` and `.githooks/pre-commit`, `.githooks/pre-push`: the forbidden-content scan already exists; Phase 1 adds the `hygiene` preset/CTest wrapper and the CI job around it (`--tree` mode).
- `LICENSE`, `README.md`, `PROVENANCE.md`, `ASSET_POLICY.md`, `AGENTS.md`, `.gitignore` already exist; `SECURITY.md`, `CONTRIBUTING.md`, `THIRD-PARTY-NOTICES.md` arrive this phase (ENGINEERING §7).

### Established Patterns
- No C code, CMake files or workflows exist yet; this phase establishes them. Conventions come from ENGINEERING.md and the decisions above.

### Integration Points
- `.planning/config.json` sets `workflow.test_command` to `cmake --workflow --preset ci`, which fails until this phase creates the preset.
- `libretro.h` is vendored at the RetroArch v1.22.2 pin given in LIBRETRO-AND-RUNNER Sources (LR.01), listed in `THIRD-PARTY-NOTICES.md` and `PROVENANCE.md` with upstream, commit and SHA-256.

</code_context>

<specifics>
## Specific Ideas

- The owner's preference applies throughout: "another copy and paste is better than another dep" — prefer a few owned lines or a platform tool (`sips`, `cmake -E`) over a library; keep the dependency tree flat.
- Pally (Gumball2415/pally, MIT-0) may be used in Phase 3 as an offline colour cross-check (delta-E report), with nothing committed.
- The broadcast test-card idea (SMPTE bars, PM5544) is borrowed only where it costs nothing: a full colour chart plus border, corner and overscan markers.

</specifics>

<deferred>
## Deferred Ideas

- Colorimetry in the palette generator (SMPTE-C→sRGB, CRT gamma, hue tweak) — Phase 3, when real games make colour quality matter; keep the regeneration check exact.
- Loading a host `.pal` file (192 or 1536 bytes) as a display-only libretro core option — Phase 3.
- Calibrating RetroArch's frame index against the runner's (`--max-frames` off-by-one) with a moving NROM frame — Phase 3; the static card cannot reveal it.
- Running the RetroArch test off macOS (would need ImageMagick or a decoder) — not planned.

</deferred>

---

*Phase: 01-a-test-frame-in-retroarch*
*Context gathered: 2026-10-02*
