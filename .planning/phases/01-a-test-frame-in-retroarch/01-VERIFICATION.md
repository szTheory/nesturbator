---
phase: 01-a-test-frame-in-retroarch
verified: 2026-10-03T13:19:04Z
status: passed
score: 5/5 must-haves verified
covered_files: [".clang-format",".gitattributes",".githooks/pre-commit",".githooks/pre-push",".github/dependabot.yml",".github/rulesets/main.json",".github/workflows/ci.yml",".github/workflows/release.yml",".planning/phases/01-a-test-frame-in-retroarch/01-01-PLAN.md",".planning/phases/01-a-test-frame-in-retroarch/01-01-SUMMARY.md",".planning/phases/01-a-test-frame-in-retroarch/01-02-PLAN.md",".planning/phases/01-a-test-frame-in-retroarch/01-02-SUMMARY.md",".planning/phases/01-a-test-frame-in-retroarch/01-03-PLAN.md",".planning/phases/01-a-test-frame-in-retroarch/01-03-SUMMARY.md",".planning/phases/01-a-test-frame-in-retroarch/01-04-PLAN.md",".planning/phases/01-a-test-frame-in-retroarch/01-04-SUMMARY.md",".planning/phases/01-a-test-frame-in-retroarch/01-05-PLAN.md",".planning/phases/01-a-test-frame-in-retroarch/01-05-SUMMARY.md",".planning/phases/01-a-test-frame-in-retroarch/01-06-PLAN.md",".planning/phases/01-a-test-frame-in-retroarch/01-06-SUMMARY.md",".planning/phases/01-a-test-frame-in-retroarch/01-07-PLAN.md",".planning/phases/01-a-test-frame-in-retroarch/01-07-SUMMARY.md",".planning/phases/01-a-test-frame-in-retroarch/01-08-PLAN.md",".planning/phases/01-a-test-frame-in-retroarch/01-08-SUMMARY.md",".planning/phases/01-a-test-frame-in-retroarch/01-09-PLAN.md",".planning/phases/01-a-test-frame-in-retroarch/01-09-SUMMARY.md",".planning/phases/01-a-test-frame-in-retroarch/01-10-PLAN.md",".planning/phases/01-a-test-frame-in-retroarch/01-10-SUMMARY.md",".planning/phases/01-a-test-frame-in-retroarch/01-11-PLAN.md",".planning/phases/01-a-test-frame-in-retroarch/01-11-SUMMARY.md",".planning/phases/01-a-test-frame-in-retroarch/01-12-PLAN.md",".planning/phases/01-a-test-frame-in-retroarch/01-12-SUMMARY.md",".release-please-manifest.json","ASSET_POLICY.md","CHANGELOG.md","CMakeLists.txt","CMakePresets.json","CONTRIBUTING.md","PROVENANCE.md","README.md","SECURITY.md","THIRD-PARTY-NOTICES.md","cmake/nesturbator.pc.in","cmake/nesturbatorConfig.cmake.in","cmake/packaging.cmake","host/convert.c","host/convert.h","include/nesturbator.h","libretro/CMakeLists.txt","libretro/libretro.c","libretro/libretro.h","libretro/nesturbator_libretro.info","release-please-config.json","runner/CMakeLists.txt","runner/main.c","runner/ppm.c","runner/ppm.h","runner/sha256.c","runner/sha256.h","scripts/hygiene.sh","src/frame.c","src/instance.c","src/internal.h","src/palette.c","src/palette_ntsc.c","src/testcard.c","tests/CMakeLists.txt","tests/abi/CMakeLists.txt","tests/abi/float_fixture.c","tests/check.h","tests/cmake/action_pins.cmake","tests/cmake/check_archives.cmake","tests/cmake/check_install_line.cmake","tests/cmake/check_ppm.cmake","tests/cmake/expect_output.cmake","tests/cmake/float_fixture.cmake","tests/cmake/float_scan.cmake","tests/cmake/format_check.cmake","tests/cmake/global_symbols.cmake","tests/cmake/palette_regen.cmake","tests/cmake/release_markers.cmake","tests/cmake/undefined_symbols.cmake","tests/cmake/vendored_sha256.cmake","tests/cmake/version_consistency.cmake","tests/cmake/write_hashes.cmake","tests/consumer/CMakeLists.txt","tests/consumer/main.c","tests/core/test_api.c","tests/core/test_frame.c","tests/core/test_palette.c","tests/embed/CMakeLists.txt","tests/header/header_c.c","tests/header/header_cxx.cpp","tests/host/test_convert.c","tests/hygiene/CMakeLists.txt","tests/hygiene/fixtures/pins_bad.yml","tests/hygiene/fixtures/pins_good.yml","tests/hygiene/scan_selftest.sh","tests/libretro/libretro_host.c","tests/release/two_versions.md","tests/retroarch/CMakeLists.txt","tests/retroarch/bmp_ppm.c","tests/retroarch/bmp_ppm.h","tests/retroarch/compare_frame.c","tests/retroarch/run_retroarch.cmake","tests/retroarch/test.cfg.in","tests/retroarch/test_compare_frame.c","tests/roms/manifest.txt","tests/runner/hashes.txt","tests/runner/test_sha256.c","tools/palgen/CMakeLists.txt","tools/palgen/palgen.c","version.txt"]
covered_digest: "v2:sha256:374e6ae4042dd378745ea4ab144c4567afed0ac612145e9dbcd5709011e6bb90"
behavior_unverified: 0
overrides_applied: 0
re_verification:
  previous_status: human_needed
  previous_score: 3/5
  gaps_closed:
    - "SC5 (release half): a behaviour-changing merge to main publishes the 18 archives with SHA256SUMS as a GitHub release with no manual step"
    - "SC3 (install half): the README's one-line install on an Apple Silicon Mac puts the released core where RetroArch loads it"
    - "CR-01/CR-02 hygiene fail-open warnings (scripts/hygiene.sh)"
  gaps_remaining: []
  regressions: []
