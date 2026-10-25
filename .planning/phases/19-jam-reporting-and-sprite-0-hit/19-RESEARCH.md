# Phase 19: JAM reporting and sprite-0 hit - Research

**Researched:** 2026-10-25
**Domain:** NES PPU sprite evaluation / OAMADDR model (C17 core), AccuracyCoin observe harness (runner), libretro adapter presentation and run-ahead test model
**Confidence:** HIGH (every core claim below was reproduced by a build and run in this session; hardware text is cited from NESdev)

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

The owner asked for every decision point to be fanned out to the point of diminishing returns, with one coherent recommendation per point that is then adopted without further questions. Four research passes covered: JAM attribution, run-ahead across a JAM, the sprite-0 probe, and the sprite-0 model and fix bounds. The probes ran in scratch copies of the source; no tracked file was changed.

#### Sprite-0 root cause (settled: replaces the roadmap's "not isolated")
- **D-01:** The basic sprite-0 hit already works. On a clean first pass of page 19, `Sprite 0 Hit behavior` writes `$01` at about frame 190, with every subtest from 1 to E passing; test E reads 0 at line 1 dot 248 and 1 at dot 261, and the hit sets at dot 255 (x=254). `Suddenly Resize Sprite` and `Misaligned OAM DMA` also write `$01`. `Arbitrary Sprite zero` writes `$0A` (error code 2: subtest 2, sprite-zero identity at a nonzero OAMADDR).
- **D-02:** The `0x06` results in the scoreboard come from re-runs. Subtest 1 of `Misaligned OAM behavior` fails on our core. Its top-level failure branches (asm lines 7440 and 7446) reach `FAIL_MisalignedOAM_Behavior`, whose `PLA PLA` (lines 7415-7418) drops RunTest's return address. `AutomaticallyRunEveryTestLoop` (lines 17988-18005) then re-runs rows 2-7 with X reset. On re-runs, OAMADDR is stale: `$2003=$81` stays set because `src/ppu.c` never resets it during rendering, every later DMA lands at OAM[$81], and evaluation always reads from OAM[0]. The "8000 frames ends in 0x06" measurement in research is the same artifact, and the `compose_pixel` first-opaque-slot hypothesis is not supported by any measurement. Subtest 1 of `Misaligned OAM behavior` runs the same code as Arbitrary subtest 3, so fixing SPR-02 also ends the guest loop.
- **D-03:** Record this correction in the phase research: `tests/accuracy/measurements.md` and the ROADMAP Phase 19 note ("cause is not isolated"), and mark research/SUMMARY.md Disagreement (a) as superseded, all in this phase's change.

#### Sprite-0 hardware model (SPR-01, SPR-02)
- **D-04:** OAMADDR is cleared to 0 on every dot 257-320 inside `sprite_access`. That path runs only while rendering is on (lines 0-239 and the pre-render line 261), so with rendering off OAMADDR is kept, and a `$2003` write at dots 258-320 is cleared again on the next dot. Source: NESdev "PPU registers", OAMADDR "Values during rendering". Do not model the 2C02G OAM corruption: it depends on chip revision and alignment, and the AccuracyCoin subtests are built to be immune to it.
- **D-05:** Sprite evaluation starts at OAMADDR. At dot 65, set `eval_n = oam_addr >> 2` and `eval_m = oam_addr & 3`, keeping the existing fields. Three small fixes are needed for misaligned starts:
  - (a) in range, advance `m = (m+1)&3`, with `n++` on wrap, instead of forcing `eval_m = 1`;
  - (b) index the secondary-OAM copy by copy position, `slot_base + (4 - eval_copy_remaining)`, not by `eval_m`;
  - (c) out of range with fewer than 8 found, set `m = 0` (`(addr+4)&$FC`).

  The overflow-bug path stays as it is. All three are no-ops for an aligned OAMADDR. Sources: NESdev "PPU registers" (OAMADDR at tick 65 sets the start and the sprite treated as sprite 0; a misaligned address is reinterpreted as Y; evaluation stops at the end of OAM) and the AccuracyCoin asm for `Misaligned OAM behavior` (single source, MIT, pinned).
- **D-06:** Sprite-zero identity is `eval_sprite_zero[slot] = (ppu->dot == 66u)`: the sprite found in range on the first evaluation read. It is neither `eval_n == 0` nor "the first sprite in range"; AccuracyCoin subtest 1's loop over sprites 1-63 rejects the latter. Source: AccuracyCoin asm, subtest 2 comments.
- **D-07:** The hit test stays in `compose_pixel`, gated on `video_output`. `nesturbator_run_frame` rejects a NULL video (`include/nesturbator.h` ~384-386), and `src/frame.c` sets the output pointer for the whole frame, libretro run-ahead included, so no NULL-video path exists. Add a comment saying the hit depends on that contract. Do not add a per-dot check off the video path: it would cost performance and guard a case that cannot happen.
- **D-08:** No dependency on Phase 20. Test E passes with about one CPU cycle of margin, and the `$2003` window in Arbitrary subtests 2 and 3 (line 0, dots 0-64, about 21 CPU cycles) does not depend on CPU/PPU alignment. If subtest 2 or 3 still fails after D-04 to D-06, probe the dot of the `$2003` write. If it lands after dot 64, treat it as a DMA or CPU cycle-count defect: stop and record a roadmap change, with no per-row workaround and no cap change.
- **D-09:** Out of scope even if nearby: `$2004` reads during rendering, 2C02G corruption, OAM attribute-bit masking, and the DMA rows.

#### Owned tests (ppu.sprites), written before the fix
- **D-10:** Each test fails on today's `main`, and its expected values come from NESdev text and AccuracyCoin error codes, labelled "(derived)", never from our code (PITFALLS #16). Cases:
  1. With rendering on, `$2003=$81`; after a line passes dot 320, OAMADDR = 0.
  2. With rendering off, OAMADDR survives a whole frame.
  3. A `$2003` write at dot 300 is cleared again.
  4. A `$2003` write in VBlank with rendering on: sprite zero on line 1 comes from OAM[0] after the pre-render reset.
  5. OAMADDR = `$80` at dot 65: OAM sprite 32 acts as sprite zero, and a copy at OAM[0] does not hit.
  6. Misaligned OAMADDR = `$81`: the drawn sprite's pattern bytes are correct (secondary-OAM byte order), not only the hit.
  7. A first sprite out of range followed by a later one in range: no hit.
  8. Sprite zero covered by a lower-slot opaque sprite still hits (pins the current behaviour).
  9. The hit lands on its dot (x=254 hits, x=255 does not), extending the existing `tests/ppu/test_sprites.c`.
- **D-11:** A page-19 check requires `Misaligned OAM behavior` to end with a terminal code (not `0x03` and not not-run), and the suite loop to finish, so rows cannot pass only on re-runs while the page later times out (adversarial case).

