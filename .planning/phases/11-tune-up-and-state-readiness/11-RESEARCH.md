# Phase 11: Tune-up and state readiness - Research

**Researched:** 2026-10-11
**Domain:** C17 file replacement (stdio, POSIX, Win32), libretro adapter lifecycle, CTest/CMake test registration, hosted RetroArch e2e
**Confidence:** HIGH (everything below was read in this repo this session; platform behaviours marked where only reasoned)

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

**Failed-save leftovers (TUNE-07)**

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

**libretro instance lifetime (TUNE-09)**

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
  - **Library tests:** Create, then frames, must equal create, load, unload, then frames.
  - **libretro host tests (`tests/libretro/libretro_host.c`):**
    - Load, unload, then reload the same ROM without deinit gives the same video and audio hashes as the first load and as a direct-API instance.
    - Load, unload, then load NULL gives test-card frames equal to a fresh no-content start.
    - Between unload and the next load, `retro_get_memory_data(SAVE_RAM)` is NULL and its size is 0.
    - A double unload is safe.
    - A bad image after an unload returns false, and a following good load works.
    - All of these also pass under the ASan preset.

**Where the version shows (TUNE-09)**

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

**MMC3 `.srm` round trip (TUNE-08)**

- **D-14: Which game.** The `retroarch-e2e` MMC3 round trip uses the derived `m4tkrom` ROM, an MMC3 board with a battery and 8 KiB of save RAM. `tests/holymapperel/roms.cmake` already builds it from `M4_P128K_CR8K.nes`, so no new committed ROM bytes are needed. The step mirrors the existing MMC1 step through `tests/retroarch/run_retroarch.cmake` and waits on markers, not sleeps.

**State readiness (from the roadmap notes)**

- **D-15:** The runner's `save_state` struct (`runner/main.c:252`) is renamed `battery_save`.
- **D-16:** White-box tests check that the frame-scoped output pointers are NULL and the PCM ring is empty at every frame boundary.
- **D-17:** Confirm the existing status numbers, and confirm that exit 3 is not used anywhere.

### Claude's Discretion
- The exact helper names for the power-on state and the test filenames.
- Whether the `RLIMIT_FSIZE` partial-write test is a separate test executable. It changes process limits, so it should not share a process with other tests.
- Whether a Windows sharing-lock test (`CreateFileA` with share mode 0) is added. It is not required: directory-at-target already covers rename failure on Windows.

### Deferred Ideas (OUT OF SCOPE)
- Fsync of the parent directory after rename, if anyone reports a save lost to power failure.
- A bounded retry around `MoveFileExA` for antivirus or sharing locks on Windows, if field reports show lost exit saves.
- `--capabilities` with the ABI and behaviour revision (RUN-04, future).
- A note for Phase 12: `save_generation` is deliberately never reset by unload (`src/internal.h:193`). The state format and STATE-05 must define how it relates to a loaded state.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| TUNE-07 | Battery saves and state files are written by one helper (tmp, fsync, checked close, rename). Failure-injection test (unwritable dir, dir at target) finds old file intact and no `.tmp`. | Sections "TUNE-07 ground map", pitfalls 1-4, 9; helper skeleton; test table with root and RLIMIT guards |
| TUNE-08 | `retroarch-e2e` round-trips an MMC3 battery game's `.srm` across two sessions like MMC1. | Section "TUNE-08 ground map": `run_retroarch.cmake` has MMC1 constants hard-coded (32768) that must be parameterised; ROM path and artifact list |
| TUNE-09 | `nesturbator_get_version` and `nesturbator_unload_cartridge` each have a runner/adapter caller, a ci test, and README text. | Sections "TUNE-09 ground map" (A: `--version`, B: adapter lifetime, C: power-on helper), pitfalls 5-8 |
</phase_requirements>

## Summary

The decisions in CONTEXT.md map cleanly onto the code, with **four places where the code or a test contradicts the decision text**. The planner must settle these before writing tasks (details in "Conflicts with CONTEXT.md" and Open Questions): (1) a failed *interval* flush today ends the run with exit 1 (`runner/main.c:858-860`), it does not "warn and continue" as D-04 says; the README (`README.md:925-927`) and test `runner.save.interval_retry` encode the current behaviour; (2) the runner-level "directory at the `.sav` path gives exit 1 at exit" test cannot work because the runner reads the `.sav` first and a directory there gives exit 4 on macOS (reproduced this session) and is platform-dependent elsewhere; (3) `nesturbator_unload_cartridge` returns early when no cartridge is loaded (`src/cartridge.c:236`), so with one instance for the adapter's lifetime a test-card session would leak `audio_rem`, `ticks` and `frame_number` into the next one unless the early return goes; (4) the CTest registry forbids skip codes (`policy.no-skip`), so the "skipped if root" guard must be in-process and still pass.

Everything else is straightforward. The helper replaces `nesturbator_run_save_flush`'s body (`runner/save.c:76-114`); `save_state` appears in only four places in `runner/main.c`; the test executable pattern for linking `runner/save.c` directly already exists (`runner.save_path`); `--version` slots into the option loop beside `--hash-audio`; the MMC3 e2e is the existing save-mode of `run_retroarch.cmake` with the span size and ROM parameterised (the derived `m4tkrom.nes` exists at `build/ci/tests/holymapperel/derived/m4tkrom.nes`, 131088 bytes); and the white-box boundary check reads fields of `struct nesturbator` through `src/internal.h`, as `core.save` and `core.cartridge` already do. No external packages are added.

**Primary recommendation:** Implement the helper with exactly the D-02 sequence, put the failure-injection cases in one cross-platform C test plus one POSIX-only `RLIMIT_FSIZE` executable, make `nesturbator_unload_cartridge` always call a shared power-on helper (freeing the cartridge only if present), and parameterise `run_retroarch.cmake`'s hard-coded 32768 for the MMC3 leg.

## Conflicts with CONTEXT.md (planner must resolve; recommendations given)

