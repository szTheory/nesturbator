# Phase 25 PAL Evidence

**Reviewed:** 2026-11-01
**Status:** Source-backed model dispositions reviewed for Plan 25-01. These are derived from pinned documentation and are not direct PAL hardware verification.

## PAL native row mapping

| Source pin | Tier | Access date | Supporting fact |
|---|---|---|---|
| [PAL video, revision 23684](https://www.nesdev.org/w/index.php?title=PAL_video&oldid=23684) | T2 community reference | 2026-11-01 | Lists 312 total scanlines and 240 rendering rows numbered 0–239. The active interval is dots 5–256; its note says line 0 is entirely cropped by the border. |
| [BreakingNESWiki PAL PPU circuit notes, commit `5a55bac`, V Decoder](https://github.com/emu-russia/breaks/blob/5a55bac/BreakingNESWiki_DeepL/PPU/pal.md#v-decoder) | T1 circuit analysis | 2026-11-01 | The 2C07 V Decoder table places `/PICTURE` transitions at lines 1 and 240. |
| [BreakingNESWiki PAL PPU circuit notes, commit `5a55bac`, Video Out](https://github.com/emu-russia/breaks/blob/5a55bac/BreakingNESWiki_DeepL/PPU/pal.md#video-out) | T1 circuit analysis | 2026-11-01 | `/PICTURE` receives an output delay; Color Buffer Control receives the signal in unmodified form. |

**Disposition and derivation:** Keep the native buffer in physical row order 0–239. The chart's 239-line PAL picture is physical rows 1–239: the top row is border-covered, while row 239 remains the final physical rendering row with active interior pixels. The native seam stores palette-index/raw-emphasis pixels before analog output decoding, so border coverage does not justify dropping, shifting, duplicating or filling row 0 or row 239. The fixed 256×240 contract remains intact. This mapping is inferred from the two source pins; it is not a capture of a PAL console or display.

**Owned row-marker fixture:** Generate a small NROM image and CHR/nametable pattern in the test itself. Give physical rows 0, 1, 238 and 239 distinct palette markers across a known interior span, with distinct guard pixels at the horizontal edges. With rendering enabled, assert each marker at its same-numbered native output row and assert active samples on row 239. With rendering disabled, assert the backdrop and no stale markers; surround the 256×240 output with canaries. The fixture distinguishes physical rows from the analog border and never uses emulator output as its oracle.

## PAL APU frame-counter edges

| Source pin | Tier | Access date | Supporting fact |
|---|---|---|---|
| [APU Frame Counter, revision 23448](https://www.nesdev.org/w/index.php?title=APU_Frame_Counter&oldid=23448) | T2 community reference | 2026-11-01 | Labels the PAL schedule in APU cycles with GET/PUT phase; states that an APU cycle spans two CPU cycles. It specifies the `$4017` 3/4 CPU-cycle reset delay and explicitly says PAL behavior is assumed to match. |
| [Project CPU/APU preparation, HWC.09](../../preparation/NES-HARDWARE-CPU-APU.md) and [`src/apu.c`](../../../src/apu.c) | T2 source map / T1 current implementation | 2026-11-01 | HWC.09 pins the same wiki revision. The existing CPU cursor starts at zero for a reset sequence and increments before its threshold comparisons; its NTSC thresholds match the same GET/PUT conversion. |

**Count origin and conversion:** Use the selected model's post-reset CPU cursor: `frame_cycle = 0` at sequence reset; each later CPU clock increments before comparison. For source APU cycle count `A`, map GET to CPU cursor `2A` and PUT to `2A + 1`. The NTSC cross-check is 3728 PUT → 7457, 14914 GET/PUT → 29828/29829, and 14915 GET → wrap at 29830, matching the current cursor convention. PAL expectations below are calculated from the pinned source counts and that convention, not copied from a production PAL timing table.

| PAL source edge | CPU cursor | Four-step behavior | Five-step behavior |
|---|---:|---|---|
| 4156 PUT | 8313 | Quarter clock | Quarter clock |
| 8313 PUT | 16627 | Quarter + half clocks | Quarter + half clocks |
| 12469 PUT | 24939 | Quarter clock | Quarter clock |
| 16626 GET | 33252 | Enabled frame-IRQ flag edge | No clock |
| 16626 PUT | 33253 | Quarter + half clocks; enabled frame-IRQ flag edge | No clock |
| 16627 GET | 33254 | Final enabled IRQ assertion, then wrap to zero | Continue sequence |
| 20782 PUT | 41565 | Not reached | Quarter + half clocks |
| 20783 GET | 41566 | Not reached | Wrap to zero; no frame IRQ |

The resulting event sets are: four-step quarter clocks at 8313, 16627, 24939 and 33253; half clocks at 16627 and 33253; enabled IRQ edges at 33252, 33253 and 33254, with wrap at 33254. Five-step mode's final quarter/half clock is 41565 and its wrap is 41566. Preserve the source count origin and GET/PUT phase when converting these values into tests.

**Register-edge qualification:** The pinned reference says `$4017` takes effect three CPU cycles after a write made during an APU cycle and four cycles after a write between APU cycles; it explicitly labels equal PAL behavior as an assumption. Keep that assumption visible in the implementation and tests. Preserve existing `$4015` read/acknowledgment and `$4017` inhibit ordering in tests immediately before, on and after these modeled edges. The table does not measure a PAL subcycle bus race, and it does not establish direct 2A07 behavior.

**Owned event fixture:** Use generated code and direct test instrumentation to select four- and five-step modes, set the GET/PUT phase, and record the first event cursor on each side of every listed threshold. Test `$4015` reads and `$4017` writes adjacent to the IRQ and wrap edges using the existing bus/register ordering. The expected cursors come from the source table plus the derivation above; no emulator comparison or output hash is an oracle.

## Exact PAL timing and sample arithmetic

The pinned cycle references and project time-unit definition support this integer model:

- A PAL CPU cycle is 16 master-clock periods, or 32 half-master-clock ticks. The 2A07 M2 low portion is 13/32 of the cycle and the high portion is 19/32, so its edges are at `+13` and `+32` ticks. One PPU dot is five master-clock periods, or 10 ticks.
- A PAL frame is `312 scanlines × 341 dots × 10 ticks/dot = 1,063,920 ticks`. There is no PAL odd-frame dot skip in this model.
- The PAL master clock is 26,601,712.5 Hz; the half-master-clock timebase is therefore `53,203,425 ticks/second`. Dividing by 1,063,920 and reducing by 165 gives exactly `322,445/6,448 frames/second`.
- At the fixed 48,000 Hz output rate, the sample fraction is `48,000/53,203,425 = 640/709,379 samples/tick` after reducing by 75. One frame yields `640 × 1,063,920 / 709,379 = 680,908,800/709,379 = 959 + 614,339/709,379` samples. With the per-instance remainder carried forward, frames produce 959 or 960 samples; this fraction does not produce a strict every-other-frame sequence.

The clock and dot inputs are the pinned NESdev [cycle reference chart, revision 22030](https://www.nesdev.org/w/index.php?title=Cycle_reference_chart&oldid=22030) and the PAL scanline table above. The half-master-clock conversion and reduced frame/sample fractions are separate arithmetic derivations. The project preparation records the same clock facts in [ARCHITECTURE §2](../../preparation/ARCHITECTURE.md), [HWC.03/HWC.04](../../preparation/NES-HARDWARE-CPU-APU.md), and [HWP.10](../../preparation/NES-HARDWARE-PPU-CARTRIDGE.md).

## Evidence boundary

These dispositions resolve the specified picture-row representation and PAL APU cursor model from pinned documents. They do not claim direct console measurements, universal PAL accuracy, 2A07 DMA behavior, PAL color calibration, PAL save-state continuation, or hardware-confirmed `$4017` subcycle races. PAL save-state continuation remains unsupported until Phase 26. AccuracyCoin is NTSC-focused and was not run or used as a PAL oracle. No emulator output is used as evidence.

**Review result:** The pinned PAL video table, circuit decoder and APU frame-counter table support the stated row and cursor dispositions. No factual contradiction was found that blocks dependent source-derived fixtures; the `$4017` PAL-delay assumption and all hardware-proof limits remain explicit.
