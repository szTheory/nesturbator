---
phase: 02-the-cpu-matches-the-public-vectors
plan: 03
subsystem: testing
tags: [c17, cmake, ctest, n65v, 65x02-vectors, manifest, provenance]

requires:
  - phase: 02-the-cpu-matches-the-public-vectors
    provides: "02-01: vecconv and the N65V reader; 02-02: vectors_fixture.cmake with RUN=OFF"
provides:
  - "tests/vectors/65x02-sample.n65v: 256 chunks x 100 tests, 1,678,114 B, sha256 0c318cec...e109, in the manifest"
  - "tests/cmake/vectors_regen.cmake (COMMIT VECCONV WORK OUT): rebuilds the sample from 256 range prefixes and prints the manifest line"
  - "tests/cmake/manifest_sha256.cmake (SOURCE_DIR | SELFTEST WORK): ctest manifest.sha256 and manifest.sha256.selftest"
  - "tests/vectors/test_n65v.c: ctest vectors.n65v (sample) and vectors.n65v.crafted (no argument)"
  - "tests/cmake/vecconv_negative.cmake (VECCONV FIXTURE CASE WORK FIRST): 13 vecconv.reject.* cases and vecconv.a9_tail.cut"
  - "fixtures 02-first3.json and a9-tail.json; ctest vecconv.02 and vecconv.a9_tail (convert only)"
  - ".gitattributes: *.n65v binary, tests/vectors/fixtures/*.json text eol=lf"
  - "THIRD-PARTY-NOTICES.md and PROVENANCE.md entries for the 65x02 data"
affects: [02-04, 02-05, 02-06, 02-07, 02-08]

actuals:
  tokens: 9500
  tasks: 3
  commits: 3
plan_head_before: 840e1ace31d849818755a5b4654aefd42005aff1
plan_head_after: 43c19148638075eb054517a8bc9cb4508ed1f04c

tech-stack:
  added: []
  patterns:
    - "Manifest-listed data is hash-checked by ctest manifest.sha256, so a regenerated file with a stale line fails"
    - "A negative-input test builds its malformed file from a committed fixture at test time and passes on the script's exit status alone (no regex property, no WILL_FAIL)"
    - "Crafted-buffer tests record each byte's field start while encoding, so every truncation's expected offset is exact"

key-files:
  created:
    - tests/vectors/65x02-sample.n65v
    - tests/cmake/vectors_regen.cmake
    - tests/cmake/manifest_sha256.cmake
    - tests/cmake/vecconv_negative.cmake
    - tests/vectors/test_n65v.c
    - tests/vectors/fixtures/02-first3.json
    - tests/vectors/fixtures/a9-tail.json
  modified:
    - tests/roms/manifest.txt
    - .gitattributes
    - tests/CMakeLists.txt
    - tests/vectors/fixtures/README.md
    - THIRD-PARTY-NOTICES.md
    - PROVENANCE.md
    - README.md

key-decisions:
  - "The sample is 1,678,114 B, not D-04's 1,677,602 B: the D-03 chunk header carries a u16 count, 2 B more per chunk than D-04 counted (02-RESEARCH correction 2); vecconv's output equals the research's independent encoder byte for byte"
  - "manifest.sha256 accepts a pin that is 40 lowercase hex digits or a release tag of the form v1.2(.3)"
  - "vectors.n65v.crafted checks the exact byte offset of every error, not only that the reader fails"
  - "Each task's README text lands in that task's commit (CLAUDE.md rule 6) rather than all in Task 3"

patterns-established:
  - "vecconv_negative.cmake CASE names: float sign exponent addr_range byte_range kind unknown_key missing_key count empty_file empty_array closed_short truncated as_is"

requirements-completed: [CPU-01]

