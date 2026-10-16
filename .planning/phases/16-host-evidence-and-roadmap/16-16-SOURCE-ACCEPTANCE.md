# Phase 16 Plan 16 Source Acceptance

plan: 16
status: in_progress

## Pre-edit source and retained integration failure

The starting source candidate is `2a2f709f6099d59e78cd4f23df7ba1632a6f6407` on `gsd/phase-16-host-evidence-and-roadmap`. Before editing, the five owned source/doc files had these identities:

| File | Git blob | SHA-256 |
|---|---|---|
| `tests/retroarch/interruption_trace.sh` | `c4c91cfc88bd0917faf2d1d8ad592a463c12213b` | `43821bd32c822983d1229bef812b4d5d12e7dece9024b6aae0620265df40eac2` |
| `tests/retroarch/state_session.sh` | `d04fef7bb29ea60bcd63f3552240fae973ba87d3` | `7f72aac3b6ce6c06acc84252264ebb9e132543074223d24049a446afba0872db` |
| `tests/retroarch/test_state_session.sh` | `2e30c11ebeee3db010ea78b36038abecc6c8ddfe` | `1e4a7cc36df75b6bca31c7e2baf5383d252e8409703d1715cf0098dbd62dc1d1` |
| `README.md` | `befe39a548d6f4d669d5cfff217ed997734319ae` | `0fb15c041f94ee9a83832dcb1001071882ca36b1cabb0d5cbaab687abdf8e3e3` |
| `include/nesturbator.h` | `bc084c20239931e62ff62f1df9b07bc61622cc6f` | `ab2a2c67ce76f1ab60d8678cd190e8b5b4e4c90085d0fdff818f7cf01f70ccea` |

The immutable Plan 14 stop record documents the corrected-candidate CI invocation as exit 8, 9/10 tests passed, total 218.84 seconds, with `retroarch.session.controls` failing after 218.83 seconds on `FAIL interruption recorder unusable: driver-trace-TRACE_VALUE`. The unchanged source candidate was `0e20688badb715ef6d31a10148c7fa4e1431d4da`. Its retained CTest log is `build/phase16-acceptance-execute/task1-ci-controls-corrected.log` (SHA-256 `4654c240cc85a455ace3df55354559cb90ba65063603e81782f8c966d6d6199d`); the retained ten-test JUnit report is `build/phase16-acceptance-execute/scope-stop/corrected-ci.junit.xml` (SHA-256 `5f12726e21f05244fc634f606e48c8066305643e49da7c87f743231b9bfc3761`). The retained failure summary is `build/phase16-acceptance-execute/scope-stop/corrected-ci.LastTestsFailed.log` (SHA-256 `0197cde73d65a38f97f05bb38d6ef0d1cf8016755c9ca676472264acc2788ced`). Those historical artifacts are read-only evidence, not acceptance of this source.

The retained actual driver trace `build/phase16-acceptance-execute/scope-stop/session-87792-interrupt/ra/interruption-events.tsv` has SHA-256 `6c33e0b0de710b1b6edb0c5ac0d63c7a0aa9f6d05f1d11ca77ec4c3f6cee98b7`. Its three 16-field rows record field 14 as `absent`, `absent`, `absent`; event order and statuses are `handler-entry/0`, `journal-close-success/0`, and `handler-exit/130`. In the retained failure, the actual assertion returned status 0, the owned token-bearing hard link existed during the first two driver events, and cleanup removed it before the last event. The strict unchanged validator therefore rejected field 14 with `TRACE_VALUE`. The integration mismatch is concrete: the driver creates the lock with `ln` as a regular file, while the restored writer observes only `[ -d "$LOCK_DIR" ]`. This does not explain the separate historical assertion failure; its cause remains unknown.

The supplied unstaged `.planning/config.json` remains byte-identical to its baseline (SHA-256 `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e`). No current Plan 16 runtime test has been used as RED evidence yet.

## Deterministic writer-to-validator RED/GREEN

RED was produced before changing `tests/retroarch/interruption_trace.sh`. The added target, `regular hard-link lifecycle records held occupancy until release`, independently created the driver's token-bearing regular hard link, invoked the actual `trace_interruption_event` for all three lifecycle events, removed the lock before the final event, checked the 16-field shape and every non-lock field, then called the unchanged `verify_interruption_driver_trace`. It exited 1 specifically because field 14 was `absent,absent,absent` while the lock was held for the first two events; the validator returned `TRACE_VALUE`. The hard-link inode and token, event/session/driver identity, timestamps, statuses, journal flags, QUIT flag, send count, first failure and final release were checked; no fixture, frame-index program, transport or driver was launched.

The unmodified TAP report and record are retained under ignored `build/phase16-lock-execute/`: `regular-lock-red.tap` (SHA-256 `8230346a741446bace0f6d096c867247e2e8c815fec18547acfa2ce143165609`), empty stderr (SHA-256 `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855`), exit-code file `regular-lock-red.exit` (1), and `regular-lock-red.json` (SHA-256 `5038dbf981963659319d3a53bc6ac4b0a9b2e47a44f1cf4e279a846223d7ba9c`). `gsd_run check tdd-red-evidence build/phase16-lock-execute/regular-lock-red.json --raw` returned `RED_EVIDENCE_OK`, `target_test_failed`, with exactly one TAP failure matching the named target and no parser errors. Semantic inspection confirmed the planned field-14 mismatch; this evidence authorizes the minimal GREEN correction.

GREEN pending. The full offline selector must retain the original directory lifecycle and all 36 prior trace cases, accept the actual regular hard-link lifecycle, reject absent-held and present-released field-14-only mutants, and establish dangling-symlink occupancy. Record the exact full selector result here after correction.

## Current-source acceptance and review

Pending. Bind the exact corrected source, focused CI/ASan selections, normal CI workflow and package bytes/digests, then the independent six-path review and matching disposition. A failure is terminal for this plan attempt; do not retry an unchanged candidate.
