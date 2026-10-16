---
phase: 16-host-evidence-and-roadmap
verified: 2026-10-15T03:52:41Z
status: gaps_found
score: 21/22 must-haves verified
covered_files:
  - .github/workflows/ci.yml
  - .planning/phases/16-host-evidence-and-roadmap/16-01-PLAN.md
  - .planning/phases/16-host-evidence-and-roadmap/16-01-SUMMARY.md
  - .planning/phases/16-host-evidence-and-roadmap/16-02-PLAN.md
  - .planning/phases/16-host-evidence-and-roadmap/16-02-SUMMARY.md
  - .planning/phases/16-host-evidence-and-roadmap/16-03-PLAN.md
  - .planning/phases/16-host-evidence-and-roadmap/16-03-SUMMARY.md
  - .planning/phases/16-host-evidence-and-roadmap/16-04-PLAN.md
  - .planning/phases/16-host-evidence-and-roadmap/16-04-SUMMARY.md
  - .planning/phases/16-host-evidence-and-roadmap/16-05-PLAN.md
  - .planning/phases/16-host-evidence-and-roadmap/16-05-SUMMARY.md
  - .planning/phases/16-host-evidence-and-roadmap/16-06-PLAN.md
  - .planning/phases/16-host-evidence-and-roadmap/16-06-SUMMARY.md
  - .planning/preparation/GSD-HANDOFF.md
  - AGENTS.md
  - CMakePresets.json
  - README.md
  - include/nesturbator.h
  - libretro/nesturbator_libretro.info
  - tests/CMakeLists.txt
  - tests/cmake/check_archives.cmake
  - tests/cmake/fixture_identity.cmake
  - tests/cmake/info_claims.cmake
  - tests/libretro/libretro_host.c
  - tests/retroarch/CMakeLists.txt
  - tests/retroarch/frame_index.c
  - tests/retroarch/progression.h
  - tests/retroarch/run_retroarch.cmake
  - tests/retroarch/state_session.sh
  - tests/retroarch/test_state_session.sh
covered_digest: "v3:sha256:90107b5478a220b3d3473a638fc36fb541cc93fb6ba95a7d615b0a83901b792e"
behavior_unverified: 0
overrides_applied: 0
gaps:
  - truth: "Released libretro metadata truthfully names MMC3 mapper 4 submappers 0/4, excludes MMC6 and four-screen, and agrees with documented supported profiles."
    status: failed
    reason: "The latest published release is v0.1.10, targeting da2bf973813530cb9561a004e7b774011a0a7867. Its verified libretro asset description lists NROM, MMC1, UxROM, CNROM and AxROM but omits MMC3. Current source, documentation, staged install and package checks include the supported MMC3 clause, but the published asset remains stale."
    artifacts:
      - path: "GitHub release v0.1.10 asset nesturbator-0.1.10-libretro-macos-arm64.zip"
        issue: "SHA-256 9226ddce3e95c4bb0a2b13d910e34c225f67c0a02e24ce550b315e551235142d; extracted info/nesturbator_libretro.info lacks the MMC3 support clause."
    missing:
      - "Publish the updated libretro metadata through the normal release process, then verify the released tag and actual asset metadata."
---

# Phase 16: Host evidence and roadmap Verification Report

