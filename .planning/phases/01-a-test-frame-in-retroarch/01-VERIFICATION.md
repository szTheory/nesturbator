---
phase: 01-a-test-frame-in-retroarch
verified: 2026-10-09T12:59:23Z
status: passed
score: 69/69 must-haves verified
covered_files: [".github/rulesets/main.json", ".github/workflows/ci.yml", ".github/workflows/release.yml", ".planning/phases/01-a-test-frame-in-retroarch/01-01-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-01-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-02-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-02-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-03-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-03-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-04-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-04-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-05-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-05-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-06-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-06-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-07-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-07-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-08-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-08-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-09-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-09-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-10-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-10-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-11-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-11-SUMMARY.md", ".planning/phases/01-a-test-frame-in-retroarch/01-12-PLAN.md", ".planning/phases/01-a-test-frame-in-retroarch/01-12-SUMMARY.md", "CMakeLists.txt", "CMakePresets.json", "README.md", "cmake/packaging.cmake", "include/nesturbator.h", "libretro/libretro.c", "libretro/nesturbator_libretro.info", "runner/main.c", "runner/ppm.c", "runner/sha256.c", "scripts/hygiene.sh", "src/frame.c", "src/testcard.c", "tests/CMakeLists.txt", "tests/cmake/check_archives.cmake", "tests/cmake/check_install_line.cmake", "tests/cmake/undefined_symbols.cmake", "tests/libretro/libretro_host.c", "tests/retroarch/run_retroarch.cmake"]
covered_digest: "v3:sha256:0766964b0dfc4179712a798e2621b1071281ce442e7fb884e2f6f2e7acd829fb"
behavior_unverified: 0
overrides_applied: 0
---

# Phase 1: A test frame in RetroArch Verification Report

**Phase Goal:** Anyone can build, test, download and install the library, the runner and the libretro core, and RetroArch shows the core's built-in test frame.
**Verified:** 2026-10-09T12:59:23Z
**Status:** passed
**Re-verification:** No. The previous report had no `gaps:` frontmatter; its three human-review items were re-evaluated against the completed UAT and current code/evidence.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | CI builds, installs, tests and packages the library, runner and libretro core on six platforms, including a consumer using only the public header. | ✓ VERIFIED | Current hosted CI run 37862745508 succeeded on Linux, macOS and Windows x64/arm64, with `asan`, both `nofp` jobs, `hygiene`, `hash-equality`, `retroarch-e2e` and `CI required`. `CMakePresets.json` routes the build matrix to `ci`/`ci-msvc`; `tests/CMakeLists.txt` wires `install.stage` to `install.consumer`, whose program includes the public header and creates, runs and destroys an instance. `query verify.artifacts` found all 46 declared artifacts. |
| 2 | Without a cartridge, the runner emits a stable test frame and the libretro adapter produces the same pixels. | ✓ VERIFIED | `src/frame.c` calls `nesturbator__test_pixel` on the no-cartridge path; `runner/main.c` hashes native uint16 pixels in explicit little-endian row order and writes P6 through the shared converter. `libretro/libretro.c` calls the core and forwards converted pixels; `libretro.host` loads the module and compares against the runner-produced PPM under the `testframe_ppm` fixture. Current CI's `hash-equality` job passed across six platforms. |
| 3 | The documented install lets RetroArch show the test frame and the unattended launch compares its screenshot exactly; unsupported machines skip the test. | ✓ VERIFIED | Phase UAT records the real Apple Silicon install and `retroarch.testframe` exact screenshot comparison as passed. Hosted `retroarch-e2e` passed in run 37862745508. `tests/retroarch/run_retroarch.cmake` uses an isolated configuration/home, snapshots the user's RetroArch directory, runs the core, and pixel-compares the screenshot. The CTest registration has skip behavior for unavailable RetroArch. |
| 4 | Sanitizer, no-float, ABI-symbol, and hygiene checks are wired and pass. | ✓ VERIFIED | The latest hosted run passed `asan`, both `nofp` jobs and `hygiene`. The current protected tree is checked by `scripts/hygiene.sh`; no unreferenced `TBD`, `FIXME` or `XXX` markers were found in project-authored phase implementation files. |
| 5 | Pull requests build platform archives and behavior-changing releases publish the archives, checksums and attestations automatically. | ✓ VERIFIED | Hosted CI produced successful six-platform build jobs. Release run 37862745747 succeeded including `publish`; live v0.1.4 is non-draft and has 19 assets: 18 component/platform archives plus `SHA256SUMS`. `.github/workflows/release.yml` requires the called CI workflow, asserts the exact 18 archive names, writes checksums, attests them, verifies each archive and verifies published metadata. |