| # | CONTEXT says | Code/test says | Evidence | Recommendation |
|---|---|---|---|---|
| C1 | D-04: "A failed interval flush warns and the run continues; it is retried at exit." | A failed interval flush sets `status = 1`, which ends the loop (`for ... && status == 0`), then the exit flush retries. | `runner/main.c:808` (`for (uint64_t n = 1; n <= opt.frames && status == 0; n++)`), `:858-860` (`... save_if_changed(inst, &save) != 0) status = 1;`), README `:925-927` ("A failed interval write ends the run with exit 1, and the runner tries the write once more before it exits"), test `interval_retry` (`tests/cmake/runner_save.cmake:136-145`: `--frames 3 ... --save-interval 1`, exit 1, exactly 2 attempts) | Keep current behaviour. D-04's operative clause is "Exit statuses do not change". Do not touch `main.c:858-860`. Only the message text the tests match must change (see Pitfall 3). Record in the plan that D-04's first sentence is superseded. [VERIFIED: runner/main.c:808,858-860; README.md:925-927] |
| C2 | D-06: runner-level test "a directory at the `.sav` path gives exit 1 at exit". | `nesturbator_run_save_load` runs first. On macOS a directory at the `.sav` path makes `fopen(...,"rb")` succeed and the length check fails: exit 4. | Reproduced this session: `nesturbator-run: t/saves/battery.sav is 64 bytes; the cartridge's battery RAM is 8192 bytes`, `exit=4`. Audit already lists it as open WR-03 (`.planning/milestones/v2-MILESTONE-AUDIT.md:39`). | Replace with: **non-empty directory at `<save-dir>/battery.sav.tmp`** (no `.sav` present). Load sees ENOENT (FRESH), the run completes, the exit flush fails at `fopen "wbx"` (remove fails on a non-empty dir), exit 1, stderr contains `cannot save` and the `.sav` path, the directory is still there, no `.sav` created. Works on all six platforms. The dir-at-`.sav` case stays at helper level (D-06 row 1). Do **not** try to fix WR-03 in this phase. [VERIFIED: reproduction above] |
| C3 | D-08: unload restores the no-cartridge power-on state. | `if (inst == NULL \|\| inst->cart.bytes == NULL) { return; }` (`src/cartridge.c:236-238`): an instance running the test card is not reset by unload. | With D-07, `retro_load_game(NULL)` calls unload first. After a test-card session `audio_rem` (`src/frame.c:67`), `ticks`, `frame_number` carry over, so the next session's first frame can yield 799 samples instead of 798. `tests/libretro/libretro_host.c:1348` checks `batch_frames == 798` right after `p_load_game(NULL)` following earlier test-card runs (`check_test_card_reset`, `:684-696`). | Make unload always run the shared power-on helper; free the cartridge only `if (cart.bytes != NULL)`. State this in the header comment. Add the unit check "unload on an instance that only ran the test card equals fresh". [VERIFIED: src/cartridge.c:234-252, src/frame.c:28-30,67] |
| C4 | D-06 row 5: "Skipped if a probe `fopen` still succeeds". | `policy.no-skip` fails any registered test with `SKIP_RETURN_CODE`, `SKIP_REGULAR_EXPRESSION` or `DISABLED`; "a test that cannot run somewhere is not registered there". | `tests/cmake/skip_policy.cmake:1-5,28-34`; `tests/CMakeLists.txt:753-758` | Implement the guard inside the C test: probe, and if the probe succeeds print a `note:` line and **return pass for that sub-case**. Add a root-proof sibling case (parent path component is a regular file, ENOTDIR) so a root run still exercises an unwritable-path failure. No CTest skip property. [VERIFIED: skip_policy.cmake] |

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Atomic file replace (`nesturbator_run_file_replace`) | Runner (`runner/save.c`) | — | Library does no file I/O (CONTEXT D-01, CLAUDE.md rule 2); Phase 12 reuses it from the runner |
| Battery-save flush policy (interval, exit, retry) | Runner (`runner/main.c`) | — | Host policy; the library only exposes the span and `nesturbator_save_generation` |
| Version printing | Runner (`--version`) from library `nesturbator_get_version` | libretro adapter keeps compile-time `NT_VERSION` | D-10, D-11 |
| Instance lifetime / content swap | libretro adapter (`libretro/libretro.c`) | Library `nesturbator_unload_cartridge` | D-07 |
| No-cartridge power-on state | Library (`src/instance.c`, `src/cartridge.c`) | — | D-08: one internal helper |
| Frame-boundary invariants (NULL pointers, empty ring) | Library tests (white box over `src/internal.h`) | — | D-16: no new API |
| `.srm` round trip proof | CI job `retroarch-e2e` via `tests/retroarch/run_retroarch.cmake` | libretro adapter (host reads/writes span) | Adapter does no file I/O (`libretro/libretro.c:321-324`) |

## Standard Stack

No external packages. Everything is the C standard library, POSIX, and Win32 already in use.

### Core
| Component | Version | Purpose | Why Standard |
|-----------|---------|---------|--------------|
| C17, extensions off | per `CMakeLists.txt` | All code | Project rule |
| `fopen(..., "wbx")` | C11 | Exclusive create of `<path>.tmp` | Specified by D-02. MSVC: `x` "Forces the function to fail if filename already exists. Can only be used with the "w" or "w+" specifiers" [CITED: learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fopen-wfopen]. glibc and macOS libc support it [ASSUMED] (macOS confirmed indirectly: see "Verification done" below) |
| `fsync` / `_commit` / `rename` / `MoveFileExA` | POSIX / UCRT / Win32 | Durable replace | Already used in `runner/save.c:91-107`. `MOVEFILE_REPLACE_EXISTING`: "If lpNewFileName names an existing directory, an error is reported" [CITED: learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexa] so a directory at the target fails on Windows |
| CTest + `tests/check.h` | CMake >= 3.25 | Tests | Project rule |

### Alternatives Considered
Not applicable: decisions are locked (D-01..D-17).

**Installation:** none.

## Package Legitimacy Audit

No external packages are installed or recommended by this phase. Table omitted deliberately.

**Packages removed due to [SLOP] verdict:** none
**Packages flagged as suspicious [SUS]:** none

## TUNE-07 ground map: `runner/save.c`, callers, `save_state`

### Current write path [VERIFIED: runner/save.c:76-114]
`nesturbator_run_save_flush(const char *path, const uint8_t *span, size_t size, uint8_t *shadow)`:
- `:78-84` builds `tmp` (`char tmp[4096]`, `snprintf "%s.tmp"`); too long prints `cannot write %s.tmp`.
- `:85` `fopen(tmp, "wb")` (not exclusive, no pre-remove).
- `:90-102` write, `fflush`, `fsync(fileno(f))` or `_commit(_fileno(f))`, checked `fclose`. On failure prints `cannot write %s` and returns 1 **leaving the tmp file**.
- `:103-111` `MoveFileExA(tmp, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)` or `rename`. On failure prints `cannot replace %s; the new save is in %s` and returns 1, **leaving the tmp**.
- `:112` `memcpy(shadow, span, size)`.
Includes already present: `<errno.h> <stdio.h> <string.h>`, `<io.h> <windows.h>` / `<unistd.h>`, and `#define _POSIX_C_SOURCE 200809L` at `:1` (needed for `fileno`, `fsync`).

### Header [VERIFIED: runner/save.h:24-27]
Comment: "Replaces `path` with the span's bytes through "<path>.tmp": write, flush, sync, checked close, rename. On success the span is copied to `shadow` and 0 is returned. On failure a message is printed and 1 is returned." Rewrite it for the new helper; add the prototype `int nesturbator_run_file_replace(const char *path, const uint8_t *data, size_t size);` and keep `nesturbator_run_save_flush` as the thin wrapper (its signature is used by `main.c:273` only). Add the D-05 sentence (power cut may show the old save; never torn).

### Every caller [VERIFIED: runner/main.c]
- `:264-277` `save_if_changed(nesturbator *inst, save_state *sv)` is the **only** caller of `nesturbator_run_save_flush` (`:272-274`). It records `sv->generation = gen` only after success (`:275`), so a failure is retried next look.
- Interval flush: `:858-860` inside the frame loop, `f % opt.save_interval == 0u`.
- Exit flush: `:863-865` (`if (save_if_changed(inst, &save) != 0 && status == 0) status = 1;`), runs after any loop end including JAM.
- Load side (not changed): `:758-786`.

### `save_state` -> `battery_save` (D-15) [VERIFIED: grep]
Exactly four spots in `runner/main.c`: `:252` `typedef struct save_state {`, `:258` `} save_state;`, `:264` parameter `save_state *sv`, `:667` local `save_state save;`. No other file references the name (`git grep save_state` outside `.planning`). The local variable `save` and the `sv` parameter need no rename. Also update the header block comment `:20-27`, which says "A write goes to DIR/<name>.sav.tmp and is renamed over the save, so a kill leaves the old save", to add that a failed write leaves no `.tmp`.

### Messages and tests that mention the old strings (must change together)
| Location | Today | Needed |
|---|---|---|
| `tests/cmake/runner_save.cmake:130-135` case `unwritable` | `run_err MATCHES "battery\\.sav\\.tmp"` | New message names the `.sav` path, not the `.tmp`. Match `cannot save .*battery\\.sav` |
| `tests/cmake/runner_save.cmake:136-146` case `interval_retry` | counts `battery\\.sav\\.tmp` occurrences == 2 | Count `cannot save` occurrences == 2 (frame-1 interval attempt + exit retry; run stops after the first failure, see C1) |
| `README.md:909-912` | "...renamed over the save, so a kill leaves the previous save and at most a stale `.tmp`." | Say: never torn; a failed write removes its `.tmp`; a kill mid-write can still leave a stale `.tmp`, which the next write clears; power-cut caveat (D-05) |
| `README.md:916-917, 925-927` | "A save or rename failure ... exits 1" / interval text | Keep, add the one-line message form |

