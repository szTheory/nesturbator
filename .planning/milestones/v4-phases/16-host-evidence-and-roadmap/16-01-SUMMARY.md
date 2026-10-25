---
phase: 16-host-evidence-and-roadmap
plan: "01"
subsystem: testing
tags: [retroarch, marker, nrom, state, qualification, ci]
requires:
  - phase: 13-rewind-and-run-ahead-in-retroarch
    provides: Pinned host lanes, owned image readers and native state APIs
provides:
  - Injective generated marker calibrated across absolute frames 4..10000
  - Strict observed RetroArch envelope and native origin qualification
  - 36 reason-specific controls and a required pinned-host gate
affects: [16-02, 16-03, host-evidence]
actuals:
  tokens: 11530
  tasks: 2
  commits: 5
plan_head_before: 6d43fc626020f15da5a3a76bed9212e3c81e2d1c
plan_head_after: 003e17aeba939e81b6c9af8028d715f8fa92b289
tech-stack:
  added: []
  patterns: [owned generated guest bytes, isolated native oracle, strict bounded envelope]
key-files:
  created: []
  modified:
    - tests/retroarch/frame_index.c
    - tests/retroarch/CMakeLists.txt
    - tests/retroarch/run_retroarch.cmake
    - .github/workflows/ci.yml
    - README.md
    - include/nesturbator.h
key-decisions:
  - "Accepted absolute marker window is 4..10000; final measured absolute-minus-guest offset is 3. Wall time alone does not bound frame excursion."
  - "Retained anchors require the exact observed RASTATE1/MEM/pad8/END envelope, native loading, current revision and intended absolute origin plus marker continuation."
  - "The marker gate is additional qualification; exact network evidence and speculative audio remain separate proofs."
  - "Measured hosted cost updates the existing job ceiling to 14 minutes under the sampled-run policy."
requirements-completed: [EVID-01]
coverage:
  - id: D1
    description: Generated marker is injective and calibrated throughout the accepted interval
    requirement: EVID-01
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^retroarch.marker[.]' --no-tests=error; 9997 qualified frames"
        status: pass
    human_judgment: false
  - id: D2
    description: Actual pinned-host pictures and retained native origin qualify
    requirement: EVID-01
    verification:
      - kind: e2e
        ref: "CI run 38457185699 / retroarch-e2e / source a46f76c"
        status: pass
      - kind: integration
        ref: "frame_index --marker-anchor; downloaded native origin 150 continues at 151"
        status: pass
    human_judgment: false
  - id: D3
    description: Malformed, ambiguous and stale qualification evidence reaches its intended rejection
    requirement: EVID-01
    verification:
      - kind: unit
        ref: "frame_index --marker-selftest; 36 cases"
        status: pass
      - kind: integration
        ref: "CI run 38457185699; six build legs and sanitizer marker tests"
        status: pass
    human_judgment: false
duration: 2h 1m
completed: 2026-10-13
status: complete
---

# Phase 16 Plan 01: Marker and native origin qualification Summary

**An owned generated counter identifies every accepted frame, with strict native-origin validation and actual RetroArch 1.22.2 evidence.**

## Performance

- Started: 2026-10-13T19:20:18Z
- Completed: 2026-10-13T21:22:06Z
- Duration: 2h 1m, including required hosted runs
- Tasks: 2
- Production files modified: 6
- Actual tokens: 11530, calculated as ceiling of realized production diff characters / 4; not harness usage
- Actual commits: 5, measured from the persisted plan ledger before summary/tracking commits

## Accomplishments

- Added private generate/qualify/observe/anchor/selftest modes while preserving legacy image matching and `--dups`.
- Generated final guest bytes in owned C. Complete 8x8 bit cells and complementary checker guards at scanline 224 distinguish all 9997 frames in absolute interval 4..10000. Startup and out-of-window evidence fail. Every frame proves the measured mapping and one/two advancement, including carry transitions.
- Bounded content, calibration and state reads. Calibration cannot choose its own accepted range, revision or mapping: it must equal the independently specified contract with startup remeasurement.
- Validated retained host states on an isolated same-content instance through the public loader, then explicitly bound current revision, intended absolute origin and the next calibrated marker. The live host is never changed by this oracle.
- Registered 36 reason-specific controls in normal CI. CRC-valid stale revision and genuine saved frame 149 as a wrong frame-150 origin first pass native loading. Parser failure, missing fixtures and crashes cannot pass these semantic controls.
- Added the required host marker invocation and complete owned artifact retention, preserving game, battery, Nesteroids, historical network and portable state/audio lanes.

## Task Commits

