---
phase: 13-rewind-and-run-ahead-in-retroarch
plan: 07
subsystem: retroarch-e2e
tags: [retroarch, savestate, rewind, run-ahead, network-commands, ci]
requires:
  - phase: 13-06
    provides: frame_index, state mode, ra_state_outcome_checks, nesturbator_ra_log_check
provides:
  - lanes state-save, state-load, state-load-ahead and state-net in run_retroarch.cmake
  - tests/retroarch/state_session.sh, a POSIX sh driver for one paused network-command session
  - CI step "Run the RetroArch state, rewind and run-ahead commands" and the retroarch-e2e-state failure artifact
affects: [13-08]
key-files:
  created:
    - tests/retroarch/state_session.sh
  modified:
    - tests/retroarch/run_retroarch.cmake
    - .github/workflows/ci.yml
    - README.md
key-decisions:
  - "RetroArch 1.22.2 names its state files after the content stem (states/nesturbator/nesteroids.state, .state.auto), not game.state; the globs and the session use the stem."
  - "RetroArch 1.22.2 ignores SAVE_STATE after a FRAMEADVANCE (measured), so the session's second save is taken at the frame shown after the 40 rewinds (rew-0), not after the three advances (m3); ahead-1 is checked against rew-0 + 1 + k_eff."
status: complete
requirements-completed: [LR-03]
commits: 2
plan_head_before: 719f67885c8a0a9eb77c2fa1fde6d7794147f84c
plan_head_after: 2d4f991c13733760f1dbf33d26fcc0ac7cf3a163
actuals:
  tokens: 16000
  tasks: 2
  commits: 2
completed: 2026-10-12
---

# Phase 13 Plan 07: RetroArch save, load, rewind and run-ahead lanes Summary

RetroArch 1.22.2 is shown to save, load, rewind and step run-ahead through its network commands, with every screenshot identified as a set of runner frames, the state file's container checked, and every lane log passing the allowlisted error check, locally twice and in the hosted retroarch-e2e job.

## Commits

- 88cd770: save/load lanes, `state_session.sh`, `state-net` lane and assertions.
- 2d4f991: CI step, failure artifact, README.

## Local STATE_ROM output (RetroArch 1.22.2, claim serialized, k_eff 0)

```
-- [state-save] RetroArch's frame 150 equals the runner's frame 150
-- [state-save] state file 4920 bytes, S 4895, magic ok     (frame_number 150 checked)
-- [state-load] RetroArch's frame 179 equals the runner's frame 179
-- [state-load-ahead] RetroArch's frame 179 equals the runner's frame 179
-- [state-net] identified sets: pause-a={491} pause-b={491} adv-1={492;493} adv-2={492;493} adv-3={494} adv-4={495;496} adv-5={495;496} load-1={492;493} load-2={492;493} load-3={494} m1-a={853} m1-b={853} rew-0={817} rew-1={818;819} rew-2={818;819} rew-3={820} ahead-1={818;819}
-- [state-net] state file 4920 bytes, S 4895, magic ok
-- [state-net] pause, advance, load, resume, rewind and run-ahead (lead 0) hold
```
(`;` separates the members of a set.) M0 = {491}; m1 = {853}; rew-0 = {817}; max(rew-0) 817 <= min(m1) - 20 = 833 (the 40 rewinds moved 36 frames); m3 = rew-3 = {820}; ahead-1 {818,819} contains rew-0 + 1 + 0. Two consecutive local runs passed (the other run: M0 {490}, same m1 and rew-0). The load lanes show 179 as RetroArch's measured `-1` (150 + 30 - 1); this is noted in the code comment.

Step 4 sends three FRAMEADVANCEs after LOAD_STATE_SLOT 0: a stricter superset of amended D-25 step 5 (one advance). load-1 alone is the D-25 check; load-2 and load-3 extend the chain.

The scratch edit with `savestate_thumbnail_enable = "true"` made state-save fail through `ra_state_outcome_checks` naming `nesteroids.state.auto.png`: 1.22.2 does write a thumbnail with the key true, so the check guards a real outcome. Restored (scratch template only, not committed).

## Allowlist entries added

None. All six lane logs (including state-net) pass with the two Plan 06 entries; the hosted run needed no new entry.

## Hosted run

https://github.com/szTheory/nesturbator/actions/runs/38291063413 (head 2d4f991). `retroarch-e2e`: success; the new state step took 49 s (18:32:34 to 18:33:23 UTC), well inside the job's 30-minute timeout. Hosted identified sets matched the shape of the local ones (pause {267}, m1 {453}, rew-0 {418}, ahead-1 {419}); this answers RESEARCH open question 1 (the hosted RetroArch accepts the sequence). Other jobs: asan, hygiene, title, nofp (both), build on Linux x64/arm64, macOS arm64, Windows x64/arm64 succeeded. `build (macos-15-intel)` was cancelled at its 7-minute job timeout, twice (and the rerun again), with `libretro.host` and `core.state_fork` tests still running; `CI required` fails because of it. The same job was cancelled on the previous head 719f678 (run 38289751391), before this plan's changes, so it is not caused by this plan; see deferred-items.md.

## Deviations from Plan

**1. [Rule 1 - Bug] State file name is the content stem, not game.state**
- **Found during:** Task 1, first state-save run found no `game.state.auto`.
- **Fix:** globs and the session use `<stem>.state`; the content here is `nesteroids.nes`. Commit 88cd770.

**2. [Rule 1 - Bug] Probe loop spun without delay**
- `nc` fails at once while RetroArch has not bound the port, so 30 tries took 0.2 s. Now sleeps 0.25 s and allows 120 tries (30 s). Commit 88cd770.

**3. [Rule 1 - Bug] Second SAVE_STATE is ignored after FRAMEADVANCE**
- Measured in scratch runs: after any FRAMEADVANCE the save is dropped (only one `Saving state` in the log, file unchanged), while a save after a resume and rewind works. The second save is therefore taken right after the 40 rewinds (rew-0), and ahead-1 is checked against rew-0 + 1 + k_eff instead of m3. The rewind chain rew-1..rew-3 (m3) is still asserted. The save is re-sent each second until the file changes. Commit 88cd770.

**4. [Rule 2] Explicit per-lane checks** instead of a shared function, so each of the six lanes visibly calls `ra_state_outcome_checks` and `nesturbator_ra_log_check`.

Total deviations: 4 (3 Rule 1, 1 Rule 2). Impact: D-25 step 7's save moves from m3 to rew-0; the assertion strength is unchanged.

## Deferred

The macOS Intel build job exceeds its 7-minute timeout (pre-existing); logged in `deferred-items.md`.

## Known Stubs

None.

## Self-Check: PASSED

`tests/retroarch/state_session.sh` (mode 100755), commits 88cd770 and 2d4f991 are ancestors of HEAD.
