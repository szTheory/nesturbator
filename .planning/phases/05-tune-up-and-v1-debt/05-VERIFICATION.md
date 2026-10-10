---
phase: 05-tune-up-and-v1-debt
verified: 2026-10-10T15:00:00Z
status: human_needed
score: 5/5 must-haves verified
covered_files:
  - ".github/workflows/ci.yml"
  - ".github/workflows/nightly.yml"
  - ".github/workflows/release.yml"
  - ".planning/phases/05-tune-up-and-v1-debt/05-01-PLAN.md"
  - ".planning/phases/05-tune-up-and-v1-debt/05-01-SUMMARY.md"
  - ".planning/phases/05-tune-up-and-v1-debt/05-02-PLAN.md"
  - ".planning/phases/05-tune-up-and-v1-debt/05-02-SUMMARY.md"
  - ".planning/phases/05-tune-up-and-v1-debt/05-03-PLAN.md"
  - ".planning/phases/05-tune-up-and-v1-debt/05-03-SUMMARY.md"
  - ".planning/phases/05-tune-up-and-v1-debt/05-04-PLAN.md"
  - ".planning/phases/05-tune-up-and-v1-debt/05-04-SUMMARY.md"
  - ".planning/phases/05-tune-up-and-v1-debt/05-05-PLAN.md"
  - ".planning/phases/05-tune-up-and-v1-debt/05-05-SUMMARY.md"
  - ".planning/phases/05-tune-up-and-v1-debt/05-06-PLAN.md"
  - ".planning/phases/05-tune-up-and-v1-debt/05-06-SUMMARY.md"
  - ".planning/phases/05-tune-up-and-v1-debt/05-07-PLAN.md"
  - ".planning/phases/05-tune-up-and-v1-debt/05-07-SUMMARY.md"
  - ".planning/phases/05-tune-up-and-v1-debt/05-08-PLAN.md"
  - ".planning/phases/05-tune-up-and-v1-debt/05-08-SUMMARY.md"
  - "CMakePresets.json"
  - "README.md"
  - "include/nesturbator.h"
  - "libretro/libretro.c"
  - "src/apu.c"
  - "src/cpu.c"
  - "src/instance.c"
  - "src/ppu.c"
  - "tests/CMakeLists.txt"
  - "tests/cmake/release_policy.cmake"
  - "tests/cmake/skip_policy.cmake"
  - "tests/core/test_reset.c"
  - "tests/ines.h"
  - "tests/libretro/libretro_host.c"
covered_digest: "v3:sha256:f1dcc82a1666c5779af4ca2fd5d3a79881b8754b1f907e3b7236dacc843ae6df"
behavior_unverified: 0
overrides_applied: 0
re_verification:
  previous_status: human_needed
  previous_score: 5/5
  gaps_closed:
    - "G-05-2: gcc-14 -Werror=conversion at tests/core/test_reset.c:32 (fixed in source; hosted confirmation pending)"
    - "G-05-3 / WR-02: suite-flake timeout 10 -> 30 minutes (fixed in source; first nightly run pending)"
  gaps_remaining: []
  regressions: []
human_verification:
  - test: "Push the branch and let PR #25 CI re-run. Confirm build (ubuntu-24.04), build (ubuntu-24.04-arm) and both nofp legs now compile and pass"
    expected: "All six build legs, asan and retroarch-e2e green; no ***Skipped line; policy.no-skip and retroarch.compare pass"
    why_human: "The 05-08 commits (9ed3041, 1a8fc55) are not pushed. The last hosted run (38058075941) is still red on the Linux gcc legs. Only a hosted run can show green. Scripted gh query, post-push"
  - test: "After that run, fill 05-CI-RECORD.md 'After' with the slowest leg's wall time and compare it with 181 s"
    expected: "No regression against 181 s"
    why_human: "The earlier run stopped at compile on Linux, so no complete set of leg times exists. Scripted gh query, post-push"
  - test: "First scheduled or dispatched nightly run of suite-flake and vectors-full"
    expected: "Both build, and suite-flake passes inside the 30-minute timeout; then set the timeout to twice the measured cold run"
    why_human: "Nightly run 38058075997 never reached the suite. Only a hosted run can measure it. Automated, post-merge"
---

# Phase 5: Tune-up and v1 debt Verification Report

**Phase Goal:** CI is fast and trustworthy, the v1 debt is closed, and a player who presses reset in RetroArch gets the console's soft reset.
**Verified:** 2026-10-10
**Status:** human_needed
**Re-verification:** Yes, after gap-closure plan 05-08

## What the hosted-CI state means for this verdict

PR #25 CI has not re-run since 05-08. The last hosted result (run 38058075941) is red: both Linux build legs and both nofp legs failed to compile. This report does not claim those legs are green. The evidence for the fix is local only:

- Source: `tests/core/test_reset.c:32` is `spec.chr_8k = (uint8_t)(chr_ram ? 0u : 1u);`. `git diff 6e7cebc..HEAD` over non-planning files shows exactly two changed lines (that line and the nightly timeout).
- I compiled `tests/core/test_reset.c` myself in a `gcc:14` Docker container with `-std=c17 -Wall -Wextra -Wconversion -Werror`. It produced no diagnostics. This is a single-file compile, not the full preset build. The executor reports a full gcc:14 container build of the `ci` and `nofp` presets with zero warnings; I did not repeat that.
- The orchestrator's local Apple clang `cmake --workflow --preset ci` passed 358/358. I re-ran 20 relevant tests (`core.reset`, `libretro.host`, `policy.*`, `release.*`, `vectors.registration_policy*`): 20/20 passed.

The goal is met in the code. Whether "CI is trustworthy" holds on the hosted runners is unproven until the push, so the status stays `human_needed`. The remaining items are scripted post-push checks, not manual UAT.

