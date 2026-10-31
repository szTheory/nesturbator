# Phase 24-02 bus-latch decision — approved

**Position:** OAM Task 1 and its collision continuation are implemented; bus-conflict Task 2 has a failing owned latch diagnostic and is stopped before any new saved-field/layout/meaning change. Plan 24-02 is incomplete. The earlier owner approval covered the DMC silence byte/bit count only.

## Concrete failure and why a second value is needed

The source model distinguishes the external data bus from the internal CPU data latch. `$4015` reads internal status and does not drive the external bus; it retains internal bit 5. A DMC sample GET can drive different external bits while the CPU is parked at `$4015`. The next status read must retain the internal bit, while the next controller read must merge the external sample's high bits. Consecutive status reads are relevant to indexed read-modify-write's dummy/effective accesses; the latch cannot be merely a temporary inside one read call.

The owned diagnostic runs the production bus source, with parked `$4015`, initial internal D5=0, sample GET `$FF15/$A5` on GET, then another status read and a controller read. Current results:

| Observation | Current collapsed latch | Source-supported separate latches |
|---|---|---|
| First CPU `$4015` result | `$20` | `$00` |
| Second CPU `$4015` result | `$20` | `$00` |
| Retained external sample latch | `$20` | `$A5` |
| Following controller result | `$21` | `$A1` |

`bus.unit --dma-trace conflict-latches` intentionally exits 1 on all four assertions. The actual mapper address/data and GET phase are observed separately. This is a bounded model regression, not an AccuracyCoin execution or new silicon measurement. [Pinned assembly](https://github.com/100thCoin/AccuracyCoin/blob/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm#L11436-L11467) explicitly states `$4015` does not update the data bus and limits its own evidence to CPU-visible values, not the DMC shift-register input. [DMA revision 23450](https://www.nesdev.org/w/index.php?title=DMA&oldid=23450) explains split register selection and status/controller conflicts.

One byte cannot retain both internal D5=0 and external sample D5=1 across these bus calls. Using a file-static/global latch breaks instance locality; a function-local latch loses history between reads; guessing equality at load discards the observed distinction. Preserve both values in the instance and native state.

## Proposed field/version diff — not implemented

Offsets are local to the serialized BUS visitor, excluding chunk/header envelopes.

| Field | Current format 4 | Proposed format 5 |
|---|---|---|
| `open_bus`, u8 at 2048 | Unified last read/write value; `$4015` currently replaces it | External held data only; an internal-only `$4015` read leaves it intact |
| `cpu_data`, new u8 at 2049 | Absent | Internal CPU data latch, including `$4015` return and retained internal D5; range 0..255 |
| `apu_get_put_phase` through controller strobe | Offsets 2049..2058 | Same widths/meaning, shifted to 2050..2059 |
| Serialized BUS length | 2059 bytes | 2060 bytes |
| C BUS struct size | 2061 bytes | 2062 bytes |
| DMC silence/bit state | Format-4 fields | Unchanged |
| Native format | 4 | 5 |
| Behavior revision | 11 for the OAM correction | 12 when split-bus correction lands |
| C ABI / cartridge battery-save layout | Existing | Unchanged |

The existing `open_bus` meaning is made explicitly external as part of this gated proposal; this is not hidden as a calculation-only change. The loader visitor, validation/apply paths, refusal tests, both-latch native continuation, README/header comments and necessary current-version/hash pins must change together. No old-format converter is proposed. Other future saved-field changes are not authorized by this decision.

**User consequence:** format-4 native states made with this unreleased branch would also be refused by the corrected format-5 loader, in addition to the format-3 states already refused under the previous decision. It returns `NESTURBATOR_ERR_STATE_FORMAT` and leaves the instance unchanged. No existing state/battery file is deleted or rewritten. Actual builds/releases remain separately identified; no release is published here.

## Required decision

Recommended: approve the explicit internal/external latch split, the new serialized `cpu_data` byte, format 5 and behavior revision 12, with full bounded continuation/refusal/regression/documentation follow-through. The alternative is to defer the bus-conflict correction and retain format 4; Phase 24 and dependent acceptance remain incomplete. Reusing bits of another saved field would still change its meaning and require a format decision.

Plan 24-02's execution contract says: “A newly required on-disk change is a one-way door: STOP before modifying that layout or meaning.” This checkpoint follows that required boundary; the new latch and format-5 serializer have not been implemented. The full-CI/AccuracyCoin stop remains active without reconfirmation. No campaign, workflow, push or PR is authorized by this decision.

After the owner decision, `$gsd-execute-phase 24` resumes the saved Plan 24-02 continuation. Read STATE and 24-EXECUTION-RESUME.md first; context may be cleared. The owner acts next, and execution waits for this concrete compatibility decision.

## Saved execution outcome

OAM correction/continuation and behavior11 consumers are committed in `8828225`; bounded tests and resolved review are recorded in 24-02-PARTIAL.md. The default suite passes and the separate conflict-latches diagnostic remains intentionally red. The owner acts next on this new field diff. No bus-latch implementation has occurred before this decision.

## Owner approval

On 2026-10-31 the owner explicitly approved this concrete proposal in chat (“Yeah, I approve it.”): separate serialized cpu_data, external open_bus meaning, format5, behavior12, native continuation/refusal tests and documentation. This resolves this checkpoint only; no abort-state changes, full CI/AccuracyCoin, push or PR authority is included. Continue at Task2 without repeating the OAM correction.

## Byte-count correction before implementation

The original table mistakenly included input_pending[2] in the serialized BUS length. Those two host-owned bytes are not visited by bus_io. The exact serialized change is 2059->2060, while the C struct is 2061->2062; downstream visitor offsets end at2058->2059. The approved cpu_data field, one-byte delta, meaning change, format5/revision12 and incompatibility consequence are unchanged. No new scope or decision is introduced by correcting this accounting.
