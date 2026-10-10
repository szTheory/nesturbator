---
phase: 06-mapper-seam-and-ppu-fetch-pipeline
plan: 02
subsystem: ppu-fetch-pipeline
tags: [ppu, fetch, scroll, a12, mapper, map-02, behaviour-revision-5]
requires:
  - phase: 06-mapper-seam-and-ppu-fetch-pipeline
    provides: "plan 01: page tables, nametable map, ppu_a12 hook, test board (MAP-01)"
provides:
  - "src/ppu.c: v-driven two-dot background fetches, shifters, increments and copies, per-slot sprite fetches, one set_bus driver reporting A12 edges"
  - "tests/ppu/ppu_fixture.h, ppu.fetch, ppu.split_scroll"
  - "NESTURBATOR_BEHAVIOUR_REVISION 5 and eight re-pinned frame hashes"
affects: [phase-07, phase-08, phase-09, phase-10]
actuals:
  tokens: 13944
  tasks: 3
  commits: 4
tech-stack:
  added: []
  patterns:
    - "one switch on (dot - 1) & 7 for the background tile fetch and one on the slot phase for sprites; no function-pointer table"
    - "address dot drives the bus and ALE; the read dot rebuilds bits 8-13 from live state with the same per-kind helper"
    - "tests observe by single-stepping nesturbator__ppu_run_until(ppu_ticks + 8); no production trace hook"
key-files:
  created:
    - tests/ppu/ppu_fixture.h
    - tests/ppu/test_fetch.c
    - tests/ppu/test_split_scroll.c
  modified:
    - src/ppu.c
    - src/internal.h
    - tests/ppu/test_render.c
    - tests/CMakeLists.txt
    - include/nesturbator.h
    - tests/core/test_api.c
    - tests/cmake/vector_api_policy.cmake
    - tests/runner/hashes.txt
    - README.md
key-decisions:
  - "Low pattern plane shifts in 0, high plane shifts in 1 (NESdev PPU rendering), correcting D-10 for the low plane"
  - "Dot 0 of a rendering line drives the BG-lo address of the pending tile; odd-frame skip keeps the dot-339 NT address on the bus (D-10 choice kept)"
  - "$2007 while rendering applies coarse X and Y increments together, in the pipeline commit"
  - "Empty sprite slots fetch tile $FF with the slot's own row masked to the sprite height; they load transparent zeros"
  - "Sprite slot arrays are filled on dots 257-320 (lo/hi at the read dots, x/attr/zero at the hi read)"
requirements-completed: [MAP-02]
duration: about 1 h 30 min
completed: 2026-10-10
status: complete
commits: 4
plan_head_before: f1b3309821dc2746e85b3701da542730c3a25780
plan_head_after: 62fb2bb552886ae7d7849dda3e216f09f2315d6e
coverage:
  - id: D1
    description: "Background fetched from v on the documented dots with shifters, increments, the 257 copy and the 280-304 copies"
    requirement: MAP-02
    verification:
      - kind: unit
        ref: "ctest ppu.fetch (test_fetch.c: coarse X, Y at 256, copies, addresses, shifter polarity)"
        status: pass
    human_judgment: false
  - id: D2
    description: "Every PPU address goes through one bus driver that reports both A12 edges with the dot's tick; empty sprite slots and slot order show the MMC3 pattern"
    requirement: MAP-02
    verification:
      - kind: unit
        ref: "ctest ppu.fetch (A12 at BG $1000, empty slots 8x8/8x16, slot order, rendering off, post-render lines, odd frame)"
        status: pass
    human_judgment: false
  - id: D3
    description: "Mid-frame $2005/$2006 splits render as hand-derived scanline by scanline"
    requirement: MAP-02
    verification:
      - kind: unit
        ref: "ctest ppu.split_scroll (cases A-D)"
        status: pass
    human_judgment: false
  - id: D4
    description: "Behaviour revision 5 bumped once; eight hashes re-pinned with reasons; six-platform agreement; scoreboard loses no row"
    requirement: MAP-02
    verification:
      - kind: other
        ref: "CI run 38072045594: hash-equality, six build legs (accuracy.scoreboard against protected main), asan, nofp, CI required all success"
        status: pass
    human_judgment: false
---

# Phase 6 Plan 02: PPU fetch pipeline Summary

The PPU now renders the background from `v` through two-dot NT/AT/pattern fetches with shifters, takes scroll writes at the 257 and 280-304 copies, fetches sprite patterns per slot (empty slots fetch tile `$FF`), and drives a literal address bus whose A12 edges reach the board, with the behaviour revision bumped to 5 once and eight frame hashes re-pinned with reasons.

## Performance

- **Duration:** about 1 h 30 min
- **Tasks:** 3 of 3, plus one CI fix commit
- **Files:** 3 created, 9 modified

## Accomplishments

