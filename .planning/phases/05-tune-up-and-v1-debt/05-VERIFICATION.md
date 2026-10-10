---
phase: 05-tune-up-and-v1-debt
verified: 2026-10-10T12:00:00Z
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
  - "tests/libretro/libretro_host.c"
covered_digest: "v3:sha256:daf8b87e919a4f0be091d32df24bfdf603289b1c1339b0791288acfe114db0a0"
behavior_unverified: 0
overrides_applied: 0
human_verification:
  - test: "After the phase PR's CI run completes, fill 05-CI-RECORD.md 'After' with the slowest leg's job wall time (gh run view ... --json jobs) and compare it with the Before figure of 181 s"
    expected: "Slowest leg time recorded; no regression against 181 s. This is a scripted gh query, not a manual test"
    why_human: "The branch is not pushed, so no hosted run exists. Checkable only once the PR CI run exists"
  - test: "On the PR's CI run, confirm all six build legs and the asan leg pass with CTest jobs 4, and the retroarch-e2e job passes with no ***Skipped line"
    expected: "Green on six platforms; policy.no-skip and retroarch.compare pass"
    why_human: "Hosted-runner-only evidence (parallel safety on Windows and Linux, RetroArch on a hosted Mac). Automated, post-PR"
  - test: "First scheduled or dispatched nightly run of suite-flake (ctest --preset ci --repeat until-fail:3 --schedule-random)"
    expected: "Passes inside its timeout (see WR-02)"
    why_human: "Only a hosted run can show it. Automated, post-merge"
---

# Phase 5: Tune-up and v1 debt Verification Report

