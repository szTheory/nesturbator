---
title: "Phase 21 native interrupt-entry state matrix"
summary: "Source-established legal cutpoints and CRC-correct rejection controls for serialized interrupt-entry history."
read_when: "Before changing serialized CPU entry fields, their validation, or reset behavior."
updated: 2026-10-29
---

# Native interrupt-entry state matrix

The serialized entry fields describe the **next unperformed bus action** after
all earlier actions completed. The legal rows below follow
`nesturbator__cpu_entry_begin` and `nesturbator__cpu_entry_action` in
`src/cpu.c`; the load gate is `cpu_entry_state_valid` in `src/state.c`. The
owned `core.state_reject` rows advance through those production functions and
save each cutpoint. They do not write stack bytes or entry fields to create a
legal state.

## Reachable entry cutpoints

| Origin | Next action | Selected vector | Low byte | Source-established boundary |
|---|---:|---|---|---|
| Idle | 0 | none | 0 | No active continuation; the four entry fields are canonical. |
| BRK | 2 | none | 0 | BRK opcode fetch (C1) completed; padding read C2 is next. |
| BRK | 3 | none | 0 | C2 padding read completed. |
| BRK | 4 | none | 0 | C3 PCH push completed. |
| BRK | 5 | IRQ or NMI | 0 | C4 PCL push and vector decision completed. |
| BRK | 6 | retained pair | 0 | C5 status push completed; I is set. |
| BRK | 7 | retained pair | selected low byte | C6 selected-vector low read completed. |
| IRQ | 1 | none | 0 | Before C1 discarded read. |
| IRQ | 2 | none | 0 | C1 read completed. |
| IRQ | 3 | none | 0 | C2 read completed. |
| IRQ | 4 | none | 0 | C3 PCH push completed. |
| IRQ | 5 | IRQ or NMI | 0 | C4 PCL push and vector decision completed. |
| IRQ | 6 | retained pair | 0 | C5 status push completed; I is set. |
| IRQ | 7 | retained pair | selected low byte | C6 selected-vector low read completed. |
| NMI | 1 | NMI | 0 | Before C1 discarded read. |
| NMI | 2 | NMI | 0 | C1 read completed. |
| NMI | 3 | NMI | 0 | C2 read completed. |
| NMI | 4 | NMI | 0 | C3 PCH push completed. |
| NMI | 5 | NMI | 0 | C4 PCL push completed. |
| NMI | 6 | NMI | 0 | C5 status push completed; I is set. |
| NMI | 7 | NMI | selected low byte | C6 NMI-vector low read completed. |

After C7, `cpu_entry_action` clears all four entry fields to the idle row.
At `next_action == 7`, the low-byte field is the actual byte read from the
selected pair and may be any byte value, including zero. Before that point it
must remain zero. IRQ and BRK have no selected pair through completed C4; the
decision is made after C4 and before C5, then stays coherent for both vector
reads. An NMI-origin entry always retains the NMI pair.

An NMI input that arrives during an older entry after its C4 decision is
an inhibited candidate while the old vector remains selected. `core.state_reject`
creates this case by raising the PPU NMI test pin before a production C5 bus
action; that bus cycle creates the CPU latch. The separate
`core.interrupt_entry` scheduled-edge cases exercise vector selection at C4.
Both cases preserve the distinction between an edge consumed for vector
selection, a recognized fresh edge, and an inhibited input that is not yet
recognized. The existing `nmi_pending` byte uses 0 for no request, 1 for a
recognized sticky request, and 2 for an inhibited candidate. No state byte is
added; format 3, behavior revision 9 and the public ABI remain unchanged.

A candidate 2 is reachable only after C5/C6 (next action 6/7), or idle after
C7 before the first handler opening. Its last sampled line must be asserted,
and its NMI poll, IRQ poll/acceptance and JAM markers must be clear. The current
PPU line may already be released. A later low sample cancels it; after C7 only
a still-held input is recognized at the first handler opening, before a branch
C1 poll. Recognized request 1 retains the historical line-independent contract.
The maximum byte value is therefore tested by these conditional reachable
siblings, rather than as an unconstrained one-field-at-bound mutation.
BRK/IRQ consume a recognized request at their C4 vector decision, so their
post-choice stages cannot carry recognized value1; a fresh input there is a
candidate2. NMI-origin entry can carry a distinct request recognized in its
eligible C1-C4 window, including through the later inhibited stages.