**Message form (D-04):** `nesturbator-run: cannot save <path> (<reason>); the previous save is kept`. `<reason>` = `strerror(saved_errno)`. On Windows `MoveFileExA` does not set `errno`; use `GetLastError()` (format `"Windows error %lu"` is enough; no `FormatMessage` needed). Print the message exactly once per failed call. Note `kept` is a lie when no previous save existed (first flush); acceptable, but a one-clause variant ("no previous save") is optional.

### Helper skeleton (D-01..D-04) [ASSUMED: written this session, not compiled]
```c
/* runner/save.c */
static int sync_stream(FILE *f)
{
#ifdef _WIN32
    return _commit(_fileno(f));
#else
    return fsync(fileno(f));
#endif
}

static void report_failure(const char *path, int err)
{
    fprintf(stderr, "nesturbator-run: cannot save %s (%s); the previous save is kept\n", path,
            strerror(err));
}

int nesturbator_run_file_replace(const char *path, const uint8_t *data, size_t size)
{
    char tmp[4096];
    FILE *f;
    int ok, err = 0;
    if (snprintf(tmp, sizeof tmp, "%s.tmp", path) >= (int)sizeof tmp) {
        report_failure(path, ENAMETOOLONG);
        return 1;
    }
    (void)remove(tmp);              /* stale leftover; fails harmlessly on a non-empty directory */
    f = fopen(tmp, "wbx");
    if (f == NULL) {                /* we created nothing, so we remove nothing */
        report_failure(path, errno);
        return 1;
    }
    ok = fwrite(data, 1, size, f) == size && fflush(f) == 0 && sync_stream(f) == 0;
    if (!ok)
        err = errno;
    if (fclose(f) != 0 && ok) {
        ok = 0;
        err = errno;
    }
#ifdef _WIN32
    if (ok && MoveFileExA(tmp, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == 0) {
        ok = 0;
        err = EACCES;               /* or carry GetLastError() into the message */
    }
#else
    if (ok && rename(tmp, path) != 0) {
        ok = 0;
        err = errno;
    }
#endif
    if (!ok) {
        (void)remove(tmp);          /* we created it */
        report_failure(path, err);
        return 1;
    }
    return 0;
}
```
`nesturbator_run_save_flush` becomes: `if (nesturbator_run_file_replace(path, span, size) != 0) return 1; memcpy(shadow, span, size); return 0;`.

### Behaviours the planner must not get wrong
- `remove()` on POSIX removes an **empty** directory too (it falls back to `rmdir`), so an *empty* foreign directory at `<path>.tmp` is deleted by step 1 and then replaced by the temp file. D-03's "never removes anything it did not create" is strictly true only for non-empty directories and files; the D-06 test uses a non-empty one. Harmless, but do not write a test with an empty directory at `.tmp` expecting it to survive. [ASSUMED: POSIX `remove` semantics; not exercised this session]
- `remove()` on a symlink removes the link, not its target, so D-06's symlink case passes by construction (victim unchanged) and `"wbx"` then creates a fresh regular file. [ASSUMED: POSIX semantics]
- `rename(file, existing_directory)` fails (EISDIR) on POSIX and `MoveFileExA` reports an error for a directory target [CITED: MoveFileEx doc above], so the old "directory at target" state survives and the helper removes its tmp.
- macOS `fsync` does not force the drive cache (`F_FULLFSYNC` does). D-02 specifies fsync; do not add `F_FULLFSYNC` in this phase (D-05 already accepts power-cut limits). [ASSUMED: general macOS knowledge]

## TUNE-07 test ground map

### How tests are registered [VERIFIED: tests/CMakeLists.txt]
- Runner subprocess tests: `nesturbator_runner_test(NAME ARGS EXIT [IGNORE_STDOUT] [EXPECT] [EXPECT_ERR] [CLEAN_DIR])` (`:8-22`) runs `tests/cmake/expect_output.cmake`. It can check **stdout, exit, and stderr substring**, but **not "stderr is empty"** (`expect_output.cmake:55-59` only does a substring find). D-12 needs a new check (see TUNE-09-A).
- Scratch-directory cases for the runner: `runner_save_case(<case>)` (`:139-149`, cases `:150-161`) run `tests/cmake/runner_save.cmake` with `-DWORK=${CMAKE_CURRENT_BINARY_DIR}/save_work/<case>` (recreated each run), fixture `runner_save` from `runner.save.fixture` (writes `battery.nes`, `jam.nes`, `plain.nes`, `seed.sav`, `short.sav` into `build/<preset>/tests/save_fixture`).
- C unit test that links a runner source directly: `runner.save_path` (`:181-186`): `add_executable(runner.save_path runner/test_save.c ${PROJECT_SOURCE_DIR}/runner/save.c)`, `target_include_directories(... ${PROJECT_SOURCE_DIR}/runner)`, `nesturbator_warnings`, `_CRT_SECURE_NO_WARNINGS` for MSVC, `add_test(NAME runner.save_path COMMAND runner.save_path)`. **Copy this pattern** for the new tests. `tests/runner/test_save.c` is the template (includes `"save.h"` and `"../check.h"`, ends `CHECK_DONE();`).
- Core C tests: `nesturbator_core_test(name source)` (`:473-478`), links `nesturbator::nesturbator`; white-box ones add `target_include_directories(<name> PRIVATE ${PROJECT_SOURCE_DIR}/src)` (e.g. `core.save` `:520-521`, `core.cartridge` `:489-490`).
- Platform gating: **gate at registration**, never skip. Precedent: `if(UNIX)` at `tests/CMakeLists.txt:798`. Compile-time `#ifndef _WIN32` inside a C test is the other tool. There is no existing `if(WIN32)` in `tests/CMakeLists.txt` (grep).
- Skip policy: no `SKIP_RETURN_CODE`/`SKIP_REGULAR_EXPRESSION`/`DISABLED` anywhere (`policy.no-skip`, `:753-758`).
- Formatting: the `hygiene` preset runs clang-format 18 (`.clang-format`: LLVM base, IndentWidth 4, ColumnLimit 100, Linux braces). New C files must be formatted to it.
- Scripts in `tests/cmake/*.cmake` must set `cmake_minimum_required(VERSION 3.25)` (`cmake.script_policy`).

### Recommended new tests
| Test (CTest name) | File | Registration | Cases |
|---|---|---|---|
| `runner.save_replace` | `tests/runner/test_save_replace.c` (links `runner/save.c`) | all platforms; arg1 = scratch base dir; fixture setup does `cmake -E rm -rf` then `make_directory` on it | D-06 rows 1-4 on all six; rows 5 and 7 inside `#ifndef _WIN32`; plus root-proof ENOTDIR case (a regular file as parent component) on all six |
| `runner.save_fsize` | `tests/runner/test_save_fsize.c` (links `runner/save.c`) | `if(UNIX)` only; separate executable (Discretion item: yes) | D-06 row 6 |
| `runner.save.dir_at_tmp` | case in `tests/cmake/runner_save.cmake` + `runner_save_case(dir_at_tmp)` | all platforms | C2 replacement for the runner-level test |
| updated `runner.save.unwritable`, `runner.save.interval_retry` | `tests/cmake/runner_save.cmake` | all | match new message (see table above) |

**C test portability helpers** (no existing ones; `git grep mkdtemp` is empty): `mkdir` is `mkdir(path, 0700)` from `<sys/stat.h>` on POSIX and `_mkdir(path)` from `<direct.h>` on Windows; `rmdir`/`_rmdir`; `unlink`/`remove`. Create case directories under `argv[1]`, each with a distinct name, and remove what the case created (restore `0755` before removing a `chmod 0555` directory, **even when a CHECK failed**, or the next `cmake -E rm -rf` cannot clean it).