**Score:** 69/69 must-haves verified; all 5/5 roadmap success criteria are supported. No behavior-dependent truth remains untested or awaiting human verification.

### Prior Human Findings Rechecked

| Finding in prior report | Current evidence | Result |
|---|---|---|
| Clean-room provenance of palette generator/table | Phase UAT records the owner's explicit provenance review as passed; this verifier did not open restricted emulator source. | Resolved |
| Release credential isolation | `release.credentials_policy` and its mutation self-test are registered in `tests/CMakeLists.txt`; the checker rejects a selectable-ref trigger and extra secret consumer. Both ran in the successful hosted CI run. The credential-bearing workflow now triggers only on push to `main`. | Resolved |
| Docs/chore-only changes must not publish | `release.nonbehavioral_policy` and its mutation self-test are registered; the checker covers docs/chore suppression and behavior/breaking-release cases. Phase UAT records this as passed. The positive release path is independently live in v0.1.4 and workflow run 37862745747. | Resolved |

### Required Artifacts

| Artifact group | Expected | Status | Details |
|---|---|---|---|
| PLAN-declared artifacts, plans 01-01 through 01-12 | Present, substantive, wired | ✓ VERIFIED | `query verify.artifacts` reports 46/46. Source review confirmed the core-to-runner and core-to-adapter frame path, installed consumer, archive production, and hosted workflow wiring. |
| Core frame and render paths | Real frame pixels flow through runner and adapter | ✓ VERIFIED | `src/frame.c` populates all native pixels from the test-card function when no cartridge is loaded; runner and adapter use live core frame output, not a hard-coded image. |
| Build/install/release path | Presets build, test, install and package; CI/release consume outputs | ✓ VERIFIED | Presets, CMake install fixtures, CPack layout checks, hosted CI, live release assets and release workflow all connect. |

### Key Link Verification

| Link group | Status | Details |
|---|---|---|
| 25 PLAN-declared links | ✓ WIRED | GSD pattern checks directly confirmed 22/25. The remaining three are resolved by direct inspection/evidence: the libretro host consumes the generated `testframe_ppm` fixture set by `runner.dump`; the Phase 2 `release-as` pin is gone and `release.no_release_as` is registered; GitHub ruleset `main` (id 24400039) is live and active. |
| Runner → public core → test frame | ✓ WIRED | `runner/main.c` creates an instance and calls `nesturbator_run_frame`; `src/frame.c` calls the pure test-card pixel function for the no-cartridge case. |
| Libretro host → module → runner image | ✓ WIRED | The host dynamically loads adapter symbols, passes the module path and generated runner PPM, and checks both the pixels and known test-card coordinates. The CTest fixture guarantees the PPM producer runs first. |
| CI → artifacts/hashes; release → CI → publish | ✓ WIRED | All six build legs call the named presets, upload archives and hashes, and the hash-equality job checks all six. Release publication is dependent on CI and enforces archive count, checksum and attestation steps. |

### Data-Flow Trace (Level 4)

| Artifact | Data | Source | Produces real data | Status |
|---|---|---|---|---|
| `src/frame.c` → runner | Native frame pixels | Instance state and `nesturbator__test_pixel` | Yes | ✓ FLOWING |
| Runner → PPM/hash | RGB bytes and hash bytes | Core video buffer, palette/converter and SHA-256 | Yes | ✓ FLOWING |
| `libretro/libretro.c` → host/RetroArch | Video callback pixels | Core video buffer and shared palette conversion | Yes | ✓ FLOWING |
| Installed consumer | Created instance and rendered frame | Installed CMake target and public API | Yes | ✓ FLOWING |
| CI/release archives | Per-platform packages and digests | Build jobs, package step and release artifacts | Yes | ✓ FLOWING |