- `src/ppu.c`: `set_bus` (the only writer of `ppu.bus_addr`; both A12 edges to `map.ops.ppu_a12` with `ppu.ppu_ticks`, no filter), `rendering_active`, `inc_coarse_x`, `inc_y`, `copy_horizontal`, `copy_vertical`, per-kind address helpers, `address_dot`/`read_dot`, `shift_background`, `reload_shifters`, `background_access` (one switch on `(dot - 1) & 7`), `sprite_access` (per slot), `fetch_step`. Retired: the per-pixel background function that read `t` and the dot-257 batch sprite loader.
- `$2006` second writes and `$2007` accesses drive the bus at once when rendering is off; `$2007` while rendering applies coarse X and Y increments together.
- `ppu.fetch` (18 cases) asserts each D-13 dot case from the NESdev formulas, including the MAP-02 edges: empty-slot A12 at 261, 269, ..., 317 and falls 265, ..., 321; slot order at dots 261 and 269; `$2006` adjacency; post-render and mid-line-off bus; the odd-frame edge log.
- `ppu.split_scroll` compares four hand-derived split cases (six runs) scanline by scanline after proving no other origin gives the same row. Tables were derived from the NESdev register rules before the test was run; it passed on the first run, and a deliberate vertical-copy mutation made it fail with a decoded origin message.
- `ppu.render` starts on the pre-render line and keeps both pixel assertions.
- Revision 5, `PHASE6_DECLARATIONS_SHA256`, README sentences with each behaviour.

## Re-pin table (`git diff -U0 main -- tests/runner/hashes.txt`)

Revision commit: `9389198` (`feat(06-02): behaviour revision 5 with the PPU fetch pipeline`). No `sha256 <64 hex>` line was added or removed in the `tests/CMakeLists.txt` diff. `git diff main -- tests/accuracy/scoreboard.txt` is empty. All eight changed rows are in this table; **ticks are unchanged on every row**, frame 1 and all DABG rows are unchanged, and the six audio rows are unchanged.

| Row | Old hash | New hash | Reason (frames compared with `--dump-frame` before and after) |
|---|---|---|---|
| nesteroids/boot/frame 30 | 98adaa55... | 11fed47a... | Nesteroids writes `$2005` about 300 times per frame while rendering, from about line 4, and `$2007` while rendering. Scroll now takes effect at the next copy and `$2007` increments coarse X and Y. 2,352 pixels on 65 rows (0-102) differ |
| nesteroids/boot/frame 60 | 3506f2ae... | 80410782... | Same cause; 2,342 pixels on rows 0-102 differ |
| nesteroids/boot/frame 120 | 3506f2ae... | 2cf9248a... | Same cause; 2,342 pixels on rows 0-102 differ |
| nesteroids/boot/frame 180 | 3506f2ae... | 2cf9248a... | Same cause; 2,342 pixels on rows 0-102 differ |
| rhde/boot/frame 30 | 73455332... | cac57d02... | RHDE writes `$2000` (`0x89`, `0x99`) mid-line 189, toggling the background pattern table; fetches after the write now use the new table. Only row 189, 50 pixels from column 206, differs |
| rhde/boot/frame 60 | bf2f63aa... | cac57d02... | Same cause; row 189, 57 pixels from column 197 |
| rhde/boot/frame 120 | bf2f63aa... | cac57d02... | Same cause; row 189, 57 pixels |
| rhde/boot/frame 180 | bf2f63aa... | cac57d02... | Same cause; row 189, 57 pixels |

This is exactly the set research predicted (A5). Verification command `git log main..HEAD --format=%h -S'NESTURBATOR_BEHAVIOUR_REVISION 5'` also lists the two planning commits `50ce09b` and `8479617`, which quote the string in `.planning/` documents; restricted to `include src tests README.md` it names exactly one commit, `9389198`.

## CI and gates

- Local: `cmake --workflow --preset ci` 361/361, `asan` 361/361, `hygiene` pass, `nofp` see Deviations.
- First push (`9389198`, run 38071786083) failed on the Linux `build` and `nofp` legs (see Deviations). Fix `62fb2bb`.
- Green CI run **38072045594** for HEAD `62fb2bb`: `hash-equality`, all six `build` legs (which run `accuracy.scoreboard` against protected `main`), both `nofp` legs, `asan`, `hygiene`, `retroarch-e2e`, `title` and `CI required` concluded `success`. The scoreboard lost no passing row.

## Performance

`build/ci/runner/nesturbator-run --rom tests/roms/nesteroids.nes --frames 1800`, three runs each on the owner's Mac (macOS arm64):

| Build | Runs (s) | Median | Frames per second |
|---|---|---|---|
| Plan 01 baseline (recorded in 06-01-SUMMARY) | 1.60, 1.69, 1.67 | 1.67 s | about 1078 |
| Seam commit rebuilt now (same session) | 1.54, 1.47, 1.49 | 1.49 s | about 1208 |
| This plan | 2.13, 2.06, 2.04 | 2.06 s | about 874 |