#### JAM attribution (JAM-01)
- **D-12:** The JAM run in CI is the real observe path on the pinned ROM with one in-memory byte patched. A new `jam` subcommand of `accuracycoin.conformance`:
  - parses the directory from the unpatched bytes, so the pin and SHA checks hold;
  - copies the bytes and writes `$02` at the file offset of a row's `routine_address` from the inventory the harness already parses. The site is chosen by rule ("last TEST-kind row of page 3"), never as a hard-coded offset. The offset uses a bounds-checked local NROM mapping, a justified duplicate because `rom_offset` is static;
  - asserts the original byte is not already `$02`;
  - loads the copy with `nesturbator_load_cartridge` and calls `accuracycoin_run_observe_page`.

  No ROM bytes are committed, no runner flag is added, and the `nesturbator-run` CLI keeps refusing a modified ROM.
- **D-13:** Contract:
  - `frame_and_sample` returns `nesturbator_status`. Each observe stage checks `NESTURBATOR_STOP_JAM` before its generic failure branch and calls one local helper that sets `stop_cause=guest-jam`, `verdict=guest-jam` and `jam_frame=<frames>`.
  - **Execution stage:** if automation is active, the cursor names a TEST row and that row's byte is `0x03`, the JAM belongs to that row (`active_key=accuracycoin/<name>`, `active_raw=0x03`); otherwise `-`. The report is still written, and the exit is `RUNNER_EXIT_DONE` (0), as for `guest-timeout`.
  - **Earlier stages:** a JAM is named, but no report is written and the exit stays 1.
  - **Row classification:** the `active_budget_expired` argument becomes three-state. Add `ACCURACYCOIN_ACTIVE_RUNNING 0`, `_BUDGET 1` and `_JAM 2`. Raw `0x03` with JAM gives status `fail`, cause `guest-jam`. A terminal byte is never overridden. The result enum and scoreboard format do not change.
  - **Metadata:** append `jam_frame=<N>|-` after `verdict=`. This is additive, so `report_schema` stays 1.
  - **Messages:** stdout `accuracycoin observe page P guest-jam (...)`; stderr `nesturbator-run: JAM stopped AccuracyCoin page %u at frame %llu (%s=%s)`. Strict page mode prints `nesturbator-run: JAM stopped AccuracyCoin page %u at frame %u` and exits 1; today it fails silently.
  - **Campaign collector:** accepts `guest-jam` in its verdict whitelist and checks consistency as for `guest-timeout`: verdict and cause appear together, `jam_frame` is numeric, and an attributed key has exactly one matching `fail/0x03/guest-jam` row.
- **D-14:** Tests:
  - A new CTest `accuracycoin.jam` covers the observe JAM on page 3 of the pinned ROM, plus strict mode on a patched page 2 of the old ROM, exiting 1 with the stderr line.
  - The "control that drops the attribution fails" extends the `accuracycoin.report` fixture mutations: `jam` must be accepted; `jam-cause-dropped`, which reverts to `execution-frame-failure`, must be rejected; so must `jam-row-dropped` and `jam-frame-missing`.
  - `accuracycoin.classifier` gains the JAM cases.
  - `accuracycoin.tracer`, running the unpatched pin, asserts `jam_frame=-` and no `guest-jam`.
- **D-15:** A suite-restart guard (classify a drop in `menuCursorYPos` during automation as `guest-suite-restart`) is not adopted. D-11 makes the fix end the loop, and a guard that freezes first-pass bytes could hide real re-run failures. It is recorded as a deferred idea.

#### Run-ahead across a JAM (JAM-02)
- **D-16:** Change to the adapter, required for green. Today `retro_unserialize` invalidates the cached presentation (libretro.c ~289-294, Phase 18 D-10). Under run-ahead every tick loads a state, so once the real timeline is past the JAM the picture is black instead of frozen. That is a visible regression and fails JAM-02. Fix:
  - tag the cached converted frame with the `frame_number` that filled it, and do not invalidate it on a successful unserialize;
  - on `STOP_JAM`, show the cache when its tag equals the instance's current `frame_number`, and black otherwise. `STOP_JAM` leaves `frame_number` unchanged, and a load restores it;
  - reset, load_game and unload still invalidate, and a refused load still preserves the cache;
  - the cache stays out of the native state.

  This narrows Phase 18 D-10, as JAM-02 requires. The one `check_jam_presentation` assertion changes in the same commit, along with the README and the adapter comment. — **Reversibility:** reversible — adapter-local presentation policy; no state or ABI change.
- **D-17:** The test extends `libretro.runahead` only: no new CTest name and no core-level mirror, since `tests/core/test_state.c` and the header already pin the JAM latch across save/load.
  - **ROM:** synthetic, built in `libretro_host.c` from `tests/ines.h`, NROM, ignoring input. Rendering is off and a pulse tone plays. Each VBlank writes `counter & $3F` (skipping `$0D`) to `$3F00`, so every frame before the JAM has a distinct backdrop. After 12 VBlanks it executes `$02`.
  - **JAM frame J:** read from the oracle (the first `STOP_JAM`), never pinned. Assert `RA_MAX_DEPTH+2 <= J <= T-RA_MAX_DEPTH-2` for both T=30 (`--short`, asan) and T=180.
  - **Depths:** mask `0xf` with script 1, plus two-instance repeat mode at depths 1-3. Coverage counters require each window class (no JAM in sight; JAM in the speculative frames; the boundary at t=J-1; a jammed state loaded) at every depth.
  - **Oracle:** `runahead_model.h` gains a JAM-aware frame record `{hash, frame_number, valid}`; existing callers keep the strict default.
  - **Assertions:**
    - real video, visible video, real and speculative audio, and the saved state all match the oracle at every tick;
    - the saved-state match is the latch check;
    - silence means zero audio frames delivered;
    - exactly T-J real ticks are silent;
    - every tick at or after J-N shows `o->video[J-1]`, not black;
    - the "load has work to do" check is skipped for t >= J, with exactly T-J skips.
  - **Controls:** the existing four (skip-load, stale-load, late-input, audio-leak) also run on the JAM image and must each trip. A new black-oracle control must mismatch.
  - **Accounting:** `RAH_MIN_ROMS` stays file-only; add a `jam_images == 1` counter and extend the tick bounds.

#### Versions, hashes and delivery
- **D-18:** `NESTURBATOR_BEHAVIOUR_REVISION` goes from 6 to 7 in this phase, because AccuracyCoin frames change for the same inputs. Phase 20 takes 8, which corrects research/SUMMARY.md's "6→7 in Phase 20". Update `tests/core/test_api.c`, `tests/cmake/accuracycoin_activation.cmake`, the README revision table and the header comment. `NESTURBATOR_STATE_FORMAT_VERSION` stays 1: no field is added, the meanings of `eval_n` and `eval_m` are unchanged, and every state an older library wrote is aligned. Phase 20 still takes format 2.
- **D-19:** Expected scoreboard moves:
  - **newly passing:** `Sprite 0 Hit behavior`, `Arbitrary Sprite zero`, `Suddenly Resize Sprite`, `Misaligned OAM DMA`;
  - **probably terminal, possibly passing:** `Misaligned OAM behavior`;
  - **status changes:** `$2002 flag timing` probably stays `fail 0x06` but now terminal. This is Phase 20's new baseline;
  - **may move:** `OAM Corruption`, `Address $2004 behavior`, `$2004 Stress Test`, `Frozen OAM2 Increment`, `Misaligned OAM2 Address`, `DMC DMA + OAM DMA`. None passes today.

  `tests/accuracy/scoreboard.txt` is re-recorded in the same change, with each moved row's reason. A newly passing neighbour row is recorded, not chased. Frame hashes (`tests/runner/hashes.txt`, 114 rows) can move only for programs that leave OAMADDR nonzero while rendering. Measure with the campaign diff and re-pin each moved hash with its reason (PITFALLS #7).
