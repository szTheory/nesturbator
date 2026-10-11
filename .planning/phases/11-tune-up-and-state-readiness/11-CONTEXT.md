# Phase 11: Tune-up and state readiness - Context

**Gathered:** 2026-10-11
**Status:** Ready for planning

<domain>
## Phase Boundary

This phase pays off three pieces of v2 debt so that save-state work starts clean:
- A battery save is never left torn or stale.
- An MMC3 battery game's `.srm` survives two RetroArch sessions.
- `nesturbator_get_version` and `nesturbator_unload_cartridge` each have a real caller and README text.

It also carries the state-readiness items from the roadmap notes. It adds no save states (Phase 12), no signal handling (Phase 14) and no exit-status table (Phase 14).

Requirements: TUNE-07, TUNE-08, TUNE-09.

</domain>

<decisions>
## Implementation Decisions

### Failed-save leftovers (TUNE-07)

- **D-01: One write helper.** `int nesturbator_run_file_replace(const char *path, const uint8_t *data, size_t size)` lives in `runner/save.c` and `runner/save.h`.
  - It returns 0 on success, or 1 after printing one message.
  - `nesturbator_run_save_flush` becomes this call plus `memcpy(shadow, ...)` on success.
  - Phase 12's `--state-at-frame` calls the helper directly.
  - The library still does no file I/O.

- **D-02: Write sequence.**
  1. `remove("<path>.tmp")` to clear a stale leftover.
  2. `fopen(tmp, "wbx")`. This is C11 exclusive create: it never writes through a symlink or a racing file.
  3. Write, `fflush`, then fsync (`_commit` on Windows), then a checked `fclose`.
  4. `rename` on POSIX, or `MoveFileExA(REPLACE_EXISTING|WRITE_THROUGH)` on Windows.
  - The temporary name stays `<path>.tmp`, with no PID or random suffix.

- **D-03: Cleanup on failure.** On any failure after this call created the `.tmp`, the helper removes it and returns 1. That covers a failed write, flush, sync, close or rename.
  - It never removes anything it did not create. A foreign non-empty directory at `.tmp` stays where it is, and the call returns 1.
  - The old file is untouched on every path.
  - The v2 behaviour of keeping the new bytes in `.tmp` ("the new save is in X.tmp") is dropped.
  - Dropping it does not lose the newest save. Those bytes are still in emulator memory, the shadow is not updated, and the next interval flush or the exit flush retries.

- **D-04: Message and exit status.**
  - The message is one line: `nesturbator-run: cannot save <path> (<reason>); the previous save is kept`. Save `errno` before cleanup.
  - Exit statuses do not change. A failed interval flush warns and the run continues; it is retried at exit. A failed exit flush gives exit 1.

- **D-05: No parent-directory fsync.**
  - The documented guarantee is "the save is the old one or the new one, never torn".
  - A power cut just after a rename may still show the old save. The README and the header comment say so.
  - Revisit only if someone reports a save lost to power failure.

- **D-06: Failure-injection tests.** These are C tests using `tests/check.h`. They call the helper in a temporary directory and run under `cmake --workflow --preset ci`.

  | Test | Platforms | Expected result |
  |---|---|---|
  | Directory at the target path | all six | Returns 1; the directory is still there; no `.tmp` |
  | Non-empty directory at `<path>.tmp` with an old file present | all six | Returns 1; old bytes intact; the directory untouched |
  | Missing parent directory | all six | Returns 1; nothing created |
  | Stale `.tmp` file from an earlier crash | all six | The new bytes win; no `.tmp` left |
  | `chmod 0555` on the directory | Linux and macOS | Returns 1; old file intact; no `.tmp`. Skipped if a probe `fopen` still succeeds, which happens when running as root |
  | Partial write: `RLIMIT_FSIZE` set to a few bytes, `SIGXFSZ` ignored | Linux and macOS | Returns 1; old file intact; no `.tmp` |
  | Symlink at `.tmp` pointing at a victim file | Linux and macOS | The victim is unchanged |

  One runner-level test: a directory at the `.sav` path gives exit 1 at exit, and the message names the path.

### libretro instance lifetime (TUNE-09)