**Phase Goal:** Host builders can trust the current RetroArch state/progression evidence and released MMC3 description, while milestone starts and closeouts preserve evidence-backed product direction.
**Verified:** 2026-10-15T03:52:41Z
**Status:** gaps_found
**Re-verification:** No — initial verification; no prior phase VERIFICATION.md report existed.

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|---|---|---|
| 1 | A generated marker maps uniquely to absolute library frames across the permitted capture, rewind and display-lead window; startup, out-of-window and malformed markers fail. | ✓ VERIFIED | Current required CI run 38676328559 passed marker qualification and marker-control tests on the six-platform matrix. Hosted calibration recorded interval 4..10000. |
| 2 | The pinned hosted binary qualifies the final marker and native save anchor before later network assertions are accepted. | ✓ VERIFIED | Current hosted artifact records qualification=pass, pinned RetroArch 1.22.2, exact source commit 841434b46b202dd04ffbdeb0ac4c4bcbd0b07159, and hosted_marker=complete. |
| 3 | Native-valid anchors bind expected content, format, current behavior revision and absolute origin. | ✓ VERIFIED | Marker/anchor controls passed; the current hosted artifact binds content/core/info digests and rejects the valid same-content wrong-origin control as ORIGIN. |
| 4 | One production verdict uses a single retained original anchor and a single retained rewound anchor across their respective branches. | ✓ VERIFIED | The 16-row hosted ledger uses original anchor identity 2af5f5fe12c401b5 at frame 450 and rewound anchor identity a3de3d357a547db4 at frame 505. |
| 5 | Dropped, doubled, compensated identical-picture and wrong-load histories cannot pass exact progression. | ✓ VERIFIED | Current progression controls passed. Hosted dropped and doubled advance faults both terminate at boundary 3 with PROGRESSION / ADVANCE_NOT_EXACTLY_ONE; the same-content wrong-origin control returns ORIGIN. |
| 6 | A paused load is confirmed only as one load plus one planned probe advance, with separately calibrated run-ahead display lead. | ✓ VERIFIED | Hosted boundary 8 records LOAD_STATE_SLOT 0 plus one FRAMEADVANCE, two sends total, at frame 451; boundary 16 records the run-ahead load/probe and calibrated display lead 1 at frame 507. |
| 7 | Only exactly parsed read-only readiness requests retry; each mutation, publication request and QUIT is sent once per prescribed operation. | ✓ VERIFIED | Bounded session-driver tests passed in required CI; hosted journal shows readiness retries separately and one-shot operation sends, with 79 intended sends and zero cleanup sends. |
| 8 | Nested receive, decode, poll and child work consumes measured elapsed-operation and attempt budgets, with bounded cleanup. | ✓ VERIFIED | Deterministic clock/process controls passed in the required CI run; hosted intended attempt completed in 201896 ms within its recorded remaining budget. |
| 9 | Repeated/concurrent attempts use separate owned trees; an unavailable exclusive command endpoint fails before sending, and interruption reaps only owned children. | ✓ VERIFIED | The production driver and deterministic shell-control tests passed; the hosted attempt tree and journal are separately owned and attributable. |
| 10 | One outstanding artifact publication is attributed to its boundary; stale, extra, partial or contradictory evidence terminates the attempt. | ✓ VERIFIED | Driver publication/attribution controls passed in the required CI run; every hosted boundary references its own observation and prefix verdict. |
| 11 | Pinned real-host progression uses the production verdict and retains required game, battery, claim-level, portable state and audio lanes. | ✓ VERIFIED | Hosted E2E and its artifact passed; the same required job also completed the game frame, battery, MMC3 battery and deterministic auto-state lanes. |
| 12 | Only exact case-sensitive final components select the two existing special fixture roles; misleading parents and similar names stay generic. | ✓ VERIFIED | libretro.fixture_roles and libretro.fixture_identity passed in the current matrix; full tests passed 637/637 on both Windows targets. |
| 13 | Empty, null and trailing-separator names select no special role; separator behavior is platform-correct and paths with spaces remain intact. | ✓ VERIFIED | Role and identity boundary cases passed across the current platform CI matrix. |
| 14 | Fixture equality is exact basename bytes and supplied-file SHA-256, without Unicode, case or path normalization. | ✓ VERIFIED | Current fixture wrapper authenticates the actual supplied/copied file against the canonical manifest; adversarial identity tests passed. |
| 15 | Registered tests hash the actual supplied or copied special input before host invocation and retain original serializer/runahead counts. | ✓ VERIFIED | CMake routes serializer/runahead through fixture_identity.cmake; the clean-build argv-capture regression now passes on both Windows jobs and the full matrix. |
| 16 | Frontend metadata positively states NTSC MMC3 mapper 4 submappers 0/4 support and excludes MMC6 and four-screen. | ✓ VERIFIED | Current source .info, README and public-header clause agree; source, staged-install and package checks passed. |
| 17 | Localized metadata, README/header facts and existing cartridge accept/reject evidence agree; unrelated prose edits pass. | ✓ VERIFIED | libretro.info_support and core.cartridge controls passed; support mutants fail for their intended reason and harmless prose changes pass. |
| 18 | Staged and packaged info bytes agree with checked source, while publication is evidenced separately. | ✓ VERIFIED | Required CI package/archive checks passed; actual v0.1.10 release evidence was inspected separately and is not represented as a current package. |
| 19 | Released libretro metadata truthfully names MMC3 mapper 4 submappers 0/4, excludes MMC6/four-screen, and agrees with documented profiles. | ✗ FAILED | The latest actual published asset is v0.1.10 and its extracted info file omits MMC3. Source and staged/package checks do not satisfy the separate released-metadata criterion. |
| 20 | The installed product roadmap has Immediate/Near/Mid/Long horizons, stable approved RM-001/RM-002/RM-003 commitments and conditional promotion evidence. | ✓ VERIFIED | The linked product roadmap and direction-verification record enumerate all four horizons, approved IDs, conditional promotion evidence and the preferred alternative ordering. |
| 21 | Both milestone-boundary instructions require reconciliation and commit/archive inside the invoked step; the actual v4 opening is credited. | ✓ VERIFIED | AGENTS.md Rule 8 and GSD-HANDOFF.md link the same-step contract; the direction record traces the actual 2026-10-13 v4 opening without claiming future compliance. |
| 22 | Seed narratives, unresolved debt, reconstruction dates and milestone/package version distinctions remain intact; installation does not imply future compliance. | ✓ VERIFIED | Direction evidence preserves seed dispositions and debt, distinguishes the reconstructed v3 snapshot and v4 approval, and expressly limits its claim to installed instructions and the recorded opening. |