### Behavioral Spot-Checks

No tests were run during this verification, per instruction. Read-only evidence used: completed hosted CI run 37862745508 (all 14 jobs succeeded), completed release run 37862745747 (all 16 jobs succeeded), phase UAT results, and live v0.1.4 metadata/assets. Local behavior was assessed from implementation and test wiring.

| Behavior | Evidence | Status |
|---|---|---|
| Six-platform build, package and frame-hash equality | Current hosted CI run 37862745508 | ✓ PASS |
| RetroArch screenshot comparison | Hosted `retroarch-e2e` plus completed phase UAT | ✓ PASS |
| Sanitizer, no-float, hygiene lanes | Current hosted CI run 37862745508 | ✓ PASS |
| Publish exact assets and release metadata | Current release run and live v0.1.4 release | ✓ PASS |
| Credential isolation and docs/chore policy | Registered checks and mutation self-tests passed in hosted CI; UAT completed | ✓ PASS |

### Probe Execution

No plan-declared probes or conventional phase probes exist. Step 7c: SKIPPED.

### Requirements Coverage

| Requirement | Description | Status | Evidence |
|---|---|---|---|
| FRAME-01 | Six-platform `ci`/`ci-msvc` build and tests, same commands in CI | ✓ SATISFIED | Hosted run 37862745508 succeeded all six build legs and required roll-up. |
| FRAME-02 | Public-header-only program runs against installed package | ✓ SATISFIED | `install.consumer` is wired to stage the library and compile/run `tests/consumer/main.c`; hosted `ci` passed. |
| FRAME-03 | No-cartridge deterministic frame, image/hash output, identical hashes | ✓ SATISFIED | Core, runner, and hash-equality paths are wired; hosted hash-equality passed. |
| FRAME-04 | Libretro test host receives runner-equal frame | ✓ SATISFIED | Dynamic host test, fixture dependency and pixel comparison are wired; hosted `ci` passed. |
| FRAME-05 | README install and unattended RetroArch screenshot comparison | ✓ SATISFIED | UAT passed actual install and screenshot comparison; hosted `retroarch-e2e` passed. |
| FRAME-06 | ASan, no-float, allowed-symbol and hygiene checks | ✓ SATISFIED | Hosted `asan`, both `nofp`, and `hygiene` jobs succeeded. |
| FRAME-07 | PR archives and automatic release with checksums | ✓ SATISFIED | Six-platform CI success; live release has exactly 18 archives plus `SHA256SUMS`; release attestation/publish workflow succeeded. |

No phase-mapped requirements are orphaned.

### Anti-Patterns Found

| File | Pattern | Severity | Impact |
|---|---|---|---|
| `libretro/libretro.h` | Upstream vendored TODO comments | Info | Vendored public libretro header; no TODO is a project implementation stub or phase debt marker. |
| `.planning/phases/01-a-test-frame-in-retroarch/01-REVIEW-DISPOSITION.md` | WR-01 release-policy test is narrower than its prose about failed-CI gating | Warning | Review concern only; workflow explicitly declares `needs: [release-please, ci]` and a release-created guard. It does not contradict a phase must-have. |

### Human Verification Required

None. All human items from the earlier `human_needed` report were resolved in the completed phase UAT.

### Gaps Summary

No blocking gaps remain. The previous report's UAT items are all complete, including the live RetroArch comparison and release-policy checks. The phase goal and all seven FRAME requirements are supported by the source, wiring, hosted CI, UAT, and published release evidence. The three automated key-link pattern misses were resolved through direct fixture/source checks and live ruleset evidence.

---

_Verified: 2026-10-09T12:59:23Z_  
_Verifier: the agent (gsd-verifier)_