- **D-07: One instance for the adapter's lifetime.** — **Reversibility:** costly. Hosts that relied on a fresh instance after each content load would see a different lifecycle, and the libretro host tests encode this mapping.

  | Callback | What it does |
  |---|---|
  | `retro_init` | Calls `nesturbator_create`; on failure leaves `inst` NULL |
  | `retro_load_game` | Returns false if `inst` is NULL. Clears the cached save span, calls `nesturbator_unload_cartridge`, then loads the game or shows the test card for a NULL game. A failed load returns false and leaves the instance with no cartridge; it does not destroy it |
  | `retro_unload_game` | Clears `save_data` and `save_size` first, then calls `nesturbator_unload_cartridge` |
  | `retro_deinit` | Calls `nesturbator_destroy` and resets every static |

  - The current refusal of a second load while `inst != NULL` is replaced by unloading first.

- **D-08: Unload must restore the no-cartridge power-on state.** — **Reversibility:** reversible.
  - Today `nesturbator_create` sets `ppu.scanline = 261` and `dmc.buffer_empty = 1` (`src/instance.c:107-108`).
  - `nesturbator_unload_cartridge` zeroes the PPU (`src/cartridge.c:244`), so after an unload the test card starts at scanline 0 instead of 261.
  - Game loads are not affected, because `load_cartridge` already zeroes the PPU (`src/cartridge.c:355`). No pinned game hash changes.
  - The fix is one internal power-on helper shared by create and unload, so a reused instance with no cartridge is byte-identical to a fresh one.
  - The header comment for `nesturbator_unload_cartridge` says this.

- **D-09: Tests for the lifetime change.**
  - **Library tests:**
    - Create, then frames, must equal create, load, unload, then frames.
  - **libretro host tests (`tests/libretro/libretro_host.c`):**
    - Load, unload, then reload the same ROM without deinit gives the same video and audio hashes as the first load and as a direct-API instance.
    - Load, unload, then load NULL gives test-card frames equal to a fresh no-content start.
    - Between unload and the next load, `retro_get_memory_data(SAVE_RAM)` is NULL and its size is 0.
    - A double unload is safe.
    - A bad image after an unload returns false, and a following good load works.
    - All of these also pass under the ASan preset.

### Where the version shows (TUNE-09)

- **D-10: A runner `--version` flag.** — **Reversibility:** one-way. `--version` becomes part of the runner's published command line, and launchers may come to rely on its output.
  - It prints exactly one line to stdout, `nesturbator-run <major>.<minor>.<patch>`, and exits 0.
  - The number is formatted from `nesturbator_get_version()` at run time, not from the macros.
  - It is handled when the option loop reaches it, so `nesturbator-run --version` needs no other options.
  - It does not print the ABI or the behaviour revision; those belong to the deferred `--capabilities` (RUN-04).
  - `usage()` lists it.

- **D-11: The libretro adapter's version string stays compile-time.** `library_version` keeps its compile-time `NT_VERSION` literal, because a `const char*` that must stay valid is best as a literal.

- **D-12: Version test.** A ci test runs `nesturbator-run --version` and checks:
  - stdout is exactly `nesturbator-run <version.txt>` and exit is 0
  - stderr is empty

  That extends the existing chain: `version.txt` matches the header, which matches the runtime value, which matches the runner output.

- **D-13: Docs.**
  - The README describes `--version`.
  - The README says the runner calls `nesturbator_get_version` and the libretro adapter calls `nesturbator_unload_cartridge`.
  - The `get_version` header comment gains one sentence: compare the result with `NESTURBATOR_VERSION_*` when the library may differ from the header, as with a shared library.

### MMC3 `.srm` round trip (TUNE-08)

- **D-14: Which game.** The `retroarch-e2e` MMC3 round trip uses the derived `m4tkrom` ROM, an MMC3 board with a battery and 8 KiB of save RAM. `tests/holymapperel/roms.cmake` already builds it from `M4_P128K_CR8K.nes`, so no new committed ROM bytes are needed. The step mirrors the existing MMC1 step through `tests/retroarch/run_retroarch.cmake` and waits on markers, not sleeps.

### State readiness (from the roadmap notes)

- **D-15:** The runner's `save_state` struct (`runner/main.c:252`) is renamed `battery_save`.
- **D-16:** White-box tests check that the frame-scoped output pointers are NULL and the PCM ring is empty at every frame boundary.
- **D-17:** Confirm the existing status numbers, and confirm that exit 3 is not used anywhere.

