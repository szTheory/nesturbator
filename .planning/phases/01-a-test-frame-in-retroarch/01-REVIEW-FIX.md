---
phase: 01-a-test-frame-in-retroarch
fixed_at: 2026-10-03T02:10:00Z
review_path: .planning/phases/01-a-test-frame-in-retroarch/01-REVIEW.md
iteration: 1
findings_in_scope: 11
fixed: 9
skipped: 2
status: partial
---

# Phase 1: Code Review Fix Report

**Fixed at:** 2026-10-03T02:10:00Z
**Source review:** .planning/phases/01-a-test-frame-in-retroarch/01-REVIEW.md
**Iteration:** 1

**Summary:**
- Findings in scope: 11 (CR-01, CR-02, WR-01 to WR-09; scope critical_warning)
- Fixed: 9 (WR-09 only in part)
- Skipped: 2 (WR-03, WR-08)

**Where verification ran:** in the main checkout on branch `phase/01-test-frame`. `workflow.use_worktrees` is false, so no worktree was made. Before each commit, `cmake --workflow --preset ci` (31 tests, then 32 after WR-05) and `cmake --workflow --preset hygiene` (6 tests) passed. `asan` and `nofp` also passed after WR-05, and `nofp` passed again after WR-07. The `scripts/hygiene.sh` pre-commit hook ran on every commit and was never bypassed.

## Fixed Issues

### CR-01: hygiene.sh never scans tracked files whose names contain non-ASCII bytes

**Files modified:** `scripts/hygiene.sh`, `tests/hygiene/scan_selftest.sh` (new), `tests/hygiene/CMakeLists.txt`, `README.md`
**Commit:** fbd5420
**Applied fix:**
- A `git()` wrapper turns `core.quotePath` off.
- The `--staged` and `--tree` lists are now read with `-z` and changed to newlines with `tr`.
- `check_file` reports `cannot read file` when it cannot read a file's content, so the scan fails closed. A name containing a newline splits into parts that name no file, so it fails closed too.
- New test `hygiene.scan_selftest` (hygiene lane) builds a scratch repository. It checks that a file named `café.txt` is flagged for its home path and its address, and that a name containing a newline is reported as unreadable.
- I checked that the test fails against the old script.

### CR-02: Under a UTF-8 locale, grep -I skips non-UTF-8 text

**Files modified:** `scripts/hygiene.sh`, `tests/hygiene/scan_selftest.sh`, `README.md`
**Commit:** df8d840
**Applied fix:**
- The script now sets and exports `LC_ALL=C`. The per-command `LC_ALL=C` on the binary rule became redundant and was removed.
- The self-test adds a Latin-1 file and runs the scan under `LC_ALL=C.UTF-8`.
- I checked that the test fails against the previous script.

### WR-01: --frames 4294967295 makes the runner loop forever

**Files modified:** `runner/main.c`
**Commit:** 566e06e
**Applied fix:** The frame loop now counts in `uint64_t`, and `f` is cast to `uint32_t` inside the loop.
**Status:** fixed: requires human verification. No CTest is added, because hitting the limit means running 2^32-1 frames. The commit message says so. The fix is two lines and is easy to check by reading.

### WR-02: The runner exits 0 when --frames is missing

**Files modified:** `runner/main.c`, `tests/CMakeLists.txt`, `README.md`
**Commit:** 7fb81fb
**Applied fix:**
- `parse_options` returns `usage("--frames N is required")` (exit 2) when `--frames` is absent.
- The usage text and the file comment no longer show `--frames` in brackets.
- New test `runner.usage.noargs`.
- The README says the option is required.
- I did not add a test with options but no `--frames`: it would already exit 2 through the "beyond --frames" check, so it would not show this fix.

### WR-04: The allocator contract states no alignment requirement

**Files modified:** `include/nesturbator.h`
**Commit:** 22b310e
**Applied fix:** The allocator comment now says alloc returns memory aligned as malloc's is: suitable for any object type and at least the alignment of `max_align_t`. This only documents the contract and changes no behaviour, so no test was added. I did not add the review's optional runtime rejection of misaligned pointers.

### WR-05: Configuring from another CMake project fails outside the six release targets

**Files modified:** `CMakeLists.txt`, `cmake/packaging.cmake`, `tests/embed/CMakeLists.txt` (new), `tests/CMakeLists.txt`, `README.md`
**Commit:** f7befb1
**Applied fix:**
- `project()` now enables C only.
- Things that stay unconditional:
  - the library and its `library`-component install rules
  - the CMake package and pkg-config files
  - the `LICENSE` install for the `library` component
  - the `NESTURBATOR_NOFP` flag on the core