**Test sources need `_POSIX_C_SOURCE 200809L`** (for `symlink`, `fileno`, `setrlimit` declarations under `-std=c17`) at the top of the file, as `runner/save.c:1` does. On Windows skip the POSIX headers with `#ifndef _WIN32`. This compiled and ran this session on macOS with `cc -std=c17 -Wall -Wextra -pedantic -D_POSIX_C_SOURCE=200809L` for `signal(SIGXFSZ,...)`, `setrlimit(RLIMIT_FSIZE,...)`, `stat`; Linux/glibc is [ASSUMED] equivalent.

### Root and chmod (C4)
A directory with mode `0555` does not stop root (or a process with CAP_DAC_OVERRIDE). GitHub-hosted runners run the build as a non-root user [ASSUMED], so CI exercises it; a local Docker/devcontainer run as root would fail the assertion. Guard:
```c
/* after chmod(dir, 0555): */
FILE *probe = fopen(probe_path, "wb");
if (probe != NULL) {                 /* root, or an ACL: the directory is writable */
    fclose(probe);
    remove(probe_path);
    fprintf(stderr, "note: chmod 0555 does not block this user; case not applicable\n");
} else {
    CHECK(nesturbator_run_file_replace(target, data, size) == 1);
    /* old bytes intact, no .tmp */
}
chmod(dir, 0755);                    /* always restore */
```
The root-proof sibling (ENOTDIR) uses `target = "<scratch>/afile/x.sav"` where `afile` is a regular file: `fopen("<scratch>/afile/x.sav.tmp","wbx")` fails for every user including root, on every platform.

### `RLIMIT_FSIZE` feasibility [VERIFIED on macOS this session; Linux ASSUMED]
Reproduced in the scratchpad (`cc -std=c17 -D_POSIX_C_SOURCE=200809L`): with `signal(SIGXFSZ, SIG_IGN)` and `rlim_cur = 16`, `fwrite` of 8192 bytes returned 16, `errno` was `File too large` (EFBIG), `fflush` and `fclose` returned 0, and the file stayed 16 bytes. So the helper's `fwrite(...) == size` check catches it. Findings the planner needs:
1. **Ignore SIGXFSZ before the first write** (default action terminates the process).
2. **The limit applies to every regular file the process writes**, including stdout/stderr when they are redirected to a file: my first run lost its own `printf` output because the tool's stdout was a file. Restore the soft limit (`setrlimit` back to the saved `rlim_cur`; raising soft up to hard is allowed) before any CHECK output, and run it as its own executable. This is why a separate process is recommended. CTest normally captures through pipes, which are not limited, but do not rely on it.
3. Create the "old" file **before** lowering the limit; assert after restoring it.
4. Use data larger than the stdio buffer (>= 8192 bytes) and a limit smaller than it (e.g. 16) so the failure is a short `fwrite`, not only a failing `fflush`.
5. Under the `asan` preset this should still work; ASan only writes on error. [ASSUMED]
6. Windows: not registered (`if(UNIX)`).

## TUNE-09 ground map

### A. `--version` in the runner [VERIFIED: runner/main.c]
- Option loop `parse_options` `:164-249`. `--hash-audio` is the standalone-switch precedent at `:179-184`, handled **before** `value = i < argc ? argv[i++] : NULL;` at `:185`. Add `--version` the same way, before `:185`.
- Because `:245-247` returns `usage("--frames N is required")` for a run with no `--frames`/`--movie`, the loop must **return 0 immediately** on `--version` (set a new `int show_version` in `struct options` `:139-153`), so `--version` alone works (D-10). "Handled when the loop reaches it" also means `--bogus --version` is still a usage error (exit 2) and `--version --bogus` prints the version first. Then in `main` right after the `status = parse_options(...)` block `:673-677`: `if (opt.show_version) { print; free_options(&opt); return fflush(stdout) == 0 ? 0 : 1; }`. `free_options` is `:155-160`.
- Format: `nesturbator_version v; memset(&v, 0, sizeof v); v.size = (uint32_t)sizeof v; nesturbator_get_version(&v); printf("nesturbator-run %u.%u.%u\n", v.major, v.minor, v.patch);` (cast to `unsigned`/use `%lu` with casts; warnings are errors). The struct is `include/nesturbator.h:84-92` (`size, major, minor, patch, abi, behaviour_revision`), function `:225`, body `src/instance.c:34-47` (returns without writing if `out->size == 0`).
- `usage()` `:49-64`: add `[--version]` to the synopsis and an `  --version            print the version and exit` line. Update the top-of-file block comment `:3-5` and the exit-status comment `:29`.
- Test (D-12): `expect_output.cmake` cannot assert empty stderr. Add `tests/cmake/runner_version.cmake` (`cmake_minimum_required(VERSION 3.25)`; args `RUNNER`, `SOURCE_DIR`): `file(STRINGS "${SOURCE_DIR}/version.txt" v LIMIT_COUNT 1)` (this is how `tests/cmake/version_consistency.cmake:9` reads it; value today `0.1.9`), run the runner with `OUTPUT_VARIABLE out ERROR_VARIABLE err RESULT_VARIABLE rc`, strip `\r`, require `rc == 0`, `out == "nesturbator-run ${v}\n"`, `err == ""`. Register `runner.version` via `add_test(... -DRUNNER=$<TARGET_FILE:nesturbator-run> -DSOURCE_DIR=${PROJECT_SOURCE_DIR} -P .../runner_version.cmake)` next to `version.consistency` (`tests/CMakeLists.txt:725-728`). Read `version.txt` at **test time**, not configure time, because release-please rewrites it. Add a second case `--version` combined with a bad option exits 2 via `nesturbator_runner_test` if desired.

### B. libretro adapter lifetime [VERIFIED: libretro/libretro.c]
Current state: `retro_init` is empty (`:80-82`); `retro_deinit` destroys `inst` and resets all statics (`:85-102`); `retro_load_game` (`:261-296`) refuses when `inst != NULL` (`:266`), checks `SET_PIXEL_FORMAT` (`:269`), then creates the instance (`:275`), loads (`:279-284`, destroying `inst` on failure), caches the palette and the save span (`:285-294`); `retro_unload_game` (`:307-313`) destroys `inst` and clears the span; `retro_get_memory_*` read the cached span (`:325-333`); `retro_run` returns early when `inst == NULL` (`:181-183`); `retro_get_system_av_info` calls `nesturbator_get_info(inst, ...)` which is a no-op for NULL (`src/instance.c:51`).

Required edits for D-07:
- `retro_init`: if `inst != NULL` destroy first (a frontend may re-init without deinit; cheap and safe), then `nesturbator_config cfg = {size, abi}; if (nesturbator_create(&cfg, &inst) != NESTURBATOR_OK) inst = NULL;`. The `cfg` setup is currently in `retro_load_game` `:272-274` and moves.
- `retro_load_game`: keep the argument and pixel-format checks first so a rejected call changes nothing; return false if `inst == NULL`; then `save_data = NULL; save_size = 0; nesturbator_unload_cartridge(inst);` then `if (game != NULL && nesturbator_load_cartridge(...) != NESTURBATOR_OK) return false;` (**no destroy**; `nesturbator_load_cartridge` leaves the previous cartridge untouched on rejection, `include/nesturbator.h` comment above `:295`, and unload ran first, so the instance has no cartridge, as D-07 requires), then palette and span as today. The comment at `:286-287` ("fetched once, here ... stays the same for the life of the instance") must change to "from this load until the next unload".
- `retro_unload_game`: clear `save_data`/`save_size` first, then `nesturbator_unload_cartridge(inst)` (NULL-safe: `src/cartridge.c:236`). Double unload is safe because unload on a no-cartridge instance only re-applies power-on state (after C3).
- `retro_deinit`: unchanged (it already destroys and clears everything).
- Behaviour changes to expect: `retro_run` before any load now emits a test-card frame instead of nothing; `retro_get_system_av_info` before load now returns the real timing (the host test never calls it before load).
- Comments cite libretro.h lines in the adapter's style (`L7594` etc.): `retro_deinit` contract at `libretro/libretro.h:7584-7600` ("Cores should use this function to clean up all allocated resources and reset all global variables"); `retro_unload_game` at `:7803-7810` ("This is called before retro_deinit(void)"); `retro_init` at `:7582`.
- Header comment at top (`:1-7`) and README libretro text (`README.md:~996-1007`, "fetches the span once in `retro_load_game`") must be updated.

