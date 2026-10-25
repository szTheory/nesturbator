# Project Research Summary

**Project:** nesturbator, milestone v5 "NMI, VBlank and sprite-0 timing"
**Domain:** NTSC NES core timing: `$2002` flag, NMI edge and poll, CPU interrupt latency and hijack, sprite-0 hit
**Researched:** 2026-10-25
**Confidence:** MEDIUM-HIGH

## Executive Summary

v5 makes eleven AccuracyCoin rows pass at pin `6bee919`: nine VBlank, NMI and interrupt rows and two sprite-0 rows (PROJECT.md, Current Milestone). The rows are not eleven separate bugs. They share one defect: NMI is a one-shot `nmi_pending` flag poked from three PPU sites, not the physical line `vblank_flag AND NMI_enable`, sampled by an edge detector and polled by the CPU in the penultimate cycle (ARCHITECTURE.md section 1; PITFALLS.md #1). The recommended approach is one NMI-line model with an explicit edge latch and poll latch. CPU hijack and I-flag latency are built on top of it. Sprite-0 is a separate, attributable track.

No new dependency, committed ROM or runner flag is needed (STACK.md, Verdict). Each step is shown by owned dot-exact reproducers in `tests/ppu` and `tests/cpu`, plus the scoreboard check in `ci`. AccuracyCoin (MIT, pinned) is the specification. blargg ROMs cannot be committed (no licence), so the owned reproducers carry the proof. A nightly fetch-only blargg lane is optional.

Key risks: patching rows one at a time breaks neighbours (PITFALLS #1); blanket hash re-pins hide regressions (#7); new latches missing from the save-state visitor cause run-ahead divergence (#6); and the sprite-0 basic hit fails for a root cause nobody has isolated yet, so it needs a trace probe before any fix (FEATURES.md Group 5). The 101 PASS rows must not regress.

## Key Findings

### Recommended stack (STACK.md)
- No change to language, build or dependencies: C17, CMake presets, CTest with `check.h`.
- Expected behaviour per row comes from NESdev pages (HWP.03 PPU frame timing, HWC.02 and HWC.17 CPU interrupts, HWP.06 sprite evaluation) and from the AccuracyCoin asm and README at the pin. Read the failing subtest's assembly to learn what its error byte means.
- Permitted ROMs: AccuracyCoin (in the manifest). Holy Mapperel, DABG, Nesteroids, RHDE and nes-runner guard frame and audio hashes.
- Do not add: blargg, bisqwit or kevtris ROMs to the repo; GPL or LGPL source; new runner flags; a newer AccuracyCoin pin; PAL or DMA work.

### Expected behaviour (FEATURES.md)
- **G1, `$2002` split sampling:** `$2002 flag timing` (`0x06`). Bit 7 is latched at M2 rise; bits 6 and 5 are read at M2 fall. Low complexity.
- **G2, NMI edge:** `NMI Control` (`0x22`, subtests 8 and 9), `NMI Timing`, `NMI Suppression`, `NMI at VBlank end`, `NMI disabled at VBlank` (all `0x06`). High complexity; the central change.
- **G3, I-flag latency:** `Interrupt flag latency` (`0x0a`). CLI, SEI and PLP take effect one instruction late; RTI takes effect at once.
- **G4, hijack:** `NMI Overlap BRK` (`0x0a`), `NMI Overlap IRQ` (`0x06`). The vector is chosen after cycle 4 of 7.
- **G5, sprite 0:** `Sprite 0 Hit behavior` (basic hit missing, `0x06`) and `Arbitrary Sprite zero` (timeout `0x03`, see Disagreements).
- **JAM:** no ROM-side signal exists. The library returns `NESTURBATOR_STOP_JAM`; the runner must attribute it.
- **Anti-features:** per-row special cases; raising the 1800-frame cap (`budgets.txt` rejects it); fixing neighbour rows (DMA, `Misaligned OAM`); game-specific NMI hacks; setting sprite-0 on any pixel.

### Architecture (ARCHITECTURE.md)
- Keep the per-dot PPU and the 3-dot-then-access bus order. Make the edges explicit instead of reordering sub-cycles.
- `ppu.c` 482-499, 559-570, 629-643: replace the `nmi_pending` pokes with a derived `nmi_line = flag & enable`.
- `bus.c cycle()`: edge-detect `nmi_line` against `nmi_prev` (the field exists and is unused) and latch `nmi_pending`. Add penultimate-cycle polls for NMI, IRQ and the I flag.
- `cpu.c`: one shared `interrupt_entry` for NMI, IRQ and BRK, replacing the three copies at 496-525 and 1046-1054. Decide from polled values, not live `p`.
- Sprite 0: move the hit test out of the `video_output` gate; identity follows the first sprite in range (PITFALLS #9).
- State: new fields bump `NESTURBATOR_STATE_FORMAT_VERSION` to 2; behaviour revision goes from 6 to 7. `ppu.c` must not call `cpu.c` (link trap).

### Pitfalls (PITFALLS.md)
1. One NMI-line model, not per-row patches; fix the order inside a CPU cycle once, in a comment.
2. `$2002` window: a read one dot before set gives no flag and no NMI; a read on the set dot or one later gives flag set and NMI suppressed; two or more dots later is normal. Test with a dot-offset table.
3. Boundary sampling cannot satisfy NMI Control 8/9 or the branch polls.
4. CLI, SEI and PLP change I after the poll; RTI changes it before. Keep the polling I separate from the architectural I.
5. Clear the NMI latch only when an NMI vector is actually taken, so a hijacked NMI is not lost.
6. Add every new field to the state visitor. Fork state at VBL set, VBL clear and mid-BRK/IRQ.
7. Hash drift: one re-pin per commit, with a reason and a frame diff. Phase 10 moved 24 rows for a smaller change.
8. Sprite-0 and VBL sync are measurement tools for other AccuracyCoin rows; a fix can flip rows that do not mention it.
9. Sprite-zero identity is the first sprite in range at dot 66 of the previous line, not OAM index 0. Subtest E needs dot-exact set timing.
10. Alignment: one fixed power-on and reset phase, saved as state. No random alignment (breaks Rule 5).
11. Moderate: odd-frame skip (ppu_vbl_nmi 09/10), MMC3 A12 timing, Bases Loaded II `$00` then `$80`, silent JAM, tests that mirror the implementation.

## Implications for Roadmap

One build order, which supersedes the three orders in the source files (see Disagreements):

| Step | Delivers | Rows | Must avoid |
|---|---|---|---|
| 0 | JAM attribution in the AccuracyCoin runner (`STOP_JAM` gives `guest-jam`); classifier mutation test. No hash change. | none; makes later failures diagnosable | PITFALLS #15 |
| 1 | Owned reproducers, marked expected-fail: `$2002` dots 0-3 of line 241 and 0-1 of 261; `$2000` bit-7 toggles near dot 1; CLI/SEI/PLP/branch polls; NMI at cycles 1-7 of BRK and IRQ; ppu_vbl_nmi 02, 05-10 windows as owned tests. Plus a sprite-0 trace probe of subtest 1. No behaviour change. | none | #16 (derive expectations from NESdev and AC codes; label "(derived)") |
| 2 | Sprite-0 basic hit and first-in-range sprite-zero identity. Separate PR. Basic hit does not depend on the alignment constant. | `Sprite 0 Hit behavior` subtest 1; `Arbitrary Sprite zero` (re-measure) | #8, #9 |
| 3 | NMI line, edge latch, `$2002` split sampling (G1, G2 sweeps); state bump to format 2 and revision 7. | `$2002 flag timing`, `NMI Timing`, `NMI Suppression`, `NMI at VBlank end`, `NMI disabled at VBlank` | #1, #2, #6, #7, #12 |
| 4 | Penultimate-cycle NMI poll (NMI Control 8/9): the poll reads the edge latch from step 3. | `NMI Control` | #3, #14 |
| 5 | I-flag latency: polled-I snapshot; branch polls (before cycle 2; taken branch skips one poll). | `Interrupt flag latency` | #4 |
| 6 | Shared interrupt entry and vector hijack, vector chosen after cycle 4 of 7. | `NMI Overlap BRK`, `NMI Overlap IRQ` | #5 |
| 7 | Sprite-0 subtests 2-3 and E (positional, dot-exact set) if still failing; re-measure page 19 and decide the cap from measurement. | `Sprite 0 Hit behavior` all subtests; `Arbitrary Sprite zero` | #9, #10 |
| 8 | Close-out: 11 rows at `pass 0x01`, zero lost PASS, README and `nesturbator.h` comments, revision and format pins checked. | all 11 | #7, #11 |

**Ordering rationale:**
- JAM first: zero hash impact, and every later failure becomes classifiable (PITFALLS #15; ARCHITECTURE section 6).
- Reproducers before fixes: they are the only proof available, since blargg cannot be committed.
- Sprite-0 basic hit early, dot-exact E late: the basic hit does not use the alignment constant that step 3 introduces. Keeping sprite-0 in its own PR makes regressions attributable (PITFALLS #8).
- NMI line before poll: the poll reads the edge latch, so the latch must exist first. Steps 3 and 4 may share one PR if the reproducers check each sub-step.
- Hijack last among CPU steps: it needs the polled NMI from step 4 and the shared entry.
- Cap decision last: page 19 is slow because earlier rows consume the frame budget, so it changes only after steps 2 and 3.

**Research flags:**
- **Needs a probe before planning steps 2 and 7:** sprite-0 root cause is not isolated. Candidates: index-based `eval_n == 0` flag; first-opaque slot-order return in `compose_pixel` (ppu.c 402-419); the `video_output` gate; mid-VBlank rendering enable; `STX $4014` while rendering. Settle with a PC and `$2002` trace of subtest 1.
- **Needs a probe for step 3:** which CPU/PPU alignment AccuracyCoin assumes (PITFALLS #10; the AC README is silent). FEATURES reports that each table tolerates a one-dot shift, which may be enough. Test before adding any constant.
- **Needs an owned test for step 6:** the exact vector-choice cycle (see Disagreements).
- **Standard patterns, skip research-phase:** step 0 (harness) and step 5's I-flag snapshot (documented in NESdev CPU interrupts). Branch-poll details still need the owned tests from step 1.

## Disagreements Between the Four Files

| Issue | Positions | Resolution (better supported) |
|---|---|---|
| **(a) `Arbitrary Sprite zero`** | FEATURES: the `0x03` timeout is a budget artifact. Run to 8000 frames, it ends `fail 0x06`, the same missing basic hit. ARCHITECTURE: the timeout is a wait loop that spins because of slot-order return. STACK: it may clear from `$2002` timing, otherwise it is its own fix. | **FEATURES.** It is the only measured result (local observe runs). ARCHITECTURE's slot-order cause is a hypothesis. Both rows share the basic-hit fault, so fix that first (step 2). Do not raise the cap (`budgets.txt`). |
| **(b) Build order: poll first or NMI line first** | FEATURES: poll seam first (smallest visible wins). STACK and ARCHITECTURE: NMI line and edge first, then poll. | **Line and edge first.** FEATURES' seam already includes the edge latch, and NMI Control 8/9 read that latch. Poll-first would latch NMI with no edge source. |
| **(b) JAM tune-up position** | FEATURES: last (independent). PITFALLS: first. ARCHITECTURE: earliest suggested. | **First.** No hash impact, and it makes later failures diagnosable. FEATURES calls it independent but does not argue for last. |
| **(b) Sprite-0 position** | STACK and ARCHITECTURE: after NMI, because timing sub-tests need the alignment. FEATURES: independent. PITFALLS: separate plan. | **Split.** Basic hit early (step 2); dot-exact subtests late (step 7). |
| **(b) Hijack vector cycle** | ARCHITECTURE: "cycle 4/5". FEATURES and STACK: "after cycle 4 of 7". | **FEATURES and STACK**, both citing NESdev CPU interrupts. Step 1 tests cycles 1-7, as PITFALLS requires. |
| **(c) blargg ROMs** | PITFALLS #2 and #12: run ppu_vbl_nmi "if in manifest / add if licence allows". STACK: no licence, so never committed; optional fetch-only nightly lane. | **STACK.** The lane is optional. Consequence: dot-offset and odd-frame checks are unchecked unless owned tests cover them (step 1). |
| **(c) Campaign cadence** | PITFALLS: run the full 146-row campaign after every behaviour commit. STACK: the scoreboard regression check suffices. | **Both are satisfied.** PROJECT.md says the PASS set is protected on each CI lane. Add the named-row diff output that PITFALLS asks for. |
| **Hash count** | PITFALLS and PROJECT cite a 95-key list (Phase 10). ARCHITECTURE cites 114 rows in `tests/runner/hashes.txt`. | **ARCHITECTURE,** which read the file. Recount in step 1. |
| **Bases Loaded II** | FEATURES: `$00` then `$80` gives an extra NMI (verified). PITFALLS: "NMI at VBL end" claim unverified. | **Agree on the verified part only.** Do not use the unverified claim. |
| **Basic-hit root cause** | ARCHITECTURE: the `video_output` gate is a suspect. FEATURES: unit tests pass; cause not isolated. | **FEATURES on the status.** ARCHITECTURE's suspects are tested by the step 1 probe. |

## Confidence Assessment

| Area | Confidence | Notes |
|---|---|---|
| Stack | HIGH (repo); MEDIUM (blargg licence) | Repo files read directly; blargg licence from the project's 2026-10-02 survey, not re-fetched. |
| Features | MEDIUM-HIGH | Test intent and codes read from pinned asm (HIGH). Hardware windows are NESdev T2 (MEDIUM). Sprite-0 root cause not isolated. |
| Architecture | HIGH for current code; MEDIUM for target windows | Code read directly. Exact dots are pinned only once step 1 tests exist. |
| Pitfalls | MEDIUM-HIGH | Wiki facts and the ppu_vbl_nmi readme are HIGH. Alignment and frame-boundary facts are unverified. |

**Overall confidence:** MEDIUM-HIGH

### Gaps to Address

- **Sprite-0 basic-hit root cause:** unisolated. Trace probe in step 1 before any fix.
- **AccuracyCoin's alignment assumption:** unread in the AC asm; the README is silent. Read the asm of the race tests in step 1.
- **Frame boundary against the VBL race window:** PITFALLS #6 says this is unverified; read `src/frame.c`. A frame ending inside the window would put save states inside it.
- **blargg-only checks:** ppu_vbl_nmi 02, 05-10 and odd-frame skip are unchecked without the optional lane. Covered by owned tests in step 1, or recorded as unsupported.
- **Hijack cycle exactness:** resolved by owned tests at cycles 1-7 in step 1.
- **Page 19 budget:** re-measure after steps 2 and 3; decide the cap then.
- **Optional nightly `vbl-oracle` lane (STACK):** a recommendation, not a requirement. Defer for minimum scope; if built, record failing ROMs as `unsupported` first.

## Sources

- `.planning/research/STACK.md`, `FEATURES.md`, `ARCHITECTURE.md`, `PITFALLS.md`: read in full for this synthesis.
- `.planning/PROJECT.md`, Current Milestone section.
- AccuracyCoin asm and README at `6bee919529289c555803b612b8c9e4f7eaaeeabb` (MIT): specification and error codes. HIGH.
- NESdev wiki: PPU frame timing, NMI, CPU interrupts, PPU registers, Tricky-to-emulate games. MEDIUM (T2).
- Visual 6502 interrupt-recognition analysis, as mirrored on NESdev (HWC.17). MEDIUM.
- ppu_vbl_nmi readme, christopherpow/nes-test-roms (read as text; the ROM is not committed). HIGH for stated behaviour.
- Clean room: this synthesis consulted no GPL or LGPL emulator source.

---
*Research completed: 2026-10-25*
*Ready for roadmap: yes*