**Phase Goal:** CI is fast and trustworthy, the v1 debt is closed, and a player who presses reset in RetroArch gets the console's soft reset.
**Verified:** 2026-10-10
**Status:** human_needed (the remaining items are automated checks that exist only after the PR's CI run; none is a person-run UAT)
**Re-verification:** No, initial verification

## Goal Achievement

### Observable Truths (ROADMAP success criteria)

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | CTest runs in parallel from test presets; nightly flake job; slowest-leg before/after and ccache decision recorded | VERIFIED locally; "After" pending hosted run | `CMakePresets.json` test presets carry `"jobs": 4`. `nightly.yml:181` runs `ctest --preset ci --repeat until-fail:3 --schedule-random`. `05-CI-RECORD.md` records Before (slowest leg `build (macos-15-intel, ci, macos, x64)`, 181 s) and the ccache decision (compile share 4 to 22 percent, under the 40 percent rule, no ccache). "After" is recorded as pending the PR run (carried as a post-PR check below) |
| 2 | Release-policy self-test rejects an extra publish-gate condition; pins/labels current; AccuracyCoin pin reviewed | VERIFIED | `tests/cmake/release_policy.cmake:225-227` mutates the gate with `\|\| always()` and expects rejection; also line 257. `release.*` tests pass (re-run by me). `05-CI-RECORD.md` lists six action pins against latest releases and the AccuracyCoin pin `673ef550...` reviewed, 7 commits behind, not moved |
| 3 | Synthetic trainer-bearing iNES built in C test code; runner and libretro frames equal in a CTest case | VERIFIED | `tests/ines.h` builder; `check_trainer_frame_parity` in `tests/libretro/libretro_host.c:389` with negative control (no trainer gives a different colour); `libretro.host` passes |
| 4 | CI job fails when an expected test skips; local RetroArch tests removed, hosted `retroarch-e2e` carries the evidence | VERIFIED (hosted run pending) | `policy.no-skip` and `.self_test` registered and passing (`tests/cmake/skip_policy.cmake`); only `retroarch.compare` and `.cli` remain in the suite, driven by `ci.yml` job `retroarch-e2e`. The first hosted run with no `***Skipped` line is post-PR |
| 5 | `nesturbator_reset()` keeps CPU RAM, cart RAM; SP-3, I set; APU silenced; PPU write-ignore window; `retro_reset()` calls it; libretro frames equal direct-API frames; README and header updated | VERIFIED | `src/instance.c` `nesturbator_reset`, `src/cpu.c` `cpu_reset` (S lowered by 3, vector fetch), `ppu_reset` sets `reset_flag` ignoring `$2000/$2001/$2005/$2006`, `apu_reset`. `libretro/libretro.c:155` `retro_reset` calls it. `core.reset` and `libretro.host` pass. Header `nesturbator.h:307`; README sections at lines 523 and 694 |

**Score:** 5/5 truths verified (no behavior-dependent truth left unexercised: `core.reset` exercises the transitions)

### CR-01 assessment (reset leaves scanline-0 pixels unwritten)

Decision: the claim does not hold, and TUNE-06 is met. The mechanism the review cites is real: the reset's 7 CPU cycles advance the PPU to scanline 0 dot 21 with `video_output == NULL` (`src/ppu.c:300`, `src/frame.c:53-60`). The consequence is not. `run_frame` runs a whole-frame tick count (a whole number of scanlines), so a frame that starts at dot 21 of scanline 0 ends at dot 21 of the next scanline 0 and writes x=0..20 of row 0 in the same buffer at the end of the frame. I confirmed this by running a probe program against `build/ci/libnesturbator.a` (public API only, synthetic NROM): run 2 frames, `nesturbator_reset`, then run a frame into a buffer filled with 0xFFFF. Result: 0 sentinel pixels remain, and 0 on the following frame. The determinism rule is therefore not broken for the first post-reset frame; the frame is merely phase-shifted by 21 dots, deterministically.

What remains true of the finding: no test pins this. Recommend adding the sentinel-buffer assertion the review suggests (as a regression guard), but this is not a blocker. The disposition file still shows CR-01 and all others as `open`; the orchestrator should triage it.

### Required Artifacts

| Artifact | Status | Details |
|----------|--------|---------|
| `src/instance.c`, `src/cpu.c`, `src/ppu.c`, `src/apu.c` reset halves | VERIFIED | Substantive and wired (instance calls ppu, apu, cpu resets) |
| `libretro/libretro.c` `retro_reset` | VERIFIED | Calls `nesturbator_reset` when an instance exists |
| `tests/core/test_reset.c` | VERIFIED | Registered as `core.reset`, passes |
| `tests/ines.h` | VERIFIED | Used by core and libretro tests |
| `tests/cmake/release_policy.cmake`, `skip_policy.cmake` | VERIFIED | Registered, passing, with mutation self-tests |
| `nightly.yml` `suite-flake`, `05-CI-RECORD.md` | VERIFIED (After pending) | See truth 1 |

### Requirements Coverage

| Requirement | Status | Evidence |
|-------------|--------|----------|
| TUNE-01 | SATISFIED except the "after" timing, which awaits the PR CI run | parallel presets, nightly flake job, ccache decision recorded |
| TUNE-02 | SATISFIED | pins bumped (upload and download artifact), labels reviewed, AccuracyCoin pin reviewed, README and header updated |
| TUNE-03 | SATISFIED | `|| always()` mutation rejected |
| TUNE-04 | SATISFIED | trainer parity test in `libretro.host` |
| TUNE-05 | SATISFIED (hosted run pending) | `policy.no-skip`, local RetroArch tests removed |
| TUNE-06 | SATISFIED | see truth 5 and CR-01 assessment |

All six IDs are in REQUIREMENTS.md (marked Complete) and in the plans; no orphaned requirements.

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|----------|---------|--------|--------|
| Full suite | `cmake --workflow --preset ci` | completed, packages generated; 358 tests registered | PASS |
| Reset, libretro host, no-skip, release policies | `ctest --preset ci -R ...` | 12/12 passed | PASS |
| First post-reset frame has no unwritten pixels | scratch probe program | 0 sentinels | PASS |

### Anti-Patterns Found

No unreferenced TBD, FIXME or XXX markers were scanned in a blocking way; none were noted in the files read. Review warnings, for triage (none block the goal):

| Item | Severity | Note |
|------|----------|------|
| WR-02 `suite-flake` `timeout-minutes` of 10 against a cold build plus three suite runs | Warning | Likely a false nightly flake; the reviewer suggests 30 |
| WR-03 APU `$4017` re-apply delay taken from the pre-reset phase | Warning | Deterministic approximation; state it in the header or fix |
| WR-01 parity test compares the core with itself | Warning | `core.reset` holds the hardware assertions, so the claim is covered there |
| IN-01, IN-02, IN-03 | Info | Cosmetic or test-hardening |

### Pre-existing, not a phase gap

`cmake --workflow --preset nofp` fails `abi.undefined_symbols` (`__stack_chk_fail`, `__stack_chk_guard` in `src/synth.c`). The orchestrator reproduced it on base commit 751f8b7, so it predates this phase. I did not re-run it. It should be tracked separately, since `nofp` is a documented preset.

### Human Verification Required

These are automated post-PR checks, not manual UAT (see the frontmatter list): the "After" CI timing, six-platform parallel safety and the retroarch-e2e result with no skips, and the first nightly `suite-flake` run.

### Gaps Summary

No gaps block the phase goal. CR-01 was tested and refuted for the observable symptom. The status is `human_needed` solely because three checks need a hosted CI run that cannot exist until the branch is pushed and a PR is opened.

---

_Verified: 2026-10-10_
_Verifier: Claude (gsd-verifier)_