coverage:
  - id: D1
    description: "The committed sample equals an independent encoding, is listed in the manifest, and reads end to end through the shared reader"
    requirement: CPU-01
    verification:
      - kind: other
        ref: "cmake -E sha256sum tests/vectors/65x02-sample.n65v (0c318cec...e109, 1678114 B)"
        status: pass
      - kind: integration
        ref: "ctest --preset ci -R '^vectors\\.n65v$'"
        status: pass
      - kind: integration
        ref: "sh scripts/hygiene.sh --tree; cmake --workflow --preset hygiene (6 of 6)"
        status: pass
    human_judgment: false
  - id: D2
    description: "Every manifest file's SHA-256 is checked, and the check reports a wrong hash"
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^manifest\\.sha256(\\.selftest)?$'"
        status: pass
    human_judgment: false
  - id: D3
    description: "The reader rejects every truncation and each D-06 malformed buffer at the right byte offset"
    requirement: CPU-01
    verification:
      - kind: unit
        ref: "ctest --preset ci -R '^vectors\\.n65v\\.crafted$'"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset asan (56 of 56)"
        status: pass
    human_judgment: false
  - id: D4
    description: "vecconv converts the compact layout and a cut prefix, and exits 1 with an offset on all 13 D-02 malformed inputs and on a prefix too short for --first 4"
    requirement: CPU-01
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^(vecconv\\.|vectors\\.n65v)' (19 of 19)"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset asan (56 of 56)"
        status: pass
    human_judgment: false
  - id: D5
    description: "The 65x02 data carries its MIT notice and provenance, and the README explains the data and regeneration"
    verification:
      - kind: other
        ref: "grep -c 'Thomas Harte' THIRD-PARTY-NOTICES.md; grep -c '65x02-sample.n65v' PROVENANCE.md; grep -c 'vectors_regen.cmake' README.md"
        status: pass
    human_judgment: false

duration: 9min
completed: 2026-10-03
status: complete
---

# Phase 2 Plan 03: Committed vector sample with provenance and fail-closed parsers Summary

**`tests/vectors/65x02-sample.n65v` holds the first 100 upstream tests of every opcode: 1,678,114 B, byte for byte the research's independent encoding. It is regenerable from 256 HTTP range prefixes, listed in the manifest, hash-checked by ctest, and read end to end by the shared reader. Thirteen malformed JSON inputs, the crafted N65V buffers, and every truncation are rejected with a byte offset.**

## Performance

- **Duration:** about 9 min
- **Started:** 2026-10-03T15:21:38Z
- **Completed:** 2026-10-03T15:30:20Z
- **Tasks:** 3
- **Files modified:** 14

## Accomplishments
- `vectors_regen.cmake` requires a 40-hex-digit COMMIT. It downloads bytes 0-65535 of each `nes6502/v1/<xx>.json` with `TLS_VERIFY ON`, runs `vecconv --first 100` on each, and joins the 256 chunks with `cmake -E cat`. It then prints the size, the SHA-256 and the tab-separated manifest line. Run at pin `2f6980a2`, it took about 58 s. The output was 1,678,114 B with SHA-256 `0c318cec0bd4031396964ca9a0de0daf25c2651c28f34a0d0616a9294430e109`, and the first 12 bytes were `4e 36 35 56 01 00 64 00 00 00 81 e8`. All three match 02-RESEARCH.
- `manifest.sha256` checks each manifest line. It must have 5 fields, a commit or release-tag pin, a licence, and a 64-hex-digit sha256, and the file it names must exist and have that hash. Every failure names the path. A scratch manifest also confirmed the field-count, pin, licence and missing-file errors. `manifest.sha256.selftest` passes only on the exact wrong-hash report.
- `vectors.n65v` reads the sample as 256 x 100 tests. It requires opcode k in chunk k, indexes 0-99, and an end at the last byte. A copy cut short fails at `offset 1677999`, and one with a byte appended fails with `1 trailing bytes`.
- `vectors.n65v.crafted` encodes a 66-byte, two-test buffer. It checks the error offset for each truncation from 0 to 65 bytes, and for bad magic (0), version 2 (4), kind 2 (36), the wrong opcode (5), a header count of 3 over 2 tests (66), a short count (6), a repeated index (37) and a trailing byte (66).
- `vecconv_negative.cmake` and 13 `vecconv.reject.*` tests: each edits `a9-first3.json` into one malformed input. Each test passes only if vecconv exits 1 with `offset [0-9]+`. The script fails if an edit leaves the text unchanged.
- `vecconv.02` converts the compact `[{` layout. `vecconv.a9_tail` converts three tests from a prefix cut inside the fourth. `vecconv.a9_tail.cut` asks the same prefix for four and passes on vecconv's exit 1 at offset 1084.

## Task Commits

1. **Task 1: Range prefixes to the committed blob, its manifest line, and a reader pass over it** - `173afe6` (feat)
2. **Task 2: Converter fixtures and rejections, reader crafted buffers** - `1c464e6` (test)
3. **Task 3: Licence notice and provenance** - `43c1914` (docs)

