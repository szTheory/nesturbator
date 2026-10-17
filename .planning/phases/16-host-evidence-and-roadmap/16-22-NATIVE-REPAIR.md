# Phase 16 Plan 22: Native Repair Evidence

This record keeps the failed native run as immutable failure evidence and separates its observed symptoms from local diagnoses. The plan has not established cross-platform native acceptance; that remains Plan 07's exact-final-head gate.

## Baseline native failures

Run `38976400182` was a required pull-request run for head `65c3a7fb0b320bfe7a51ac486bf922c5ad5872d9`, base `9f9386c661f6fb8f6063bf756655713aedadae77`, preview merge `aabd1d940d8deb2dd3c5369eddf074158517b92e`. The retained required-run summary and six raw job logs were copied without modification under `build/phase16-native22/round-1/native-baseline/`; `SHA256SUMS` records their copied digests. The copied hashes match the source logs:

| Job | Observed result | Raw log SHA256 |
| --- | --- | --- |
| Hygiene `117227153307` | `frame_index.c` formatting failure at lines 1276–1283 | `75cba0e1cc870298cf7283369a84bb613fa9f23ef1dbb4ce7794f983a3e84bb4` |
| Linux x64 `117227153269` | `actual-driver-noclobber-refusal-not-proven` | `f203ed23a3eda55981beb6160b3c5a64fb46525bd77c39e6974faa1a3ba3c1f9` |
| Linux ARM `117227153527` | `actual-driver-noclobber-refusal-not-proven` | `37761b41bd228ce34ccf99321105547de3034ec46be1495cf853ad41ecda81ba` |
| ASan `117227153348` | Same refusal assertion and the registered readiness process timed out at 60.03s after the delayed case recorded a 20,054ms operation; the second case did not finish | `072d68d6044a237e39b4d13f175823329c9063f29325b2958a570172cbe0113a` |
| macOS Intel `117227153526` | Cleanup matrix exceeded its recorded wall budget and readiness CTest timed out at 60.26s; this log contains no sidecar assertion | `c0947352f3fa3291e74f8b613e198d3321a9112400c7aaff0fdfbdc93e7dd552` |
| Required CI `117232139742` | Required CI failed because a hash-equality test was skipped | `d14be368d06847ee7ababf101f2c197ab27263df332424cc2d57b97c0b18ca04` |

The failure-summary SHA256 is `15aceccbd3db78ee4fe9bae51b0a9342b883e2ca6ce7b8a0d18c616d26ca64e8`. The copied run, jobs, poll, context, and PR payload hashes are retained in the same checksum file. These records establish the failures and their measured categories, not their original platform causes.

Nightly run `38976400185` is separate ancillary evidence, not a required-CI failure. Its `suite-flake` job used `ctest --preset ci --repeat until-fail:3 --schedule-random` and failed `retroarch.session.controls` with the same old owner-sidecar assertion; the other nightly jobs passed. Its raw log remains at `build/phase16-release/suite-flake-job-117226779425.log` with SHA256 `b8d962b3f3eb12678a2117fc15727331f98d45824bcbf408cd2660761d651108`. No historical run was rerun.

## Task 1: actual owner-sidecar refusal

The old harness required status 1 plus `state_session: LOCK_OWNER_UNAVAILABLE`. On the host, `/bin/sh` is GNU bash 3.2.57. Its pre-repair actual-driver case returned status 1, retained the explicit message, preserved the injected sidecar, and passed the original checker. An installed `/bin/dash` was also available locally; this is a local shell observation, not native-platform acceptance.

With the original checker and `/bin/dash`, the test drove the real eight-argument `state_session.sh` entry against a foreign file created at the exact clock/PID-bound owner path. The outer selftest returned 1 in 85ms with `actual-driver-noclobber-refusal-not-proven`. The actual driver returned 2 and emitted `tests/retroarch/state_session.sh: 90: cannot create <exact injected path>: File exists`; it did not reach the driver's explicit message. No host or transport launch marker existed. The foreign regular file remained byte-identical, with lstat identity unchanged at device `16777230`, inode `636514809`, mode `33188`, link count `1`, and size `37`. The exact relative fixture path and raw driver output are retained in `build/phase16-native22/round-1/sidecar-dash-old-checker/`.

This reproduces the old assertion failure locally through the actual helper and shows safe refusal on this shell. It does not prove that the historical Linux/ARM jobs had the same cause. The repair changes only the test proof: when the shell returns control, the harness requires one exact `LOCK_OWNER_UNAVAILABLE` line and permits only an exact-path failed-redirection diagnostic; when the shell terminates on the failed `exec`, it accepts status 2 only with the exact occupied-path diagnostic. Both paths independently require the exact injected path, no host/transport launch, and unchanged sentinel bytes and lstat identity. No production lock, trap, cleanup, QUIT, or timing code changed.