1. Task 1 RED: `9f49c9e` — require independently decoded marker identity.
2. Task 1 GREEN: `f4053ee` — qualify generated marker and retained native origin.
3. Task 2 RED: `467d3a7` — require duplicate qualification rejection.
4. Task 2 GREEN: `a46f76c` — reject malformed and stale qualification evidence.
5. Task 2 measured-cost follow-up: `003e17a` — retain measured hosted timeout headroom.

The last follow-up changes only the host timeout/comment. Runtime code and gate wiring are exactly those proven at `a46f76c`; fresh hygiene verifies the 14-minute configuration. No deletion was introduced.

## Verification

Commands executed from the repository root:

```sh
cmake --workflow --preset ci
ctest --preset ci -R '^retroarch.marker[.]' --no-tests=error
cmake --workflow --preset hygiene
```

Final local results: CI 637/637 in 78.52 s; marker 3/3 with 36 control cases and 9997 qualified frames; final timeout-only hygiene 13/13 in 17.20 s. The modified C tool also compiles with GCC 14, C17, `-O3`, all repository warning flags and `-Werror`; clang-format 18 checks pass.

The exact additional hosted invocation is retained in `.github/workflows/ci.yml`, using the existing pinned app and these parameters:

```sh
cmake \
  -DCORE="$GITHUB_WORKSPACE/build/ci/libretro/nesturbator_libretro.dylib" \
  -DINFO="$GITHUB_WORKSPACE/libretro/nesturbator_libretro.info" \
  -DRUNNER="$GITHUB_WORKSPACE/build/ci/runner/nesturbator-run" \
  -DCOMPARE="$GITHUB_WORKSPACE/build/ci/tests/retroarch/compare_frame" \
  -DFRAME_INDEX="$GITHUB_WORKSPACE/build/ci/tests/retroarch/frame_index" \
  -DRA_DIR="$GITHUB_WORKSPACE/build/retroarch-e2e/marker-gate" \
  -DTEMPLATE="$GITHUB_WORKSPACE/tests/retroarch/test.cfg.in" \
  -DSTATE_TEMPLATE="$GITHUB_WORKSPACE/tests/retroarch/state.cfg.in" \
  -DSTATE_ROM="$GITHUB_WORKSPACE/build/ci/tests/retroarch/marker.nes" \
  -DMARKER_GATE=ON \
  -DRETROARCH="$GITHUB_WORKSPACE/build/retroarch-e2e/RetroArch.app/Contents/MacOS/RetroArch" \
  -DEXPECTED_VERSION=1.22.2 \
  -DVERSION_PLIST="$GITHUB_WORKSPACE/build/retroarch-e2e/RetroArch.app/Contents/Info.plist" \
  -P tests/retroarch/run_retroarch.cmake
```