- After those, `if(NOT PROJECT_IS_TOP_LEVEL) return()`. Everything below runs only for the top-level project:
  - `enable_language(CXX)`
  - `nesturbator_host`
  - the runner and libretro `LICENSE`/notice installs
  - `enable_testing`
  - the runner, libretro, palgen, tests, abi and hygiene subdirectories
  - packaging
- `packaging.cmake` no longer stops with `FATAL_ERROR` on an unknown system or processor. It names the archives with the lower-cased value CMake reports and prints a `WARNING`.
- New test `embed.subdirectory` builds `tests/embed`, a C-only project that calls `add_subdirectory` on the source tree. It fails if embedding turns on C++, adds `nesturbator_host`, `nesturbator-run`, `nesturbator_libretro` or `palgen`, or writes a `CPackConfig.cmake`. It then runs one frame using `tests/consumer/main.c`.
- I checked that the test fails against the old tree ("nesturbator enabled C++").
- The README now has an embedding section.
- **Not exercised:** the packaging fallback on an actual non-release platform. There is no such toolchain here.

### WR-06: --history never checks commit message bodies

**Files modified:** `scripts/hygiene.sh`, `tests/hygiene/scan_selftest.sh`, `README.md`
**Commit:** c795d10
**Applied fix:**
- History mode now scans each commit's message (`git log -1 --format=%B`) for home paths and addresses.
- Messages use `MESSAGE_EMAIL_OK`, which is `EMAIL_OK` plus the Co-Authored-By trailer address. That address is the Co-Authored-By trailer the project requires on every commit, and it is the only address in the existing history. File contents still use the stricter `EMAIL_OK`.
- The self-test commits a message holding a home path and an address and expects both findings. It then amends to a clean message carrying the trailer and expects a pass.
- `scripts/hygiene.sh --history` over the whole real history exits 0.
- **For the owner:** this allowance is my choice. Please confirm you want the trailer address allowed in commit messages.

### WR-07: The float scan misses hex floating literals

**Files modified:** `tests/cmake/float_scan.cmake`, `README.md`
**Commit:** c6f0d81
**Applied fix:**
- Each line is first checked against `0[xX][0-9a-fA-F]*\.?[0-9a-fA-F]*[pP][+-]?[0-9]`, before hex integers are removed.
- The SELFTEST sample now has `z = 0x1p3;` and `w = 0x1.8p1;` and expects exactly lines 3, 4 and 5.
- The real scan of `src/` and `include/` is still clean, and the `nofp` lane passes.

### WR-09: CI installs Ninja without apt-get update; tight timeouts (partial)

**Files modified:** `.github/workflows/ci.yml`
**Commit:** 9d3dc4a
**Applied fix:**
- Both Ninja steps now run `command -v ninja || { sudo apt-get update && sudo apt-get install -y ninja-build; }`.
- I checked that the YAML still parses with PyYAML. This can only really be tested on GitHub Actions.
- **Not changed: the timeouts.** Plan 01-12 and ENGINEERING section 5 set them on purpose to twice the measured cold run. Raising them to 10 minutes or more would replace that rule, which is the owner's decision. 01-12-SUMMARY already names the risk: Windows x64 took 65 s against its 2-minute limit.

## Skipped Issues

### WR-03: The size-tag rule rejects larger structs whose padding bytes are not zero

**File:** `include/nesturbator.h:11-16`
**Reason:** The owner needs to decide this. The review offers two contracts and leaves the choice open:
- callers must `memset` boundary structs to zero, or
- the library promises to append only fields that leave no padding, checked with `_Static_assert`.

Either one changes the header's convention, which it says is "fixed; later releases only append". This should be settled before the first release.
**Original issue:** In C, a brace-initialised automatic struct does not guarantee zero padding. A host built against a newer, larger struct can then randomly get `NESTURBATOR_ERR_STRUCT_SIZE` from an older library, because the bytes past the old `sizeof` include padding that may not be zero.

### WR-08: The one-time release-as pin has only a manual todo to remove it

**File:** `release-please-config.json:10`
**Reason:** The owner needs to decide this. The suggested check fails while the pin is present and `.release-please-manifest.json` is at or above it. But the 0.1.0 release PR itself bumps the manifest to 0.1.0 while the pin is still needed to produce that version. So the check would turn that PR red, and under `CI required` the release could not merge. release-please also rewrites its own branch, so the pin cannot be removed inside the release PR. A safe check needs a decided order, for example:
- check for a published `v0.1.0` tag, which needs a full-history fetch, or
- run the check only on pushes to `main` after the release merge.

The STATE.md Pending Todo stays the safeguard for now.
**Original issue:** If `release-as: "0.1.0"` is left in place, every later release PR proposes 0.1.0 again.

---

_Fixed: 2026-10-03T02:10:00Z_
_Fixer: Claude (gsd-code-fixer)_
_Iteration: 1_