advisory:
  - finding: "release-please-config.json still carries the one-time release-as: \"0.1.0\" pin (WR-08). The next releasable merge would propose 0.1.0 again, so SC5 would not hold for the second release unless the pin is removed."
    category: other
    reason: "Tracked as the first Phase 2 task in STATE.md (jq removal plus a jq -e check). Nothing automated enforces it yet. Resolve by removing the pin before the next feat/fix merge."
    evidence_status: "none provided (no failing run; the risk applies to the next release only)"
  - finding: "CI job timeouts are still 1-2 minutes on the only required check, which the release run reuses (WR-09, timeouts part left to the owner)"
    category: other
    reason: "The v0.1.0 release run 37125205144 passed every leg, so there is no failure evidence. It is a margin risk only."
    evidence_status: "none provided"
---

# Phase 1: A test frame in RetroArch Verification Report

**Phase Goal:** Anyone can build, test, download and install the library, the runner and the libretro core, and RetroArch shows the core's built-in test frame.
**Verified:** 2026-10-03T13:19:04Z
**Status:** passed
**Re-verification:** Yes. The previous report (human_needed, 3/5) went stale when PR #3 and release PR #2 changed covered files.

Every success criterion now holds, and I proved each one with a command I ran myself. v0.1.0 is published, not a draft, with 18 archives and a SHA256SUMS file that verifies. The release PR was opened and merged by the release App, with no person acting. The README block, run with HOME set to a scratch directory, downloads the release and installs the core and its .info file. RetroArch running that **installed, released** core produces a screenshot equal to the runner's frame. That closes the previous report's caveat that the RetroArch test only loaded the build-tree core. CR-01 and CR-02 are fixed, and I reproduced the fix myself.

## Goal Achievement