The corrected actual-driver selector passed under installed `/bin/dash` in 4,396ms and under host `/bin/sh` in 4,384ms. The dash run classified `PORTABLE_SHELL_OCCUPIED_PATH_REFUSAL`; the host shell classified `LOCK_OWNER_UNAVAILABLE`. Both retained the existing capture and 51 trace cases. Seven proof mutants were rejected: success on an occupied path, changed bytes, removed file, wrong path, host launch, transport launch, and an unrelated `INPUT_MISSING` diagnostic. Launch records, stdout/stderr, exact path, identity, and timing are retained under `build/phase16-native22/round-1/`.

The protected `.planning/config.json` remained unchanged at SHA256 `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e` throughout these checks.

## Task 2: readiness process scope and cleanup matrix

The retained ASan and Intel readiness logs report successful delayed-read operations of 1,624ms and 1,983ms, with later prefix work of 12,887ms and 13,732ms. Their first readiness cases took 20,054ms and 19,004ms driver-plus-cleanup, while fixture generation took 8,549ms and 4,376ms. The registered process had consumed 49,448ms and 41,923ms respectively by the end of the first case, then reached the unchanged 60-second CTest timeout before reporting the second case. These measurements isolate the timeout from the readiness operation window; they do not establish how the unreported second case would have finished.

The readiness selector also ran the independent startup-lock TERM race. Its two actual prefix-driver invocations measured 5,592ms locally. The test now runs that race once in the default `all` controls path and reports its monotonic duration; it remains part of normal CI's registered `retroarch.session.controls` test. The timing-report selftest also stays in the default-all path. The focused readiness path still runs both real delayed and stalled readiness cases with the same assertions and registered 60-second CTest property.

Before moving the unrelated control, the direct readiness lane took 24,085ms. Afterward, it took 18,308ms; the registered readiness CTest passed in 18,447ms. Its fresh direct case measurements were:

| Case | Setup | Readiness operation | Later prefix | Attempt through cleanup | Driver plus cleanup |
| --- | ---: | ---: | ---: | ---: | ---: |
| `readiness-delayed` | 12ms | 1,564ms | 4,677ms | 6,846ms | 6,898ms |
| `readiness-response-stall` | 11ms | 5,053ms | 4,807ms | 10,466ms | 10,521ms |

Both replies remain exact one-send readiness operations; the original routing, journal, prefix, child/host reap and sanitizer checks passed. Fixture generation was 576ms and the complete direct lane was 18,308ms. The change recovers the locally measured 5.6-second unrelated control duration. Its effect on slower hosted ASan scheduling is still unverified until the single formal ASan pair.

The original Intel controls log ends after 261.56 seconds with `cleanup matrix exceeded measured wall budget`; it retained no per-case report or matrix evidence bundle, so the log cannot distinguish the 420-second total check from the 90-second non-`already-quit` check. No native per-case cause is inferred and neither bound was changed. A fresh local eight-case matrix after the Task 2 harness edit passed with invocation-bound evidence at `build/phase16-native22/round-1/cleanup-after-move/evidence.json`:

| Measurement | Result |
| --- | ---: |
| Matrix elapsed | 104,895ms of 420,000ms |
| `already-quit` primary | 58,291ms of 250,000ms |
| Other seven primary invocations combined | 5,355ms; each below 20,000ms |
| Seven reacquisitions combined | 38,795ms; each below 20,000ms |
| Matrix elapsed minus only `already-quit` primary | 46,604ms of 90,000ms |
| Remaining fixture/evidence/finalization time | 2,454ms |

All eight observations passed with no watchdog intervention. The original first reasons and statuses, per-invocation send counts, host/child reaping, foreign endpoint/owner identities and bytes, and cleanup journaling remained checked. Since this local sample is within every unchanged bound and the historical Intel log lacks the breakdown required to identify an optimization target, Task 2 makes no cleanup-driver or matrix-budget change. Native Intel cleanup cause remains unknown for Plan 07 to resolve with fresh exact-head evidence.

Both historical readiness and cleanup failures remain recorded as failures; the local results do not supersede those native results. The 60/900-second CTest timeouts, 120,000/30,000/5,000ms production defaults, 420,000/90,000ms matrix limits, one-QUIT rule, and all eight cleanup cases are unchanged.

## Task 3: formatting-only candidate change