The five roadmap success criteria are all represented above: SC1 by truths 1–6 and 11; SC2 by truths 7–10; SC3 by truths 12–15; SC4 by truth 19; SC5 by truths 20–22. Plan-specific detail in truths 16–18 remains separately covered.

**Score:** 21/22 truths verified (0 present, behavior-unverified).

### Required Artifacts

The SDK artifact query passed all 15 plan-declared artifact entries. I manually checked wiring because the key-link query does not detect several CMake-script, shell-variable and relative-Markdown references.

| Artifact | Expected | Status | Details |
|---|---|---|---|
| tests/retroarch/frame_index.c | Marker generation/qualification, anchor validation, observations and verdict modes | ✓ VERIFIED | Substantive C implementation; included by the pinned host tool and exercised by marker/progression controls and hosted evidence. |
| tests/retroarch/run_retroarch.cmake | Fail-closed hosted marker/progression qualification | ✓ VERIFIED | Hosted CI invokes it with the marker gate and retains its evidence tree. |
| tests/retroarch/CMakeLists.txt | Portable marker, anchor, progression and session controls | ✓ VERIFIED | CTest registrations execute in required CI. |
| tests/retroarch/progression.h | Bounded records and shared reason-coded verdict | ✓ VERIFIED | Included by frame_index.c; current production host controls exercise its verdict. |
| tests/retroarch/state_session.sh | Bounded one-shot production driver and journal | ✓ VERIFIED | Called from hosted CMake orchestration; intended and fault attempts produced attributable records. |
| tests/retroarch/test_state_session.sh | Deterministic driver controls | ✓ VERIFIED | Registered tests exercise the actual driver through transport, clock and process seams. |
| .github/workflows/ci.yml | Required pinned-host lane and retained artifact | ✓ VERIFIED | Current run executes qualification/progression and uploads artifact 11947290516. |
| tests/libretro/libretro_host.c | Exact final-component selector | ✓ VERIFIED | Both role selectors use the helper; current Windows and full platform tests pass. |
| tests/cmake/fixture_identity.cmake | Supplied-file identity wrapper and reason-specific controls | ✓ VERIFIED | Hashes copied bytes against manifest and reaches the recording host with intact argv in the corrected clean-build test. |
| tests/CMakeLists.txt | Registered authenticated fixture, metadata and cartridge tests | ✓ VERIFIED | Directly launches fixture wrapper and registers supporting metadata/cartridge checks. |
| libretro/nesturbator_libretro.info | Current localized MMC3 description | ✓ VERIFIED | Source description states the exact mapper/submapper and MMC6/four-screen exclusions. |
| tests/cmake/info_claims.cmake | Metadata/docs consistency controls | ✓ VERIFIED | Reads actual source and docs; staged install and independent mutants are registered. |
| tests/cmake/check_archives.cmake | Actual package metadata extraction and validation | ✓ VERIFIED | Reads and checks actual archive inventory and metadata; required CI archive checks passed. |
| .planning/phases/16-host-evidence-and-roadmap/16-DIRECTION-VERIFICATION.md | Installed product-direction/opening evidence | ✓ VERIFIED | Contains source-linked opening record, four-horizon review and lifecycle-boundary evidence. |
| GitHub release v0.1.10 libretro asset | Released MMC3 description | ✗ FAILED | The extracted published info omits the MMC3 clause; digest and target commit are recorded in the gap. |

