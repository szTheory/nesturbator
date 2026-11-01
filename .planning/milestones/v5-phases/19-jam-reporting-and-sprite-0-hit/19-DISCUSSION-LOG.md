# Phase 19: JAM reporting and sprite-0 hit - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-10-25
**Phase:** 19-jam-reporting-and-sprite-0-hit
**Areas discussed:** JAM attribution contract, Run-ahead across a JAM, Sprite-0 probe method, Sprite-0 model and fix bounds

The owner asked for every decision point to be fanned out (stakeholder lenses, primary sources, adversarial pass) and for one coherent recommendation per point to be adopted without further questions. Four research agents ran in parallel; their recommendations were checked against the code and synthesized.

---

## JAM attribution contract

| Option | Description | Selected |
|--------|-------------|----------|
| In-memory one-byte patch of the pinned ROM, real observe path | `accuracycoin.conformance jam` patches `$02` at an inventory routine address and runs `accuracycoin_run_observe_page` | ✓ |
| Synthetic ROM plus classifier mutation | Small, but has no AccuracyCoin menu, so it cannot show "an AccuracyCoin run that stops on a guest JAM" | |

**Choice:** In-memory patch. `guest-jam` verdict and cause; the row scored `fail/0x03/guest-jam`; `jam_frame` metadata; the collector's own checks act as the control.
**Notes:** Scoreboard format and result enum unchanged; no runner flag; no committed ROM bytes.

---

## Run-ahead across a JAM

| Option | Description | Selected |
|--------|-------------|----------|
| Extend `libretro.runahead` only, with an adapter presentation fix | Synthetic VBlank-counted JAM ROM, JAM-aware oracle, existing controls plus a black-oracle control | ✓ |
| Also mirror at core level | Duplicates `test_state.c` latch coverage, and cannot test the picture | |

**Choice:** Extend `libretro.runahead`. A frame-number-tagged presentation cache narrows Phase 18 D-10.
**Notes:** Verified in code that `retro_unserialize` invalidates the cache, which would show black after a JAM under run-ahead.

---

## Sprite-0 probe method

| Option | Description | Selected |
|--------|-------------|----------|
| Scratch instrumented copy, then owned reproducers | Zero tracked or API change | ✓ (done during discussion) |
| New public trace or debug API | ABI and docs cost for a one-off question | |

**Choice:** Scratch probe. It found the cause: the basic hit works; re-runs caused by a guest stack bug fail because OAMADDR is never reset during dots 257-320.
**Notes:** A one-line scratch fix confirmed it. Verified by grep that nothing in `src/` resets `oam_addr` during rendering.

---

## Sprite-0 model and fix bounds

| Option | Description | Selected |
|--------|-------------|----------|
| Keep `eval_n`/`eval_m`, start at OAMADDR, identity `dot == 66`, three misaligned fixes | No state format change; aligned behaviour identical | ✓ |
| Single 8-bit evaluation address | Closer to hardware, but bumps the state format and pulls in `$2004` rows | |
| Move the hit off the `video_output` gate | Guards a NULL-video path that `nesturbator_run_frame` forbids | |
| Keep the hit gated, with a comment | Remove the need | ✓ |

**Choice:** As selected; revision 6→7 in Phase 19, state format stays 1; one phase PR with ordered commits.
**Notes:** Adversarial cases each get an owned test (D-10, D-11). A suite-restart harness guard was considered and deferred (D-15).

## Claude's Discretion

- Helper names, fixture mutation spellings, the synthetic ROM's backdrop sequence, and shared message formatting.

## Deferred Ideas

- Suite-restart guard; 2C02G OAM corruption; `$2004` during rendering; a single evaluation address; a per-dot sprite-0 check off the video path; a public trace API.
