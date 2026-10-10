# Phase 5 CI record

Public run ids, numbers and SHAs only.

## Before

Run 38011892333 (CI on `main`, "chore: archive v1 milestone (#24)", 2026-10-10). Job wall time is completedAt minus startedAt. Re-read with:

`gh run view 38011892333 --json jobs -q '.jobs[] | "\(.name)\t\((.completedAt|fromdateiso8601)-(.startedAt|fromdateiso8601))"'`

| Leg | Job wall (s) | CTest total (s) | Longest test (s) |
|---|---|---|---|
| build (macos-15-intel, ci, macos, x64) | 181 | 152.21 | vectors.registration_policy 107.21; .self_test 21.07 |
| asan | 171 | 151.35 | runner.write_hashes 48.4; vectors.registration_policy 48.3 |
| build (windows-11-arm, ci-msvc, windows, arm64) | 144 | 93.12 | vectors.registration_policy 55.41 |
| build (macos-15, ci, macos, arm64) | 113 | 87.77 | n/a |
| retroarch-e2e | 111 | 78.65 | same suite |
| build (windows-2025, ci-msvc, windows, x64) | 92 | 58.63 | vectors.registration_policy 37.12 |
| build (ubuntu-24.04, ci, linux, x64) | 78 | 52.16 | n/a |
| build (ubuntu-24.04-arm, ci, linux, arm64) | 77 | 64.75 | n/a |

The job wall times were re-read on 2026-10-10 and match the research figures. CTest totals are from the research log reads.

Slowest leg before: `build (macos-15-intel, ci, macos, x64)`, 181 s job wall.

Local check of the change (macOS arm64, `ctest --preset ci`, 357 tests, jobs 4, with COST): Total Test time (real) 31.53 s. The research serial figure on this machine was 37.75 s for 355 tests.

## ccache decision

Compile share (configure, compile and package time as the workflow step minus CTest time) is 4 to 22 percent on every leg, under the 40 percent rule. No ccache and no `actions/cache`.

## After

Run 38061698752 (CI on PR #25, head e853abe, 2026-10-10). Every job passed. Job wall time read with the same command as Before.

| Leg | Job wall (s) | CTest total (s) |
|---|---|---|
| build (macos-15-intel, ci, macos, x64) | 191 | 140.37 (358 tests) |
| build (windows-2025, ci-msvc, windows, x64) | 128 | n/a |
| build (windows-11-arm, ci-msvc, windows, arm64) | 113 | n/a |
| retroarch-e2e | 88 | n/a |
| build (macos-15, ci, macos, arm64) | 78 | n/a |
| build (ubuntu-24.04-arm, ci, linux, arm64) | 64 | n/a |
| asan | 56 | n/a |
| build (ubuntu-24.04, ci, linux, x64) | 48 | n/a |

Slowest leg after: `build (macos-15-intel, ci, macos, x64)`, 191 s job wall against 181 s before. Its CTest total fell from 152.21 s (355 tests) to 140.37 s (358 tests). The extra wall time is in configure and compile, which this phase did not change. The same leg's job wall on the six latest green `main` runs was 181, 182, 257, 182, 314 and 201 s, so 191 s is within runner noise. Verdict: no regression. asan fell from 171 s to 56 s.

Nightly run 38061698773 (same head): suite-flake passed in 172 s job wall, inside its 30-minute timeout. vectors-full took 122 s and rom-loader-fuzz 74 s.

## Action pins

Checked 2026-10-10 with `gh api repos/<owner>/<repo>/releases/latest` and `gh api repos/<owner>/<repo>/git/ref/tags/<tag>` (object type `commit` in every case).

| Action | Pin now | Latest release | Disposition |
|---|---|---|---|
| actions/upload-artifact | cf430e030ddbb5b0abf93d22962f4752f3646cd9 | v7.0.2, same SHA | bumped from v7.0.1 (043fb46d1a93c77aae656e7c1c64a875d1fc6a0a) |
| actions/download-artifact | 9000827ccba6bdab643e8b6fd33ac0654aef8333 | v8.0.2, same SHA | bumped from v8.0.1 (3e5f45b2cfb9172054b4087a40e8e0b5a5461e7c) |
| actions/checkout | 3d3c42e5aac5ba805825da76410c181273ba90b1 | v7.0.1, same SHA | reviewed, unchanged-current |
| actions/attest | 1e69f48acb82d1966a394da916b4c1698aa569d6 | v4.2.2, same SHA | reviewed, unchanged-current |
| actions/create-github-app-token | bcd2ba49218906704ab6c1aa796996da409d3eb1 | v3.2.0, same SHA | reviewed, unchanged-current |
| googleapis/release-please-action | 45996ed1f6d02564a971a2fa1b5860e934307cf7 | v5.0.0, same SHA | reviewed, unchanged-current |

The upload-artifact SHA also lives in `tests/cmake/nightly_workflow_policy.cmake`, and the download-artifact SHA in the `release_policy.cmake` self-test fixtures; both were updated.

## Runner labels

The six labels in `ci.yml` (ubuntu-24.04, ubuntu-24.04-arm, macos-15, macos-15-intel, windows-2025, windows-11-arm) are listed without a deprecation badge in the actions/runner-images README as read during research on 2026-10-10. Unchanged.

## AccuracyCoin pin

- Pin in `tests/roms/manifest.txt`: `673ef550db296136d52229961e7d39366116882a`, commit dated 2026-09-23T17:43:46Z.
- Upstream HEAD on 2026-10-10: `74613de3a77f1abad072ba424a55dc8b94a03e46`, 7 commits ahead (`gh api repos/100thCoin/AccuracyCoin/compare/673ef550db296136d52229961e7d39366116882a...HEAD --jq '.ahead_by'`).
- Subjects: "Saved space and removed dead code"; "Fixed an issue with Misaligned OAM Behavior"; "Prevented potential issues with the stack."; "Various cleanup, Added 2 new tests"; "Fixed a minor issue with BG Serial IN"; "Saved 546 bytes."; "Saved another 186 bytes."
- Disposition: reviewed, not moved. A pin move changes the ROM, `tests/accuracy/scoreboard.txt` and the hash inventory, and STACK.md makes a pin move its own change.

## Labels

`mappers` and `games` labels are not added in Phase 5: no mapper or game test exists yet to carry them. Phases 7 to 10 add each label with the first test that uses it.