Against the recorded baseline frame time rises 23 percent (frames per second fall 19 percent); against the same-session seam build it rises 38 percent. The cost is the per-dot fetch work, not the A12 hook (a bit-12 change with the watch bit clear costs one compare), so the plan's switch-dispatch fallback does not apply. Measured, not gated; the research prototype predicted about 35 percent.

## Phase 9 inputs

- With PPUCTRL bit 4 set (background at `$1000`), an even frame logs an A12 rise at line 0 dot 0 (the dot-0 low-pattern address of the pending tile); an odd frame does not, because the skip leaves the dot-339 nametable address on the bus. Its first rise on line 0 is at dot 5. The MMC3 filter therefore sees one fewer rise per odd frame with BG at `$1000`. `ppu.fetch` asserts both logs. Recorded assumption: the NESdev wiki describes the last dummy fetch replacing the idle tick, which would add an NT read a mapper could see; that read is A12 low and hash-neutral.
- `ppu_a12` ticks are `ppu.ppu_ticks` at the end of the dot (A6). The dots in code are 1-based; the MMC3 page's 260 and 324 are dots 261 and 325 here.
- A `$2006` second write while rendering is off drives the bus (and A12) at once.

## Deferred

- OAMADDR forced to 0 during dots 257-320 of the pre-render and visible lines (NESdev "PPU registers", OAMADDR). It moves no fetch address and no A12 edge, so it is not part of MAP-02; to be added in a later PPU accuracy phase when an AccuracyCoin OAM test is traced to it.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] GCC `-Wconversion` error in the test fixture**
- **Found during:** Task 3 (first CI run, Linux `build` and `nofp` legs)
- **Issue:** `spec.chr_8k = chr_ram != 0u ? 0u : 1u;` converts `unsigned` to `uint8_t`; Apple clang accepted it, GCC 14 with `-Werror` did not.
- **Fix:** explicit `(uint8_t)` cast.
- **Files modified:** tests/ppu/ppu_fixture.h
- **Commit:** 62fb2bb

**2. [Rule 3 - Blocking] Acceptance greps conflict with naming**
- **Found during:** Task 1
- **Issue:** the acceptance criterion `grep -c background_pixel src/ppu.c` prints 0 and `grep -nE "switch \(\(.*dot"` must find the dispatch, but the first draft kept the old helper name and passed the phase as a parameter.
- **Fix:** the shifter-based colour helper is `background_color`; `background_access` has no phase parameter and switches on `(ppu->dot - 1u) & 7u`.
- **Commit:** b6a41b9

**3. [Plan check wording] Revision-commit `git log -S` also matches planning commits**
- The verify command `git log main..HEAD -S'NESTURBATOR_BEHAVIOUR_REVISION 5'` names three commits because `50ce09b` and `8479617` quote the string in `.planning/`. Scoped to the source paths it names exactly one, `9389198`. No code change.

### Test-design notes

- D-13 trace hook dropped as directed: tests single-step and read `ppu.bus_addr`, `v`, `scanline`, `dot` and the test board's edge log. No production tracing code.
- D-15 correction applied: the sampler is 512 x 480, and because vertical mirroring repeats nametables 2 and 3, the uniqueness check runs over the 512 x 240 content plane.
- Task 1 and 2 intermediate commits leave `runner.write_hashes.content` red until the Task 3 re-pin, by design (D-17). Nothing was pushed before Task 3.

### Known local-only failure (out of scope)

`abi.undefined_symbols` fails in the local macOS `nofp` run on `___stack_chk_fail` and `___stack_chk_guard`, as recorded in 06-01-SUMMARY. The Linux `nofp` legs pass in CI (the real gate).

**Total deviations:** 2 auto-fixed (Rule 3), 1 plan-check wording note. **Impact:** none on behaviour.

## Known Stubs

None.

## Threat Flags

None. T-06-04 (14-bit mask in `set_bus`, NULL CHR pages read 0) and T-06-05 (hook called only on a bit-12 change with the watch bit set) hold; `asan` ran `ppu.fetch` and `ppu.split_scroll`. T-06-06 is covered by the table above, the single revision commit and `hash-equality`.

## Self-Check: PASSED

- Created files exist: tests/ppu/ppu_fixture.h, tests/ppu/test_fetch.c, tests/ppu/test_split_scroll.c.
- Commits b6a41b9, 8d524b4, 9389198, 62fb2bb are ancestors of HEAD; CI run 38072045594 concluded success for 62fb2bb.
- `grep -c background_pixel src/ppu.c` 0; `grep -c sprite_fetch src/ppu.c` 0; one `bus_addr =` assignment, in `set_bus`; no file-scope function-pointer table; `grep -c split_scroll tests/runner/hashes.txt` 0.
