# Phase 22 execution evidence

All four plans and three review corrections are committed. The player README
routes to API, runner, release, testing and progress guides. Runtime sources,
public ABI, archive definitions, ROM pins and hash baselines are unchanged.

## Current verification

| Check | Outcome and limit |
|---|---|
| Plan-scoped tracers, controls and preservation ledgers | Passed; see the four summaries. |
| Full `cmake --workflow --preset ci` | Build succeeded; 699 of 700 CTests passed. Only `accuracycoin.activation` failed because historical provenance retained the pre-phase build-input fingerprint. The workflow stopped before packaging. This initial failure is retained here, not reported as a passing workflow. |
| Final-source configure/build | Passed after review corrections. |
| Final focused document/state/runner/release/info/policy CTest selection | All 18 checks passed after review corrections, including the new canonical-API-path rejection control. |
| Final `cpack --preset ci` and archive checks | Passed for library, runner and libretro on the configured macOS arm64 platform; archive layouts remain unchanged. |
| Final README install-line extraction | Passed against the local macOS package. No release fetch or real-host installation is implied by this check. |
| Repaired `accuracycoin.activation` | Passed after recording the existing measured final-source campaign provenance. Required fixture addition was disabled for this check with `-FA accuracycoin_full_report`, so it reused the retained report without another campaign. |
| Review | Three warnings; all fixed and verified. See review/disposition/fix artifacts. |

The full workflow's other successful tests remain evidence from that run;
focused checks cover the subsequent documentation/checker corrections and the
single failed provenance gate. The full expensive matrix was not repeated.
Raw local logs are outside Git; sanitized outcomes are recorded here and in
the summaries. No skip, threshold, protected PASS or assertion was weakened.

## Provenance failure and measured repair

The configured build already had `NESTURBATOR_ACCURACYCOIN_FULL=ON`. The full
workflow ran the native campaign successfully. A later narrow activation
selection automatically included that required fixture again, which should
have been checked before invoking CTest. It completed under the unchanged
registered bounds; no further campaign was started for the refresh. An
interim scope question about starting a new campaign was therefore superseded
by discovery of the already-produced evidence; it supplies no additional
runtime authority.

Only `tests/CMakeLists.txt` changed among the 56 fingerprinted inputs.
Substituting its pre-phase bytes reconstructs the previous accepted fingerprint
exactly. The final capture at compiled source `4c9b07f2c8b1`, dirty tree,
completed all 22 pages and 146 scored rows with 119 PASS / 27 FAIL, five DRAW,
zero skip/not-run/timeout and complete infrastructure. Its report SHA-256 is
the accepted `56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`.
The scoreboard and its protected results were not changed. Measured source,
input fingerprint and elapsed cost are recorded in
`tests/accuracy/baseline-metadata.txt`; historical identities remain in
`tests/accuracy/measurements.md`. This repairs stale current-input provenance,
not hardware correctness or hosted acceptance.

## Preserved limits

- Release/menu observations and unsuccessful local Sigstore initialization
  remain dated and qualified in the Plan 04 summary.
- Local packaging proves the configured platform, not six-platform execution.
- No real host/device, private content, daily save, other-core state, push,
  publication or gameplay acceptance was used.
- Cross-build battery reuse and incompatible host-container recovery remain
  unqualified; newest battery and both backup sets must remain preserved.
- Collector/platform/provenance debt and historical runtime stops remain.
- Owner edits to AGENTS.md and `.planning/config.json` remain unstaged.
