---
phase: 12-save-states-in-the-library
reviewed: 2026-10-12T00:00:00Z
depth: standard
files_reviewed: 37
files_reviewed_list:
  - CMakeLists.txt
  - README.md
  - include/nesturbator.h
  - runner/CMakeLists.txt
  - runner/main.c
  - runner/state_file.c
  - runner/state_file.h
  - src/cartridge.c
  - src/crc32.c
  - src/internal.h
  - src/mapper.h
  - src/mapper_axrom.c
  - src/mapper_cnrom.c
  - src/mapper_mmc1.c
  - src/mapper_mmc3.c
  - src/mapper_nrom.c
  - src/mapper_uxrom.c
  - src/state.c
  - src/state.h
  - tests/CMakeLists.txt
  - tests/cmake/hash_inventory.cmake
  - tests/cmake/hash_keys.cmake
  - tests/cmake/runner_state.cmake
  - tests/cmake/state_format_pin.cmake
  - tests/cmake/status_table.cmake
  - tests/cmake/vector_api_policy.cmake
  - tests/cmake/write_hashes.cmake
  - tests/core/test_state.c
  - tests/core/test_state_fork.c
  - tests/core/test_state_generation.c
  - tests/core/test_state_reject.c
  - tests/header/header_c.c
  - tests/header/header_cxx.cpp
  - tests/mapper_test.h
  - tests/runner/state_mutate.c
  - tests/state_test.h
findings:
  critical: 1
  warning: 2
  info: 1
  total: 4
status: issues_found
---

# Phase 12: Code Review Report

**Reviewed:** 2026-10-12
**Depth:** standard
**Files Reviewed:** 37 (source and header files read in full; the test, cmake and README files were checked against the library behaviour only)

## Summary

The visitor design holds up. I checked these points and found them sound:

- SIZE, WRITE, VALIDATE and APPLY share one field list, so the lists cannot drift apart.
- The mapper register order matches the `reg` structs in `src/mapper.h` for MMC1 and MMC3.
- `sio_i32` and `sio_i64` decode without implementation-defined casts.
- The synthesis invariant `integrator + sum(impulse) == mixed_level * 32768` is preserved by `synth_transition` and `synth_sample`. The 2^40 and 2^52 bounds keep `multiply_q30` and `integrator * 32768` inside 64 bits.
- Every array index that comes from the state is bounded: `eval_n`, `eval_count`, `eval_copy_remaining`, `sprite_count`, the APU phases and the DMC output.
- The load check order matches D-04.
- VALIDATE runs completely before any APPLY write, so the load is all-or-nothing as far as validation goes.
- The runner's state read is bounded by the state size plus one, so a hostile file cannot make it allocate more.

One validated-but-hostile state still hangs the next frame, which breaks the stated guarantee that a CRC-valid hostile state cannot make the next frame run long.

## Critical Issues

### CR-01: A CRC-valid state with `ticks` near 2^64 makes `nesturbator_run_frame` loop about 2^61 times

**File:** `src/state.c:316-323` (rule), `src/ppu.c:472` (loop), `src/frame.c:52-58`
**Issue:**
- The header `ticks` field is unbounded. The only rule on it is `ppu_ticks <= ticks && ticks - ppu_ticks < NESTURBATOR_TICKS_PER_FRAME`.
- `nesturbator__ppu_run_until` tests `ppu_ticks + 8u <= ticks`. If `ppu_ticks > UINT64_MAX - 8`, the sum wraps to a small number and the test passes forever. Each iteration adds 8, wraps again and keeps going, for about 2^61 iterations.
- Example: a state with `ticks = UINT64_MAX` and `ppu_ticks = UINT64_MAX` passes the CRC check, passes VALIDATE (gap 0) and loads. The next `nesturbator_run_frame` computes `target = ticks + 714732`, which wraps to a small number. The CPU loop is skipped, then `ppu_run_until(inst, UINT64_MAX)` never returns.
- A state with `ticks = UINT64_MAX - 3` and `ppu_ticks = UINT64_MAX - 20` also hangs: the PPU steps up by 8 and then wraps.
- This contradicts the claim in `include/nesturbator.h` (check 7) and the comment at `src/state.c:317-321` that a hostile state cannot make the next frame run longer than about one frame of work.
- It is a denial of service for any host that loads untrusted states, which is exactly the Phase 13 libretro and fuzzing scenario.
- `H_FRAME` (`frame_number`) is unbounded too. It only wraps, so it is harmless.