### Key Link Verification

| From | To | Via | Status | Details |
|---|---|---|---|---|
| tests/retroarch/frame_index.c | include/nesturbator.h | Isolated instance and public state APIs | WIRED | Direct include and public save/load/format/revision use. |
| tests/retroarch/run_retroarch.cmake | tests/retroarch/frame_index.c | Generated marker, screenshots and retained native state | WIRED | CMake target path is passed into orchestration modes and used for qualification/verdict. |
| tests/retroarch/frame_index.c | tests/retroarch/progression.h | Shared production verdict | WIRED | frame_index.c includes the header; both real-observation and control modes call the same verdict. |
| tests/retroarch/progression.h | tests/retroarch/frame_index.c | Only qualified observations enter semantic comparison | WIRED | This is an admission invariant enforced at frame_index.c call sites; the pure header does not call its caller. |
| tests/retroarch/state_session.sh | tests/retroarch/frame_index.c | Validate each observation before the next mutation | WIRED | The driver invokes frame-index prefix/complete verdict commands through its configured executable. |
| tests/retroarch/test_state_session.sh | tests/retroarch/state_session.sh | Exercise actual driver through narrow seams | WIRED | Test harness executes the production shell script with controlled transport, clock and process helpers. |
| .github/workflows/ci.yml | tests/retroarch/run_retroarch.cmake | Required pinned-host lane | WIRED | The workflow invokes the CMake orchestration and uploads the marker-progression evidence. |
| tests/CMakeLists.txt | tests/cmake/fixture_identity.cmake | Host argv checked before launch | WIRED | Registered test passes the actual host target and ROM list to the wrapper. |
| tests/cmake/fixture_identity.cmake | tests/roms/manifest.txt | Exact record and SHA-256 of supplied bytes | WIRED | Wrapper derives the source manifest and hashes each copied special fixture. |
| tests/cmake/info_claims.cmake | tests/core/test_cartridge.c | Support claim plus existing mapper admission tests | WIRED | Both are required CTest registrations; positive MMC3 and excluded-profile cases exercise core behavior. |
| tests/cmake/check_archives.cmake | libretro/nesturbator_libretro.info | Archive metadata equals checked source | WIRED | Extracted info is compared to source and run through the support checker. |
| AGENTS.md | .planning/PRODUCT-ROADMAP.md | Rule 8 lifecycle instruction | WIRED | Rule 8 names the roadmap and both milestone boundaries. |
| .planning/preparation/GSD-HANDOFF.md | .planning/PRODUCT-ROADMAP.md | Relative Markdown link and boundary procedure | WIRED | The relative link resolves to the roadmap and the handoff repeats the same-step contract. |

### Data-Flow Trace (Level 4)

| Artifact | Data variable | Source | Produces real data | Status |
|---|---|---|---|---|
| frame_index.c / progression.h | observed frame and absolute identity | Pinned RetroArch screenshots decoded and checked against generated marker plus native anchors | Yes; current hosted artifact retains all 16 observations | FLOWING |
| state_session.sh | response, command attribution and boundary record | Actual local command socket, screenshot artifacts and decoded frame records | Yes; journal and boundary ledger bind each send/observation | FLOWING |
| fixture_identity.cmake | selected ROM path and digest | Supplied/copied ROM bytes plus exact manifest entry | Yes; hashes actual files before host launch | FLOWING |
| info_claims.cmake / check_archives.cmake | checked metadata bytes | Source .info/docs, isolated staged install and extracted package entry | Yes; required package checks passed | FLOWING |
| current published v0.1.10 asset | released description | Actual downloaded GitHub asset, SHA-256 9226ddce…5142d | Yes, but it is older content and lacks MMC3 | FLOWING; stale release is the EVID-04 gap |
| PRODUCT-ROADMAP.md direction evidence | outcome/horizon facts | Linked project, requirement, audit and seed records | Yes; source-linked, static planning evidence | FLOWING |