- **D-20:** One phase pull request, squash-merged; the phase branch is its own PR before any NMI work. Commit order inside it:
  1. JAM attribution and its tests (no hash impact);
  2. adapter presentation and the run-ahead-across-JAM test;
  3. owned sprite tests marked expected-fail;
  4. OAMADDR reset with its campaign diff;
  5. evaluation from OAMADDR, the dot-66 identity, fixes a-c and revision 7;
  6. scoreboard re-record and hash re-pins, one reason each.

  Splitting commits 4 and 5 lets each moved row be traced to a cause. JAM changes move no rows, so a squash keeps every row move attributable to sprite-0.

### Claude's Discretion
- Exact helper names, the fixture mutation spellings and the backdrop sequence of the synthetic ROM, as long as the contracts above hold.
- Whether the strict and observe JAM messages share one formatting helper.

### Deferred Ideas (OUT OF SCOPE)
- A suite-restart guard in the AccuracyCoin harness (`guest-suite-restart`, D-15): reconsider if a later phase sees a guest re-run loop that a core fix does not end.
- 2C02G OAM corruption on a `$2003` write or at the start of rendering; `$2004` reads during rendering; a single 8-bit evaluation address replacing `eval_n` and `eval_m` (needs a state format bump). All belong in a future OAM/DMA milestone.
- A per-dot sprite-0 check off the video path: only if a NULL-video fast path is ever added.
- A public trace or debug API: not needed, since the cause was found without one.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| JAM-01 | An AccuracyCoin run that hits a guest JAM reports a JAM rather than a generic frame failure. | Reproduced today's behaviour (`execution-frame-failure` / `guest-failure`, exit 1, no report); code seams and the required control flow change are in "JAM attribution" below. |
| JAM-02 | `libretro.runahead` shows that run-ahead across a JAM gives the same picture and silence as running without run-ahead. | Adapter has no public `frame_number` getter; a design that needs no ABI change is given. Existing test passes in 22 s locally. |
| SPR-01 | Sprite-0 hit is set under the console's conditions and on its dot; the `Sprite 0 Hit behavior` row passes. | A scratch build of D-04 alone turns the row to `pass 0x01`. |
| SPR-02 | The first sprite evaluated acts as sprite 0 wherever it sits in OAM, and the row finishes within the page's frame cap; the `Arbitrary Sprite zero` row passes. | A scratch build of D-04..D-06 turns the row to `pass 0x01` at 521 execution frames and ends the guest loop. |
</phase_requirements>

## Summary

CONTEXT.md's design is sound and was largely reproduced in this session. A scratch copy of the tree (outside the repo, in the session scratchpad) with only D-04 applied (clear `oam_addr` at the top of `sprite_access`) turns `Sprite 0 Hit behavior`, `Suddenly Resize Sprite` and `Misaligned OAM DMA` from `fail 0x06` into `pass 0x01`. Adding D-05/D-06 (evaluation from OAMADDR, dot-66 identity, copy-position index, `m = 0` on a miss) turns `Arbitrary Sprite zero` from `timeout 0x03` into `pass 0x01`, ends the guest re-run loop (page 19 now completes with `verdict=complete` at 521 execution frames, not the 1800 cap), and makes `Misaligned OAM behavior` terminal (`fail 0x16`) and `OAM Corruption` terminal (`fail 0x0a`). A full `ctest -j8` of that scratch tree (669 tests) failed only in three places, all explained below; no `tests/runner/hashes.txt` row moved.

Four findings are not in CONTEXT.md and change the plan: (1) three additional rows on page 20 newly pass (`INC $4014`, `Rendering Flag Behavior`, `$2007 read w/ rendering`), so the activation constants and scoreboard re-record are bigger than D-19 lists; (2) the existing `ppu.sprites` DMA test breaks under D-04 and must be edited; (3) `accuracycoin_activation.cmake` pins pass/fail counts and a build-input fingerprint that covers `src/`, `runner/`, `include/`, `cmake/`, `tests/CMakeLists.txt`, `test_conformance.c`, `test_scoreboard.c` and the activation script itself, so the metadata refresh must be the last commit; (4) the libretro adapter has no public way to read the instance's current `frame_number`, so D-16's "instance's current frame_number" must be tracked by the adapter (design below).

**Primary recommendation:** Follow CONTEXT.md D-01..D-20 as written; add the four findings above as explicit plan tasks (edit the existing DMA test; record 7 new passes; predicted activation constants; adapter-side frame_number tracking), and keep the baseline-metadata/activation refresh as the final commit because everything else changes its fingerprint.

## Project Constraints (from CLAUDE.md)

- Clean room (rule 4): implement from NESdev hardware documentation and test behaviour; never open GPL/LGPL emulator source. The AccuracyCoin asm (MIT, pinned) is the sanctioned test-behaviour source. This research opened none.
- Own code, no new dependency; core links only C memory functions; C17, extensions off, integer-only, deterministic, all state in the instance.
- Every behaviour change lands with a test run by `cmake --workflow --preset ci`, and README plus public-header comments are updated in the same change (rule 6).
- No ROM bytes except `tests/roms/manifest.txt`; no personal paths/emails/names (the JAM test must patch bytes in memory only; `scripts/hygiene.sh` runs on commit and push).
- One GSD step per command, then stop and report; work on a phase branch (`gsd/phase-19-jam-reporting-and-sprite-0-hit`, already current); merge by PR with a Conventional Commit title.
- Rule 8 (rolling direction) applies to milestone steps, not to this phase.
- `.claude/CLAUDE.md`: file-changing work goes through GSD commands.
- Hardware comments cite their source (existing style: `[NESdev Wiki, ...]`, `[HWP.xx]`).
- `workflow.nyquist_validation` is `false` in `.planning/config.json`, so no separate Validation Architecture section is emitted; the test map is in "Test Plan" below.

## Architectural Responsibility Map