Actual-adapter runahead separately loads four production-created candidate-2
histories: active C6 and idle after C7, each held and released. The real
`retro_unserialize`/`retro_serialize` round-trip echoes every cutpoint byte before
execution. Both scripts and depths one through three compare all committed and
speculative frame/audio/state/timing outputs with native continuation, with
absolute first-frame IRQ marker1 and NMI marker1 for held/0 for released.
These24 additional history/script/depth cases preserve the existing20 base
cutpoints,60 depth cases,120 script cases and all four adapter controls.

The historical detector and poll latches are independent of the current PPU
input. `core.state_reject` creates a vblank/NMI pin transition, samples it
through a production bus cycle, lowers the live input, and samples again. It
saves the retained pending/polled latches with the live NMI line low. The CPU
latch bytes are never seeded directly.

## Rejection controls and their legal siblings

Every invalid row repairs the state CRC before calling the production loader.
`st_refused_with` checks the intended status, byte-for-byte native instance and
cartridge RAM preservation, and `save_generation`. The test then executes the
next production entry action in both the refused instance and a fresh instance
loaded from the legal sibling, and compares their serialized states.

| Invariant | Legal sibling | CRC-correct invalid control |
|---|---|---|
| Idle continuation is canonical | Idle `0/none/0` | Nonzero next action, vector enum, or low byte while idle |
| Origin range and action range | IRQ C1 | Origin 4; active next action 0 or 8 |
| BRK starts after its opcode fetch | BRK C2 | BRK next action 1 |
| Pair selection follows C4 and remains present | IRQ C4 (`next=4`, none) and IRQ C5 (`next=5`, selected) | Selected pair before C5 or none at C5 |
| NMI origin is fixed to its pair | NMI C1 and NMI C6 | IRQ/none vector for NMI origin |
| Vector enum is bounded | IRQ C1 | Vector enum 3 |
| Low byte is absent until the C6 read completes | IRQ C6 and NMI C6 | Nonzero low byte while `next_action=6` |
| Active entry excludes jam and poll/accepted markers | BRK C2 | JAM, `nmi_polled`, `poll_latch`, or `irq_accepted` set |
| An inhibited input is not a recognized poll | IRQ C6 with selected IRQ, sampled-high candidate 2 | Candidate with sampled line 0, NMI poll 1, JAM 1, or pre-C5 stage rejected |
| A candidate can outlive today's input at a cutpoint | IRQ C6 candidate 2 with live input released | Save/load round-trip and cancellation on subsequent production sample |
| Idle candidate precedes any handler opcode/arbitration | Candidate 2 sampled at C7, continuation idle, current input released | CRC-correct accepted IRQ, JAM, raw IRQ/NMI poll, or sampled-low neighbors refused before APPLY |

Old-format precedence remains checked separately: CRC-correct formats 1 and 2
with a bad header size and cartridge identity still return FORMAT before any
later validation result. These checks remain in `case_two_faults` in
`tests/core/test_state_reject.c`.

## Reset and generation lifecycle

`core.state_generation` creates every active BRK/IRQ/NMI next-action cutpoint
through production actions, including each BRK/IRQ-selected pair. It loads the
cutpoint into a matched control with only the continuation cleared, resets both,
and compares canonical state plus the next ordinary instruction. Reset cases
check seven CPU cycles, the reset vector, I, S minus three, unchanged stack RAM,
and an ordinary `LDA #$A5` continuation with no entry service. The test also
preserves the existing battery-span generation cases and adds same-state and
refused-load generation controls for mappers 0, 1, 2, 3, 4, and 7. On each
family, successful same-state loads and CRC-refused loads leave the generation
unchanged; refusal leaves the full native instance and cartridge RAM unchanged.

No public behavior change is intended by this plan; it records and tests the
state contract introduced in Plan 21-04.