### Behavioral Spot-Checks

| Behavior | Command/evidence | Result | Status |
|---|---|---|---|
| Required source CI | GitHub Actions run 38676328559 on source 841434b46b202dd04ffbdeb0ac4c4bcbd0b07159; six platform builds, CI required rollup, hash-equality, ASan, no-float, hygiene and title | Run conclusion success; all required jobs passed. Perf was skipped by workflow. | ✓ PASS |
| Exact fixture argv boundary | Windows x64 job 116230661413 and Windows ARM64 job 116230661463 | libretro.fixture_identity passed on both; each full test matrix reports 637/637. | ✓ PASS |
| Intended real-host progression | Artifact 11947290516, hosted-evidence.txt and intended boundaries.tsv | 16 boundaries, 79 sends, 0 cleanup sends, terminal VERDICT_OK; complete intended ledger passed within the recorded attempt budget. | ✓ PASS |
| Production dropped/doubled/wrong-origin controls | Same hosted artifact fault-drop-advance, fault-double-advance and fault-valid-wrong-origin trees | Both advance faults rejected at boundary 3 as PROGRESSION / ADVANCE_NOT_EXACTLY_ONE; valid wrong-origin record rejected at boundary 13 as ORIGIN. | ✓ PASS |
| Staged and archived MMC3 info | Required CI libretro.info_support, libretro.info_staged and check_archives steps | Source, staged and package checks passed; actual published v0.1.10 remains stale as listed above. | ✓ PASS for implementation; published release criterion fails |

### Probe Execution

No standalone scripts/tests/probe-*.sh were declared or discovered. This phase uses registered CTest controls and the required hosted RetroArch lane; no separate probe execution was applicable.

### Requirements Coverage

| Requirement | Source plan | Description | Status | Evidence |
|---|---|---|---|---|
| EVID-01 | 16-01, 16-02, 16-03 | Real-host progression rejects ambiguous or wrong histories using distinguishable frames/state anchors | SATISFIED | Current pinned-host artifact passes intended progression and reason-specific negative controls. |
| EVID-02 | 16-03 | Bounded retries and one-shot non-idempotent commands | SATISFIED | Deterministic driver controls and hosted journal pass; 79 intended sends are attributed. |
| EVID-03 | 16-04 | Exact fixture identity and adversarial parent-directory resistance | SATISFIED | Fixture role/identity controls pass on all required platform builds, including Windows. |
| EVID-04 | 16-05 | Released libretro metadata truthfully describes supported MMC3 profiles | BLOCKED | Latest actual published v0.1.10 asset omits MMC3; source and package checks do not count as publication. |
| DIR-01 | 16-06 | Four-horizon product roadmap with stable outcome IDs and evidence/promotion triggers | SATISFIED | Product roadmap and direction-verification evidence establish all four horizons and commitments. |
| DIR-02 | 16-06 | Same-step reconciliation and archive/commit obligations at both milestone boundaries | SATISFIED | AGENTS Rule 8, GSD handoff and actual v4 opening evidence are linked and inspected. |

Orphaned Phase 16 requirements: none.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|---|---|---|---|---|
| None | — | No TBD/FIXME/XXX/TODO/HACK/PLACEHOLDER markers or empty implementation stubs found in the 16 reviewed source files | None | No anti-pattern blocker. |

### Human Verification Required

None. Runtime-dependent host/session behaviors were exercised by current automated tests and the exact-source hosted run. The release metadata gap is directly verifiable from the published asset.

### Gaps Summary

The implementation and direction artifacts pass the automated checks. The corrected clean-build fixture test now passes the complete required CI run, including both Windows architectures; the pinned RetroArch artifact demonstrates all 16 intended boundaries and rejects the dropped, doubled and wrong-origin histories with their production reasons.

The phase goal remains incomplete because EVID-04 and roadmap success criterion 4 require truthful released metadata. The only published release is v0.1.10 at an earlier target commit, and its actual libretro info description omits MMC3. The current source, docs and tested staged/package metadata are truthful, but they do not establish a release. Roadmap Phases 17 and 18 do not cover this publication gap, so it is not deferred.

---

_Verified: 2026-10-15T03:52:41Z_  
_Verifier: the agent (gsd-verifier)_