- Initial real tracer: [CI 38455439227](https://github.com/szTheory/nesturbator/actions/runs/38455439227), exact source `f4053ee0ab06a0193a3e27033180ffc3a5d27c46`, complete success. Its gate passed before Task 2 expansion. Fresh portable tracer recheck passed 3/3.
- Final runtime: [CI 38457185699](https://github.com/szTheory/nesturbator/actions/runs/38457185699), exact source `a46f76cba7f7753ba115b546591dac1ce039fc3d`, complete success including `CI required`, six build legs, sanitizer, hygiene, both nofp legs and hash equality.
- Final normal builds: Linux/macOS 637 tests per leg, Windows 627 per leg; every marker control/qualification test passed. Sanitizer 632/632; exhaustive marker qualification 519.52 s, within its finite 600 s ceiling. No marker tests skipped.
- Both real runs retained `retroarch-marker-gate`. Final images and anchor were downloaded and independently decoded/loaded by the final local tool. No local launch failure or prototype was substituted for host proof.

### Final hosted observations

| Quantity | Proven value |
|---|---|
| Encoding / behavior revision | 1 / 6 |
| Accepted absolute interval | 4..10000 inclusive |
| Guest to absolute mapping | absolute = guest + 3 |
| Base / ahead display | 120 / 121 |
| Save / load / load-ahead display | 150 / 179 / 180 |
| Measured display lead | 1 |
| Retained native origin / continuation | 150 / 151 |
| Native format / size | 1 / 4895 bytes |
| Complete observed envelope | 4920 = 16 + roundup8(4895) + 8 bytes |

Final record identities:

- Content SHA-256: `c0c80278c61958a86c5fae30b72a5d98cb0c68de6ce316e9d06678bcf9a1558e`
- Copied core SHA-256: `655d6f5f5d94cbb1f801afcc67165c1b095a2751d869dec5b075db9ef7a9b5ef`
- Metadata SHA-256: `76686299bc7b25c4b25a3524197946471672731e6dd028e79094207848157850`
- Retained anchor SHA-256: `d948c4946a871eaa4216560f1529cd533287adbca7f0a61551709968056508aa`
- Each lane's configuration SHA-256, sanitized configuration, actual images, original native envelope, logs and calibration are in the artifact's `marker-gate-success.txt` and owned tree.

The app asset pin remains `81b79121ba26d539064ae13b4d0419a120c3d165afbe656cf5f5412b15fdb434`, version 1.22.2, hosted `macos-15`. Supplied/copied marker SHA-256 is compared with two independent deterministic generation invocations before host execution.

## TDD Gate Compliance

Both task-scoped RED records returned `RED_EVIDENCE_OK`, using unchanged JUnit reports from actual CTest runs with one named failing assertion and exit 8:

- Task 1: expected independently authored marker 257; incomplete decoder returned guard/value 0. Target executed and failed that identity assertion, with no fixture/setup fault.
- Task 2: independently specified 255 -> 255 one-step transition expected `duplicate-identity`; the incomplete shared transition verdict accepted it. Target executed and failed that duplicate assertion, with no parse/fixture fault. The completed verdict is used by exhaustive qualification and controls.

Records remain in ignored `build/ci/marker-red.json` and `marker-controls-red.json`; semantic assessments accompany them. Each RED commit precedes its GREEN commit. No separate refactor commit was needed.

## Decisions Made

The bounded absolute excursion is independent of host speed; later network attempts must reject observations outside it. Native validity alone does not establish current behavior or the intended origin. The accepted container is a measured released-binary observation, not a universal host format. Screenshots qualify displayed identity/lead; portable native state/audio tests retain their stronger proofs.

Final hosted job cost was 402 s (19:44:17Z–19:50:59Z), with 41 s for the additional gate. The latest 40 completed workflow histories contained 29 successful host jobs, slowest run 38457185699 at 402 s. Twice that cost rounds to 14 minutes. The measured policy follow-up changed no pin or runtime logic. Official [workflow syntax](https://docs.github.com/en/actions/reference/workflows-and-actions/workflow-syntax) was checked for job timeout and unconditional retention syntax.

## Deviations from Plan

1. **[Rule 1 - Bug] Marker row repair.** Exhaustive qualification found a split bit cell at frame 1467 in the initial early-row fixture, due to instruction-boundary drift in frame calls. Moved the generated row and complete-pixel decoder to scanline 224. Every final frame 4..10000 passed; no core behavior or decoder validity rule changed. Fixed in `f4053ee`; resolved WINDOWS entry 11.
2. **[Rule 3 - Blocking] Earlier hosted wiring.** Moved the Task 2 CI invocation/retention wiring into Task 1 to obtain its mandatory actual hosted tracer proof before expansion. The host gate passed first, then Task 2 expanded fault coverage. `f4053ee`; resolved WINDOWS entry 12.
3. **[Rule 1 - Bug] Truthful bound documentation.** Removed an unsupported 61-fps wall-time assumption. The enforced contract is absolute interval 4..10000 and fail-closed out-of-window evidence. `a46f76c`; resolved WINDOWS entry 13.

Three resolved deviations; no architectural amendment or unresolved defect. The measured timeout update follows the planned cost policy.

## Issues Encountered

The historical local signal-6 launch remains an unsuccessful observation; required final proof came from the pinned hosted lane. An optional workflow-monitor script was absent, so the existing authenticated GitHub CLI and actual hosted artifacts/logs supplied evidence. No package was installed. Generated ROMs, states, images, reports and sampled cost records remain ignored build artifacts.

## Threat Flags

| Flag | File | Description |
|---|---|---|
| threat_flag: file-input | tests/retroarch/frame_index.c | New private test-tool content/calibration/image/envelope inputs; bounded reads, strict size/layout checks and isolated public native loading constrain this test-only surface. |

## User Setup Required

None. No authentication gate or human verification was needed.

## Next Phase Readiness

Ready for 16-02. This plan contributes qualification to EVID-01; EVID-01 is **not fully complete** until Plans 02/03 prove the shared exact-network verdict and bounded driver. `requirements.ready-ids` reports EVID-01 blocked by sibling plans, so the requirement is not marked complete. Neither exact network progression nor speculative-audio correctness is claimed by this marker gate.

## Self-Check: PASSED

All six modified production files and this summary exist. All five recorded plan commits are
reachable on this branch; evaluation-scope resolves exactly those five commits. Final hosted
source identity and artifact observations were checked, all required tests passed, and the
coverage classifier accepts all three deliverables as fully automated. No unresolved stub,
skipped marker test, unrun required gate or accidental deletion remains.