## Files Created/Modified
- `tests/vectors/65x02-sample.n65v`: the 25,600-test sample (binary, in the manifest)
- `tests/cmake/vectors_regen.cmake`: pin-bump regeneration (D-07)
- `tests/cmake/manifest_sha256.cmake`: the manifest check and its self-test
- `tests/cmake/vecconv_negative.cmake`: builds one malformed case and requires exit 1 with an offset
- `tests/vectors/test_n65v.c`: the sample pass (one argument) and the crafted buffers (no argument)
- `tests/vectors/fixtures/02-first3.json`, `a9-tail.json`: bytes 0-1591 of `02.json` and 0-1083 of `a9.json` at the pin
- `tests/vectors/fixtures/README.md`: attribution rows for the new fixtures
- `tests/roms/manifest.txt`: lines for the sample and the two fixtures
- `.gitattributes`: `*.n65v binary`, fixtures `text eol=lf`
- `tests/CMakeLists.txt`: the 21 new tests
- `THIRD-PARTY-NOTICES.md`, `PROVENANCE.md`: the upstream LICENSE verbatim, the files, pin and SHA-256
- `README.md`: Checks text for every new test, the test data in "ROMs", and "Regenerating the vector sample"

## Decisions Made
- The crafted-buffer test checks the exact byte offset of each error, not only that -1 comes back. An off-by-one in the reader's error offset fails it (shown by a mutation).
- `a9-tail.json` is cut at byte 1083 as planned. Byte 1083 lies inside the fourth object's `initial` state, which starts near byte 987, so no other cut was needed.

## Deviations from Plan

### Auto-fixed Issues

**1. [Locked-wording departure, as the plan directs] D-04's 1,677,602 B becomes 1,678,114 B**
- **Found during:** Task 1
- **Issue:** D-04 gives the sample as 1,677,602 B. In the D-03 layout every chunk header holds a u16 count, which adds 2 B per chunk (256 x 2 = 512 B; 02-RESEARCH correction 2).
- **Fix:** None needed. vecconv's output is 1,678,114 B with SHA-256 `0c318cec...e109`, identical to the research's separate scratch encoder, so neither side is wrong. **Owner: this departs from D-04's locked number.** Plan 07 corrects the CONFORMANCE figures.
- **Committed in:** 173afe6

**2. [Rule 2 - Docs together] README text for each task's tests went into that task's commit**
- **Found during:** Tasks 1 and 2
- **Issue:** The plan puts all README changes in Task 3. CLAUDE.md rule 6 requires the README to change in the same change as the tests.
- **Fix:** Task 1's commit describes `vectors.n65v` and `manifest.sha256`. Task 2's commit describes `vectors.n65v.crafted` and the `vecconv.*` tests. Task 3 added the ROMs text and the regeneration paragraph.
- **Committed in:** 173afe6, 1c464e6, 43c1914

**3. [No-op] The "Phase 1 has no test ROM" sentence was already gone**
- **Found during:** Task 1, step 3
- **Issue:** Plan 01 had already removed that sentence from `tests/roms/manifest.txt`, in fef3fda.
- **Fix:** None. The manifest line was added.

---

**Total deviations:** 3 (1 locked-wording departure the plan asked to record, 1 documentation timing, 1 no-op)
**Impact on plan:** None on scope.

## TDD Gate Compliance

Task 2 is marked `tdd="true"`, but the reader and the converter it tests shipped in plan 01, so every new test passed as soon as it was written and no real RED state existed. To show the tests can fail, they were run against mutated scratch copies, none of them committed:
- `n65v.c` with the trailing-byte check off: `one trailing byte: rc 0`.
- `n65v.c` allowing a repeated index: `index 0 twice: rc 0`.
- `n65v.c` accepting kind 2: `kind 2: rc 0`.
- `n65v.c` with the version check off: `version 2: rc 0`.
- `n65v.c` reporting a truncation one byte late: `truncated to 4: ... "offset 5: truncated version", expected "offset 4"`.
- `vecconv.c` with FULL_COUNT 3: `vecconv.reject.closed_short` failed with `vecconv exited 0, expected 1`.

Removing vecconv's fraction check, or widening its 255-cycle limit, did not make the `float` and `count` cases pass. The parser still rejected those inputs at a later byte with an offset, which is the fail-closed behaviour D-02 requires, and the tests check exactly that contract (exit 1 with an offset).

The task has a `test(02-03)` commit and no `feat` commit, because the implementation is plan 01's. `workflow.tdd_mode` is off, so this is recorded here and was not enforced.

## Issues Encountered
None.

## User Setup Required
None.

## Next Phase Readiness
- Plans 04 onward can run the committed sample through the CPU with `cpu.vectors tests/vectors/65x02-sample.n65v <xx> 256 100`.
- `vecconv.02` converts only. Plan 06 adds JAM through the CPU and can switch it to `RUN=ON`.
- Plan 07 corrects CONFORMANCE's size and bound figures (1,678,114 B; 54,565 B).

---
*Phase: 02-the-cpu-matches-the-public-vectors*
*Completed: 2026-10-03*

## Self-Check: PASSED
