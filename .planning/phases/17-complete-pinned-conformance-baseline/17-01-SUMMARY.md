---
phase: 17-complete-pinned-conformance-baseline
plan: "01"
subsystem: conformance
tags: [AccuracyCoin, NES, C, CTest, ROM-pinning]

requires: []
provides:
  - Authenticated 6bee919 AccuracyCoin comparison ROM and complete decoded inventory, with page 3 observed through the production runner.
  - Production inventory, status-classifier and tracer controls, including key-scoped source pass variants and inventory-only DRAW rows.
  - Retained old-pin measurements for all 17 protected keys and a source-backed migration record with new-pin measurements left unmeasured.
affects: [17-02, 17-03, 17-04, AccuracyCoin conformance]

actuals:
  tokens: 44154
  tasks: 3
  commits: 3
plan_head_before: 211a3f66db5d3131dcbd6816bb8d805114165e34
plan_head_after: 125d204bcb2f30d107bb17ac150963dd0e75ee75

tech-stack:
  added: []
  patterns: [ROM-derived immutable inventory fingerprints, key-scoped result classification, old/new pin migration records]

key-files:
  created:
    - runner/accuracycoin.c
    - runner/accuracycoin.h
    - tests/roms/accuracycoin-6bee.nes
    - tests/cmake/accuracycoin_tracer.cmake
    - tests/accuracy/migration.tsv
    - tests/accuracy/old-673ef-scoreboard.txt
    - tests/accuracy/old-673ef-metadata.txt
  modified:
    - runner/main.c
    - runner/CMakeLists.txt
    - tests/CMakeLists.txt
    - tests/accuracy/test_conformance.c
    - tests/cmake/hash_keys.cmake
    - tests/roms/manifest.txt
    - README.md
    - include/nesturbator.h

key-decisions:
  - "Keep 673ef550 as the canonical strict fixture while admitting 6bee919 as a comparison fixture; no scoreboard or canonical pin activation occurs in this plan."
  - "Accept nonstandard pass bytes only for the exact decoded tests named by the pinned assembly; classify $02 as a valid failure and shared DRAW rows as inventory-only."
  - "Record all 17 old protected keys with source references and mark their new-pin observations unmeasured until the later campaign."

patterns-established:
  - "Authenticate exact ROM bytes, then derive every page and row identity from that image and verify an immutable inventory fingerprint."
  - "Treat result bytes as a pin-specific protocol keyed by decoded test name; never infer a new-pin result from an old-pin observation."

requirements-completed: []
coverage:
  - id: D1
    description: "The page 3 comparison tracer decodes compressed names, qualifies a real start, retains raw results and produces a bounded verdict."
    verification:
      - kind: integration
        ref: "accuracycoin.tracer; build/ci/tests/accuracycoin/page3-tracer/report.tsv, raw.tsv, metadata.txt"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset ci (654/654 tests and package step)"
        status: pass
    human_judgment: false
  - id: D2
    description: "Both pinned inventories and their malformed-input and status-classification controls are registered in normal CI."
    verification:
      - kind: unit
        ref: "accuracycoin.inventory, accuracycoin.classifier"
        status: pass
      - kind: unit
        ref: "accuracycoin.migration_source"
        status: pass
    human_judgment: false
  - id: D3
    description: "All 17 old protected observations have retained raw codes and one source-backed old/new disposition."
    verification:
      - kind: integration
        ref: "accuracycoin.page2, accuracycoin.page14, accuracycoin.page17 (old authenticated ROM)"
        status: pass
      - kind: unit
        ref: "accuracycoin.migration_source; tests/accuracy/migration.tsv"
        status: pass
    human_judgment: true
    rationale: "The migration test checks pin, key, symbol, disposition and source-field coverage; review of whether the cited assembly changes alter an assertion's meaning remains a source-review judgment."

metrics:
  duration: 54min
  completed: 2026-10-23
  status: complete
---

# Phase 17 Plan 01: Pinned AccuracyCoin comparison tracer and migration summary

**A pinned AccuracyCoin comparison now runs a real token-expanded page through the production observer, and preserves the old protected results with source-backed migration dispositions.**

## Performance

- **Duration:** about 54 min
- **Started:** 2026-10-23T21:08:39Z (phase-start STATE timestamp)
- **Completed:** 2026-10-23T22:03:07Z
- **Tasks:** 3
- **Files modified:** 15

## Accomplishments

- Added the MIT AccuracyCoin comparison ROM at pin `6bee919529289c555803b612b8c9e4f7eaaeeabb`, verified against SHA-256 `8ac8904a3cf26fbdfa6be501e414d45886ff4478d30dee8e58fa436cf5fd5adc`. Its decoded inventory contains 22 pages, 146 scored rows, five DRAW rows and fingerprint `7d022800d01ad81159109bf7457af94ab6f80e4a682fce716ec476f7c454a4a5`. The existing canonical ROM remains pin `673ef550db296136d52229961e7d39366116882a`, with 144 scored rows, five DRAW rows and fingerprint `3271803cdc63c4b4a83cf65453b817dc66ec65ed97863ad19261b56f36821c84`.
- Ran the new-pin page 3 tracer through the real runner. It decoded all seven SLO row names, observed the run state and a result transition, and recorded seven raw `$01` passes. The bounded attempt used 51 total frames and 13 execution frames, completed with `verdict=complete`, and reported 47,436,000 ns host elapsed time. This is a page tracer result, not a new-pin campaign or protected-key score.
- Added inventory mutations and classifier controls for exact expansion limits, malformed page/row fields, duplicate identities/addresses, DRAW sentinels, missing observations, skip/not-run/running/failure values, and the source-defined pass variants. `$05`, `$09`, `$0D`, `$11`, `$39` and `$41` are accepted only for source-defined keys; `$02` is a valid zero-error-index failure. DRAW rows are classified as `draw` and excluded from result-transition and completion checks.
- Re-ran old-pin page observations with a 1,800-frame attempt bound. Pages 2, 14 and 17 completed after 34, 203 and 249 total frames (10, 119 and 150 execution frames). The 17 protected rows all passed: 16 raw `$01` and PPU Read Buffer raw `$41`. The separate page 14 report also records APU Register Activation and Controller Strobing failures at `$12`, plus the source-defined Controller Clocking pass at `$09`; those three rows are not part of the protected snapshot.
- Added `old-673ef-scoreboard.txt`, measurement metadata, and 17 migration rows. The old inventory size is recorded separately from a full score. Every new-pin observation in the migration table remains `unmeasured`.

