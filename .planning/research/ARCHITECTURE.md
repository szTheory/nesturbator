# Architecture Research: v5 NMI, VBlank and sprite-0 timing

**Domain:** NES emulator core, integration into the existing CPU/PPU/bus model
**Researched:** 2026-10-25
**Confidence:** HIGH for what the code does today (read directly); MEDIUM for the target hardware windows (the exact dot of each edge must be pinned by owned dot-exact tests first, from NESdev docs [HWP.03], [HWC.02], [HWC.17]; no emulator source was consulted).

## 1. How it works today (file:line)

### Time model
- `src/bus.c:16-31` `cycle()` is the only place time moves for the CPU. Per CPU cycle (24 ticks): `ticks += 9`, `ppu_run_until` (1 dot at 8 ticks; 1 tick carried), `cpu.poll_latch = cpu.irq_line` (bus.c:22), flip `apu_get_put_phase`, `apu_clock`, `ticks += 15`, `ppu_run_until` (2 more dots), `cpu_cycle++`.
- The bus access (`bus_read_data`, or the register write in `nesturbator__bus_write`, bus.c:141) happens AFTER `cycle()` returns, so every CPU read or write sees the PPU after all 3 dots of its cycle. There is no sub-cycle position for the access: the PPU is 3 dots "ahead" of where hardware samples a register. Today's `$2002` windows (dot 0 vs dot 1 of line 241) are therefore shifted by a fixed, untuned CPU/PPU alignment (`ticks=0`, `ppu.scanline=261` at create, `src/instance.c:85-93`). The architecture doc mentions a profile alignment field; it does not exist yet.
- `src/frame.c:55` loops `cpu_step` until `ticks >= target`, then one final `ppu_run_until` (frame.c:58). `NESTURBATOR_STOP_JAM` is returned at frame.c:13 and :63.

### PPU flag, NMI and `$2002`
- `src/ppu.c:470` `ppu_run_until`: each dot, line 241 dot 1 sets `status|=0x80` and, if `control&0x80`, sets `cpu.nmi_pending=1` directly (ppu.c:482-491; `vblank_suppress` skips both). Line 261 dot 1 clears `status&=0x1f` and `nmi_pending=0` (ppu.c:492-499), plus `reset_flag`.
- `ppu.c:559-570` `$2002` read: returns `status&0xe0 | io_bus&0x1f`, clears bit 7, sets `vblank_suppress` only when `scanline==241 && dot==0`, clears the w latch and `nmi_pending=0` unconditionally.
- `ppu.c:629-643` `$2000` write: `nmi_pending=1` only on bit 7 0 to 1 with status bit 7 set; any write with bit 7 clear zeroes `nmi_pending`. This is behaviour revision 6 (`include/nesturbator.h:56-60`).
- Defect class: NMI is a one-shot latch (`nmi_pending`) poked from three PPU sites, not a level line plus an edge detector. `cpu.nmi_prev` exists (`internal.h:46`, serialised `state.c:102`) and is never used. Nothing models: edge detection at a fixed cycle phase, the flag read race at dots 1-3 of line 241, NMI deferral by one instruction when it arrives in the last cycles, the 2-instruction window after enabling NMI, or `$2000` writes cancelling an NMI in the few dots before it fires.