### C. Shared power-on helper [VERIFIED: src/instance.c, src/cartridge.c, src/internal.h]
What `nesturbator_create` sets (`src/instance.c:103-110`): `memset(inst, 0, sizeof *inst)`; `allocator`; `ppu.scanline = 261u`; `apu.dmc.buffer_empty = 1u`; `profile.ane_magic`/`lxa_magic`.
What unload resets (`src/cartridge.c:239-251`): frees `cart.bytes`, zeroes `cart`, `apu` (then `dmc.buffer_empty = 1`), `synth_reset`, **`ppu` to zero**, `cpu`, `mapper`, `map`, and `cpu_cycle`, `ticks`, `frame_number`, `audio_rem`. It does **not** touch `bus`, `profile`, `transition_sink(_context)`, or `save_generation`.
What load resets (`src/cartridge.c:352-363`): `bus`, `apu`(+`buffer_empty`), `synth`, `ppu` (zero, scanline 0), `cpu` (+`s=0xfd,p=0x24`), counters. **Leave load alone** (D-08: no pinned hash changes).

Difference between a fresh instance and an unloaded one, field by field: `ppu.scanline` 261 vs 0; `bus` dirty vs zero (RAM, open bus, controller state); everything else matches once `audio_rem`/`ticks`/`frame_number` are reset (they are, but only when a cartridge was loaded: C3).

Recommended shape (name is Discretion): declare `void nesturbator__power_on_no_cartridge(struct nesturbator *nes);` in `src/internal.h` next to the other `nesturbator__*` declarations; define it in `src/instance.c`:
```c
/* The state of a created instance with no cartridge. Create calls it on a zeroed
   instance; unload calls it after releasing the cartridge. NESdev "PPU power up
   state": rendering starts before visible line 0 so pre-render work can populate state. */
void nesturbator__power_on_no_cartridge(struct nesturbator *nes)
{
    memset(&nes->bus, 0, sizeof nes->bus);
    memset(&nes->apu, 0, sizeof nes->apu);
    nes->apu.dmc.buffer_empty = 1u;
    nesturbator__synth_reset(nes);
    memset(&nes->ppu, 0, sizeof nes->ppu);
    nes->ppu.scanline = 261u;      /* [HWP.03][HWP.06] as in create today */
    memset(&nes->cpu, 0, sizeof nes->cpu);
    memset(&nes->mapper, 0, sizeof nes->mapper);
    memset(&nes->map, 0, sizeof nes->map);
    nes->cpu_cycle = 0; nes->ticks = 0; nes->frame_number = 0; nes->audio_rem = 0;
}
```
`nesturbator_create` keeps `allocator` and `profile`, calls the helper. `nesturbator_unload_cartridge` becomes: `if (inst == NULL) return; if (inst->cart.bytes != NULL) { free; } memset(&inst->cart,0,...); helper;`. `nesturbator__mapper_load`'s failure path (`src/cartridge.c:373-377`) already calls unload and stays correct. `nesturbator_reset` (`src/instance.c:158-176`) is unaffected. Do not reset `save_generation` (header `:383-394` and `src/internal.h:~190-193` say never). Wiping `bus` clears pending host input (`bus.input_pending`), same as a load does; acceptable.

Header comment for unload: there is currently **no comment on the declaration** (`include/nesturbator.h:295`; only the sentence "Unload releases cartridge state." ends the load comment). Add one above it: releases the cartridge and returns the instance to its just-created, no-cartridge state (test card, counters at zero); safe with no cartridge; keeps `nesturbator_save_generation`; the battery span pointer becomes NULL/0. Comments are stripped by the API policy hash (`tests/cmake/vector_api_policy.cmake:11-12` removes `/* */` and `//`), so comment-only header edits do not trip `vectors.api_policy`; do not add declarations. Also add the `get_version` sentence (D-13) above `:225`.

White-box equality test (D-09 library): in a new or existing core test with `src/` on the include path, build A = create; B = create, load a synthetic image, run a few frames, unload, and compare `memcmp` per sub-struct (`bus`, `apu`, `synth`, `ppu`, `cpu`, `mapper`, `map`, `cart`) and the scalars `ticks`, `frame_number`, `cpu_cycle`, `audio_rem`, excluding `save_generation` and the allocator. Also: A and B run N frames of the test card and compare `io.ticks`, `io.frame_number`, `io.audio_count` and video bytes per frame; and a third instance that ran the test card, then unload (no cartridge), then frames again. Synthetic images: `tests/ines.h` `ines_build` (as `tests/core/test_save.c:20-45`). `core.cartridge` already includes `src/` (`tests/CMakeLists.txt:489-490`); `core.api` does not.

### D. D-16 frame-boundary white-box [VERIFIED: src/frame.c, src/apu.c, src/synth.c, src/internal.h]
Frame-scoped output pointers and their owners:
- `nes->ppu.video_output` and `nes->ppu.video_pitch` (`src/internal.h` struct `nesturbator__ppu`): set `src/frame.c:53-54`, cleared `:60-61` (cartridge path; the JAM return at `:62-64` comes after the clear).
- `nes->apu.sample_output`, `sample_limit`, `sample_count` (`src/internal.h:~98-101`): set by `nesturbator__apu_begin_frame` (`src/apu.c:177-184`), cleared by `nesturbator__apu_end_frame` (`:186-199`).
- PCM ring: `nes->synth.pcm_count` (and `pcm_read`, `pcm_write`), filled by `nesturbator__synth_sample` (`src/synth.c:2764-2770`), drained by `nesturbator__synth_drain` (`:2772-2784`) in `apu_end_frame` for exactly `sample_count` samples. So at every return from `nesturbator_run_frame` the invariant is `video_output == NULL && video_pitch == 0 && sample_output == NULL && sample_limit == 0 && sample_count == 0 && synth.pcm_count == 0 && synth.pcm_read == synth.pcm_write` (and `!synth.overflow`).
- Early error returns (`frame.c:9-37`) happen before any state change; the no-cartridge path never sets pointers.
Test: `core.boundary` (name Discretion), `nesturbator_core_test` + `src/` include dir. Assert the invariant after: test-card frames; many frames of a synthetic image that enables APU channels (so the ring is non-trivially used: `$4015`, `$4000-$4003` writes via `tests/ines.h` code; 798/799 alternating); the frame that returns `NESTURBATOR_STOP_JAM` (opcode `$02`, as `tests/runner/save_rom.c` `jam.nes`); after a `NESTURBATOR_ERR_BUFFER_TOO_SMALL` call (audio_capacity 10); and after unload/reload. No API addition is needed.

### E. D-17 status numbers [VERIFIED]
- Runner exit codes: `0` (`runner/main.c:77,248,432,619`...), `1` (many), `2` (`:63` `usage()`, `:690`), `4` (`:773` `ls == NESTURBATOR_RUN_SAVE_MISMATCH ? 4 : 1`). A scan of `return [3-9]` / `exit(` in `runner/*.c` finds only that `4`; no `3`, no `77`. `tests/CMakeLists.txt` has no `EXIT 3` or `EXIT 77`. `main.c:29-30` and `README.md:932-933`: "3 and 77 are reserved". `README.md:490` says the AccuracyCoin mode "never exits 77".
- Library status enum (`include/nesturbator.h:55-74`), values quoted verbatim: `NESTURBATOR_OK = 0`, `NESTURBATOR_ERR_ARGUMENT = 1`, `NESTURBATOR_ERR_STRUCT_SIZE = 2`, `NESTURBATOR_ERR_ABI = 3`, `NESTURBATOR_ERR_NO_MEMORY = 4`, `NESTURBATOR_ERR_BUFFER_TOO_SMALL = 5`, `NESTURBATOR_STOP_JAM = 6`, `NESTURBATOR_ERR_CARTRIDGE = 7`. The runner never returns a library status as its exit code (it prints it), so library 3/4 do not collide with runner exit 3/4. Keep `4` for the `.sav` size mismatch; `runner.save.mismatch` (`runner_save.cmake:74-95`) already pins it.
- Confirmation task: a verification step that re-runs the grep and records it; a permanent source-scan test is optional (small cmake script) and not needed by the success criteria.