### Observable Truths (ROADMAP success criteria)

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | `cmake --workflow --preset ci` (`ci-msvc` on Windows) builds library, runner and libretro core and passes every test on six platforms; CI runs the same commands; an installed-package consumer creates an instance, runs one frame and destroys it | ✓ VERIFIED | Local run on dd16c53 + UAT edit: `cmake --workflow --preset ci` exit 0, "100% tests passed out of 34", including `install.consumer`, `install.stage` and `embed.subdirectory`; it produced 3 zips. On GitHub, the Release run 37125205144 (push, dd16c53, attempt 1) called ci.yml. All six `ci / build (...)` legs succeeded, as did `CI required`. |
| 2 | With no cartridge, `nesturbator-run --frames 1 --dump-frame 1:FILE --hash-frame 1` writes the frame as an image and prints a SHA-256 identical on all six platforms; the libretro test host receives an equal frame | ✓ VERIFIED | I ran the **released** `nesturbator-0.1.0-runner-macos-arm64.zip` binary. It prints `frame 1 ticks 714732 sha256 b49e9be4…0453` and writes P6 `256 240`. The local build prints the same digest, which also matches the previous report. The `ci / hash-equality` job passed in the release run. `libretro.host` passed locally. |
| 3 | After the README's one-line install on Apple Silicon, `ctest -L retroarch` launches RetroArch unattended and its screenshot matches the runner's frame; skipped where RetroArch is absent | ✓ VERIFIED | The README block (lines 327-328, `NESTURBATOR_VERSION=0.1.0` then curl and tar) ran with `HOME=<scratch>/home`: exit 0. It extracted exactly `cores/nesturbator_libretro.dylib` and `info/nesturbator_libretro.info`, both byte-identical (`cmp`) to the release zip. The URL returns HTTP 200. `ctest --preset ci -L retroarch`: retroarch.testframe passed. **In addition**, I ran `run_retroarch.cmake` with `-DCORE=` and `-DINFO=` pointing at the scratch-installed released files and `RA_DIR` in scratch: "RetroArch's frame equals the runner's frame", exit 0. The skip path was proven earlier on macos-15 CI and that code is unchanged. |
| 4 | `asan`, `nofp` and `hygiene` presets pass: sanitizer-clean, no floating point and no undefined symbols beyond the C memory functions in the core, no personal data and no unlisted ROM in the tree | ✓ VERIFIED | Local: `asan` exit 0 (33/33), `nofp` exit 0 (5/5), `hygiene` exit 0 (6/6, including `hygiene.scan_selftest`). `scripts/hygiene.sh --tree` exit 0 and `--history` exit 0. The release run's `asan`, `nofp` ×2 and `hygiene` jobs all succeeded. **CR-01/CR-02 fixed:** the script exports `LC_ALL=C` and wraps git with `core.quotePath=false`. Lists are read with `-z`, and `check_file` fails closed on unreadable content. I repeated my own repro in a scratch repo under `LC_ALL=en_US.UTF-8`: `café.txt` and a Latin-1 file, each holding a home path. Both were now reported, with exit 1; before the fix neither was. |
| 5 | Every PR builds library, runner and libretro archives for each platform; a behaviour-changing merge to main publishes them with SHA256SUMS as a GitHub release with no manual step | ✓ VERIFIED | **PR half:** the PR #3 CI run 37118749962 uploaded `archives-{linux,macos,windows}-{x64,arm64}` and six `hashes-*` artifacts. **Release half:** the owner merged `fix:` PR #3 at 13:07:24Z. The Release run on that push updated release PR #2. The App (`app/nesturbator-release`, bot) auto-merged PR #2 at 13:08:58Z. Release run 37125205144 (push, attempt 1) passed release-please, the full CI and `publish`. `gh release view v0.1.0`: isDraft false, isPrerelease false, target dd16c53. It has 19 assets: the 18 expected `nesturbator-0.1.0-{library,libretro,runner}-{linux,macos,windows}-{x64,arm64}.zip` files plus SHA256SUMS. I downloaded it to scratch: `shasum -a 256 -c SHA256SUMS` exit 0, 18 OK. No person acted after the merge. See Info for how PR #2 stalled before PR #3. |

