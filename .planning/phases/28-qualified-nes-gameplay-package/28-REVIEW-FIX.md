---
phase: 28
fixed_at: 2026-11-03T19:30:56Z
review_path: .planning/phases/28-qualified-nes-gameplay-package/28-REVIEW.md
iteration: 7
findings_in_scope: 2
fixed: 2
skipped: 0
status: all_fixed
---

# Phase 28: Code Review Fix Report

**Fixed at:** 2026-11-03T19:30:56Z
**Source review:** `.planning/phases/28-qualified-nes-gameplay-package/28-REVIEW.md`
**Iteration:** 7

**Summary:**
- Findings in scope: 2
- Fixed: 2
- Skipped: 0

## Fixed Issues

### Iteration 1

### CR-01: Windows moved-prefix test rejects the normal library filename

**Files modified:** `tests/cmake/qualified_consumer.cmake`
**Commit:** `a701c89`
**Applied fix:** The moved-prefix check validates the imported library path recorded by the consumer and requires that exact path in generated link evidence, without assuming a Unix `lib` prefix.
**Status:** fixed

### CR-02: Profile self-test can recursively delete an arbitrary path

**Files modified:** `tests/CMakeLists.txt`, `tests/cmake/provider_profile.cmake`
**Commit:** `ab0f77d`
**Applied fix:** The profile controls use isolated owned scratch and validate its configured build root before cleanup.
**Status:** fixed

### WR-01: Pixel-difference evidence compares frame 80 with frame 110, not frame 95

**Files modified:** `tests/consumer/main.c`
**Commit:** `da49b56`
**Applied fix:** The consumer snapshots the frame-95 pixels and uses that snapshot for the documented ROI comparison and after-frame evidence.
**Status:** fixed

### WR-02: Provider JSON checks accept values with the wrong JSON type

**Files modified:** `tests/cmake/provider_profile.cmake`, `tests/cmake/provider_receipt.cmake`
**Commit:** `db0b133`
**Applied fix:** Provider profile and receipt checks validate JSON types before comparing values and include wrong-type mutations.
**Status:** fixed

### WR-03: Consumer trusts the library's audio count as an array bound

**Files modified:** `tests/consumer/main.c`
**Commit:** `46f0cd5`
**Applied fix:** The consumer rejects an audio count larger than the supplied buffer capacity before any comparison or sample access.
**Status:** fixed

### Iteration 2

### WR-01: Collection validators accept JSON objects in place of arrays

**Files modified:** `tests/cmake/provider_profile.cmake`, `tests/cmake/provider_receipt.cmake`
**Commit:** `6dd8342`
**Applied fix:** Profile and receipt collection fields now require their documented JSON array types; numeric-object controls reject malformed collection shapes.
**Status:** fixed

### WR-02: Published receipt accepts a dirty source tree with untracked files

**Files modified:** `tests/cmake/provider_receipt.cmake`
**Commit:** `49d3ba5`
**Applied fix:** Published-source cleanliness validation includes untracked files and uses an isolated repository control to prove that dirty source is rejected.
**Status:** fixed

### Iteration 3

### WR-01: Profile contract does not pin gameplay recipe values

**Files modified:** `tests/cmake/provider_profile.cmake`
**Commit:** `8febaa5`
**Applied fix:** The profile checker requires the exact gameplay recipe arrays and the mutation controls reject changed recipes.
**Status:** fixed

### WR-02: 32-bit x86 hosts are classified as x64

**Files modified:** `tests/cmake/provider_receipt.cmake`
**Commit:** `555c52a`
**Applied fix:** Target detection now rejects i386/i686 architecture names as x64, with an explicit architecture control.
**Status:** fixed

### Iteration 4

### CR-01: Receipt self-test can recursively delete a caller-selected directory

**Files modified:** `tests/cmake/provider_receipt.cmake`, `tests/CMakeLists.txt`
**Commits:** `55b8c00`, `38417de`
**Applied fix:** Self-test `WORK` must resolve beneath a configured build directory whose CMake cache identifies this source checkout. It creates a unique child with a per-run marker, verifies the canonical child path and marker before removing only that child, and leaves the caller's marked root untouched. The control confirms a caller marker and unrelated file survive cleanup.
**Status:** fixed; requires human verification of the path and cleanup logic.

### WR-01: Published receipt does not bind candidate CI to the tagged source

**Files modified:** `tests/cmake/provider_receipt.cmake`
**Commit:** `19a7669`
**Applied fix:** Published mode now runs local `git merge-base --is-ancestor` for the accepted candidate SHA and exact resolved release-tag source SHA before querying candidate CI. A failed or unprovable ancestry check fails closed with `candidate-source-mismatch`. The offline control builds two unrelated local root commits and verifies rejection of the valid-format candidate SHA.
**Status:** fixed; requires human verification of the ancestry gate.

### Iteration 5

### WR-01: Provider profile does not validate the declared A and Right masks

