# Phase 24 native-state decision — blocking

**Position:** Plan 24-01 Task 1, before a behavior correction. No task or requirement is complete. No format, behavior revision, serialized field, or emulation behavior has changed. This checkpoint does not authorize full CI, AccuracyCoin, a push or a PR.

## Concrete finding

The current DMC output unit only reloads `bits` when `buffer_empty == 0`. Once a sample ends and its buffer is empty, `bits` stays zero across timer clocks. When another byte arrives it is consumed on the next timer clock, losing the phase of the intervening eight-clock silent output cycles.

An independent owned probe starts with `nesturbator_create` / cartridge load and executes NOP instructions through the production CPU/bus/APU, without assigning any DMC field or GET/PUT phase. Timer reload edges at CPU-cycle snapshots 1, 429, 857, 1285, 1713 and 2141 all retain `bits: 0 -> 0`, empty buffer and zero remaining bytes. The named wrong model is **freeze-at-zero-while-empty**. The probe fails because that model is the current implementation. Normal and instrumented instances agree.

Reproduce: `cmake --build build/phase24-owned --target bus.unit && build/phase24-owned/tests/bus.unit --dma-trace silence` (current expected exit 1). The build-tree log is `build/phase24-owned/phase24-silence-baseline.txt`.

Independent reference: [blargg APU reference, Output Unit](https://www.nesdev.org/apu_ref.txt), describes complete sample bytes or equal-duration silences; its counter advances through both, while a separate silence flag inhibits DAC changes. The [pinned circuit analysis](https://github.com/emu-russia/breaks/blob/5a55bac/BreakingNESWiki_DeepL/APU/dpcm.md) also distinguishes the sample bit counter, buffer and output control. These are source/model claims, not new hardware measurements.

## Landing trace and limits

The pinned test failure encoding `(ErrorCode << 2) | 2` maps historical raw `0x26` to Landing Test 9. The owned reconstruction originally used two incorrect addressing modes in the sync helper. Those were corrected before this conclusion: its initial result load is zero page (3 cycles); the final Copy_A load is absolute (4 cycles). The corrected trace now observes the expected write retry on `$5000`, followed by a four-cycle PUT-start DMA. Therefore the earlier theory of an incorrectly early fresh-load halt is withdrawn.

The subsequent reload occurs only 38 CPU cycles after `$4015` in that reconstruction, instead of the pinned 413-cycle interval, and its final open-bus assertion returns `$40`. The stopped bit counter is an independently reproduced mechanism consistent with that early reload. The approximate Test 8 frame wait and reconstructed helper bus patterns mean the full row's causal attribution is still provisional. No exact pinned ROM execution or new PASS/FAIL row is claimed. The correction must pass the independent silent-clock fixture and a tightened landing trace before it can be accepted; the five-row localization and later plans remain incomplete.

## Proposed field and version diff — not implemented

The serialized DMC visitor currently writes 22 bytes. DMC-local serialized offsets below exclude the enclosing APU/state envelope.

| Field | Current format 3 | Proposed format 4 |
|---|---|---|
| `bits`, offset 14, u8 | Remaining audible shift bits; zero waits indefinitely for a full buffer | Position in a continuously clocked eight-bit output cycle, including silent cycles; initial zero allowed until the first clock |
| `silence`, new u8 at offset 15 | Absent; cannot be reconstructed reliably from the shift register, whose zero is valid audio data | Boolean 0/1, latched when an output cycle starts; suppresses DAC updates while bit phase continues |
| `sample_buffer` through `enable_delay` | Offsets 15–21 | Same meanings and widths, shifted to offsets 16–22 |
| DMC serialized size | 22 bytes | 23 bytes |
| `NESTURBATOR_STATE_FORMAT_VERSION` | 3 | 4 |
| `NESTURBATOR_BEHAVIOUR_REVISION` | 9 | 10 when the correction lands |
| C ABI/public cycle API | Existing | No change proposed |

All other field changes discovered during implementation require a fresh concrete disposition. Later collision/conflict/abort work is not pre-authorized to reinterpret additional fields by this decision.

**User consequence:** the corrected loader would refuse existing format-3 native save states with `NESTURBATOR_ERR_STATE_FORMAT`; there is no converter. Existing cartridge save data is a separate format. No existing save files are deleted or rewritten by this decision. Native continuation tests must cover reachable silent output phase and compare uninterrupted/resumed effects, video, PCM/transitions and final bytes. Refusal tests must prove failed old-format load leaves the instance unchanged. README and public-header comments accompany the correction.

## Decision needed

Recommended: approve the explicit `silence` field, continuously clocked `bits`, state format 4 and behavior revision 10 for this correction, with the native continuation/refusal tests and documentation above. This preserves complete internal state and makes incompatibility explicit.

Alternative: defer this correction and retain format 3. Phase 24 remains incomplete and dependent corrections stay gated; this is not permission to keep the incorrect behavior and call the phase complete. Encoding silence into existing bytes still changes their meaning and requires the same format bump, so it does not preserve compatibility.

The execution contract in [24-01-PLAN.md](24-01-PLAN.md) requires: “A newly required on-disk change is a one-way door: STOP before modifying that layout or meaning.” The [execute-phase skill](~/.agents/skills/gsd-execute-phase/SKILL.md) requires preserving checkpoint handling. Owner approval is needed for the concrete compatibility consequence, not to keep the existing campaign stop active.

## Verification at this checkpoint

- Read-only generated CTest JSON selects exactly `core.apu` and `bus.unit`; neither has a fixture dependency. No campaign is selected.
- `ctest --test-dir build/phase24-owned -R '^(bus\.unit|core\.apu)$' --no-tests=error --timeout 30 --output-on-failure`: both passed after the last code edit.
- `--dma-trace silence`: intentional exit 1 on the independent silent-clock assertion; six stopped-bit-counter observations.
- `--dma-trace landing`: intentional exit 1 on the unresolved final open-bus/late-halt assertions; corrected addressing modes included.
- Production preprocessing contains no `NESTURBATOR_DMA_` observer tokens; hooks compile away.
- No full workflow, AccuracyCoin ROM, campaign, push or PR ran. No fresh compatibility or phase acceptance is claimed.

After an explicit decision, resume with `$gsd-execute-phase 24`. Context may be cleared; first read STATE and its linked execution resume. The owner acts next; no automatic continuation or test restart is authorized while this decision is pending.
