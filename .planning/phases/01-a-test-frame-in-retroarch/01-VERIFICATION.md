---
phase: 01-a-test-frame-in-retroarch
verified: 2026-10-03T01:40:00Z
status: human_needed
score: 3/5 must-haves verified
covered_files: [".clang-format", ".gitattributes", ".github/dependabot.yml", ".github/rulesets/main.json", ".github/workflows/ci.yml", ".github/workflows/release.yml", ".planning/phases/01-a-test-frame-in-retroarch/01-01-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-01-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-02-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-02-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-03-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-03-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-04-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-04-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-05-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-05-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-06-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-06-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-07-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-07-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-08-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-08-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-09-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-09-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-10-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-10-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-11-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-11-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-12-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-12-SUMMARY.md", ".release-please-manifest.json", "ASSET_POLICY.md", "CMakeLists.txt", "CMakePresets.json", "CONTRIBUTING.md", "PROVENANCE.md", "README.md", "SECURITY.md", "THIRD-PARTY-NOTICES.md", "cmake/nesturbator.pc.in", "cmake/nesturbatorConfig.cmake.in", "cmake/packaging.cmake", "host/convert.c", "host/convert.h", "include/nesturbator.h", "libretro/CMakeLists.txt", "libretro/libretro.c", "libretro/libretro.h", "libretro/nesturbator_libretro.info", "release-please-config.json", "runner/CMakeLists.txt", "runner/main.c", "runner/ppm.c", "runner/ppm.h", "runner/sha256.c", "runner/sha256.h", "scripts/hygiene.sh", "src/frame.c", "src/instance.c", "src/internal.h", "src/palette.c", "src/palette_ntsc.c", "src/testcard.c", "tests/CMakeLists.txt", "tests/abi/CMakeLists.txt", "tests/abi/float_fixture.c", "tests/check.h", "tests/cmake/action_pins.cmake", "tests/cmake/check_archives.cmake", "tests/cmake/check_install_line.cmake", "tests/cmake/check_ppm.cmake", "tests/cmake/expect_output.cmake", "tests/cmake/float_fixture.cmake", "tests/cmake/float_scan.cmake", "tests/cmake/format_check.cmake", "tests/cmake/global_symbols.cmake", "tests/cmake/palette_regen.cmake", "tests/cmake/undefined_symbols.cmake", "tests/cmake/vendored_sha256.cmake", "tests/cmake/version_consistency.cmake", "tests/cmake/write_hashes.cmake", "tests/consumer/CMakeLists.txt", "tests/consumer/main.c", "tests/core/test_api.c", "tests/core/test_frame.c", "tests/core/test_palette.c", "tests/header/header_c.c", "tests/header/header_cxx.cpp", "tests/host/test_convert.c", "tests/hygiene/CMakeLists.txt", "tests/hygiene/fixtures/pins_bad.yml", "tests/hygiene/fixtures/pins_good.yml", "tests/libretro/libretro_host.c", "tests/retroarch/CMakeLists.txt", "tests/retroarch/bmp_ppm.c", "tests/retroarch/bmp_ppm.h", "tests/retroarch/compare_frame.c", "tests/retroarch/run_retroarch.cmake", "tests/retroarch/test.cfg.in", "tests/retroarch/test_compare_frame.c", "tests/roms/manifest.txt", "tests/runner/hashes.txt", "tests/runner/test_sha256.c", "tools/palgen/CMakeLists.txt", "tools/palgen/palgen.c", "version.txt"]
covered_digest: "v2:sha256:34c1febc283068974ecc7da71a3faadb752882d5bb49ee88edb8efd786f31fa8"
behavior_unverified: 2
overrides_applied: 0
behavior_unverified_items:
  - truth: "SC5 (release half): a behaviour-changing merge to main publishes the 18 archives with SHA256SUMS as a GitHub release with no manual step"
    test: "Owner marks PR #1 ready and squash-merges it. Then, by command: gh run list --workflow release.yml; gh pr list --label 'autorelease: pending' (release PR opened and auto-merged); gh release view v0.1.0 --json isDraft,assets (isDraft false, 18 zips + SHA256SUMS); gh release download v0.1.0 -D <scratch> && (cd <scratch> && shasum -a 256 -c SHA256SUMS)"
    expected: "Release v0.1.0 is published (not draft) with exactly 18 nesturbator-0.1.0-{library,runner,libretro}-{linux,macos,windows}-{x64,arm64}.zip files and a SHA256SUMS that verifies; no person acted after the merge of PR #1"
    why_human: "The pipeline has never run: no release exists (gh release list is empty) and it can only start from a merge to main, which needs the owner's consent. Every step after the merge is checkable by command."
  - truth: "SC3 (install half): the README's one-line install on an Apple Silicon Mac puts the released core where RetroArch loads it"
    test: "After v0.1.0 is published: run the README line with HOME pointed at a scratch directory (or extract into a scratch dir), confirm cores/nesturbator_libretro.dylib and info/nesturbator_libretro.info arrive; then ctest --preset ci -L retroarch"
    expected: "curl returns 200 for the v0.1.0 URL the release PR wrote into README.md; the two files extract; retroarch.testframe passes"
    why_human: "Today the line names v0.0.0, whose URL returns 404, so the download half cannot run until the first release. The tar half is already proven against the built archive, locally and on the macos-15 arm64 CI leg."