## TUNE-08 ground map: `retroarch-e2e` MMC3 leg

### How the MMC1 round trip works today [VERIFIED]
`.github/workflows/ci.yml:178-259` job `retroarch-e2e` (macos-15): builds with `cmake --workflow --preset ci`, installs and checksum-verifies the pinned RetroArch 1.22.2 app, runs `run_retroarch.cmake` in **game mode** (Nesteroids, frame 60, `:211-223`) and in **save mode** (`:224-238`):
`-DSAVE_ROM="$GITHUB_WORKSPACE/tests/roms/hm/M1_P512K_CR8K_S32K.nes" -DSAVE_FRAME=400 -DRA_DIR=".../build/retroarch-e2e/run-save"`, then uploads artifacts (`:239-259`, including `run-save/saves/M1_P512K_CR8K_S32K.srm`).
`tests/retroarch/run_retroarch.cmake`:
- Waits on **markers, not sleeps**: RetroArch is run with `--max-frames=<frame> --max-frames-ss --max-frames-ss-path=...` and `TIMEOUT 50` (`:134-139`); the process exits itself after that frame and writes the screenshot; there is no `sleep`. The `.srm` is written at content unload when RetroArch exits. So "mirror it" means reuse this machinery unchanged.
- Save mode (`:196-229`): `check_one_srm` requires exactly one file `saves/<stem>.srm` **of 32768 bytes (hard-coded, `:206-209`)**; after session 1 it requires `SAVEDATA` (`5341564544415441`) at offset 256 (`:218-222`); session 2 copies the `.srm` to `runner-saves/<stem>.sav` and runs the runner with `--save-dir`; screenshots of s1 and s2 must differ (`:226-229`); the `<stem>` comes from `cmake_path(GET mode_rom STEM stem)` (`:84-85`); each session's frame equals the runner's frame for the same save state (`ra_session`, `:119-188`).
- The header comment (`:6-19`) documents the modes.

### What the MMC3 leg needs
1. **Parameterise the span size.** Add `-DSAVE_SIZE=<bytes>` (default 32768 to keep the MMC1 invocation unchanged) and use it in `check_one_srm` (`:206-209`, message at `:208`). For `m4tkrom` it is `8192` (roms.cmake entry `m4tkrom:derived/m4tkrom.nes:600:8K PRG RAM OK:save:8192:0=b6`, `tests/holymapperel/roms.cmake:28`). Update the header comment and the stale "SXROM" comment at `:216`.
2. **ROM path:** `$GITHUB_WORKSPACE/build/ci/tests/holymapperel/derived/m4tkrom.nes` (the derived ROM is built by the `ALL` target `holymapperel-derived`, `tests/CMakeLists.txt:241`; exists locally at 131088 bytes; the e2e job runs `cmake --workflow --preset ci` first so it exists there). Stem will be `m4tkrom`, so the `.srm` is `saves/m4tkrom.srm`. [VERIFIED locally; CI path ASSUMED to follow `build/ci` as the other `-D` paths do]
3. **Frame:** `600` (roms.cmake N for `m4tkrom`).
4. **Marker:** the same `SAVEDATA` at 0x100 holds: `holymapperel.m4tkrom.run1.sav` uses the shared `-DOFFSET=256 -DHEX=5341564544415441` check for every `:save` entry (`tests/CMakeLists.txt:317-320`), and `run1.sav_byte` pins `0=b6` (MMC3 write-protect byte). Session screens differ because run 1 reads `8K PRG RAM OK` and run 2 `8K PRG RAM OK + BATTERY` (`tests/CMakeLists.txt:313-316,364-366`).
5. **New workflow step** after the MMC1 step: same `cmake -D... -P tests/retroarch/run_retroarch.cmake` with `-DRA_DIR=".../build/retroarch-e2e/run-save-mmc3"`, `-DSAVE_ROM`, `-DSAVE_FRAME=600`, `-DSAVE_SIZE=8192`. Use a **separate `RA_DIR`**: the script `file(REMOVE_RECURSE "${RA_DIR}")` at `:99` and `check_one_srm` requires exactly one file in `saves/`.
6. **Artifacts** (`:239-259`): add `run-save-mmc3/runner-s1.ppm`, `runner-s2.ppm`, `shot-s1.{png,bmp}`, `shot-s2.{png,bmp}`, `saves/m4tkrom.srm` (`if-no-files-found: error` is set, so a wrong path fails the job).
7. **Timing:** 600 frames at 60 fps is ~10 s per session against `TIMEOUT 50`, up from ~6.7 s for 400. [ASSUMED fine; the macOS runner is not throttled by audio because `audio_driver = "null"`]
8. **README** `:1074-1087` describes only the SXROM step; add the MMC3 step (and artifact list).
9. This cannot be run locally in CI form; the script header says it needs a logged-in GUI session and no CTest case launches it (`tests/retroarch/CMakeLists.txt:30-31`). A local macOS dry run is possible (`build/retroarch-local` exists) but is not a gate. A script change should still be sanity-checked with `cmake -P` argument validation paths, and the CI job is the proof.

## Architecture Patterns

### System flow: battery save write
```
frame loop (runner/main.c:808)
   |-- interval: f % N == 0 --> save_if_changed (:264) --.
   `-- loop end / JAM ---------> save_if_changed (:864) --+--> generation changed? and span != shadow?
                                                          |        no -> return 0 (file untouched)
                                                          v        yes
                                        nesturbator_run_save_flush(path, span, size, shadow)
                                                          |
                                                          v
                                        nesturbator_run_file_replace(path, span, size)   [NEW, runner/save.c]
                       remove(path.tmp) -> fopen(path.tmp,"wbx") -> fwrite/fflush/fsync -> fclose
                                                          |                                   |
                                     fopen fails: return 1                          rename / MoveFileExA
                                     (nothing of ours to remove)                      |            |
                                                                                   ok            fail
                                                                                    |              |
                                              memcpy(shadow, span) <- return 0      |   remove(path.tmp); message; return 1
                                                                                    |   (shadow & generation not advanced -> retried at exit)