### CPU interrupt sequencing
- `src/cpu.c:478` `cpu_step` (instruction granularity; every instruction is straight-line bus calls). At entry: jammed read (cpu.c:480-484); NMI branch if `nmi_pending` (cpu.c:496-508: two dummy reads, 3 pushes, I set, vector read from `$FFFA/B`, clears `nmi_pending`); IRQ branch if `poll_latch && !(p&I)` (cpu.c:513-525, hard-coded `$FFFE/F`); else fetch and dispatch.
- So the NMI check is "pending at the moment the previous instruction returned", i.e. it includes edges raised in the previous instruction's last cycle (hardware polls one cycle earlier), and it reads the current `p` I flag (after CLI/SEI/PLP have taken effect), so I-flag latency is wrong for CLI, SEI, PLP (hardware polls before the I change; RTI is the exception that takes effect at once). `poll_latch` is overwritten at the start of every cycle (bus.c:22), which equals the "penultimate cycle" sample for IRQ; there is no NMI equivalent and no taken-branch exception (taken non-page-crossing branch skips its poll) - `branch()` is cpu.c:403-418.
- BRK (cpu.c:1046-1054) and the IRQ/NMI entry code hard-code their vectors; a NMI arriving during cycles 1-4 of BRK/IRQ must redirect the vector fetch to `$FFFA` (hijack), and today it cannot. The three entry sequences are copies; PLP (cpu.c:975) and RTI (cpu.c:1033) share `pull_p` (cpu.c:365 region).
- `cpu_reset` (cpu.c:1366-1381) clears `poll_latch`; `ppu_reset` clears `nmi_pending/nmi_prev` (ppu.c:614-615, deliberately, because ppu.c may not call cpu.c: "bus.unit link trap").
- IRQ line: sole writer `nesturbator__irq_update` (`src/apu.c:108-112`) ORs frame IRQ, DMC IRQ and `mapper.irq`.

### Sprite-0
- Evaluation `ppu.c:175-247`: `eval_sprite_zero[slot] = (eval_n == 0)`; always starts at n=0 regardless of `oam_addr` (ppu.c:~197 reads `oam[eval_n*4+eval_m]`). Latched into `sprite_zero[slot]` at `ppu.c:328` during sprite fetch.
- Hit: only in `compose_pixel` (ppu.c:396-421), which is called only inside the `video_output != NULL` visible-pixel block at the end of `run_until` (ppu.c:~531-538). Consequences: (a) hit detection is coupled to the video-output pointer, so any PPU catch-up outside `run_frame` (`instance.c:182`, `ppu_reset` catch-up) skips it; (b) the loop returns at the FIRST opaque sprite in slot order (ppu.c:402-419), so a sprite-0 that is not in slot 0, or is hidden behind a lower-slot opaque sprite, never sets the flag: this is the likely cause of `Arbitrary Sprite zero` (the AccuracyCoin wait loop spins forever, hence timeout, not JAM); (c) the left-clip test uses `mask&0x06` correctly; x=255 exclusion is present; BG-opaque uses the same `bg_opaque` as rendering; the flag is set at the dot that produces the pixel (dot x+1), with the same 3-dots-ahead bus ordering as above, which is what `Sprite 0 Hit behavior` timing sub-tests probe. Clearing at 261 dot 1 is correct.

### State, hash and harness surfaces
- State: `src/state.c` one field list per struct (`cpu_io` state.c:94-110, PPU list ~255-340), `NESTURBATOR_STATE_FORMAT_VERSION 1`, `BEHAVIOUR_REVISION 6` (`include/nesturbator.h:60,71`). Size/layout asserts via `offsetof` (state.c:72).
- Harness: `runner/accuracycoin.c`. `accuracy_frame` (546-565) returns the library status; `frame_and_sample` (929-935) returns only a boolean, so STOP_JAM is indistinguishable from any failure: the execution loop reports `stop_cause="execution-frame-failure"`, `verdict="guest-failure"` (accuracycoin.c:~1196-1199); the page-run helper (accuracycoin.c:729-785) returns `RUNNER_EXIT_FAILURE` with no cause. Classifier: `accuracycoin_classify` (462) and `_row` (519) with causes like `active-test-budget-exhausted`; result enum `accuracycoin.h:82-90` (pass, fail, skip, not-run, timeout, draw, invalid) - JAM is not a status. Unit coverage: `tests/accuracy/test_conformance.c:402-500`. Scoreboard: `tests/accuracy/scoreboard.txt` rows (`$2002 flag timing fail 0x06`, `Arbitrary Sprite zero timeout 0x03`, etc.).
- Run-ahead: `libretro.runahead` (`tests/CMakeLists.txt:1659-1673`) runs `libretro.host --runahead` (`tests/libretro/libretro_host.c:2370-2650`: `rah_run` 2435, `rah_matrix` 2545, controls/broken hosts 2606) over seven ROMs with `tests/runahead_model.h`; `tests/core/test_state_runahead.c` is the core-level counterpart. JAM presentation tests already exist separately (`libretro_host.c:981-1300`, `check_jam_presentation`), but none run-ahead across a JAM. Adapter JAM path: `libretro/libretro.c:234-241`.