human_verification:
  - test: "Mark draft PR #1 ready for review and squash-merge it into main (owner consent; the only manual action)"
    expected: "The Release workflow starts on the push to main; from there the automated checks listed under behavior_unverified_items prove SC5's release half and SC3's install half without further owner action"
    why_human: "Merging to main and publishing a first public release need the owner's consent (PROJECT.md Hand-offs constraint). Everything after it is a command."
---

# Phase 1: A test frame in RetroArch Verification Report

**Phase Goal:** Anyone can build, test, download and install the library, the runner and the libretro core, and RetroArch shows the core's built-in test frame.
**Verified:** 2026-10-03T01:40:00Z
**Status:** human_needed
**Re-verification:** No (initial verification)

Build, test, the six-platform pull-request archives and RetroArch showing the test frame are all proven by commands I ran myself. "Download and install" is not proven yet. No release exists, so the README's install URL currently returns 404. The release pipeline has never run, and it can only start after the owner merges PR #1. Nothing in the code blocks the goal. The phase's own code review raised two Critical findings in the hygiene script (CR-01, CR-02). They do not make SC4 false today, but they should be fixed before merge (see Warnings).

## Goal Achievement

### Observable Truths (ROADMAP success criteria)

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | `cmake --workflow --preset ci` (`ci-msvc` on Windows) builds library, runner, libretro core and passes every test on six platforms; CI runs the same commands; an installed-package consumer creates an instance, runs one frame, destroys it | ✓ VERIFIED | Locally (macOS arm64) `cmake --workflow --preset ci` exit 0, 30/30 tests, 3 zips packaged. CI run 37084997715 (PR #1 head a9eb2cb, conclusion success): all six `build` legs ran `cmake --workflow --preset ${{ matrix.preset }}`, each "100% tests passed out of 30". `install.consumer` passed on windows-11-arm, windows-2025 and ubuntu-24.04-arm (logs checked). `tests/consumer/main.c` includes only `<nesturbator.h>`, uses `find_package(nesturbator CONFIG)` against `build/<preset>/stage`, checks create, run_frame and frame_number==1, then destroy. Local HEAD e02c158 differs from a9eb2cb only in 01-REVIEW*.md. |
| 2 | With no cartridge, `nesturbator-run --frames 1 --dump-frame 1:FILE --hash-frame 1` writes the frame as an image and prints a SHA-256 identical on all six platforms; the libretro test host receives an equal frame | ✓ VERIFIED | Ran it: exit 0, prints `frame 1 ticks 714732 sha256 b49e9be4…0453`, and writes a valid P6 `256 240 255`. Downloaded all six `hashes-*` artifacts from run 37084997715: byte-identical (same digest), and identical to the local macOS arm64 `write_hashes.cmake` output. The `hash-equality` job passed. `libretro.host` dlopens the core, resolves every retro_* symbol, calls it in RetroArch's order (set_environment, init, load_game(NULL), get_system_av_info, run) and compares XRGB8888 pixels with the runner's P6, requiring 0 mismatches. It passes on all six CI legs. |
| 3 | After the README's one-line install on Apple Silicon, `ctest -L retroarch` launches RetroArch unattended and its screenshot matches the runner's frame; skipped where RetroArch is absent | ⚠️ PRESENT_BEHAVIOR_UNVERIFIED (partial) | **Proven:** `ctest --preset ci -L retroarch -V` on this Mac (RetroArch 1.22.2) passed: "RetroArch's frame equals the runner's frame". `run_retroarch.cmake` uses an isolated config, `--max-frames=5 --max-frames-ss`, strips colour management with sips, does a pixel-exact `compare_frame` against runner frame 5, and checks the user directory is untouched. The skip path works: on macos-15 CI (no RetroArch) the test shows `***Skipped`. `check_install_line.cmake` parses the README line's tar arguments and extracts exactly `cores/nesturbator_libretro.dylib` and `info/nesturbator_libretro.info` from the built zip. It passed locally and on the macos-15 arm64 CI leg. **Unproven:** the line's URL `…/releases/download/v0.0.0/nesturbator-0.0.0-libretro-macos-arm64.zip` returns HTTP 404, and no release exists yet. The release PR must rewrite it to v0.1.0 (README.md is in release-please `extra-files`, inside an `x-release-please-start-version` block), and that has not happened yet. Also, the RetroArch test loads the build-tree core, not the installed one. |
| 4 | `asan`, `nofp` and `hygiene` presets pass: sanitizer-clean, core has no floating point and no undefined symbols beyond the C memory functions, tree holds no personal data and no unlisted ROM | ✓ VERIFIED (with warning) | Locally: `asan` exit 0 (29/29), `nofp` exit 0 (5/5 abi: float_scan, selftest, undefined_symbols, global_symbols, float_fixture), `hygiene` exit 0 (5/5). The same lanes are green in CI run 37084997715 (asan, nofp ×2, hygiene). I scanned the tree myself, independently of the hygiene script and fail-closed (`core.quotePath=false`, `LC_ALL=C`). There are 0 non-ASCII file names and 0 non-UTF-8 text files among 141 tracked files, so neither the CR-01 nor the CR-02 bypass applies to today's tree. The only `/Users/` hits are the patterns in scripts/hygiene.sh. There are no email addresses beyond noreply addresses. All commit identities are `szTheory@users.noreply.github.com`. There are no ROM-extension files. `hygiene.sh --history` exit 0. **Warning:** I reproduced CR-01 and CR-02 in a scratch repo. With `café.txt` and a Latin-1 file both holding a home path, `hygiene.sh --tree` reported only the ASCII control file. The gate fails open, so the guarantee for future commits is weaker than CLAUDE.md rule 3 claims. |
| 5 | Every PR builds library, runner and libretro archives for each platform; a behaviour-changing merge to main publishes them with SHA256SUMS as a GitHub release with no manual step | ⚠️ PRESENT_BEHAVIOR_UNVERIFIED (partial) | **PR half proven:** run 37084997715 uploaded six `archives-<os>-<arch>` artifacts. I downloaded `archives-windows-arm64`: library, runner and libretro zips. `check_archives.cmake` ran on every leg. **Release half present and wired but never run:** I ran actionlint on release.yml and ci.yml and it was clean. release.yml runs release-please with an App token, auto-merges the release PR, then re-runs ci.yml on the release commit. Publish requires exactly the 18 archives, writes SHA256SUMS, attests, uploads, verifies, runs a tamper check, then `--draft=false`. Repository: `NESTURBATOR_APP_ID` variable and `NESTURBATOR_APP_PRIVATE_KEY` secret present. `allow_auto_merge` true, squash only. Ruleset `main` active and semantically equal to `.github/rulesets/main.json` (the only differences are server defaults). PR #1 title `feat: …` is releasable. **Not provable before merge:** `gh release list` is empty, and release.yml has never run. I could not confirm the App installation on the repository with a user token (HTTP 403/401). |

**Score:** 3/5 truths verified (2 present, behavior-unverified)

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `include/nesturbator.h` | Public API | ✓ VERIFIED | Used by consumer, header.c/header.cxx tests |
| `src/{instance,frame,testcard,palette,palette_ntsc}.c` | Core, test card, palette | ✓ VERIFIED | core.api/frame/palette tests pass; palette.regen byte-exact |
| `runner/main.c`, `ppm.c`, `sha256.c` | Runner, P6 dump, hash | ✓ VERIFIED | Ran directly; runner.* and runner.sha256 tests pass |
| `host/convert.c` | Shared palette conversion | ✓ VERIFIED | host.convert passes; used by runner and adapter |
| `libretro/libretro.c`, `.info` | Adapter | ✓ VERIFIED | libretro.host passes on 6 legs; RetroArch loads it locally |
| `cmake/packaging.cmake`, `nesturbatorConfig.cmake.in`, `.pc.in` | Install + CPack | ✓ VERIFIED | install.stage/consumer pass; 3 zips per platform |
| `scripts/hygiene.sh` | Privacy/ROM gate | ⚠️ VERIFIED with defects | Runs and passes; fails open on non-ASCII names / non-UTF-8 text (CR-01/CR-02) |
| `tests/retroarch/*` | Unattended RetroArch compare | ✓ VERIFIED | Passed locally against RetroArch 1.22.2 |
| `.github/workflows/ci.yml` | Six-platform CI | ✓ VERIFIED | Green run 37084997715 |
| `.github/workflows/release.yml`, `release-please-config.json` | Release on merge | ⚠️ PRESENT, never run | actionlint clean; no run history |
| `tests/cmake/check_install_line.cmake` | README line check | ✓ VERIFIED | Passes locally and on macos-15 arm64 CI |

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| ci.yml build legs | CMakePresets `ci`/`ci-msvc` | `cmake --workflow --preset` | WIRED | Same command as local |
| ci.yml | hash-equality | `hashes-*` artifacts | WIRED | 6 artifacts, byte-identical |
| ci.yml `CI required` | ruleset required check | context `CI required`, integration 15368 | WIRED | Remote ruleset active |
| release.yml publish | ci.yml archives | `workflow_call` + `archives-*` download | WIRED (unexercised) | Never run |
| README install line | release version | release-please `extra-files` generic block | WIRED (unexercised) | Still v0.0.0 |
| libretro.host | runner P6 | `NESTURBATOR_TESTFRAME_PPM` fixture | WIRED | Pixel-exact compare |
| retroarch.testframe | runner + compare_frame | run_retroarch.cmake | WIRED | Passed locally |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|----------|---------|--------|--------|
| ci lane | `cmake --workflow --preset ci` | exit 0, 30/30 | ✓ PASS |
| asan lane | `cmake --workflow --preset asan` | exit 0, 29/29 | ✓ PASS |
| nofp lane | `cmake --workflow --preset nofp` | exit 0, 5/5 | ✓ PASS |
| hygiene lane | `cmake --workflow --preset hygiene` | exit 0, 5/5 | ✓ PASS |
| RetroArch on Apple Silicon | `ctest --preset ci -L retroarch -V` | "RetroArch's frame equals the runner's frame" | ✓ PASS |
| Runner hash and dump | `nesturbator-run --frames 1 --dump-frame 1:f.ppm --hash-frame 1` | sha256 b49e9be4…, P6 256x240 | ✓ PASS |
| Cross-platform hash | downloaded 6 `hashes-*` artifacts + local write_hashes | all identical | ✓ PASS |
| Install-line extraction | `check_install_line.cmake` | extracts the core and its .info file | ✓ PASS |
| Install-line download | `curl -sIL …/v0.0.0/…-libretro-macos-arm64.zip` | 404 | ? SKIP (no release yet) |
| Workflow lint | `actionlint ci.yml release.yml` | clean | ✓ PASS |
| History hygiene | `scripts/hygiene.sh --history` | exit 0 | ✓ PASS |
| CR-01/CR-02 repro | scratch repo, `hygiene.sh --tree` | missed `café.txt` and Latin-1 file | ✗ confirms defect |

### Probe Execution

No `scripts/*/tests/probe-*.sh` exist, and no PLAN declares a probe. Step 7c: SKIPPED.

### Requirements Coverage

| Requirement | Source Plan(s) | Status | Evidence |
|-------------|----------------|--------|----------|
| FRAME-01 | 4 plans | ✓ SATISFIED | SC1 |
| FRAME-02 | 2 plans | ✓ SATISFIED | install.consumer on all legs |
| FRAME-03 | 4 plans | ✓ SATISFIED | SC2 |
| FRAME-04 | 01-05 | ✓ SATISFIED | libretro.host |
| FRAME-05 | 3 plans | ? PARTIAL | RetroArch compare and skip proven; download half of install line awaits first release |
| FRAME-06 | 2 plans | ✓ SATISFIED (warning) | Lanes pass; tree clean; hygiene gate fails open (CR-01/CR-02) |
| FRAME-07 | 3 plans | ? PARTIAL | PR archives proven; publish on merge unrun |

All seven IDs appear in plan frontmatter. None is orphaned. REQUIREMENTS.md already marks FRAME-05 and FRAME-07 `[x] Complete`. That is premature until the first release is published.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| scripts/hygiene.sh | 30, 41-53, 94, 100 | Fails open: C-quoted non-ASCII paths unreadable, `grep -I` under UTF-8 locale skips non-UTF-8 text (CR-01, CR-02, reproduced) | ⚠️ Warning | Future commits can leak home paths, emails or ROM bytes past the pre-commit hook and CI. The current tree is clean. Fix: `-c core.quotePath=false` (or `-z` lists), `LC_ALL=C` exported, and fail closed on unreadable content |
| .github/workflows/ci.yml | 33-155 | 1-2 min timeouts on all jobs; `apt-get install` without `update` (WR-09) | ⚠️ Warning | release.yml reuses ci.yml, so a slow runner can fail the release run's CI and stop the publish |
| release-please-config.json | 10 | `release-as: "0.1.0"` with only a STATE.md todo to remove it (WR-08) | ⚠️ Warning | The second release would re-propose 0.1.0. This is tracked for Phase 2 but not by an automated check |
| runner/main.c | 36 | Runner with no args exits 0 (WR-02, reproduced: `no-args exit=0`) | ℹ️ Info | Contradicts README "--frames required" |
| — | — | TBD/FIXME/XXX/TODO in phase files | none found | — |

### Human Verification Required

#### 1. Merge PR #1 (owner consent only)

**Test:** Mark draft PR #1 ready and squash-merge it.
**Expected:** release.yml runs. Release-please opens the v0.1.0 PR and auto-merges it once `CI required` passes. The release CI re-runs and publish makes v0.1.0 public.
**Why human:** Only merging to main and publishing need the owner's consent. Every check after that is a command, listed under `behavior_unverified_items`: `gh release view v0.1.0`, `shasum -c SHA256SUMS`, curl of the README URL, a scratch run of the install line, and `ctest --preset ci -L retroarch`. Run them as an automated follow-up, not as manual testing.

### Gaps Summary

There is no blocking gap in the code. Two success criteria are only half proven, and both for the same reason: the release-on-merge path (SC5) and the README install download that depends on it (SC3) cannot run until PR #1 merges. Everything checkable before merge is checked. The workflows lint clean. The App credential variable and secret exist. Auto-merge is enabled. The ruleset is live and matches the file. The install line's extraction works against the real archive.

Recommended before merge, so the first release is not exposed to known risks:
1. Fix CR-01/CR-02 in scripts/hygiene.sh. It is a few lines, and the review gives the fix. The public repository's only privacy gate currently fails open.
2. Raise the CI job timeouts and add `apt-get update` (WR-09). The release run reuses ci.yml, so the current margins put the "no manual step" publish at risk.
3. Triage the 01-REVIEW-DISPOSITION.md ledger. All 21 findings are still `open`.

After the merge, the post-merge commands above close SC3 and SC5. Re-run verification then, or mark them passed.

---

_Verified: 2026-10-03T01:40:00Z_
_Verifier: Claude (gsd-verifier)_