**Score:** 5/5 truths verified (0 present, behavior-unverified)

### Advisory (New Scope, Unevidenced)

| # | Finding | Category | Why Advisory |
|---|---------|----------|--------------|
| 1 | `release-as: "0.1.0"` pin still in `release-please-config.json` (WR-08). The next releasable merge would propose 0.1.0 again. | other | Tracked as the first Phase 2 task in STATE.md. It affects the next release, not v0.1.0. No failing run. |
| 2 | 1-2 min CI timeouts on the required check that the release run reuses (WR-09 remainder) | other | The release run passed on every leg. Margin risk only. |

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `include/nesturbator.h` | Public API, version macros | ✓ VERIFIED | Release bumped to 0.1.0 via `x-release-please-*` markers; `version.consistency` passes |
| `src/*.c` | Core, test card, palette | ✓ VERIFIED | core.* and palette.regen pass |
| `runner/*` | Runner, P6 dump, hash | ✓ VERIFIED | Released binary run directly; no-args exits 2 (WR-02 fixed) |
| `libretro/libretro.c`, `.info` | Adapter | ✓ VERIFIED | Released dylib loaded by RetroArch, frame equal |
| `cmake/packaging.cmake` etc. | Install + CPack | ✓ VERIFIED | 3 zips locally, 18 in the release |
| `scripts/hygiene.sh` | Privacy/ROM gate, fail-closed | ✓ VERIFIED | CR-01/CR-02 fix reproduced; `hygiene.scan_selftest` in the hygiene lane |
| `.github/workflows/release.yml`, `release-please-config.json` | Release on merge | ✓ VERIFIED | Run 37125205144 published v0.1.0; actionlint exit 0 |
| `tests/cmake/check_install_line.cmake` | README line check | ✓ VERIFIED | Wired in ci.yml:105 (macOS leg) |
| `tests/cmake/release_markers.cmake`, `tests/release/two_versions.md` | One version per marked line (regression for the PR #2 stall) | ✓ VERIFIED | `release.one_version_per_line` and its self-test are in tests/CMakeLists.txt:158-165 and pass |

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| release.yml | ci.yml | `workflow_call` | WIRED (exercised) | `ci / *` jobs ran inside run 37125205144 |
| release.yml publish | `archives-*` artifacts | download + SHA256SUMS | WIRED (exercised) | 18 zips + sums published; sums verify |
| README install block | release tag | release-please generic `x-release-please-start-version` block | WIRED (exercised) | Line reads 0.1.0; URL 200 |
| Release App | release PR auto-merge | `auto_squash_enabled` by bot | WIRED (exercised) | PR #2 merged by `nesturbator-release[bot]` |
| retroarch.testframe | runner + compare_frame | run_retroarch.cmake | WIRED | Passes with build core and with released installed core |
| pre-commit / CI hygiene | scripts/hygiene.sh | `.githooks`, ci.yml hygiene job | WIRED | hygiene job green in release run |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|----------|---------|--------|--------|
| ci lane | `cmake --workflow --preset ci` | exit 0, 34/34 | ✓ PASS |
| asan lane | `cmake --workflow --preset asan` | exit 0, 33/33 | ✓ PASS |
| nofp lane | `cmake --workflow --preset nofp` | exit 0, 5/5 | ✓ PASS |
| hygiene lane | `cmake --workflow --preset hygiene` | exit 0, 6/6 | ✓ PASS |
| RetroArch, build core | `ctest --preset ci -L retroarch` | 1/1 passed | ✓ PASS |
| RetroArch, released installed core | `cmake -DCORE=<scratch HOME>/…/cores/nesturbator_libretro.dylib -DINFO=… -P tests/retroarch/run_retroarch.cmake` | "RetroArch's frame equals the runner's frame", exit 0 | ✓ PASS |
| Release published | `gh release view v0.1.0 --json isDraft,assets` | false; 18 zips + SHA256SUMS | ✓ PASS |
| Release integrity | `gh release download v0.1.0` + `shasum -a 256 -c SHA256SUMS` | exit 0, 18 OK | ✓ PASS |
| Install URL | `curl -sIL …/v0.1.0/nesturbator-0.1.0-libretro-macos-arm64.zip` | 200 | ✓ PASS |
| README install | README lines 327-328 under `HOME=<scratch>` | exit 0; both files extracted, byte-identical to zip | ✓ PASS |
| Released runner hash | released `nesturbator-run --frames 1 --hash-frame 1` | b49e9be4…0453, equals local build | ✓ PASS |
| No person after merge | `gh pr view 2`, issue timeline | author, auto-merge and merge all by the release App bot | ✓ PASS |
| Hygiene fail-closed | scratch repo with `café.txt` + Latin-1 file under a UTF-8 locale | both flagged, exit 1 | ✓ PASS |
| History hygiene | `scripts/hygiene.sh --history` | exit 0 | ✓ PASS |
| Workflow lint | `actionlint ci.yml release.yml` | exit 0 | ✓ PASS |

### Probe Execution

No `scripts/*/tests/probe-*.sh` exist, and no PLAN declares a probe. Step 7c: SKIPPED.

### Requirements Coverage

| Requirement | Status | Evidence |
|-------------|--------|----------|
| FRAME-01 | ✓ SATISFIED | SC1 |
| FRAME-02 | ✓ SATISFIED | `install.consumer` |
| FRAME-03 | ✓ SATISFIED | SC2 |
| FRAME-04 | ✓ SATISFIED | `libretro.host` |
| FRAME-05 | ✓ SATISFIED | SC3: download, install and RetroArch compare of the released core |
| FRAME-06 | ✓ SATISFIED | SC4; hygiene gate now fails closed |
| FRAME-07 | ✓ SATISFIED | SC5: v0.1.0 published by the pipeline |

All seven IDs are claimed by plans, and none is orphaned. REQUIREMENTS.md marks all seven Complete, which is now accurate.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| release-please-config.json | 10 | `release-as: "0.1.0"` pin still present (WR-08) | ⚠️ Warning | It must be removed before the next releasable merge; tracked as the first Phase 2 task |
| .github/workflows/ci.yml | jobs | 1-2 min timeouts (WR-09 remainder, owner's call) | ℹ️ Info | The release run passed |
| 01-REVIEW-DISPOSITION.md | — | IN-01 to IN-07 still `open` | ℹ️ Info | Info-level only |
| — | — | TBD/FIXME/XXX/TODO in files changed since the previous report | none found | — |

**Info: the first release PR stalled.** Release PR #2 was opened and set to auto-squash at 02:28Z, after PR #1 merged. It did not merge until 13:08Z. It needed a code fix (PR #3) because release-please changes only one version per line, and the old README install line held two versions. The fix put the version on its own line and added the `release.one_version_per_line` test, which fails on any marked line holding two versions. After PR #3, the pipeline ran without anyone acting. SC5 is about the pipeline as shipped, so the stall does not make it false, and a regression test now guards it.

**Info:** in the uncommitted 01-UAT.md, `updated: 2026-10-03T13:30:00Z` is later than the system clock at verification time (13:19Z). This is cosmetic.

### Human Verification Required

None. Every check above ran as a command.

### Gaps Summary

There are no gaps. Both open truths from the previous report (SC3 install half, SC5 release half) are closed by commands run in this verification, and the CR-01/CR-02 fail-open defects are fixed. One forward-looking warning: remove the `release-as` pin before the next `feat:`/`fix:` merge, or the next release will not publish as a new version. STATE.md already schedules this as the first Phase 2 task.

---

_Verified: 2026-10-03T13:19:04Z_
_Verifier: Claude (gsd-verifier)_