## 2. Recommended architecture for the change

Keep the per-dot PPU and the 3-dot-then-access bus ordering; make the edges explicit instead of re-ordering sub-cycles.

```
PPU dot loop ──sets──> status.vblank (flag)  ┐
$2000 writes ──set───> control.nmi_enable    ├─> nmi_line = flag & enable  (derived, in PPU, no CPU call)
$2002 reads  ──clear─> flag                  ┘
bus cycle():  edge = nmi_line & ~nmi_prev  at a fixed M2 phase  -> cpu.nmi_pending (latched edge)
              nmi_prev = nmi_line
              penultimate-cycle poll: nmi_polled = nmi_pending; irq_polled = irq_line; i_polled = P.I
cpu_step():   take NMI if nmi_polled; take IRQ if irq_polled && !i_polled; vector chosen at cycle 4/5 of the entry
```

### Component boundaries (modify vs new)
| Component | Change | New or modified |
|-----------|--------|-----------------|
| `ppu.c` flag/NMI sites (482-499, 559-570, 629-643) | Replace direct `nmi_pending` pokes with a derived `nmi_line` (flag AND enable), exposed as a PPU field read by the bus; model the one/two-dot `$2002` read race at line 241 and the 261 clear; `$2000` bit-7 toggles near dot 1 handled by the line, not special cases | Modified |
| `bus.c cycle()` (16-31) | Edge detector: compare `nmi_line` with `nmi_prev` at the existing M2-fall catch-up and set `nmi_pending` on a rising edge (activates the unused `nmi_prev`); replace the single `poll_latch = irq_line` with a poll step that also latches NMI and the I flag | Modified |
| `cpu.c cpu_step` (478-525) | One shared `interrupt_entry(vector_source)` for NMI/IRQ/BRK (dedupes cpu.c:496-525 and 1046-1054); vector decision after the pushes from the live NMI edge (hijack); take decisions from polled values, not live `p` | Modified, one helper new |
| CLI/SEI/PLP/RTI/branch (cpu.c:838, 841, 975, 1033, 403) | PLP/CLI/SEI: poll uses I from before the instruction; RTI: new I used; taken branch without page cross: skip that poll (set a one-cycle `no_poll` flag consumed by the bus poll) | Modified |
| `ppu.c` sprite eval/hit (175-247, 328, 396-421, 531) | Hit test moved out of `compose_pixel`'s `video_output` gate into a per-dot step that runs even with no output pointer; check sprite-0 independently of draw priority; sprite-zero identity follows the OAM start index (`oam_addr`) | Modified |
| `runner/accuracycoin.c` | Introduce JAM attribution (below) | Modified |
| `tests/` | Dot-exact fixtures, CPU interrupt programs, run-ahead-across-JAM case | New files plus edits |

### Time-order change (the real design decision)
Today: `[1 dot] poll IRQ [2 dots] ACCESS`. Hardware samples interrupt lines at a half-cycle inside the cycle and a register access lands at a fixed dot within it. Recommended minimum change: keep `cycle()`'s two catch-up points, do edge detection after the second catch-up (so a flag set at dot k of this cycle is a visible edge for the NEXT poll), and tune the per-access dot position through one named constant (alignment offset applied to where the access is considered to fall relative to line 241 dot 1). Decide by dot-exact unit tests first; do not guess. If the 3-dot-ahead model cannot match the `$2002`-read window, the fallback is catching the PPU up to the access in two steps (run to M2 fall minus N ticks, access, run the rest) for register accesses to `$2000-$2007` only; this changes PPU-visible ordering and re-pins all hashes, so treat it as a last resort.

## 3. State fields and save-state impact

Existing, serialised: `nmi_prev, nmi_pending, irq_line, poll_latch` (state.c:102-105), `vblank_suppress` (state.c:274), `status`, `control`.

