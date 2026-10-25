# Phase 19: JAM reporting and sprite-0 hit - Context

**Gathered:** 2026-10-25
**Status:** Ready for planning

<domain>
## Phase Boundary

A developer reading an AccuracyCoin run sees a guest JAM named as a JAM (JAM-01). Run-ahead across a JAM in RetroArch gives the same picture and silence as running without it (JAM-02). The `Sprite 0 Hit behavior` and `Arbitrary Sprite zero` rows pass within the 1800-frame cap (SPR-01, SPR-02). Every row that passes today still passes. No NMI, `$2002` split-sampling or interrupt work: that is Phases 20 and 21.

</domain>

<decisions>
## Implementation Decisions

The owner asked for every decision point to be fanned out to the point of diminishing returns, with one coherent recommendation per point that is then adopted without further questions. Four research passes covered: JAM attribution, run-ahead across a JAM, the sprite-0 probe, and the sprite-0 model and fix bounds. The probes ran in scratch copies of the source; no tracked file was changed.

### Sprite-0 root cause (settled: replaces the roadmap's "not isolated")
- **D-01:** The basic sprite-0 hit already works. On a clean first pass of page 19, `Sprite 0 Hit behavior` writes `$01` at about frame 190, with every subtest from 1 to E passing; test E reads 0 at line 1 dot 248 and 1 at dot 261, and the hit sets at dot 255 (x=254). `Suddenly Resize Sprite` and `Misaligned OAM DMA` also write `$01`. `Arbitrary Sprite zero` writes `$0A` (error code 2: subtest 2, sprite-zero identity at a nonzero OAMADDR).
- **D-02:** The `0x06` results in the scoreboard come from re-runs. Subtest 1 of `Misaligned OAM behavior` fails on our core. Its top-level failure branches (asm lines 7440 and 7446) reach `FAIL_MisalignedOAM_Behavior`, whose `PLA PLA` (lines 7415-7418) drops RunTest's return address. `AutomaticallyRunEveryTestLoop` (lines 17988-18005) then re-runs rows 2-7 with X reset. On re-runs, OAMADDR is stale: `$2003=$81` stays set because `src/ppu.c` never resets it during rendering, every later DMA lands at OAM[$81], and evaluation always reads from OAM[0]. The "8000 frames ends in 0x06" measurement in research is the same artifact, and the `compose_pixel` first-opaque-slot hypothesis is not supported by any measurement. Subtest 1 of `Misaligned OAM behavior` runs the same code as Arbitrary subtest 3, so fixing SPR-02 also ends the guest loop.
- **D-03:** Record this correction in the phase research: `tests/accuracy/measurements.md` and the ROADMAP Phase 19 note ("cause is not isolated"), and mark research/SUMMARY.md Disagreement (a) as superseded, all in this phase's change.

### Sprite-0 hardware model (SPR-01, SPR-02)
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

### Owned tests (ppu.sprites), written before the fix
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

### JAM attribution (JAM-01)
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

### Run-ahead across a JAM (JAM-02)
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

### Versions, hashes and delivery
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

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Phase scope
- `.planning/ROADMAP.md`: Phase 19 goal, success criteria and note (update the note per D-03)
- `.planning/REQUIREMENTS.md`: JAM-01, JAM-02, SPR-01, SPR-02, KEEP-01 to KEEP-03
- `.planning/PRODUCT-ROADMAP.md`: RM-004

### Research (v5)
- `.planning/research/SUMMARY.md`: build order; Disagreement (a) is superseded by D-01 and D-02
- `.planning/research/FEATURES.md`: Group 5 and the JAM section
- `.planning/research/ARCHITECTURE.md`: section 5 (harness, run-ahead) and the Sprite-0 section
- `.planning/research/PITFALLS.md`: #6, #7, #8, #9, #15, #16