The installed formatter is `/opt/homebrew/opt/llvm@18/bin/clang-format`, version 18.1.8. At candidate `a957f1cdb23f50a906d4b51f949b5e35562f603f`, `tests/retroarch/frame_index.c` had blob `204d3ff64cc288176b0e6f47a1788c122a36815c`; formatting only lines 1276–1283 took 50ms and produced blob `a014977266d91db6d0cc76895ff51d3c3243f8d8`. The diff changes indentation and wraps an adjacent format-string literal in the two existing diagnostic calls. It changes no expressions or verdicts. `clang-format --dry-run --Werror tests/retroarch/frame_index.c`, `cmake --build build/ci --target frame_index --parallel 4`, and `git diff --check` passed. Raw launch identity and timing are retained under `build/phase16-native22/round-2/`.

An exploratory dry-run across every tracked C/C++ file exited 1 only on the vendored `libretro/libretro.h`; that file is intentionally not reformatted under the project's vendoring rule. Its full diagnostics are retained in `build/phase16-native22/round-2/c-format-preflight.stderr`. This was not the formal hygiene workflow and does not replace it; the plan's changed-source hygiene slot remains unused.

## Task 3: formatting and current-candidate acceptance

The formatter correction was committed as `3643d6ac04c8c09391a6f2ea68c7217ba4c5ba25`. The candidate's ten-path source manifest is unchanged before and after each formal gate, and `.planning/config.json` retained SHA256 `0517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e`. Installed tools were clang-format 18.1.8, CMake/CTest 4.4.3, Apple clang 21.0.0 and Ninja. Full argv, actual wait status, monotonic elapsed, stdout/stderr digests, current CTest registration, raw reports and local package files are retained in `build/phase16-native22/formal/`.

The single hygiene workflow `cmake --workflow --preset hygiene` passed with status 0 in 17.949 seconds. The focused registered CI pair passed once with status 0 and no skips: controls 285.07 seconds, readiness 18.30 seconds. It retained the real driver sidecar refusal and all seven rejected proof mutants, exact foreign identity and replacement preservation, 51 trace cases, 44 progression cases, eight cleanup-write controls and the moved startup-lock race in default controls. The protected source/config identities matched before and after.

After `cmake --preset asan` and `cmake --build --preset asan --parallel 4` both passed, the single focused ASan/UBSan pair passed with status 0 and no skips or sanitizer diagnostics: controls 365.20 seconds and readiness 21.75 seconds. Both CTest registrations and source/config identities remained unchanged. The 900-second controls and 60-second readiness registration bounds were not altered.

The one `cmake --workflow --preset ci` completed with status 0 in 359.898 seconds under the 900-second process guard. Its configure, build, test and package steps all completed. CTest reported 649/649 successes with jobs 4 and no skips. The current registration and report each contain the same 649 unique test names; 611 outcomes use CTest's ordinary pass result and 38 use its registered `PASS_REGULAR_EXPRESSION` pass reason. Both registered `WILL_FAIL` tests were accepted by CTest as passes. The full report parser binds these result forms to current registration metadata instead of treating the ordinary `Test Passed.` marker as the only success form. Registration SHA256 before and after was `ed266d23a352baa9b93c9072ada64e0cfaab07c0a0ef2ab77d4429f905953e79`.

The previous report and ZIP directories were preserved in formal history before this run. Exactly three fresh version `0.1.10` macOS arm64 component ZIPs were generated and checked once with `cmake -DPACKAGES=build/ci/packages -DVERSION=0.1.10 -P tests/cmake/check_archives.cmake`; the checker returned status 0 and reported library, libretro and runner archives `ok`. Their SHA256 digests are:

| Archive | SHA256 |
| --- | --- |
| `nesturbator-0.1.10-library-macos-arm64.zip` | `7d066966df499eaa4332d52797a996207be87da324a7c4fcb5809f7e17ac0aad` |
| `nesturbator-0.1.10-libretro-macos-arm64.zip` | `7e7a84d273a38257d4f5c66758d8b690f2c32cd7e0a15212c58205009d4ca74a` |
| `nesturbator-0.1.10-runner-macos-arm64.zip` | `897ddc2a4c7816dc7d9d388d78b3e1f0bdae4cf7706f92989d01e869fb51f438` |

These are local package checks, not published release-byte evidence. The historical native run remains failed and immutable. Its owner-sidecar Linux/ARM/ASan assertions, ASan/Intel readiness timeouts and Intel cleanup-budget failure are not converted to passes; native causes remain unknown where the logs do not prove them.

## Remaining work

The changed candidate's local source, focused CI/ASan, full workflow and package acceptance passed. A fresh independent source review and exact source/report disposition are still required for this plan. The configured parent post-wave/execute:post/review/full-workflow repeat gates are separate parent obligations and remain pending until observed. Plan 07 retains fresh exact-final-head native CI and affirmative owner-consent gates; Plan 08 retains actual release-byte proof for EVID-04. No local result replaces those native, consent or publication checks.