**Fix:** Bound `ticks` in VALIDATE. Do it before the visitor runs, or as a `sio_check` in `ppu_io`. Leave room for one frame plus one PPU step and CPU overshoot:
```c
/* nesturbator_load_state, before body_io in VALIDATE mode */
#define STATE_TICKS_MAX (UINT64_C(1) << 62) /* 2^62 ticks is about 200,000 years of emulated time */
if (get_le(b + H_TICKS, 8u) > STATE_TICKS_MAX)
    return NESTURBATOR_ERR_STATE_INVALID;
```
The same bound could instead go into the `ppu_io` check: `ppu_ticks <= io->ticks && io->ticks <= STATE_TICKS_MAX && ...`. Add a regression case to `tests/core/test_state_reject.c` that sets `ticks` and `ppu_ticks` to `UINT64_MAX` with a recomputed CRC, expects `ERR_STATE_INVALID`, and runs one frame to prove it returns. Update the check-7 comment in the header to list the ticks bound.

## Warnings

### WR-01: The frame-range checks now run after ROM load, so a usage error can turn into a ROM or I/O exit code

**File:** `runner/main.c:1040-1051` (the `check_ranges` call), removed block near the old `main.c:955`
**Issue:**
- Before this phase, `--hash-frame N` and `--dump-frame N` beyond `--frames` were rejected with exit 2 right after option parsing, before any file was touched.
- They are now checked only after the instance, ROM and battery save are loaded (D-10 step 5).
- For the same bad command line, a bad or missing ROM now reports its own exit code instead of 2, and a `--save-dir` problem now takes precedence over the usage error.
- That is a behaviour change for existing invocations. D-10 requires the range check against K only after the load. The checks that do not depend on K (`n > o->frames`, and `n == 0` for hash and dump) can still run early.

**Fix:** Keep the K-independent checks (`n > frames`) in an early pass before the instance is created. Leave only the `n <= K` and `n < K` comparisons, and `frames < K`, after the state load.

### WR-02: `cpu.halted_in_read` is validated as exactly 0 although the field is documented as used

**File:** `src/state.c:104-105`, `src/internal.h:47`
**Issue:**
- `internal.h` says `halted_in_read` is "nonzero while the current read is stalled", and the DMA work in progress may set it.
- `cpu_io` forces its maximum to 0. A later change that makes the field nonzero at a frame boundary produces states this library saves but refuses to load with `ERR_STATE_INVALID`.
- The comment says the field is never read. If so, the pair of rules is only consistent today by accident.
- The same applies to any future use of the field, and the failure would only show up in the fork tests if one of them hits it.

**Fix:** Use `1u` as the bound, which matches the other flag fields and costs nothing, or drop the field from the state and zero it on load. If it stays at 0, add a note at the field in `internal.h` saying a nonzero value breaks save states.

## Info

### IN-01: VALIDATE reads the instance through `*v`, contradicting the stated invariant

**File:** `src/state.h:99-107` (every `sio_u8`, `sio_u16`, `sio_u32`, `sio_i32` and `sio_i64`), claim at `src/state.c:472-473`
**Issue:**
- Each primitive passes `*v` to `nesturbator__sio_raw` before the mode is known, so VALIDATE reads the field's current value and then ignores it.
- The comment in `nesturbator_load_state` says VALIDATE "reads nothing of the instance". The read is harmless today: every field is initialised and the instance is not shared with another thread.
- It would matter if VALIDATE were ever run on a different or partially built instance, or with a tool that flags uninitialised reads on a half-built object.

**Fix:** Either weaken the comment to "changes nothing in the instance", or read `*v` only in SIZE and WRITE, for example `mode == VALIDATE || mode == APPLY ? 0 : *v`.

---

_Reviewed: 2026-10-12_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