## Task Commits

1. **Task 1: Wire a real compressed page through the complete observation path** — `9ee6797` (`feat`)
2. **Task 2: Reject malformed inventories and protocol results through production helpers** — `29742d2` (`fix`)
3. **Task 3: Retain all old protected measurements and source-backed migration meaning** — `125d204` (`fix`)

## Files Created/Modified

- `runner/accuracycoin.c`, `runner/accuracycoin.h`, `runner/main.c`, `runner/CMakeLists.txt` — private inventory decoder, key-scoped result protocol, observer and CLI wiring.
- `tests/roms/accuracycoin-6bee.nes`, `tests/roms/manifest.txt`, `tests/cmake/hash_keys.cmake` — admitted exact comparison fixture and narrow conformance hash exemption.
- `tests/cmake/accuracycoin_tracer.cmake`, `tests/accuracy/test_conformance.c`, `tests/CMakeLists.txt` — production tracer, inventory, classifier and migration-source controls.
- `tests/accuracy/old-673ef-scoreboard.txt`, `tests/accuracy/old-673ef-metadata.txt`, `tests/accuracy/migration.tsv` — retained old observations and per-key source dispositions.
- `README.md`, `include/nesturbator.h` — observer, protocol and comparison contract documentation.

## Decisions Made

- Keep the old pin and strict protected checks active until the later activation plan; the new ROM remains comparison-only here.
- Preserve source order and decoded key identity from the ROM instead of adding a second hand-maintained test table.
- Scope exceptional pass bytes to exact keys from the two MIT assembly pins; old raw bytes do not prove new-pin behavior.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Added the CMake version declaration to the tracer script**
- **Found during:** Task 1
- **Issue:** The first full workflow rejected the new script under `cmake.script_policy` because it lacked `cmake_minimum_required(VERSION 3.25)`.
- **Fix:** Added the required declaration; the focused policy/tracer checks and a full 651/651 workflow then passed.
- **Files modified:** `tests/cmake/accuracycoin_tracer.cmake`
- **Verification:** `cmake --workflow --preset ci` passed after the fix.
- **Committed in:** `9ee6797`.

**2. [Rule 1 - Bug] Corrected source-defined AccuracyCoin result classification**
- **Found during:** Task 3
- **Issue:** The classifier rejected valid `$02` failure results and source-defined pass bytes such as Controller Clocking `$09`; its existing PPU special case also omitted Address `$2004` behavior.
- **Fix:** Added exact decoded-key allowlists from the pinned assembly, made low-bit `10` values failures including `$02`, and tested every pinned exceptional pass byte/key pair.
- **Files modified:** `runner/accuracycoin.c`, `runner/accuracycoin.h`, `tests/accuracy/test_conformance.c`, `README.md`, `include/nesturbator.h`.
- **Verification:** Focused classifier/tracer tests passed; the old page 14 observer retained the `$09` result as a source-defined pass.
- **Committed in:** `125d204`.

**3. [Rule 1 - Bug] Excluded DRAW sentinel bytes from scored observations**
- **Found during:** Task 3
- **Issue:** The observer previously treated the shared `$03FF` DRAW value as a regular status byte and counted it for transitions/completion.
- **Fix:** Added row-aware `draw` classification and ignored DRAW rows in result-transition and run-completion checks.
- **Files modified:** `runner/accuracycoin.c`, `runner/accuracycoin.h`, `tests/accuracy/test_conformance.c`, `README.md`.
- **Verification:** Classifier unit control and the final full workflow passed.
- **Committed in:** `125d204`.

**Total deviations:** 3 auto-fixed (2 Rule 1 bugs, 1 Rule 3 blocking build issue).
**Impact on plan:** These fixes make the observed protocol match both exact pinned assemblies and keep sentinel data outside scored evidence; no scope beyond plan correctness was added.

## Issues Encountered

- The first page 14 observer run stopped at Controller Clocking raw `$09`; source inspection established that byte as a valid pin-specific success branch, and the corrected run produced a complete report. Both other unprotected page 14 rows remain observed failures at `$12`.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

The comparison fixture, page 3 tracer, complete inventory controls and old protected migration evidence are ready for Plan 17-02. The new-pin outcomes for all 17 old protected keys remain explicitly unmeasured, the full campaign is not complete, and canonical activation remains deferred to Plan 17-04. Keep all phase-level CONF requirements pending until the whole phase is verified.

## Self-Check: PASSED

- The new summary path exists.
- Task commits `9ee6797`, `29742d2` and `125d204` are ancestors of the current plan head.
- Task commit count measured from the ledger is 3; plan head before the task commits is `211a3f66db5d3131dcbd6816bb8d805114165e34`.

---
*Phase: 17-complete-pinned-conformance-baseline*
*Completed: 2026-10-23*