This is a single-process C project, not a web tiers app; the "tiers" are its three deliverables plus tests.

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| OAMADDR clear, evaluation start, sprite-zero identity | Core library (`src/ppu.c`) | Tests (`tests/ppu`) | Hardware behaviour; deterministic, in instance state |
| Sprite-0 hit flag | Core library (`compose_pixel`) | — | Stays in the video path per D-07 |
| `guest-jam` classification, report, metadata | Runner (`runner/accuracycoin.c`) | Tests (`tests/accuracy/test_conformance.c`, CMake fixtures) | Harness policy, not emulation |
| Patched-ROM JAM run | Test program (`accuracycoin.conformance jam`) | Runner library function | Keeps `nesturbator-run` refusing modified ROMs |
| JAM picture/silence under run-ahead | libretro adapter (`libretro/libretro.c`) | Test host (`tests/libretro/libretro_host.c`, `tests/runahead_model.h`) | Presentation policy is adapter-local; core stays unchanged |
| Scoreboard / baseline / revision constants | Test data (`tests/accuracy/*`, `tests/cmake/accuracycoin_activation.cmake`) | README, `include/nesturbator.h` comments | Re-recorded in the same change |

## Standard Stack

No packages are added; the Package Legitimacy Gate is not applicable (no external package installs in this phase).

| Tool | Version | Purpose | Source |
|------|---------|---------|--------|
| C17 (Apple clang 21.0.0 locally) | extensions off | all code | [VERIFIED: `cc --version` this session] |
| CMake / Ninja | 4.4.3 / 1.13.2 locally (project minimum 3.25) | `cmake --workflow --preset ci` | [VERIFIED: `cmake --version`, `ninja --version`] |
| In-repo `tests/check.h`, `tests/ines.h`, `tests/ppu/ppu_fixture.h`, `tests/runahead_model.h` | n/a | tests | [VERIFIED: read this session] |

**Alternatives considered:** none. CONTEXT.md forbids new dependencies and a public API addition is avoided (see JAM-02 design).

## Package Legitimacy Audit

No external packages are installed or recommended in this phase. **Packages removed due to [SLOP] verdict:** none. **Packages flagged [SUS]:** none.

## Architecture Patterns

### System Architecture Diagram

```
 AccuracyCoin ROM (pinned, 6bee919)             accuracycoin.conformance `jam` (new)
        │                                              │ parse unpatched bytes (pin + SHA ok)
        │                                              │ copy bytes, write $02 at routine_address
        ▼                                              ▼ of last TEST row of page 3
 nesturbator_load_cartridge ───────────────► accuracycoin_run_observe_page
                                                       │
   per frame: frame_and_sample ──► nesturbator_run_frame ──► cpu step ... JAM opcode
        │                                  │ returns NESTURBATOR_STOP_JAM, frame_number unchanged
        ▼ (today: int 0 → "execution-frame-failure"/"guest-failure", exit 1, no report)
   NEW: status propagates ──► jam helper: stop_cause=verdict=guest-jam, jam_frame=N
        │
        ├─ execution stage: final sample ► classify rows (3-state active arg) ► report.tsv ► exit 0
        └─ earlier stage:   metadata only, stderr line, exit 1
                                   │
                  metadata.txt (+jam_frame) ──► campaign collector (whitelist + consistency)

 PPU per dot (src/ppu.c):
   dot 65 ─ sprite_evaluate: eval_n=oam_addr>>2, eval_m=oam_addr&3 (NEW)
   dot 66 ─ first read in range ► eval_sprite_zero[slot]=1 (NEW identity rule)
   dots 65-256 ─ copy to secondary OAM by copy position (NEW)
   dots 257-320 ─ sprite_access: oam_addr = 0 each dot (NEW), pattern fetch
   next line ─ compose_pixel: sprite_zero[i] && bg_opaque && x!=255 ► status |= 0x40

 libretro: retro_run ─► STOP_JAM ─► show cache iff tag == current frame_number else black
           retro_unserialize ─► no invalidate; current frame_number := state bytes [32..40)
```

### Recommended file touch list

```
src/ppu.c                          D-04..D-07 (sprite_evaluate, sprite_access, compose_pixel comment)
include/nesturbator.h              revision 6→7 comment; JAM presentation comment (lines ~385-411)
runner/accuracycoin.{c,h}          frame_and_sample status, jam helper, 3-state classify, metadata, strict msg, collector
runner/main.c                      (message already mirrors "JAM stopped replay at frame %u"; strict msg lives in accuracycoin.c)
libretro/libretro.c                cache tag, retro_unserialize, STOP_JAM branch, comments
tests/ppu/test_sprites.c           D-10 tests; edit the existing DMA test (see Pitfall 2)
tests/accuracy/test_conformance.c  `jam` subcommand, report fixture mutations, classifier cases
tests/CMakeLists.txt               add accuracycoin.jam (inside the input fingerprint)
tests/libretro/libretro_host.c, tests/runahead_model.h   JAM image, oracle record, controls
tests/core/test_api.c, tests/cmake/accuracycoin_activation.cmake   revision 7 + new counts
tests/accuracy/scoreboard.txt, baseline-metadata.txt, measurements.md, README.md, ROADMAP.md, research/SUMMARY.md
```

### Pattern 1: Sprite evaluation from OAMADDR (prototype that passed the rows)

The scratch edit that produced the measured results (reproduce, then add the owned tests first per D-20 order):

```c
/* dot 65 */
ppu->eval_n = (uint8_t)(ppu->oam_addr >> 2);
ppu->eval_m = (uint8_t)(ppu->oam_addr & 3u);

/* copy phase: index by copy position, not eval_m */
ppu->secondary_oam[slot_base + (4u - ppu->eval_copy_remaining)] = ppu->eval_latch;

/* in range, slot free */
ppu->eval_sprite_zero[slot] = ppu->dot == 66u;
ppu->eval_m = (uint8_t)((ppu->eval_m + 1u) & 3u);
if (ppu->eval_m == 0u) ppu->eval_n++;
ppu->eval_copy_remaining = 3u;

/* out of range */
ppu->eval_n++;
if (ppu->eval_count == 8u) ppu->eval_m = (uint8_t)((ppu->eval_m + 1u) & 3u);
else ppu->eval_m = 0u;

/* sprite_access(), first statement */
ppu->oam_addr = 0u;
```
[VERIFIED: scratch build and run this session; hardware basis CITED: nesdev.org/wiki/PPU_registers]

Hardware text (fetched this session): "OAMADDR is set to 0 during each of ticks 257–320 (the sprite tile loading interval) of the pre-render and visible scanlines."; "The value of OAMADDR at this tick determines the starting address for sprite evaluation for this scanline" and "can cause the sprite at OAMADDR to be treated as it was sprite 0"; a misaligned address is "reinterpreted as a Y position". [CITED: nesdev.org/wiki/PPU_registers]

### Pattern 2: JAM propagation through the observe stages

Today `frame_and_sample` returns `int` built from `accuracy_frame(...) == NESTURBATOR_OK && read_controls && sample_rows` [VERIFIED: runner/accuracycoin.c:929-935]. It has five call sites (boot ~1053, navigation ~1086, start ~1123, ~1144, execution ~1196) [VERIFIED: grep]. Change it to return `nesturbator_status` (map the rest of its failure to a non-OK, non-JAM status such as `NESTURBATOR_ERR_ARGUMENT` or a local sentinel), and at each site test `== NESTURBATOR_STOP_JAM` before the generic failure branch.

