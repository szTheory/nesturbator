---
phase: 02-the-cpu-matches-the-public-vectors
plan: 01
subsystem: testing
tags: [c17, cmake, ctest, n65v, 65x02-vectors, release-please]

requires:
  - phase: 01-a-test-frame-in-retroarch
    provides: CMake presets, nesturbator_warnings, palgen host-tool pattern, embed.subdirectory, hygiene lane, release-please config
provides:
  - "vecconv host tool: upstream 65x02 JSON to N65V v1 with a schema tokenizer and a re-read check"
  - "tests/vectors/n65v.{c,h}: bounds-checked N65V reader (n65v_init, n65v_next) and the four structs"
  - "tests/vectors/fixtures/a9-first3.json with MIT notice and manifest line"
  - "ctest vecconv.a9 and release.no_release_as"
  - "release-please-config.json without release-as"
affects: [02-02, 02-03, 02-04, 02-05, 02-06, 02-07, 02-08, releases]

actuals:
  tokens: 10082
  tasks: 3
  commits: 3
plan_head_before: f23cca45885f1758601500b57cc7af2bece38e85
plan_head_after: 30197fe

tech-stack:
  added: []
  patterns:
    - "Host tool built from tools/<name>/ plus test sources, below the top-level return, on the embed forbidden list"
    - "Reader bounds check n > len - off with a sticky error that starts 'offset <n>:'"
    - "Converter proves itself by re-reading its output from disk and re-encoding it byte for byte"

key-files:
  created:
    - tools/vecconv/vecconv.c
    - tools/vecconv/CMakeLists.txt
    - tests/vectors/n65v.h
    - tests/vectors/n65v.c
    - tests/vectors/fixtures/a9-first3.json
    - tests/vectors/fixtures/README.md
    - tests/cmake/release_config.cmake
  modified:
    - CMakeLists.txt
    - tests/CMakeLists.txt
    - tests/embed/CMakeLists.txt
    - tests/roms/manifest.txt
    - release-please-config.json
    - README.md

key-decisions:
  - "JSON fixture attribution lives in tests/vectors/fixtures/README.md with the upstream MIT LICENSE verbatim, not in a comment (corrects D-06 wording; JSON has no comments)"
  - "vecconv requires a two-hex-digit opcode and --first 1..10000; a prefix that closes the array before N objects is an error"
  - "The re-read reads the written file back from disk and re-encodes every decoded test, so a writer/reader mismatch or a short write fails"
  - "Without release-as, the next feat: release is 0.1.1 (bump-patch-for-minor-pre-major), per 02-RESEARCH Open Question 1"

patterns-established:
  - "N65V errors name a byte offset; vecconv errors are 'vecconv: <in>: offset <n>: <reason>' and exit 1, usage errors exit 2"

requirements-completed: [CPU-01]

coverage:
  - id: D1
    description: "a9 first three tests convert from upstream JSON to N65V and read back through the shared reader"
    requirement: CPU-01
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^vecconv\\.a9$' -V (prints 'vecconv: a9: 3 tests written and re-read')"
        status: pass
      - kind: integration
        ref: "cmake --workflow --preset asan (vecconv.a9 under ASan and UBSan)"
        status: pass
    human_judgment: false
  - id: D2
    description: "vecconv is never built by an embedding project"
    verification:
      - kind: integration
        ref: "ctest --preset ci -R '^embed\\.subdirectory$'"
        status: pass
    human_judgment: false
  - id: D3
    description: "release-as pin removed and guarded"
    verification:
      - kind: unit
        ref: "jq -e '.packages[\".\"] | has(\"release-as\") | not' release-please-config.json"
        status: pass
      - kind: unit
        ref: "ctest --preset ci -R '^release\\.no_release_as$'; scratch copy with release-as exits 1"
        status: pass
    human_judgment: false
  - id: D4
    description: "Fixture accepted by the hygiene lane and existing frame hashes unchanged"
    verification:
      - kind: integration
        ref: "cmake --workflow --preset hygiene"
        status: pass
      - kind: integration
        ref: "ctest --preset ci -R 'runner\\.hash|runner\\.write_hashes\\.content'"
        status: pass
    human_judgment: false

duration: 8min
completed: 2026-10-03
status: complete
---

# Phase 2 Plan 01: Vector conversion tracer Summary

**`vecconv` turns upstream 65x02 JSON into N65V v1 using a schema-only tokenizer. A bounds-checked reader, shared with the tests, reads the output back. The `$A9` fixture passes end to end, and the `release-as` pin is gone with a test that keeps it out.**

## Performance

- **Duration:** about 8 min
- **Started:** 2026-10-03T14:57:02Z
- **Completed:** 2026-10-03T15:04:55Z
- **Tasks:** 3
- **Files modified:** 13

