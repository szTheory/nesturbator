---
quick_id: 261026-i8z
status: complete
date: 2026-10-26
implementation_commit: dbbf04f
---

# Reduce repeated CI work

Implemented the CI efficiency findings on the existing Phase 20 branch. The
owner's pre-existing config edits remain unstaged. No Phase 20 feature work or
testing-proposal capture was performed.

## Changes

- Local `ci` retains all POSIX session controls. Hosted POSIX build lanes use
  `ci-build`, excluding only the three `retroarch-session` tests. Required
  Linux x64 and macOS arm64 session jobs run that matrix once per invocation;
  nightly retains Intel coverage and excludes the matrix from suite-flake.
  Short-run ASan omits the shell matrix but retains native host coverage.
- The hosted RetroArch job builds its tools and generated cartridge fixtures
  without rerunning the local suite. Its real-host progression, screenshot,
  battery-save and deterministic-state checks remain intact.
- The session test uses the driver's `STATE_SESSION_TEST_QUIT_GRACE_MS`
  override. Production deadlines and assertions are unchanged.
- A required job generates the actual full-vector CTest inventory once per
  hosted invocation. All six build lanes read that artifact while checking
  their own ordinary inventory. Local runs retain the configure-only fallback;
  nightly still fetches the full vectors cold. Property queries now parse
  individual test objects instead of repeatedly parsing the entire inventory.
- Pre-push checks outgoing committed C/C++ blobs with clang-format 18 after
  the privacy/history scan. Scratch controls cover dirty worktrees, outgoing
  intermediate commits, unusual filenames, vendored files, deletion, symlinks
  and the wrong formatter version. Real clang-format red/green was also checked.
- README, public-header comments and the engineering reference were updated
  together. The measured AccuracyCoin report is unchanged; its fingerprint
  provenance was re-recorded because build inputs and header comments changed.

## Verification

| Command or check | Result |
|---|---|
| `cmake --workflow --preset ci` | 675/675 passed; 356.11 s of tests; all three packages generated |
| `cmake --workflow --preset ci-build` | 672/672 passed; 70.69 s of tests; all three packages generated |
| `cmake --workflow --preset hygiene` | 13/13 passed |
| Focused session matrix | 3/3 passed; 284.61 s; also passed inside full ci (275.22 s) |
| Actual preset inventories | Exactly the three session tests excluded from ci-build; all other registrations retained |
| `ctest --preset asan -R '^ci.session_inventory$' --output-on-failure` | Passed; short-run inventory has no session matrix |
| Vector policy before/after | 44.29 s before; 8.07 s after in the focused local run |
| Shared-inventory path | Passed with an unused fallback build directory; missing artifact and disabled-opcode controls rejected |
| Full 22-page AccuracyCoin campaign | 108 pass / 38 fail / 0 timeout; report SHA-256 unchanged |
| `python3 scripts/check_push_format.py` after implementation commit | 26 committed source blobs formatted |
| Actionlint workflow syntax/expressions, ShellCheck disabled | Passed; default ShellCheck reports four advisories in unchanged nightly scripts |
| CI rollup controls | Every required job's failure, cancellation or skip rejects a pull request |
| `git diff --check` | Passed |

AccuracyCoin report SHA-256:
`b73c7b255ef4c0fe091c2d50bf94685e5b666760aaead6e28985bc3931ceeb7c`.
Its build-input fingerprint is
`3211e94fcf6be05fce0f93ae898c71221dccf73b7fead878e80ffc0547d5a4ae`.
No emulation behavior, hash pin, ABI, state format, ROM pin or guest frame budget changed.

## Limits and deferred work

These are local macOS measurements. No new hosted run or overall PR wall-time
improvement is claimed. New session jobs retain the previous measured build
timeout envelope pending isolated hosted measurements. The skill's CI monitor
script was absent; local CMake and Actionlint checks supplied verification.

The title workflow split remains deferred because it requires a coordinated
required-check ruleset migration; the existing title job and edited trigger
remain in place. The external nightly vector fetch issue is preserved. Details
are in `261026-i8z-deferred-items.md`.

An initial session baseline run was invalidated by editing the shell script
while it ran; that result was discarded. The focused and full successful runs
above used unchanged scripts. The first new inventory-test run exposed an
empty-list comparison bug in the test; quoting both sides fixed it, and normal
and short-run checks then passed. No failing or interrupted run is used as
completion evidence.

## Next step

Run `$gsd-capture` to record the saved property-helper, deterministic signal
and later mutation-testing ideas as proposed scope. Then discuss Phase 20.