**Control-flow trap in the execution stage (not in CONTEXT.md):** the active-row branch after the loop requires `execution_frames != budgets->execution_frames` to be false, otherwise it sets `stop_cause = "active-row-not-current-in-automation"; verdict = "driver-failure"; goto done;` [VERIFIED: runner/accuracycoin.c, the `else if (active_count != 0)` block]. A JAM ends the loop early, so that guard would misfire. The JAM path must therefore leave the loop, take the final sample and classification itself (with the 3-state active argument), and not fall into that branch. The simplest structure is a `jam_stop` flag set by the helper: skip the `active_count`/cap chain, run the same report-writing block, and let `verdict`/`stop_cause` stay `guest-jam`.

### Pattern 3: Adapter frame-number tracking (JAM-02, no ABI change)

`nesturbator_frame.frame_number` is only written on success ("On STOP_JAM, frame output fields are not written") [VERIFIED: include/nesturbator.h ~395-410, src/frame.c:60-71]. There is no public getter [VERIFIED: grep of the header's function list]. D-16 says compare the cache tag with "the instance's current `frame_number`". Recommended adapter-only implementation:

- `static uint64_t current_frame_number;` set from `io.frame_number` after each `NESTURBATOR_OK` run, together with `cached_frame_number = io.frame_number` when the cache is filled.
- In `retro_unserialize`, after `nesturbator_load_state` returns OK, set `current_frame_number` from the state header: 8 bytes little-endian at offset 32 ("32  8  frame_number" in the documented layout) [VERIFIED: include/nesturbator.h state layout, lines ~660-670]. The load already validated size and CRC, so this read is safe.
- `invalidate_presentation()` (reset, load_game, unload, init, deinit) also zeroes `current_frame_number`.
- On `STOP_JAM`: show the cache iff `completed_frame_valid && cached_frame_number == current_frame_number`, else black.

Walkthrough with depth N=2 and JAM frame J: at real state frame_number J-2, the speculative runs complete frame J-1 (cache tag J-1) and then jam; instance frame_number is J-1 (unchanged by the jammed call), tag matches, so the frozen picture is shown. Loading the saved state restores J-2 without touching the cache; the real tick then reproduces J-1 normally. At or after J the latched state loads with frame_number J-1, tag J-1 stays, picture is `video[J-1]`, which equals what a no-run-ahead run shows. [ASSUMED: derived by reasoning from src/frame.c semantics; the D-17 oracle assertions are exactly what will confirm it]

Residual risk (cosmetic, JAM only): a frame number can recur on a diverged timeline (rewind then different input), so a tag match could show a picture from the other timeline. Acceptable; record it in the adapter comment.

### Anti-Patterns to Avoid

- **Tagging JAM by exit code only:** exit 1 for earlier-stage JAMs is part of D-13; do not turn it into 0.
- **Hard-coding the patch offset:** derive it from the parsed inventory by rule (last TEST row of page 3). In this session that row is `$1F   SLO absolute,X`, routine `0xa876`, file offset 10374, original byte `a9`. [VERIFIED: scratch harness output `row $1F   SLO absolute,X routine a876 off 10374 orig a9`]
- **Per-row workarounds or cap changes** for Arbitrary Sprite zero (D-08 forbids).
- **Chasing newly passing neighbour rows** (D-19): record them with a reason, do not tune them.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Read ROM routine file offset | A second inventory parser | The already-parsed `accuracycoin_test.routine_address` plus the 16-byte header + `address - 0x8000` NROM mapping (justified local duplicate of static `rom_offset`) | Pin and SHA checks stay authoritative |
| Campaign/fixture mutation framework | New fixture writer | The existing `write_fixture_page`/`write_campaign_fixture` mutation switch in `tests/accuracy/test_conformance.c` (~532-704) | The `accuracycoin.report` controls already exercise reject paths |
| Run-ahead model | New harness | `tests/runahead_model.h` plus `rah_run`/`rah_matrix`/controls in `libretro_host.c` | JAM-aware record is an additive extension (D-17) |
| Synthetic ROM image | Hand-written iNES bytes | `tests/ines.h` (`ines_build`) | Already used by `ppu_fixture.h` |
| Hash/scoreboard re-record | Hand-edited files | Campaign output `build/ci/accuracycoin/full/report.tsv` and `metadata.txt` | Format and SHA are derived, hand edits fail activation |

**Key insight:** every new fact this phase needs is already produced by an existing harness; the work is wiring a third state through it, not building new machinery.

## Runtime State Inventory

Not a rename/refactor/migration phase. Omitted. (Closest analogue: saved states from older libraries stay loadable because `NESTURBATOR_STATE_FORMAT_VERSION` stays 1 and every older state has an aligned OAMADDR-derived `eval_n/eval_m`; D-18.)

## Measured Results (this session)

All numbers are from `build/ci/runner/nesturbator-run` (current tree) and from a scratch copy of the tree in the session scratchpad (never in the repo).

### Baseline reproduction (current `main` code)

Page 19, 1800 execution frames [VERIFIED: observe run this session, total budget 1924]:

| Row | Status | Raw | Cause |
|-----|--------|-----|-------|
| Sprite overflow behavior | pass | 0x01 | terminal-pass |
| Sprite 0 Hit behavior | fail | 0x06 | terminal-fail |
| $2002 flag timing | fail | 0x06 | terminal-fail |
| Suddenly Resize Sprite | fail | 0x06 | terminal-fail |
| Misaligned OAM DMA | fail | 0x06 | terminal-fail |
| Arbitrary Sprite zero | timeout | 0x03 | active-test-budget-exhausted |
| Misaligned OAM behavior | not-run | 0x03 | noncurrent-running-marker |
| OAM Corruption | not-run | 0x00 | unstarted |

This matches `build/ci/accuracycoin/full/report.tsv`. Note: at 1794 execution frames the same page reports `Arbitrary Sprite zero fail 0x06` instead of the timeout (the guest is mid re-run loop), which confirms D-02 that these rows are loop artifacts and are sensitive to the exact cap.

Tooling gotcha: `--frames N` on `--accuracycoin-observe` is the TOTAL budget (boot + 5 per page switch + start + execution). For page 19 at cap 32/5/1800 the campaign formula gives `26 + 90 + 2 + 1800 = 1918` frames used; pass at least `--frames 1924` to get 1800 execution frames (see `tests/cmake/accuracycoin_campaign.cmake:96`). Using `--frames 1800` silently runs only 1676 execution frames and gives different results.

### Prototype outcomes

| Variant | Page 19 result |
|---------|----------------|
| D-04 only | Sprite 0 Hit, Suddenly Resize, Misaligned OAM DMA become `pass 0x01`; `$2002 flag timing` stays `fail 0x06`; `Arbitrary Sprite zero` still `timeout 0x03` at 1800 |
| D-04 + D-05 + D-06 (+ `m=0` on a miss) | `verdict=complete`, 639 total frames / 521 execution frames; Arbitrary Sprite zero `pass 0x01`; Misaligned OAM behavior `fail 0x16`; OAM Corruption `fail 0x0a` |

Full-campaign raw-byte diff (D-04..D-06 prototype vs the committed campaign report), every other row unchanged:

| Row | Before | After |
|-----|--------|-------|
| Sprite 0 Hit behavior | fail 0x06 | pass 0x01 |
| Suddenly Resize Sprite | fail 0x06 | pass 0x01 |
| Misaligned OAM DMA | fail 0x06 | pass 0x01 |
| Arbitrary Sprite zero | timeout 0x03 | pass 0x01 |
| Misaligned OAM behavior | not-run 0x03 | fail 0x16 |
| OAM Corruption | not-run 0x00 | fail 0x0a |
| INC $4014 (page 20) | fail 0x0a | pass 0x01 (not in D-19) |
| Rendering Flag Behavior (page 20) | fail 0x0a | pass 0x01 (not in D-19) |
| $2007 read w/ rendering (page 20) | fail 0x06 | pass 0x01 (not in D-19) |

No row regressed. [VERIFIED: scratch campaign run; the scratch tree has no git, so the collector labelled all rows `untrusted-final-snapshot`, but the raw bytes in `report.tsv` are valid. Re-measure with the real tree and `NESTURBATOR_ACCURACYCOIN_FULL=ON`.]

Predicted new baseline counts (to be re-measured, then written to `accuracycoin_activation.cmake` and `baseline-metadata.txt`): before `pass=101 fail=42 skip=0 not_run=2 timeout=1 terminal_scored_rows=143`; after `pass=108 fail=38 skip=0 not_run=0 timeout=0 terminal_scored_rows=146`. [ASSUMED: arithmetic from the table above; confirm with the real campaign]

### Full suite on the prototype

`ctest -j8` in the scratch ci tree: 669 tests, 3 failures:

1. `ppu.sprites`: existing `test_oam_dma_copies_page_and_wraps_destination` fails (`nes.ppu.oam[0xefu] == (255u ^ 0xa5u) failed: 255 != 90`, `nes.ppu.oam_addr == 0xf0u failed: 34 != 240`, and the second DMA block at lines 249-250). Cause: `setup()` sets `nes.ppu.scanline = 261u` and `nes.ppu.mask = 0x1eu`, so the 515-cycle DMA crosses dots 257-320 with rendering on and OAMADDR is now (correctly) cleared. [VERIFIED: scratch ctest output + tests/ppu/test_sprites.c:11-33,223-250]
2. `accuracycoin.activation`: `baseline metadata build_inputs_sha256 is 'a3a8ec47...' expected '0dea7fb8...'` (fingerprint, expected after any source edit).
3. `accuracycoin.driver`: failed only because the scratch tree has no git (`source_revision=unknown`); it passes in the real tree.

Every `runner.*hash*`, `runner.write_hashes`, `libretro.*`, `core.*`, `mmc3.*`, `vectors` and `accuracy.scoreboard*` test passed. So none of the 114 `tests/runner/hashes.txt` rows moved under D-04..D-06. The planner should still schedule the measurement (campaign diff + `runner.write_hashes.content`) but expect zero re-pins. [VERIFIED: scratch ctest]

### JAM today (before changes)

Patched in memory (scratch harness linking `runner/accuracycoin.c` and `build/ci/libnesturbator.a`), page 3 with `$02` at the last row's routine: `status=1`, metadata `stop_cause=execution-frame-failure`, `verdict=guest-failure`, `active_key=-`, `execution_frames=11`, and no `report.tsv` written (only `diagnostics.tsv`, `metadata.txt`, `raw.tsv`). RAM at the JAM: rows 1 to 6 (`$0409`-`$040e`) hold `01`, row 7 (`$040f`) holds `03`, `menuCursorYPos` (`$0016`) is `06`, `AutomateTestSuite` (`$0034`) is `01`. This confirms D-13's attribution rule (cursor names a TEST row whose byte is `0x03`) and shows the JAM occurs after the row's running marker is set, so the patch site is valid. [VERIFIED: scratch harness]

## Common Pitfalls

### Pitfall 1: Execution-stage JAM falls into the "active-row-not-current" driver-failure
**What goes wrong:** the post-loop chain requires `execution_frames == budgets->execution_frames` for an active row; a JAM ends the loop early, so it reports `driver-failure`, exit 1, no report.
**How to avoid:** handle the JAM before that chain (Pattern 2). Test it: `accuracycoin.jam` must see `verdict=guest-jam`, exit 0 for the execution stage, and a report with exactly one `fail 0x03 guest-jam` row.

### Pitfall 2: The existing DMA test encodes stale OAMADDR across rendering
**What goes wrong:** D-04 makes `test_oam_dma_copies_page_and_wraps_destination` fail (measured).
**How to avoid:** in the same commit as D-04 (commit 4), change that test's `setup`-derived state so the DMA runs with rendering off (`nes.ppu.mask = 0`) or compute expectations from the new rule; label the expectations "(derived)" from NESdev (rendering off keeps OAMADDR). Do not delete the wrap coverage. D-10 case 2 (rendering off keeps OAMADDR) pairs with it.

### Pitfall 3: The campaign fingerprint couples all commits to the last one
`accuracycoin_compute_input_fingerprint` hashes `src/*.c|h`, `runner/*.c|h`, `include/*.h`, `cmake/*.cmake`, `CMakeLists.txt`, `CMakePresets.json`, `tests/CMakeLists.txt`, `tests/accuracy/test_conformance.c`, `tests/accuracy/test_scoreboard.c` and three `tests/cmake/accuracycoin_*.cmake` scripts including `accuracycoin_activation.cmake` itself [VERIFIED: tests/cmake/accuracycoin_input_fingerprint.cmake:1-24]. `accuracycoin_activation.cmake` compares `build_inputs_sha256`, `build_inputs_count`, the pass/fail/not_run/timeout/terminal counts, `scoreboard_sha256`, `behaviour_revision|6` and more against `tests/accuracy/baseline-metadata.txt` [VERIFIED: activation.cmake:117-150, baseline-metadata.txt]. Consequences:
- `accuracycoin.activation` (and CI) will be red on every intermediate commit that touches those inputs (commits 1, 4, 5 and any test edit). Acceptable inside one squash-merged PR, but run `accuracycoin.activation` only as a gate on the final commit, or run per-commit gates with `-E accuracycoin.activation`.
- The refresh order is fixed: finish all code/test edits, then update `activation.cmake` constants (this changes the fingerprint), then run the full campaign with `-DNESTURBATOR_ACCURACYCOIN_FULL=ON` and copy `build/ci/accuracycoin/full/metadata.txt` fields into `baseline-metadata.txt`, then re-run activation. The previous phase did exactly this ("the full campaign was run again during the `ci` workflow before refreshing its provenance", tests/accuracy/measurements.md).
- `source_dirty`/`source_revision` in baseline metadata must be valid hex and 0/1; follow the Phase 18 precedent.

### Pitfall 4: Scoreboard counts are asserted, not only rows
`terminal_scored_rows`, `not_run`, `timeout`, `pass`, `fail` and `scoreboard_sha256` are pinned. Update them from the real campaign, not from this research's prediction. Also update `tests/accuracy/measurements.md` ("101 pass, 42 fail, ... 1 timeout" and the Arbitrary Sprite zero narrative) per D-03.

### Pitfall 5: `jam_frame` placement and schema
Metadata is a single `fprintf` with positional arguments [VERIFIED: runner/accuracycoin.c ~1322-1345]; adding `jam_frame=%s` after `verdict=` means adding the argument in the same position. The campaign collector reads metadata key by key (`META_STRING`), so also add `jam_frame` there, and the fixture writer in `test_conformance.c` (~734-760) which fabricates metadata for the report tests must emit the new key or the collector will reject valid fixtures. `report_schema` stays 1.

### Pitfall 6: Classifier signature change ripples
`accuracycoin_classify` and `accuracycoin_classify_row` take `int active_budget_expired` [VERIFIED: runner/accuracycoin.h]. Changing it to the three-state value touches `accuracycoin_classify_final_row`, the collector's re-classification, and every call in `test_conformance.c`. Keep numeric compatibility: `ACCURACYCOIN_ACTIVE_RUNNING 0` and `_BUDGET 1` equal today's 0 and 1, so existing callers compile unchanged; only the JAM caller passes 2. Today's `raw 0x03` without budget gives `ACCURACYCOIN_RESULT_INVALID` cause `active-test`; the JAM case must instead return `ACCURACYCOIN_RESULT_FAIL` cause `guest-jam` and never override a terminal byte.

### Pitfall 7: Run-ahead test accounting and runtime
`libretro.runahead` passes in about 22 s on the ci preset locally (asan runs 30 ticks, TIMEOUT 600). The JAM image adds a matrix at several depths; keep T=30/T=180 and the D-17 bounds so the asan budget holds. `RAH_MIN_ROMS` is 7 and file-only [VERIFIED: libretro_host.c:2387]; add the JAM image counter separately instead of raising it.

### Pitfall 8: Misaligned-start evaluation reads and the end-of-OAM stop
`sprite_evaluate` returns early at `eval_n >= 64u`, which implements "evaluation stops at the end of OAM". With a misaligned start the copy phase can wrap `m` and increment `n` past 63; the early return handles it. Owned test D-10-6 must include a start near the end (for example `$FD`) to pin that no secondary-OAM overflow or OOB read occurs under asan (`oam[n*4+m]` is indexed by a `uint16_t`; `n` is below 64 at the read, so the maximum index is 255).

### Pitfall 9: D-05(c) is a model assumption
Setting `m = 0` after an out-of-range read with fewer than 8 sprites found (rather than leaving the misaligned `m`) is what the prototype used and what D-05 specifies; its NESdev basis is the "(addr+4)&$FC" behaviour only indirectly. The AccuracyCoin result is the check (Arbitrary passes; Misaligned OAM behavior reaches code `$16`, so its remaining failure is a later subtest, which is Phase 20+ territory per D-09). If any owned test is derived, label it from the asm error code and NESdev text, not from our output.

## Code Examples

### Hit gate (unchanged logic, add the contract comment per D-07)

```c
/* src/ppu.c compose_pixel, existing */
if (ppu->sprite_zero[i] != 0u && bg_opaque != 0u && x != 255u &&
    (ppu->mask & 0x08u) != 0u && (x >= 8u || (ppu->mask & 0x06u) == 0x06u))
    ppu->status |= 0x40u;
```
[VERIFIED: src/ppu.c:412-414] Add: this runs only through `compose_pixel`, which requires `video_output != NULL`; `nesturbator_run_frame` rejects a NULL video, so there is no NULL-video path.

### Owned-test shape (tests/ppu/test_sprites.c)

`ppu_fixture.h` includes `internal.h`, and `test_sprites.c` already drives a static `struct nesturbator nes` and reads `nes.ppu.oam_addr` and `eval_*` fields directly [VERIFIED: tests/ppu/test_sprites.c:7-33, tests/ppu/ppu_fixture.h]. Case 1 (rendering on, `$2003=$81`, run past dot 320, expect 0) and case 5 (OAMADDR `$80` at dot 65, sprite 32 is sprite zero, a copy at OAM[0] does not hit) both fit that pattern. Expected values must be written as constants derived from the NESdev text (for example "OAMADDR is 0 after dot 320"), not computed from the PPU.

### JAM CTest wiring

The existing page tests run through `cmake/expect_output.cmake` with `-DCMD=...;-DEXPECT=...;-DEXPECT_EXIT=...` [VERIFIED: tests/CMakeLists.txt:703-712]. `accuracycoin.jam` can be a plain `add_test(NAME accuracycoin.jam COMMAND accuracycoin.conformance jam <new-rom> <old-rom>)` that does its own assertions (as `inventory`, `classifier`, `migration` do), with `TIMEOUT 60`. Strict mode on a patched page 2 of the old ROM calls `accuracycoin_run_strict_page`, which today fails silently on a JAM; its message goes to stderr, so capture it in-process (redirect or pipe) or compare via the returned status and a small output hook, not by spawning the CLI (the CLI refuses modified ROMs).

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| OAMADDR persists through rendering | OAMADDR is set to 0 on dots 257-320 of pre-render and visible lines | NESdev documented behaviour | Fixes DMA-after-`$2003` and sprite-0 identity rows |
| Sprite zero = `eval_n == 0` | Sprite zero = the sprite found on the first evaluation read after OAMADDR start | NESdev: sprite at OAMADDR "treated as it was sprite 0" | Arbitrary Sprite zero |
| JAM = generic frame failure | JAM named and attributed | This phase | JAM-01 |

**Deprecated/outdated in project docs:** the roadmap's "cause is not isolated" note and `research/SUMMARY.md` Disagreement (a) (D-03); `research/SUMMARY.md`'s "6→7 in Phase 20" (D-18: Phase 20 takes 8).

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | Predicted post-fix counts `pass=108 fail=38 not_run=0 timeout=0 terminal=146` | Measured Results | Activation constants written from prediction would fail; mitigated by instructing to take values from the real campaign |
| A2 | Adapter walkthrough: tag == current frame number gives identical picture at every tick at or after J-N | Pattern 3 | JAM-02 test fails; the D-17 oracle assertions will detect it before merge |
| A3 | The three page-20 rows (`INC $4014`, `Rendering Flag Behavior`, `$2007 read w/ rendering`) pass for the right reasons (OAMADDR state after the stale-state fix) and not by chance | Measured Results | Scoreboard records a pass that a later phase loses; low risk, D-19 says record, do not chase |
| A4 | `NESTURBATOR_ERR_ARGUMENT` (or a local sentinel) is acceptable as the non-JAM failure mapping from `frame_and_sample` | Pattern 2 | Cosmetic; any non-OK, non-JAM value works |
| A5 | Strict-mode JAM message can be asserted in-process without spawning the CLI | Code Examples | Test design only; fallback is a thin hook or a captured stderr pipe |

## Open Questions

1. **Does the real-tree campaign agree with the scratch campaign?**
   - Known: raw bytes agree between baseline and prototype for all rows except the nine listed.
   - Unclear: the final counts and `terminal_scored_rows` until run with git and `NESTURBATOR_ACCURACYCOIN_FULL=ON`.
   - Recommendation: schedule the campaign run in commit 4 (D-04 alone) and again in commit 5/6, and record each diff.
2. **Per-commit gates vs the fingerprint.** Which intermediate commits must be green under `accuracycoin.activation`? Recommendation: treat only the final commit and the squash result as gated; use `ctest -E accuracycoin.activation` for intermediate commits.
3. **Is D-05(c) necessary on its own?** Not isolated; the prototype included it. Low value to isolate; keep it per D-05.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| CMake | workflow presets | yes | 4.4.3 | none needed |
| Ninja | build | yes | 1.13.2 | none needed |
| C compiler | build | yes | Apple clang 21.0.0 | none needed |
| `build/ci` configured with `NESTURBATOR_ACCURACYCOIN_FULL=ON` | campaign refresh, activation test | yes (accuracycoin/full exists, `accuracycoin.activation` passes today) | n/a | `cmake --preset ci -DNESTURBATOR_ACCURACYCOIN_FULL=ON` |
| git (for `source_revision`) | campaign collector trust | yes in repo | n/a | scratch trees without git yield `untrusted-final-snapshot` |

Missing with no fallback: none.

## Security Domain

`security_enforcement` is not set in `.planning/config.json` (treated as enabled). This phase has no network, auth or crypto surface; the relevant controls are input handling.

| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V2/V3/V4 Auth, Session, Access | no | n/a |
| V5 Input Validation | yes | Bounds-checked file offset in the JAM patcher; existing size/CRC checks on `retro_unserialize` before reading the header's frame number; fuzz target `fuzz.state` already covers the loader |
| V6 Cryptography | no | existing SHA-256 pin check reused, not extended |

| Threat pattern | STRIDE | Mitigation |
|----------------|--------|-----------|
| Patcher writes outside the ROM image | Tampering | Local NROM mapping rejects addresses below `$8000` or at/after PRG end (as `rom_offset` does) and asserts the original byte is not already `$02` |
| Adapter reads the frame number from an unvalidated buffer | Tampering/DoS | Read bytes 32-39 only after `nesturbator_load_state` returned `NESTURBATOR_OK` (size and CRC already checked) |
| Modified ROM accepted by the CLI | Spoofing | Unchanged: the JAM run exists only inside `accuracycoin.conformance`, never in `nesturbator-run` |
| Personal paths in committed output | Information disclosure | Hygiene hook; tests write under the CMake binary dir; no ROM bytes committed |

## Test Plan (maps requirements to commands)

| Req | Behavior | Test | Command |
|-----|----------|------|---------|
| JAM-01 | Observe stops on patched page 3 and names `guest-jam`, report row `fail 0x03 guest-jam`, exit 0; strict mode on patched old page 2 prints the stderr line, exit 1 | new `accuracycoin.jam` | `ctest -R accuracycoin.jam` |
| JAM-01 | Control: attribution dropped is rejected | extended `accuracycoin.report` mutations (`jam` accepted; `jam-cause-dropped`, `jam-row-dropped`, `jam-frame-missing` rejected) | `ctest -R accuracycoin.report` |
| JAM-01 | Classifier three-state cases | `accuracycoin.classifier` | `ctest -R accuracycoin.classifier` |
| JAM-01 | Unpatched pin has `jam_frame=-` and no `guest-jam` | `accuracycoin.tracer` | `ctest -R accuracycoin.tracer` |
| JAM-02 | Run-ahead across a JAM equals no-run-ahead picture and silence; controls trip | `libretro.runahead` | `ctest -R libretro.runahead` |
| SPR-01/02 | Owned PPU cases D-10-1..9 | `ppu.sprites` | `ctest -R ppu.sprites` |
| SPR-01/02 | Rows pass within the cap; page-19 terminal and loop ends (D-11) | `accuracy.scoreboard`, `accuracycoin.activation`, campaign | `cmake --workflow --preset ci` |
| KEEP | No lost pass | `accuracy.scoreboard.no_lost_pass`, `accuracycoin.protected` | `cmake --workflow --preset ci` |
| Hashes | No moved frame hash (measured: none) | `runner.write_hashes.content` | `ctest -R runner.write_hashes` |

Quick loop: `cmake --build build/ci && (cd build/ci && ctest -R "ppu.sprites|accuracycoin.(classifier|report|jam)" )` runs in about a second. Full gate: `cmake --workflow --preset ci`; also run `cmake --workflow --preset asan` because `libretro.runahead` and the new JAM image run 30 ticks there.

The D-11 page-19 check (Misaligned OAM behavior terminal and the loop finished) is best expressed through the scoreboard: after re-record the row is `fail 0x16` and `$2002 flag timing`/`OAM Corruption` are terminal, so `terminal_scored_rows=146` and `timeout=0` in activation already enforce it.

## Sources

### Primary (HIGH confidence)
- Repository files read this session: `src/ppu.c` (sprite_evaluate, sprite_access, compose_pixel, fetch_step, register read/write), `src/frame.c`, `include/nesturbator.h` (frame API, state layout, revision), `libretro/libretro.c`, `runner/accuracycoin.{c,h}`, `runner/main.c`, `tests/ppu/test_sprites.c`, `tests/ppu/ppu_fixture.h`, `tests/CMakeLists.txt`, `tests/cmake/accuracycoin_{activation,campaign,input_fingerprint,protected}.cmake`, `tests/accuracy/*` (scoreboard, baseline metadata, measurements)
- Builds and runs this session: `build/ci` observe runs for pages 3 and 19, ctest subsets, a scratch-copy prototype of D-04..D-06 with a 669-test ctest and a 22-page campaign, a scratch harness patching page 3 in memory
- [nesdev.org/wiki/PPU_registers](https://www.nesdev.org/wiki/PPU_registers): OAMADDR reset on ticks 257-320, tick 65 start address and sprite 0, misaligned-address reinterpretation

### Secondary / not re-fetched (taken from CONTEXT.md, [CITED] by its authors)
- NESdev "PPU sprite evaluation"; AccuracyCoin asm and README at pin `6bee919529289c555803b612b8c9e4f7eaaeeabb` (line numbers in D-02, D-05, D-06 were not re-opened; the asm was deliberately not opened here to stay within the clean-room rule, the behaviour was confirmed by running the ROM); docs.libretro.com run-ahead guide

## Metadata

**Confidence breakdown:**
- Sprite-0 fix and expected row moves: HIGH, reproduced end to end in a scratch build.
- JAM harness changes: HIGH for current behaviour and control flow; MEDIUM for the exact edit set (not yet implemented, derived from code reading plus a patched run).
- Run-ahead adapter design: MEDIUM, reasoned from `src/frame.c` semantics; the D-17 oracle is the check.
- Baseline/activation refresh mechanics: HIGH for what is pinned, MEDIUM for the predicted new values.

**Research date:** 2026-10-25
**Valid until:** 2026-11-24 (stable code; revalidate if Phase 18 artifacts or the AccuracyCoin pin change)