Add (all `uint8_t`, CPU struct `internal.h:46` unless noted):
- `nmi_polled` (NMI decision latched at the penultimate cycle; distinct from live `nmi_pending`)
- `irq_polled` replaces/renames `poll_latch` semantics; `i_polled` (I flag at poll time)
- `poll_skip` (taken-branch exception; one cycle lifetime)
- PPU: `nmi_line` is derivable from `status` and `control` and need not be stored; any delayed-`$2002` latch (flag-read race) must be stored. `vblank_suppress` may be removed or reworked.

Rules that follow from the state contract (`include/nesturbator.h:62-71`, `state_format_pin.cmake`, `write_hashes.cmake:288`):
- Any added, removed or reordered state byte raises `NESTURBATOR_STATE_FORMAT_VERSION` to 2 in the same commit (release 0.1.11 already contains save states, so the "stays 1" window is closed). Update `tests/runner/hashes.txt` row `state_format 2`; `state_format_pin.cmake` fails otherwise. Add the fields to `cpu_io`/PPU lists in `state.c` and to the `offsetof` size assertions (state.c:72); extend `tests/core/test_state.c` (header version check line 116) and `test_state_reject.c`/`test_state_fork.c` (they touch `nmi_pending` directly).
- Raise `NESTURBATOR_BEHAVIOUR_REVISION` to 7 (frame output changes), update `include/nesturbator.h` comment, `tests/core/test_api.c:96`, `tests/cmake/accuracycoin_activation.cmake:138` (`behaviour_revision|6`), README line 29 ("behaviour revision is 6") and the behaviour-revision table at README ~1113-1136.
- Boundary rule: fields must be zero/valid at a frame boundary (`tests/core/test_boundary.c`); poll latches are per-cycle values, so confirm they are consistent after a frame ends mid-instruction-sequence (frames end between instructions, so they are).
- Run-ahead safety: all new state must be inside the saved lists, or `libretro.runahead` and `test_state_runahead.c` break.

## 4. Hash and test re-pin map

Likely to change (any ROM that enables NMI or polls `$2002`/IRQ timing): `tests/runner/hashes.txt` (114 rows): `dabg/*` frame and audio rows (frame ticks like 714792 show instruction-boundary jitter, so any interrupt-timing change shifts `ticks` values too), `nesteroids/boot`, `nesrunner/boot|play`, `rhde/boot`, `holymapperel/*` (frame, audio pcm, audio transitions), `state/*` (12 rows: state CRCs), `state_format`. Audio transitions hash moves only if the APU is clocked on a different CPU cycle (an instruction taking a different number of cycles around an interrupt; expect some rows to change, treat unexpected ones as a bug). Also `tests/retroarch` and `libretro.serialize/rom-parity` fixtures that consume the same inventory, and `tests/accuracy/scoreboard.txt` (11 rows to `pass 0x01`; `scoreboard-main*.txt` and `baseline-metadata.txt` track the baseline; `verify_scoreboard_regression.cmake` is the no-lost-PASS gate).
Unit tests that touch the old model directly and will be rewritten: `tests/ppu/test_registers.c` (lines 20-85 assert `nmi_pending` after reads/writes), `tests/core/test_nmi_edge.c`, `tests/core/test_reset.c`, `test_apu.c`, `test_mapper*.c` (read `irq_line`), `tests/cpu/test_bus.c`, `test_cpu_unit.c`, `tests/ppu/test_sprites.c` (hit cases). `tests/vectors` (65x02) exclude interrupts and are unaffected. Re-pin each hash in the same commit as the behaviour change that moved it, with the reason stated.

## 5. AccuracyCoin harness and JAM attribution