```

### System flow: libretro lifetime after D-07
```
retro_set_environment -> retro_init: create instance (inst or NULL)
retro_load_game(game|NULL): checks -> clear span -> unload_cartridge (always power-on) -> load or test card -> cache palette+span
retro_run*: frames on the same instance
retro_unload_game: clear span -> unload_cartridge (instance stays)
retro_load_game ... (any number of times)
retro_deinit: destroy instance, reset statics
```

### Recommended file touch list
```
runner/save.c, runner/save.h      helper + wrapper + comments (D-01..D-05)
runner/main.c                     rename battery_save; --version; usage; header comments
src/internal.h, src/instance.c    power-on helper declaration + definition; create uses it
src/cartridge.c                   unload always resets; header unaffected otherwise
include/nesturbator.h             comments only (unload, get_version)
libretro/libretro.c               lifecycle (D-07), comments
tests/runner/test_save_replace.c  NEW  (all platforms + #ifndef _WIN32 parts)
tests/runner/test_save_fsize.c    NEW  (UNIX)
tests/core/test_boundary.c        NEW  (D-16) ; plus lifecycle equality check (D-09) in it or core.cartridge
tests/cmake/runner_version.cmake  NEW
tests/cmake/runner_save.cmake     update unwritable, interval_retry; add dir_at_tmp
tests/CMakeLists.txt              register the above
tests/libretro/libretro_host.c    lifecycle checks (D-09)
tests/retroarch/run_retroarch.cmake  SAVE_SIZE
.github/workflows/ci.yml          MMC3 step + artifacts
README.md                         see list below
```

### README sections that must change (CLAUDE.md rule 6) [VERIFIED line numbers]
- Runner synopsis `:838-840` and options list: add `--version`; state the runner calls `nesturbator_get_version`.
- `--save-dir` bullet `:902-921`: the `.tmp` sentence `:909-912`, failure text, power-cut caveat (D-05), message form.
- `--save-interval` bullet `:922-930`: unchanged semantics (see C1); keep wording accurate.
- Exit status line `:932-933`: unchanged.
- libretro section `:975-1008`: instance lives from `retro_init` to `retro_deinit`; `retro_load_game` unloads first; `retro_unload_game` calls `nesturbator_unload_cartridge`; span fetched at each load and dropped at unload (`:1003-1008`).
- Test descriptions: `:222-232` area and `:340-381`: mention `runner.save_replace`, `runner.save_fsize`, `runner.version`, `core.boundary`, the new host checks.
- RetroArch e2e `:1074-1090`: MMC3 step and artifacts.
- Public header comments: `nesturbator_unload_cartridge` (new), `nesturbator_get_version` (sentence), and the `nesturbator_get_memory` comment `:383-394` still says "valid ... until unload, the next successful load or destroy" (still true).

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Atomic replace | A second temp/rename routine for Phase 12 | `nesturbator_run_file_replace` | One helper is TUNE-07; Phase 12 and 14 call it |
| Process limits / signals in tests | A fork-based harness | Separate executable run by CTest | `RLIMIT_FSIZE` affects every file the process writes (see feasibility notes) |
| Mkdir/tempdir helpers | A cross-platform fs library | `mkdir`/`_mkdir`, `remove`, `rmdir`/`_rmdir` under `#ifdef`; scratch dir created by a CTest fixture | Tiny surface; project forbids dependencies |
| Version text | Parsing `version.txt` in C | `nesturbator_get_version()` in the runner, `version.txt` read by a cmake test script | D-10/D-12 chain |
| New CTest skip mechanism | `SKIP_RETURN_CODE` | In-process guard that passes, or registration gating | `policy.no-skip` |
| Synthetic ROMs | Hand-written header bytes | `tests/ines.h` `ines_build` | Existing, used by `core.save`, `libretro.host` |

## Common Pitfalls

### Pitfall 1: Cleaning up something the helper did not create
**What goes wrong:** removing `<path>.tmp` after `fopen "wbx"` failed deletes a foreign non-empty directory's... nothing (remove fails) but could delete a foreign *file* created by a racing process.
**How to avoid:** only `remove(tmp)` on paths reached after `fopen` returned non-NULL (skeleton above).
**Warning signs:** a test with a directory at `.tmp` that is gone afterwards.

### Pitfall 2: Reporting `errno` after cleanup
`remove(tmp)` can overwrite `errno`. Save the first failing `errno` before cleanup (D-04). In the skeleton `err` is captured at each failure.

### Pitfall 3: Old message strings in existing tests
`runner.save.unwritable` and `runner.save.interval_retry` match `battery.sav.tmp`. They will fail until updated (table in TUNE-07 ground map). They are the only two tests that depend on the old strings (`git grep` of `sav.tmp` in tests).

### Pitfall 4: RLIMIT_FSIZE kills or silences the test
Default `SIGXFSZ` terminates the process; and the limit also truncates the test's own redirected stdout/stderr. Ignore the signal, restore the limit before printing, separate executable.

### Pitfall 5: Unload early return (C3)
`src/cartridge.c:236` returns before touching a no-cartridge instance. Adapter-lifetime reuse then carries `audio_rem` etc. into the next session and can flip `libretro_host.c:1348` (`batch_frames == 798`). Remove the early return for the reset.

### Pitfall 6: Putting the power-on helper into `load_cartridge`
Load already zeroes the PPU to scanline 0; calling a helper that sets 261 there would change every pinned game hash (`tests/runner/hashes.txt`, `write_hashes`). Only create and unload call it.

### Pitfall 7: `--version` blocked by the "--frames required" check
`parse_options` ends with `usage("--frames N is required")` (`:245-247`). Return from the loop at `--version`.

### Pitfall 8: Existing libretro host tests assuming a fresh instance per load
`libretro.host` calls `p_unload_game(); ... p_load_game(...)` many times. With one instance every such call now reuses state. Tests that assume "first frame after load(NULL)" produce 798 samples (`:1348`) and the exact test-card frame hash work only if unload fully resets (C3). Run the whole `libretro.host` and all `libretro.parity.*` after the adapter change, and under `asan`.

### Pitfall 9: Scratch directories left read-only
A failed CHECK between `chmod 0555` and the restore leaves a directory `cmake -E rm -rf` cannot empty. Restore unconditionally.

### Pitfall 10: `run_retroarch.cmake` size and comment constants
`check_one_srm` hard-codes 32768 (`:206-209`). Forgetting `SAVE_SIZE` makes the MMC3 leg fail with "m4tkrom.srm is 8192 bytes, expected 32768".

### Pitfall 11: Reusing `RA_DIR` for the second save round trip
The script wipes `RA_DIR` and asserts exactly one `.srm`. Give MMC3 its own directory.

## Code Examples

### `--version` handling (in `parse_options`, before `value = ...`)
```c
/* runner/main.c, next to --hash-audio (:179-184) */
if (strcmp(arg, "--version") == 0) {
    o->show_version = 1;
    return 0;                       /* nothing else is required for --version */
}
```
```c
/* main(), right after the parse_options status check (:673-677) */
if (opt.show_version) {
    nesturbator_version v;
    memset(&v, 0, sizeof v);
    v.size = (uint32_t)sizeof v;
    nesturbator_get_version(&v);
    printf("nesturbator-run %u.%u.%u\n", (unsigned)v.major, (unsigned)v.minor, (unsigned)v.patch);
    free_options(&opt);
    return fflush(stdout) == 0 ? 0 : 1;
}
```
[ASSUMED: written this session, not compiled]

### libretro `retro_load_game` after D-07
```c
bool retro_load_game(const struct retro_game_info *game)
{
    enum retro_pixel_format format = RETRO_PIXEL_FORMAT_XRGB8888;

    if (inst == NULL || (game != NULL && (game->data == NULL || game->size == 0u))) {
        return false;
    }
    if (env_cb == NULL || !env_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &format)) {
        return false;
    }
    save_data = NULL;
    save_size = 0u;
    nesturbator_unload_cartridge(inst);   /* power-on state; a failed load below leaves it so */
    if (game != NULL &&
        nesturbator_load_cartridge(inst, game->data, game->size) != NESTURBATOR_OK) {
        return false;
    }
    nesturbator_get_palette(inst, palette, 512u);
    if (game != NULL && nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &save_data,
                                               &save_size) != NESTURBATOR_OK) {
        save_data = NULL;
        save_size = 0u;
    }
    return true;
}
```
[ASSUMED: written this session, not compiled]

### Host lifecycle checks to add to `tests/libretro/libretro_host.c` (call from `main` near `:1313`)
- `check_lifecycle_reuse()`: load image A (use `build_save_image` `:578-601` or `dummy_bytes`), run N frames capturing `frame[]` and `batch_samples`, `p_unload_game()`, `CHECK(p_get_memory_data(RETRO_MEMORY_SAVE_RAM) == NULL && p_get_memory_size(...) == 0)`, load A again without `p_deinit`, run N frames, `memcmp` video and audio of the last frame (and a running hash across all N) against the first pass and against a direct-API instance as `check_save_memory` (`:603-681`) and `check_reset_frame_parity` (`:469`) do.
- `load -> unload -> load(NULL)` vs a first-ever `load(NULL)` (capture the first test-card frame early in `main`, before other checks, or compare with `argv[2]` testframe PPM as `compare_with_ppm(argv[2])` does at `:1358`).
- Double `p_unload_game()`; bad image (`game.size = 1`) returns false then good load works; reload with a battery image after a non-battery one returns the new span.
- `rom_parity` (`:1083-1198`) already loads once; optionally extend with load-twice.
All run under the asan preset because `libretro.host` is a normal CTest.

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Keep new bytes in `<save>.tmp` after a failed rename | Remove the `.tmp`, keep memory copy, retry later | this phase (D-03) | No stale or confusing files |
| Instance created at `retro_load_game`, destroyed at `retro_unload_game` | Created at `retro_init`, destroyed at `retro_deinit` | this phase (D-07) | `retro_serialize_size` can answer without content (Phase 13) |

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | glibc supports `fopen` mode `x` (C11) | Standard Stack | Linux legs fail to compile/behave; trivially detectable by the first ci run |
| A2 | POSIX `remove()` removes empty directories and removes (not follows) symlinks | TUNE-07 behaviours | A test expectation for an empty directory at `.tmp` or the symlink case would differ; plan only uses non-empty dir and symlink-victim checks |
| A3 | GitHub-hosted runners run tests as non-root | Root and chmod | If root, the in-process guard handles it anyway |
| A4 | Linux `RLIMIT_FSIZE` yields a short `fwrite`/EFBIG like macOS | RLIMIT feasibility | The executable would fail on Linux; fix by checking `fflush` result too (helper already does) |
| A5 | `GetLastError()` is the right source for the Windows rename failure reason | Skeleton | Message text only |
| A6 | 600 RetroArch frames stay well inside `TIMEOUT 50` | TUNE-08 | Raise the timeout in `ra_session` |
| A7 | macOS `fsync` limits (no `F_FULLFSYNC`) are acceptable | TUNE-07 behaviours | Covered by D-05 |
| A8 | The CI path of the derived ROM is `build/ci/tests/holymapperel/derived/m4tkrom.nes` | TUNE-08 | Step fails fast (`-DSAVE_ROM` existence check at `run_retroarch.cmake:73-75`) |

## Open Questions (RESOLVED)

1. **C1: interval-flush behaviour vs D-04 wording.**
   - Known: code, README and `interval_retry` all say a failed interval flush ends the run with exit 1.
   - Unclear: whether the owner intended to change it.
   - Recommendation: keep behaviour; treat D-04's first sentence as a wording error; record in PLAN and README unchanged.
   - **RESOLVED (C1):** keep today's behaviour; only the message text changes (plan 11-01).
2. **C2: dir at `.sav` path (WR-03).** The runner reports exit 4 (macOS) or something platform-dependent. Out of scope; use the `.tmp`-directory runner test. If the owner wants WR-03 fixed, it is a separate change to `nesturbator_run_save_load` (`fstat`/`S_ISDIR`).
   - **RESOLVED (C2):** the runner-level case uses a non-empty directory at `<save-dir>/battery.sav.tmp` (`runner.save.dir_at_tmp`); the helper-level test covers a directory at the target; WR-03 stays out of scope.
3. **Where the D-09 library equality test lives:** extend `core.cartridge` (already has `src/` include) or add it to `core.boundary`. Either works; choose one file to avoid duplicated image builders.
   - **RESOLVED:** the D-09 equality test extends `core.cartridge` (`tests/core/test_cartridge.c`).

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| cmake | build, tests | yes | `/opt/homebrew/bin/cmake` (needs >= 3.25) | — |
| ninja | build | yes | `/opt/homebrew/bin/ninja` | — |
| C compiler (`cc`) | build | yes | `/usr/bin/cc` (Apple clang) | — |
| clang-format 18 | `hygiene` preset | not probed | — | install per `CONTRIBUTING.md`; format check runs only in hygiene lane |
| RetroArch 1.22.2 app | TUNE-08 proof | only on the hosted `macos-15` job | pinned by SHA-256 in `ci.yml` | none; the e2e job is the gate (no local test, `tests/retroarch/CMakeLists.txt:30-31`) |
| root vs non-root shell | chmod test | local user is uid 501 (non-root) | — | in-process guard |

**Missing dependencies with no fallback:** none for planning.
**Missing dependencies with fallback:** none.

## Validation Architecture

`workflow.nyquist_validation` is `false` in `.planning/config.json`: section omitted per instructions. A requirement-to-test map for the planner:

| Req | Test | Command |
|-----|------|---------|
| TUNE-07 | `runner.save_replace`, `runner.save_fsize` (UNIX), `runner.save.dir_at_tmp`, updated `runner.save.unwritable` / `interval_retry` | `ctest --test-dir build/ci -R "runner\.save"` then `cmake --workflow --preset ci` |
| TUNE-08 | hosted `retroarch-e2e` MMC3 step (CI only) | pull-request CI |
| TUNE-09 | `runner.version`, new `libretro.host` checks, core equality test, `core.boundary` | `ctest --test-dir build/ci -R "runner\.version|libretro|core\."`; `cmake --workflow --preset asan` |

## Security Domain

`workflow.security_enforcement` is `false`: section omitted. (Relevant hardening already in the decisions: exclusive create `"wbx"`; no `.tmp` follow-through symlinks.)

## Project Constraints (from CLAUDE.md)

- Own code only; `libretro.h` the only vendored file; core links only C memory functions; runner/helper file I/O stays in the runner (rule 1, 2).
- Clean tree: no ROM bytes outside `tests/roms/manifest.txt`; no personal paths/emails/names; derived `m4tkrom` is built, not committed (rule 3, D-14).
- Clean room: do not open GPL/LGPL emulator source or RetroArch source; `libretro.h` and docs are fine (rule 4). This research opened none.
- Deterministic, integer-only core (rule 5): the power-on helper adds no state outside the instance.
- Every behaviour change lands with a test run by `cmake --workflow --preset ci`, and README plus public-header comments change in the same change (rule 6).
- One GSD step per command; phase branch; PR with a Conventional Commit title (rule 7).
- Style: small modules, plain control flow, fixed-width types; hardware comments cite sources; ponytail ladder (reuse what is here, standard library, platform, then minimum new code).
- Workflow: file changes go through a GSD command (`.claude/CLAUDE.md`); `CMAKE` commands: `cmake --workflow --preset ci`, `... asan`.
- Memory notes: verification must be automated, no human UAT; end each step naming the finished phase and the exact next command.

## Sources

### Primary (HIGH confidence)
- Repo files read this session with line ranges cited inline: `runner/save.c`, `runner/save.h`, `runner/main.c`, `libretro/libretro.c`, `libretro/libretro.h` (7570-7600, 7752-7830), `src/instance.c`, `src/cartridge.c`, `src/frame.c`, `src/apu.c`, `src/synth.c`, `src/internal.h`, `include/nesturbator.h`, `tests/CMakeLists.txt`, `tests/cmake/runner_save.cmake`, `tests/cmake/expect_output.cmake`, `tests/cmake/skip_policy.cmake`, `tests/cmake/version_consistency.cmake`, `tests/cmake/vector_api_policy.cmake`, `tests/libretro/libretro_host.c`, `tests/retroarch/run_retroarch.cmake`, `tests/holymapperel/roms.cmake`, `.github/workflows/ci.yml`, `README.md`.
- Experiments this session (macOS arm64, Apple clang, `-std=c17 -D_POSIX_C_SOURCE=200809L`): `RLIMIT_FSIZE` short-write behaviour; runner exit 4 with a directory at the `.sav` path.

### Secondary (MEDIUM confidence)
- [CITED: learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fopen-wfopen] mode `x`.
- [CITED: learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexa] `MOVEFILE_REPLACE_EXISTING` and `MOVEFILE_WRITE_THROUGH` semantics.

### Tertiary (LOW confidence)
- POSIX/glibc/macOS libc behaviours marked [ASSUMED] in the Assumptions Log.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH, no new dependencies.
- Architecture / ground map: HIGH, all line numbers read this session.
- Pitfalls: HIGH for repo-derived items (C1-C4, 3, 5-7, 10, 11); MEDIUM for Linux/Windows runtime behaviours.

**Research date:** 2026-10-11
**Valid until:** 2026-11-10 (stable; re-check line numbers if other phases land first)