## Goal Achievement

### Observable Truths (ROADMAP success criteria)

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | CTest runs in parallel; nightly flake job; slowest-leg before/after and ccache decision recorded | VERIFIED locally; "After" pending hosted run | `CMakePresets.json` test presets carry `"jobs": 4`. `nightly.yml:161` suite-flake `timeout-minutes: 30`, running `ctest --preset ci --repeat until-fail:3 --schedule-random`. `05-CI-RECORD.md` has Before (181 s) and the ccache decision. "After" cannot be measured until a complete hosted run exists |
| 2 | Release-policy self-test rejects an extra publish-gate condition; pins/labels current; AccuracyCoin pin reviewed | VERIFIED | `release.*` tests pass (re-run). Mutation with `\|\| always()` is rejected in `tests/cmake/release_policy.cmake`. Pins and the AccuracyCoin pin are recorded in `05-CI-RECORD.md` |
| 3 | Synthetic trainer-bearing iNES built in C test code; runner and libretro frames equal in a CTest case | VERIFIED | `tests/ines.h` builder, `check_trainer_frame_parity` in `tests/libretro/libretro_host.c`; `libretro.host` passes |
| 4 | CI job fails when an expected test skips; local RetroArch tests removed; hosted `retroarch-e2e` carries the evidence | VERIFIED locally; hosted run pending | `policy.no-skip` and its self-test pass. On the last hosted run, retroarch-e2e and macOS/Windows/asan passed; the Linux legs did not compile |
| 5 | `nesturbator_reset()` semantics; `retro_reset()` calls it; libretro frames equal direct-API frames; README and header updated | VERIFIED | `core.reset` and `libretro.host` pass; reset halves in `src/instance.c`, `src/cpu.c`, `src/ppu.c`, `src/apu.c`; `libretro/libretro.c` `retro_reset` calls `nesturbator_reset`; header and README updated |

**Score:** 5/5 truths verified in the codebase (3 of them still lack hosted confirmation, listed above).

### Gap closure (05-08)

| Gap | Status | Evidence |
|-----|--------|----------|
| G-05-2 gcc-14 `-Werror=conversion` | Fixed in source, hosted confirmation pending | Cast present at `test_reset.c:32`; my single-file gcc:14 compile is clean; executor reports full container builds of `ci` and `nofp` clean |
| G-05-3 / WR-02 suite-flake timeout | Fixed in source, unmeasured | `nightly.yml:161` is 30. The value is a ceiling, not a measurement; the comment says so |

### Code review weighing

- **CR-01** (first post-reset frame leaves scanline-0 pixels unwritten): still open in the disposition file. The prior verification refuted the symptom with a probe against the public API (the frame is a whole number of scanlines, so the 21 skipped dots are written at the end of the same frame; 0 sentinel pixels remained). It does not block TUNE-06 or the phase goal. A sentinel-buffer regression assertion is still worth adding. Triage it as `deferred` or `fixed`, not left `open`.
- **WR-01, WR-03, IN-01, IN-02, IN-03**: none blocks the goal. WR-03 is a deterministic approximation of APU frame-counter timing after reset; state it in the header or fix it later.
- **WR-02**: fixed by 1a8fc55.

### Requirements Coverage

| Requirement | Status | Evidence |
|-------------|--------|----------|
| TUNE-01 | SATISFIED except "After" timing | parallel presets, nightly flake job, ccache decision; the After figure awaits a complete hosted run |
| TUNE-02 | SATISFIED | pins and labels reviewed, AccuracyCoin pin reviewed |
| TUNE-03 | SATISFIED | publish-gate mutation rejected |
| TUNE-04 | SATISFIED | trainer parity test in `libretro.host` |
| TUNE-05 | SATISFIED locally, hosted run pending | `policy.no-skip`; local RetroArch tests removed |
| TUNE-06 | SATISFIED | `core.reset`, `libretro.host`, `retro_reset` wiring |

All six IDs appear in REQUIREMENTS.md and in the plans; no orphans.

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|----------|---------|--------|--------|
| Relevant tests | `ctest --preset ci -R "core.reset\|libretro.host\|policy\|release"` | 20/20 passed | PASS |
| gcc-14 compile of the fixed file | `gcc:14` container, `-Wall -Wextra -Wconversion -Werror -c tests/core/test_reset.c` | no diagnostics | PASS (single file) |
| Full `ci` workflow (orchestrator, Apple clang) | `cmake --workflow --preset ci` | 358/358 | PASS (reported, not re-run by me) |

### Anti-Patterns Found

No unreferenced TBD, FIXME or XXX markers in the two files 05-08 changed. Review items are listed above; none blocks.

### Pre-existing, not a phase gap

`cmake --workflow --preset nofp` previously failed `abi.undefined_symbols` (`__stack_chk_fail`, `__stack_chk_guard` in `src/synth.c`) on base commit 751f8b7. The 05-08 summary reports nofp passing 5 of 5 inside the gcc:14 container, which may mean the failure is toolchain-specific to macOS. I did not re-run it; confirm on the hosted nofp legs.

### Human Verification Required

Three scripted post-push checks (see frontmatter): the PR CI re-run on all legs, the "After" timing in `05-CI-RECORD.md`, and the first nightly `suite-flake` and `vectors-full` run.

### Gaps Summary

No code gap blocks the phase goal. The two UAT blockers (G-05-2, G-05-3) are fixed in source with local and container evidence, but the hosted evidence that closes them does not exist until the 05-08 commits are pushed. Do not mark TUNE-01 or TUNE-05 hosted-proven before then.

---

_Verified: 2026-10-10_
_Verifier: Claude (gsd-verifier)_