- Where: make `frame_and_sample` (accuracycoin.c:929) return the library status (or add an out-parameter) instead of a boolean; at the execution loop (~1196) map `NESTURBATOR_STOP_JAM` to `stop_cause="guest-jam"`, `verdict="guest-jam"` and record the active row (`controls.cursor`) and raw byte; same mapping in the page-run loop (729-785) and `accuracy_frame` (546) callers so the strict-page path prints "JAM at frame N" instead of a bare failure. Optional peek of `cpu.jammed` is unnecessary: `STOP_JAM` already reaches the runner (main.c:1254 does this for replay).
- Classifier: keep the result enum (no new status, so scoreboard format is unchanged); a JAM during an active row classifies as `fail`/`timeout` with a new cause string next to `active-test-budget-exhausted` (accuracycoin.c:~488). Extend `test_conformance.c` (402-500, the mutation harness at 534-704) with a JAM mutation, and record the cause in the evidence metadata (`tests/accuracy/*-metadata.txt` writer, accuracycoin.c:~1322-1340).
- Run-ahead test: add one case to `libretro_host.c` `runahead_mode`/`rah_matrix` (2545): a synthetic ROM built with `tests/ines.h` that JAMs at a known frame; run N frames ahead across it, load state, and require the real stream (frame and audio hashes, `runahead_model.h`) to equal the uninterrupted run, with the jam latch cleared by `load_state`. Reuse `check_jam_presentation`'s `delay_then_jam` generator (libretro_host.c:1086). Register under the existing `libretro.runahead` test (no new CTest name needed); the `RAH_MIN_ROMS` rule (2400s) counts ROMs, so the synthetic case needs its own counter. Mirror at core level in `tests/core/test_state_runahead.c`.

## 6. Build order (dependencies)

1. **Owned dot-exact reproducers first** (no source change): `tests/ppu` reads of `$2002` at line 241 dots 0-3, 261 dots 0-1, `$2000` toggles near dot 1; `tests/cpu` interrupt programs (CLI/SEI/PLP latency, branch poll, NMI at cycles 1-7 of BRK/IRQ). Mark expected-failing; they define the target windows from NESdev text.
2. **NMI line + edge detector** (ppu.c flag/NMI sites, bus.c `cycle()`, state fields, format/behaviour bump): clears `$2002 flag timing`, `NMI Timing`, `NMI Control`, `NMI Suppression`, `NMI at VBlank end`, `NMI disabled at VBlank`. Re-pin hashes and scoreboard here.
3. **Poll stage and I-flag latency** (bus.c poll, cpu.c polled fields, CLI/SEI/PLP/RTI/branch): clears `Interrupt flag latency`. Depends on 2 for the NMI edge being right.
4. **Shared interrupt entry and vector hijack** (cpu.c entry dedupe, BRK): clears `NMI Overlap BRK`, `NMI Overlap IRQ`. Depends on 3 (polled NMI) and 2.
5. **Sprite-0** (ppu.c hit step, slot-independent test, OAM-start identity): `Sprite 0 Hit behavior`, `Arbitrary Sprite zero`. Largely independent of 2-4 but its timing sub-tests depend on the same bus alignment; run after 2 so the alignment constant is settled. Re-check whether the timeout clears as a side effect of 2.
6. **Harness tune-up** (JAM attribution, classifier mutation, run-ahead-across-JAM): independent; can land first or last, but land before the final scoreboard flip so any JAM during v5 development is visible. Suggest earliest (zero hash impact, makes steps 2-5 diagnosable).
7. Final: README and `nesturbator.h` comments updated per step (rule 6), scoreboard confirmation of 11 passes and zero lost PASS.

## 7. Pitfalls specific to this integration
- `ppu.c` must not call `cpu.c` (link trap, ppu.c:610); the PPU exposes `nmi_line` by field, the bus/CPU read it.
- Changing the register-access dot alignment moves every PPU-register-dependent hash (MMC3 IRQ tests in `tests/mmc3`, `test_split_scroll`); keep the alignment change in one commit with its own re-pin.
- `frame_and_sample` boolean-to-status change touches the metadata writers and the mutation tests; keep the scoreboard text format byte-identical.
- Sprite-0 decoupled from `video_output` must not add pixel work when output is NULL (performance bench `tests/perf`, `runner/bench.c`).

## Sources
- Repository code read directly (HIGH): files and lines above.
- `.planning/preparation/ARCHITECTURE.md` sections 2-4 (time model, interrupts), `.planning/preparation/NES-HARDWARE-CPU-APU.md` ([HWC.02], [HWC.17]), `.planning/research/STACK.md` (sibling output; hardware sources [HWP.03], [HWP.06]) (MEDIUM for exact dot windows until pinned by step 1 tests).
