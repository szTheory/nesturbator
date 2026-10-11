# Phase 11: Tune-up and state readiness - Discussion Log

> **Audit trail only.** Do not use this as input to planning, research or execution agents.
> The decisions are in CONTEXT.md. This log keeps the alternatives that were considered.

**Date:** 2026-10-11
**Phase:** 11-tune-up-and-state-readiness
**Areas discussed:** Failed-save leftovers, libretro instance lifetime, where the version shows

The owner chose all three areas. For each one they asked for a deep fan-out across role lenses, an adversarial pass and primary sources, then one coherent recommendation to follow without further questions. Advisor mode used the minimal_decisive tier, with one researcher per area.

---

## Failed-save leftovers

| Option | Description | Selected |
|--------|-------------|----------|
| Always remove our `.tmp` | Clear a stale `.tmp` first and open it with `"wbx"`. On any failure, remove the `.tmp` this call created, keep the old file and return 1. No parent-directory fsync. | ✓ |
| Keep `.tmp` on rename failure | Today's behaviour, optionally with a parent-directory fsync and a Windows retry. It contradicts TUNE-07 and leaves junk next to the save. | |

**User's choice:** follow the synthesized recommendation.
**Notes:** The new bytes are not lost, because the next interval flush or the exit flush retries from emulator memory. Partial-write coverage uses `RLIMIT_FSIZE` on Linux and macOS.

## libretro instance lifetime

| Option | Description | Selected |
|--------|-------------|----------|
| Keep create and destroy per game | The runner calls unload before destroy. That meets the requirement's wording but does nothing. | |
| One instance from `retro_init` to `retro_deinit` | `retro_unload_game` calls `nesturbator_unload_cartridge`. This is the shape Phases 12 and 13 need. | ✓ |
| Runner calls unload before destroy | It satisfies the requirement's wording and adds no value. | |

**User's choice:** follow the synthesized recommendation.
**Notes:** The researcher found that unload zeroes the PPU, so the test card starts at scanline 0 instead of create's 261. I verified this in `src/instance.c` and `src/cartridge.c`. Game loads are unaffected. Fix: one power-on helper shared by create and unload.

## Where the version shows

| Option | Description | Selected |
|--------|-------------|----------|
| Runner `--version` from `get_version` | One stdout line, exit 0, tested against `version.txt`. | ✓ |
| Also print the macro version and warn on a mismatch | The runner is built together with the library, so the mismatch branch cannot be tested. | |
| libretro `library_version` built at run time | Lifetime and init-order risk for a `const char*` that must stay valid. | |
| Test-only caller | Does not meet TUNE-09. | |

**User's choice:** follow the synthesized recommendation.

## Claude's Discretion

- Helper names, test file layout, a separate process for the `RLIMIT_FSIZE` test, and whether to add an optional Windows sharing-lock test.
- Decided without asking:
  - the MMC3 ROM is the derived `m4tkrom`
  - the helper lives in the runner
  - failure-injection tests are split by platform
  - the roadmap's state-readiness items stay as written

## Deferred Ideas

- Fsync of the parent directory after rename.
- A Windows `MoveFileExA` retry.
- `--capabilities` (RUN-04).
- How `save_generation` relates to a loaded state, for Phase 12.
