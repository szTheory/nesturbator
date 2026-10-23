---
reviewed_source_commit: 1b1589a49f735195b9dbf89a238dc0518777335c
reviewed: 2026-10-15T18:14:30Z
depth: deep
status: clean
reviewed_paths:
  - tests/retroarch/state_session.sh
  - tests/retroarch/test_state_session.sh
  - tests/retroarch/frame_index.c
  - README.md
  - include/nesturbator.h
findings:
  critical: 0
  warning: 0
  info: 0
  total: 0
---

# Plan 16-10: Independent Native Timing Repair Source Review

**Source binding:** `1b1589a49f735195b9dbf89a238dc0518777335c`. I reviewed exactly these five paths at that commit: `tests/retroarch/state_session.sh`, `tests/retroarch/test_state_session.sh`, `tests/retroarch/frame_index.c`, `README.md`, and `include/nesturbator.h`.

## Review

The prior warning WR-01 is resolved. The registered readiness path sets `STATE_SESSION_CONTROL=readiness`; the script now runs the synthetic over-bound and early-error reporting checks for that control, then continues into fixture setup and both real readiness prefixes. The explicit `STATE_SESSION_REPORT_SELFTEST=1` mode still exits after its standalone selftest. The synthetic checks use fixed values and do not sleep or change production deadlines.

The reporter bounds and sanitizes case, numeric fields, status, and reason before printing. It preserves unavailable measurements as `unavailable`, prints before the full-duration assertion, and leaves the driver failure status intact. The actual readiness case still asserts the original operation windows, strict `<25000 ms` driver duration, exact readiness reply, three prefix judgments, one-shot actions, capture counts, and child/host reaping.

The frame-index repair skips only the pre-load replay when the target image follows the native anchor. It keeps cold rendering for anchor-frame images because pixels are not serialized. Each boundary still hashes and reads its own artifacts, validates the native container and state, checks behavior revision and origin, loads the state, advances as needed, and decodes its own image. The reason-specific regression requires the two anchor-image replays and rejects both the old redundant count and removal of required rendering. The driver, state loader, CTest registration, and public claims remain consistent with those behaviors. No actionable correctness, security, or quality finding remains in the reviewed scope.

## Verification evidence

- `build/phase16-native-timing/review-fix-ci-controls.log` — 10/10 tests passed in 245.25 s; SHA-256 `c90ab2254a5f1917dad3f1e877b52e503091c11c37174337f45dcc6564aedf9e`.
- `build/phase16-native-timing/review-fix-asan-sessions.log` — readiness and controls passed 2/2 in 304.88 s; the readiness test output confirms the timing-report selftest ran before the real prefixes; SHA-256 `40cf25ba4c377788b85e477ea7fd04d5ceba4ab27219398e50a27bf388be6dd6`.
- `build/phase16-native-timing/review-fix-ci-workflow.log` — 649/649 tests passed in 263.37 s and all three library, runner, and libretro ZIPs were generated; SHA-256 `a5709b1df1d551defc7884e9f7676e4cd13a650a8c38c21bbfd525dea78da35d`.
- The focused readiness check passed 1/1. Local passes establish neither Intel nor hosted ASan acceptance; those native failures remain a separate exact-head gate.

## Result

Clean independent review of the exact final source commit. No actionable findings remain open.
