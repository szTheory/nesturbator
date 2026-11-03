---
phase: 26-pal-dma-and-regional-state-continuation
plan: "01"
requirements-completed: [DMA-01]
---

# Phase 26 Plan 01: PAL DMC SYNC model

## Outcome

PAL ordinary DMC arbitration now waits for the opcode-dispatch/SYNC read. The
private read-kind argument is transient and adds no instance or serialized
state. NTSC eligibility and the existing OAM/abort paths retain their prior
logic. Public documentation describes the trace evidence, the proposed SYNC
mechanism and the limits separately; behavior revision is 17 while native
format 6 and ABI 1 remain unchanged.

## Model evidence

The generated in-memory PAL NROM guest configures the DMC through `$4010`,
`$4012`, `$4013` and `$4015`, then loops over `LDA $00` and `JMP`. It produces a
first sample GET from `$C000`, then a real refill request during an ordinary
read. With no prefix, the refill request appears at CPU cycle 749 during the
`LDA $00` data read at `$0000`; the correct model accepts the next eligible
opcode read at `$8014` on cycle 754, fetches `$C001` on cycle 757 and resumes
the parked read at `$8014` on cycle 758. With the three-cycle `BIT $00` prefix,
the request appears at cycle 749 during the `JMP` operand read at `$801A`; the
eligible opcode halt is `$8018` on cycle 754, followed by GET `$C001` on cycle
757 and resume at `$8018` on cycle 758. Both guests continue without JAM.

The wrong-model control changes only PAL halt eligibility, accepting any
ordinary read. It runs on the same guest and region from an instance snapshot
captured at the first GET. The no-prefix control halts on `$0000` at cycle 750,
gets `$C001` at 753 and resumes at 754. The prefixed control halts on `$801A`
at 750, gets `$C001` at 753 and resumes at 754. The request cycle, address,
APU phase, halt phase and state at the first GET match the correct run; halt,
GET and resume timing then differ. This makes the fixture sensitive to the
selected rule.

The ordinary linked CPU path and the source-included observed path are compared
byte-for-byte after every CPU step through the second GET. The guest bytes and
resolved PAL region are checked. The two observed halt alignments cover the
initial load halt phase (GET, phase 1) and the ordinary refill halt phase (PUT,
phase 0). In both cases the request is deferred from a generic read and later
accepted on an opcode fetch.

Separate classification probes retain a real pending request from guest/APU
history, begin IRQ or NMI entry, and check that C1 is passed through the SYNC
read seam. Both traces report C1 at `$8016` as SYNC and record a halt attempt;
neither happens to meet the request's phase on that edge, so neither accepts a
halt there. This checks the report's interrupt interpretation without claiming
that the owned probe is an interrupt hardware test.

## Source and claim crosswalk

| Claim | Evidence and scope |
|---|---|
| Observation | The [PAL DMA forum report](https://forums.nesdev.org/viewtopic.php?p=300930) describes captured 2A07 halt-address sequences. This project did not capture those traces. |
| Mechanism interpretation | The report's authors attribute the observed first-instruction-cycle halt to SYNC rather than R/W; the forum follow-up applies the SYNC interpretation to IRQ/NMI C1. This is an interpretation of the reported trace. |
| Model implemented | PAL ordinary pending requests are eligible at opcode dispatch and the report-interpreted IRQ/NMI C1 read. NTSC ordinary reads remain eligible as before. |
| Regression oracle | An owned PAL guest and any-read counterfactual exercise the selected distinction through CPU/APU/bus code. This validates implementation sensitivity, not hardware accuracy. |
| Unresolved | Later held no-op-cycle bus addresses/register effects and direct project-run hardware proof remain unverified and are not asserted. Other unsettled PAL abort/reload behavior is outside this oracle. |

The public contract links the report and the [NESdev DMA](https://www.nesdev.org/wiki/DMA)
and [DPCM conflict](https://www.nesdev.org/wiki/DPCM_conflict) references. No
later no-op address, register side effect or physical console comparison is an
acceptance assertion. Phase 27 still owns PAL presentation and final NTSC
acceptance.

## State-pin reconciliation

The runner's existing state-at-frame route saved old and revision-17 states at
frames 0 and 60 for `dabg`, `m1sxrom`, `m2`, `m3`, `m4tkrom` and `m7`. For all
12 pairs, sizes were unchanged; header size remained 64, format remained 6,
region byte 23 remained NTSC/0, and ticks were identical. The only differing
byte offsets were 16 (behavior revision 16 to 17) and 28–31 (the consequent
state CRC). The frame, PCM, transition and state-size pins were not changed.

All 12 old NTSC format-6 states were loaded by the revision-17 runner and
completed a bounded run successfully. The updated state SHA rows in
`tests/runner/hashes.txt` are therefore attributable to revision metadata and
its checksum only; the saved files and temporary comparison script remain in
ignored build scratch.

## Checks

- `cmake --preset dev` completed.
- `cmake --build --preset dev --target bus.unit core.api header.c header.cxx`
  completed.
- `ctest --test-dir build/dev --output-on-failure --no-tests=error -R
  '^(bus\.unit|core\.api|header\.(c|cxx)|vectors\.api_policy(\.self_test)?|docs\.links)$'`
  passed all 7 tests in 0.94 seconds. `bus.unit` took 0.30 seconds in this run.
- `git diff --check` on the Plan 26-01 implementation, tests, docs and pins
  passed.

The API, C/C++ header, documentation-link and normalized declaration-policy
checks also passed. The final unfiltered local workflow is intentionally
deferred to Plan 26-02, which adds regional state continuation and adapter
coverage before the single required full-CI run.

## Remaining boundaries

The DMA-01 spec-less edge and transparency prohibition remain flagged and
unverified. The model is based on the cited report and its stated
interpretation; it is not direct hardware proof or a complete PAL DMA
contract. See [26-02-SUMMARY.md](26-02-SUMMARY.md) for the final combined
candidate, state behavior, review and unfiltered CI receipt.