### Hardware and project preparation
- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md`: PPU sprite evaluation and OAM
- `.planning/preparation/CONFORMANCE.md`: AccuracyCoin pin, scoreboard and campaign rules
- `.planning/preparation/LIBRETRO-AND-RUNNER.md`: adapter presentation and run-ahead
- `.planning/preparation/ENGINEERING.md`: tests, hashes, delivery
- `.planning/milestones/v4-phases/18-execution-and-lifecycle-robustness/18-03-PLAN.md` and `18-RESEARCH.md`: the original D-10 JAM presentation rule that D-16 narrows

### External primary sources
- NESdev wiki "PPU registers" (OAMADDR: values during rendering; tick 65 sets the evaluation start) and "PPU sprite evaluation"
- AccuracyCoin asm and README at pin `6bee919529289c555803b612b8c9e4f7eaaeeabb` (MIT, github.com/100thCoin/AccuracyCoin): `TEST_Sprite0Hit_Behavior` (~6908-7158), `TEST_ArbitrarySpriteZero` (~7161-7290), `TEST_MisalignedOAM_Behavior` and `FAIL_MisalignedOAM_Behavior` (~7415-7700), `AutomaticallyRunEveryTestInSuite` (~17988-18005), `RunTest` (~18161), `TEST_Fail` (~3377)
- docs.libretro.com run-ahead guide (single- and second-instance order)

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `tests/accuracy/test_conformance.c`: the `accuracycoin.conformance` program, which already links `runner/accuracycoin.c` and the library and has a fixture mutation harness (~532-704); D-12 and D-14 extend it.
- `tests/libretro/libretro_host.c`: `rah_run` (~2435), `rah_matrix` (~2545), controls (~2606), `check_jam_presentation` (~1084) with its `delay_then_jam` snippet; `tests/runahead_model.h`; `tests/ines.h` for synthetic images.
- `tests/ppu/ppu_fixture.h`: includes `internal.h`, so owned PPU tests can drive dots and read `oam_addr` and `eval_*` directly.
- `runner/main.c` (~1254): an existing "JAM stopped replay at frame N" message to mirror.

### Established Patterns
- Hardware comments cite their source ([HWP.xx] or NESdev page), as in `sprite_evaluate`.
- Observe verdict and stop-cause strings, metadata key=value lines and collector consistency rules (`guest-timeout`, ~2286-2297) are the model for `guest-jam`.
- Every behaviour change updates the README and the comments in `nesturbator.h` in the same commit.

### Integration Points
- `src/ppu.c`: `sprite_evaluate` (~175-247; line 230 for identity), `sprite_access` (dots 257-320, ~287-330), `compose_pixel` (~396-421), and the visible-pixel block (~531-538).
- `runner/accuracycoin.c`: `accuracy_frame` (~546), `frame_and_sample` (~929), the observe stages (~1054-1270), metadata (~1322-1345), strict page (~729-785), and the collector whitelist (~1875).
- `libretro/libretro.c`: the `STOP_JAM` presentation (~234-241), `retro_unserialize` (~289-294), `invalidate_presentation`.
- `src/state.c` (~255-300): the PPU fields are already serialized; no change.

</code_context>

<specifics>
## Specific Ideas

- The owner's working method: fan out each decision across the relevant stakeholder lenses (hardware accuracy, test architecture, CI, performance and determinism, API stewardship, release management, adversarial), prefer primary sources, then adopt one coherent recommendation without further questions.
- "Another copy and paste is better than another dep": the local NROM offset mapping in D-12 is such a justified duplicate. No dependency is added anywhere in this phase.
- The probe evidence came from scratch copies with `fprintf` lines (session scratchpad, not tracked). The planner should treat D-01 and D-02 as measured facts and still re-confirm them through the D-10 tests failing on `main`.

</specifics>

<deferred>
## Deferred Ideas

- A suite-restart guard in the AccuracyCoin harness (`guest-suite-restart`, D-15): reconsider if a later phase sees a guest re-run loop that a core fix does not end.
- 2C02G OAM corruption on a `$2003` write or at the start of rendering; `$2004` reads during rendering; a single 8-bit evaluation address replacing `eval_n` and `eval_m` (needs a state format bump). All belong in a future OAM/DMA milestone.
- A per-dot sprite-0 check off the video path: only if a NULL-video fast path is ever added.
- A public trace or debug API: not needed, since the cause was found without one.

</deferred>

---

*Phase: 19-jam-reporting-and-sprite-0-hit*
*Context gathered: 2026-10-25*