**Files modified:** `tests/cmake/provider_profile.cmake`
**Commits:** `7af4ad8`, `4c8cbcc`
**Applied fix:** Contract mode now requires the scenario A and Right masks to be numeric and equal to `input.button_bits.a` and `.right`, and compares the runtime report's port masks with those scenario fields. The profile self-test mutates each scenario mask independently and requires its exact field-specific mismatch. The runtime report's numeric `region` field is now classified correctly so the actual-report check reaches these mask comparisons.
**Status:** fixed

### Iteration 6

### WR-01: Provider profile does not pin the neutral second-port mask

**Files modified:** `tests/cmake/provider_profile.cmake`

**Commit:** `e9705ae`
**Applied fix:** The second-port mask is now classified as numeric and must equal zero in contract mode. Runtime evidence is compared with that profile value. Self-test controls require rejection of both a nonzero value and a string-typed value.
**Status:** fixed

### WR-02: Provider profile does not validate its advertised positive-predicate list

**Files modified:** `tests/cmake/provider_profile.cmake`

**Commit:** `9a2f1a3`
**Applied fix:** Contract mode now requires the profile predicate list to be an array of exactly the four expected consumer predicates, with every entry a string in the expected order. Controls reject an object in place of the array, a non-string item, and a changed predicate with a field-specific diagnostic.
**Status:** fixed

### IN-01: Missing-attestation errors can name the previous archive

**Files modified:** `tests/cmake/provider_receipt.cmake`

**Commit:** `efa65d2`
**Applied fix:** The archive's name is read after its JSON string type is validated and before the missing-attestation branch. The receipt control now checks that the diagnostic names the current archive from the fixture.
**Status:** fixed

### Iteration 7

### WR-01: Provider profile does not validate its public region enum names

**Files modified:** `tests/cmake/provider_profile.cmake`

**Commit:** `9e5e962`
**Applied fix:** The contract checker now compares both advertised region names with the identifiers extracted from the corresponding public-header enum declarations, and keeps the numeric-value checks. Mutation controls reject wrong names and non-string values for both names.
**Status:** fixed

### WR-02: Provider profile leaves most qualification limits unchecked

**Files modified:** `tests/cmake/provider_profile.cmake`

**Commit:** `7682882`
**Applied fix:** The checker now validates exact string types and values for all five qualification-limit statements and requires the nine-item `unqualified` array to match the documented strings in order. Mutation controls cover changed and missing claims, wrong types, changed/missing array members, and invalid array types.
**Status:** fixed

## Verification (Iteration 5)

**Verification ran in:** the main checkout (`workflow.use_worktrees=false`).

- `ctest --test-dir build/ci -R '^(provider\.profile|provider\.profile\.controls|provider\.receipt\.controls)$' --output-on-failure --no-tests=error` — passed all three checks, including both independent mask mismatch controls and receipt controls.
- `ctest --test-dir build/ci -R '^install\.gameplay$' --output-on-failure --no-tests=error` — passed and generated a fresh moved-prefix gameplay report.
- `cmake -DSOURCE_DIR=. -DMODE=contract -DRUNTIME_REPORT=build/ci/qualified-consumer-3eee3f759456/gameplay.json -P tests/cmake/provider_profile.cmake` — passed against the actual consumer report, including its runtime input masks.
- `git diff --check` — passed before the source fix commits.
- The full workflow and separate AccuracyCoin campaign were not run. Final acceptance still needs to be refrozen and receive one bounded full workflow after the final review is clean.

## Verification (Iteration 6)

**Verification ran in:** the main checkout (`workflow.use_worktrees=false`).

- `ctest --test-dir build/ci -R '^provider\.(profile|profile\.controls|receipt\.controls)$' --output-on-failure --no-tests=error` — passed all three focused provider checks, including the new mask, predicate-array, and archive-name controls.
- `git diff --check` — passed for each source fix before its commit.
- The full `cmake --workflow --preset ci` workflow and a separate AccuracyCoin campaign were not run. Final acceptance still needs to be refrozen and receive one bounded full workflow after the final review is clean.

## Verification (Iteration 7)

**Verification ran in:** the main checkout (`workflow.use_worktrees=false`).

- `ctest --test-dir build/ci -R '^provider\.(profile|profile\.controls|receipt\.controls)$' --output-on-failure --no-tests=error` — passed all three focused provider contract and receipt controls, including the new region-name and qualification-boundary mutations.
- `git diff --check` — passed after the source fix commits.
- The full workflow and separate AccuracyCoin campaign were not run. Final acceptance still needs to be refrozen and receive one bounded full workflow after the final review is clean.

Iterations 1–6 remain committed and unchanged. This report records iteration 7 and is intentionally uncommitted for the orchestrator.

---

_Fixed: 2026-11-03T19:30:56Z_
_Fixer: the agent (gsd-code-fixer)_
_Iteration: 7_
