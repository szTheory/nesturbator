# AccuracyCoin bus-conflict divergence

## Finding

The `$4015` conflict path acknowledges the frame IRQ correctly but the next CPU-visible read can return the wrong latch. For an external DMC sample address whose low five bits select internal `$4015` while the CPU is parked at `$4000`, the GET leaves the external sample in `bus.open_bus` and the internal status read updates `bus.cpu_data` to status (`$50` in the current fixture). When RDY releases, the resumed `$4000` read currently returns that status byte. AccuracyCoin's pinned answer table requires the sample byte there: `$00` at sample `$FFD5` and `$FF` at `$FFF5`.

This is a source-backed explanation for the `DMC DMA Bus Conflicts` failure, and it identifies a wrong independent oracle in `tests/cpu/test_bus.c`. It does not prove this is the only failing access in the campaign row: the full test stores 64 values, and the campaign result alone does not show the first mismatching index.

## Evidence chain

The pinned `TEST_DMABusConflict` at AccuracyCoin commit `6bee919` establishes these behaviors in its comments and executable checks:

1. Register selection combines the parked CPU address high bits with the DMC sample address low five bits. Parked `$4000` plus external `$FFD5` therefore selects internal `$4015` (`TEST_DMABusConflict`, around lines 11450–11459; the repository hardware note also records this split select at [NES-HARDWARE-CPU-APU.md](../../preparation/NES-HARDWARE-CPU-APU.md#22-dma)).
2. `$4015` is read for its internal status/IRQ side effect, and the assembly explicitly says that read does not update the data bus (around line 11460). The test says it checks the CPU-visible half of the conflict, not the DMC shift-register input (around lines 11441–11447).
3. The ROM sets the sample address to `$FFC0`, loops a 65-byte DMC sample, and stores 64 resumed `$4000` reads at `$0500..$053F` (around lines 11487–11516). Its answer loop checks every offset except `$16` and `$17`: indices below `$20` must be `$00`, and indices at or above `$20` must be `$FF` (around lines 11561–11583). Thus offset `$15` is checked as `$00` for `$FFD5`, and `$35` is checked as `$FF` for `$FFF5`; only offsets `$16/$17` are validated through the controller-result checks below.
4. Both controllers are checked separately with their floating bits forced low and high (around lines 11523–11553 and 11585–11599). Afterward the ROM reads `$4015` and requires the frame-counter IRQ flag to have been cleared (around lines 11602–11608).

The live bus path matches the internal/external distinction up to the resumed read. `dmc_sample_get()` first puts the mapped sample in `open_bus`, then performs the split-address register read for `$4015..$4017` (`src/bus.c:168-184`). The `$4015` branch in `bus_read_data()` updates only `cpu_data` and returns it; it leaves `open_bus` alone (`src/bus.c:145-150`). After DMC completion, `bus_read_cycle()` clocks the resumed CPU cycle and performs the parked read (`src/bus.c:264-275`). For an undriven `$4000..$401F` address, `bus_read_data()` currently returns `cpu_data` without reconnecting `open_bus` (`src/bus.c:157-160`). In the failing case, that makes the resumed result `$50` even though the external held byte is `$00` or `$FF`.

## Owned-oracle correction

The current owned matrix repeats the same incorrect conclusion:

- `tests/cpu/test_bus.c:822` sets the diagnostic's expected `$4000` result to `$50` when the internal select is `$4015`. This line only feeds diagnostic output today; it does not assert the value.
- `tests/cpu/test_bus.c:946-952` hard-codes `$50` as the expected resumed value for any non-`$4015` parked address when the conflict selects `$4015`. The parked `$4000` and `$401F` cases should instead expect the external sample byte. The normal/observed whole-struct comparison is not an independent oracle because both paths execute the same production bus code.
- Keep the GET-side status expectation at `tests/cpu/test_bus.c:922-925`: it observes the internal `$4015` effect before the resumed CPU read. Keep the frame-IRQ clear-pending/next-GET assertions at `tests/cpu/test_bus.c:928-959` and the controller clock/value assertions at `tests/cpu/test_bus.c:931-950`.

## Minimal correction to evaluate

Use the already stored external `open_bus` byte when the CPU performs the **resumed** read from an undriven `$4000..$401F` APU mirror after a DMC GET selected internal `$4015`. Copy that external byte into `cpu_data` as the resumed result. Leave the `$4015` status-read branch internal-only, so it still returns status, preserves its internal D5 history, schedules the IRQ acknowledgement, and does not overwrite the external sample. Leave `$4016/$4017` controller reads on their existing path so their serial shifts and sample-derived upper bits remain intact.

Prefer this narrow resumed-read behavior unless a separate hardware source or owned regression establishes that every ordinary undriven `$4000..$401F` read must always copy external `open_bus` into `cpu_data`. A blanket change to the shared fallback would alter non-DMA reads after `$4015`; the pinned conflict test proves the resumed conflict case, not every ordinary sequence.

This needs no new saved field or public mid-DMA state. Both latches already live in the instance and native state (`src/internal.h:84-94`); the condition can be a local result of the current synchronous DMA/read call. Keep the DMC sample-buffer and shift-register model unchanged: AccuracyCoin explicitly does not observe or qualify the byte entering that shift register.

## Fail-first owned fixture

Before changing production code, add an independent assertion to the existing source-shaped bus probe for parked `$4000`, sample `$FFD5` with byte `$A5`, frame IRQ set, and the internal `$4015` select:

- assert the actual sample GET address/data and internal select `$4015`;
- assert the internal-read event returns status `$50`, leaves external `open_bus == $A5`, and sets frame-IRQ-clear-pending;
- assert the resumed `$4000` event returns `$A5` and sets `cpu_data == $A5`;
- assert the IRQ clears on the existing next-GET schedule;
- retain neighboring `$4014/$4016/$4017/$4018` cases and both controller-port shift/value checks.

That expected `$A5` assertion should fail against the current `$50` return, providing a narrow red case. Then fix only the resumed result and update the two wrong expected-value descriptions above. Do not run or retry the full ROM/campaign as part of this diagnostic task; the owner separately authorized the campaign that produced the current evidence. The correction's required CI and regression run remain for the implementation step.

## Limits

This document is read-only source analysis; no source, test, ROM, campaign, or workflow was run or changed. The temporary pinned disassembly was inspected only in the requested `TEST_DMABusConflict` range; no emulator implementation source was opened. The diagnosis does not claim that AccuracyCoin proves DMC shift-register reload data, all undriven-register sequences, or physical board behavior beyond the tested RP2A03G case.

## Resolution

The undriven-read path now reconnects to the external held value when4015
selection ends. Corrected matrix/status-acknowledgment/latch tests pass, with
00/A5/C2/FF sample values and linked/observed parity. The earlier retained50
oracle after deselection was wrong. See24-04-CORRECTIONS.md; fresh full-ROM
acceptance of behavior14 remains separate.