## Accomplishments
- `tools/vecconv/vecconv.c` reads upstream JSON. It accepts any JSON whitespace and matches keys by name. Numbers must be unsigned decimal; ranges are checked; lists may hold at most 255 entries; kinds must be `read` or `write`; required keys must be present and may not repeat; the first cycle must be the opcode fetch. Every error gives a byte offset and exits 1. It writes one N65V chunk, then reads the file back from disk through `n65v_next` and re-encodes it to the same bytes.
- `tests/vectors/n65v.{c,h}` is the reader. It decodes byte by byte and checks `n > len - off` before each field. It checks the chunk magic, version, opcode, count, that indexes strictly increase, the kind byte, and trailing bytes. Errors are sticky and start with `offset <n>:`.
- `vecconv.a9` converts bytes 0-983 of `a9.json` at the pin, which hold three tests, and prints `vecconv: a9: 3 tests written and re-read`.
- `vecconv` is on the forbidden-target list in `tests/embed`, so an embedding project never gets it. The README describes the test.
- `release-please-config.json` no longer has `release-as`. `release.no_release_as` fails if the key comes back, and the README "Releases" section explains how the version is chosen.

## Task Commits

1. **Task 1: a9 first three tests through vecconv and back through the reader** - `fef3fda` (feat)
2. **Task 2: Keep vecconv out of the embedded package and describe it** - `0f5e20e` (test)
3. **Task 3: Remove the release-as pin and keep it gone** - `30197fe` (fix)

## Files Created/Modified
- `tools/vecconv/vecconv.c`: the JSON to N65V converter and its re-read check
- `tools/vecconv/CMakeLists.txt`: host-tool target with warnings and MSVC `_CRT_SECURE_NO_WARNINGS`; not installed
- `tests/vectors/n65v.h`, `tests/vectors/n65v.c`: the N65V v1 layout and its reader
- `tests/vectors/fixtures/a9-first3.json`: the first 984 bytes of upstream `a9.json`
- `tests/vectors/fixtures/README.md`: source, pin, byte range and the upstream MIT LICENSE, verbatim
- `tests/roms/manifest.txt`: tab-separated line for the fixture (sha256 `3195bc1b…d83b`)
- `tests/cmake/release_config.cmake`: the `release.no_release_as` check
- `CMakeLists.txt`: `add_subdirectory(tools/vecconv)` below the top-level return, and comments updated
- `tests/CMakeLists.txt`: `vecconv.a9` and `release.no_release_as`
- `tests/embed/CMakeLists.txt`: `vecconv` added to the forbidden targets
- `release-please-config.json`: `release-as` removed (jq reformatted `extra-files` to one key per line)
- `README.md`: the Checks, Using the library and Releases sections

## Decisions Made
- The re-read goes through the file on disk rather than the in-memory buffer, so a short or failed write also fails.
- The opcode argument must be exactly two hex digits, so the printed `<xx>` always matches the upstream file name.
- With `--first N`, an array that closes before N objects is an error, not a shorter conversion.

## Deviations from Plan

### Auto-fixed Issues

**1. [D-06 correction, as the plan directs] Fixture attribution in a README instead of a JSON comment**
- **Found during:** Task 1
- **Issue:** D-06 says each fixture carries an MIT attribution comment. JSON cannot hold a comment, and the D-02 tokenizer rejects anything unknown (02-RESEARCH Pitfall 8).
- **Fix:** `tests/vectors/fixtures/README.md` gives the source URL, pin, path and byte range, plus the upstream `LICENSE` verbatim (fetched at the pin), because MIT requires the permission notice to travel with copies. The fixture also has its manifest line. **Owner: this changes D-06's wording.**
- **Files modified:** tests/vectors/fixtures/README.md
- **Committed in:** fef3fda

**2. [Rule 2 - Docs together] README and CMake comments that list the top-level-only tools now name vecconv**
- **Found during:** Tasks 1 and 2
- **Issue:** Comments in `CMakeLists.txt` and the README's "Using the library" and `embed.subdirectory` text listed `palgen` as the only top-level host tool. Left alone, they would have been wrong once `vecconv` was added.
- **Fix:** Added `vecconv` to those lists (CLAUDE.md rule 6).
- **Committed in:** fef3fda, 0f5e20e

---

**Total deviations:** 2 (1 D-06 correction the plan asked to record, 1 documentation consistency)
**Impact on plan:** None on scope.

## Issues Encountered
None. Besides the committed test, a scratch program (not committed) checked the reader on the real output. Truncated at every byte from 0 to 143, or given one trailing byte, it returned -1 with an `offset` message every time. It also rejected version 2, a wrong opcode and a wrong count, and ran clean under ASan and UBSan. vecconv exited 1 with an offset on each D-02 negative input: a sign, a fraction, an exponent, 65536, 256, more than 255 RAM entries, an unknown kind, an unknown key, a missing key, a repeated key, a truncated prefix and an empty file. Plan 03 commits fixtures for these cases.

## User Setup Required
None.

## Next Phase Readiness
- Plan 02 can run `build/ci/tests/vectors/a9-first3.n65v` through the CPU object by extending `vecconv.a9`.
- The CONFORMANCE corrections D-02 calls for (the "one per line" wording and the 54,433 bound) are outside this plan's files and remain for a later plan.

---
*Phase: 02-the-cpu-matches-the-public-vectors*
*Completed: 2026-10-03*

## Self-Check: PASSED