### Claude's Discretion
- The exact helper names for the power-on state and the test filenames.
- Whether the `RLIMIT_FSIZE` partial-write test is a separate test executable. It changes process limits, so it should not share a process with other tests.
- Whether a Windows sharing-lock test (`CreateFileA` with share mode 0) is added. It is not required: directory-at-target already covers rename failure on Windows.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Requirements and decisions
- `.planning/REQUIREMENTS.md`: TUNE-07, TUNE-08, TUNE-09, and the out-of-scope list
- `.planning/ROADMAP.md`: the Phase 11 success criteria and the state-readiness notes
- `.planning/research/SUMMARY.md`: v3 decisions (state module, exit statuses, Phase 11 implications)
- `.planning/research/PITFALLS.md`: atomic write and save pitfalls
- `.planning/milestones/v2-MILESTONE-AUDIT.md`: debts G2 and G6, and 08 IN-02

### Code
- `runner/save.c`, `runner/save.h`: today's flush, to become the shared helper
- `runner/main.c`: the `save_state` struct (line 252), option parsing (lines 170-250), `usage()`
- `libretro/libretro.c`: `retro_init`, `retro_deinit`, `retro_load_game`, `retro_unload_game`, the memory hooks (lines 80-333)
- `libretro/libretro.h`: the vendored lifecycle contract (SET_SUPPORT_NO_GAME around line 1040; RETRO_MEMORY_SAVE_RAM)
- `src/instance.c:79-113`: the power-on state in `nesturbator_create`
- `src/cartridge.c:234-252` and `:351-362`: what unload and load reset
- `include/nesturbator.h`: the `get_version` comment (lines 84 and 225), the `unload_cartridge` comment (line 295), save generation (lines 383-394)
- `tests/libretro/libretro_host.c`, `tests/core/test_cartridge.c`, `tests/core/test_api.c`: existing lifecycle and version tests
- `tests/holymapperel/roms.cmake`: the derived `m4tkrom` entry
- `tests/retroarch/run_retroarch.cmake` and `.github/workflows/ci.yml` (the `retroarch-e2e` job): the MMC1 round trip to mirror

### External (documentation only; clean room)
- docs.libretro.com, "Developing cores": `retro_init`, `retro_unload_game` and `retro_deinit` semantics
- POSIX `rename` and `fsync`; Microsoft `MoveFileExA` and `_commit`; ISO C11 `fopen` mode `"x"`
- sqlite.org/atomiccommit.html; LWN 322823 ("ext4 and data loss"); danluu.com/file-consistency

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `nesturbator_run_save_flush` already does the tmp, fsync, close and rename sequence. It needs cleanup on failure and the `"wbx"` open.
- `holymapperel-derive` and the `m4tkrom` derived ROM give an MMC3 battery game without new bytes.
- `run_retroarch.cmake` already handles `SAVE_ROM` and `SAVE_FRAME` for the MMC1 `.srm` round trip.
- The libretro host test already runs init, load, unload and deinit and compares against the direct API.

### Established Patterns
- Runner messages are prefixed `nesturbator-run:` and go to stderr; `usage()` returns 2.
- Hardware and contract comments cite their source, such as the libretro.h line numbers.
- Every behaviour change updates the README and the public header comment in the same change.

### Integration Points
- Phase 12's `--state-at-frame` reuses D-01.
- Phase 14's signal flush reuses D-01 and D-04 unchanged.
- Phase 13's `retro_serialize_size` relies on D-07: one instance, and size 0 with no cartridge.

</code_context>

<specifics>
## Specific Ideas

- The owner asked for a deep fan-out on each decision: several role lenses, an adversarial pass, primary sources, and one coherent recommendation that is then followed without asking.
- They prefer the least code and owned code over dependencies. All three decisions use only the standard library, POSIX and Win32.

</specifics>

<deferred>
## Deferred Ideas

- Fsync of the parent directory after rename, if anyone reports a save lost to power failure.
- A bounded retry around `MoveFileExA` for antivirus or sharing locks on Windows, if field reports show lost exit saves.
- `--capabilities` with the ABI and behaviour revision (RUN-04, future).
- A note for Phase 12: `save_generation` is deliberately never reset by unload (`src/internal.h:193`). The state format and STATE-05 must define how it relates to a loaded state.

</deferred>

---

*Phase: 11-tune-up-and-state-readiness*
*Context gathered: 2026-10-11*
