---
phase: 02-the-cpu-matches-the-public-vectors
verified: 2026-10-03T16:40:00Z
status: human_needed
score: 50/53 must-haves verified (3 post-merge backstops, insufficient_spec until merge)
covered_files:
  - .gitattributes
  - .github/workflows/ci.yml
  - .github/workflows/nightly.yml
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-01-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-01-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-02-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-02-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-03-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-03-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-04-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-04-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-05-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-05-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-06-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-06-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-07-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-07-SUMMARY.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-08-PLAN.md
  - .planning/phases/02-the-cpu-matches-the-public-vectors/02-08-SUMMARY.md
  - CMakeLists.txt
  - CMakePresets.json
  - PROVENANCE.md
  - README.md
  - THIRD-PARTY-NOTICES.md
  - include/nesturbator.h
  - release-please-config.json
  - src/bus.c
  - src/cpu.c
  - src/instance.c
  - src/internal.h
  - tests/CMakeLists.txt
  - tests/cmake/fetch_vectors.cmake
  - tests/cmake/manifest_sha256.cmake
  - tests/cmake/pins_check.cmake
  - tests/cmake/release_config.cmake
  - tests/cmake/vecconv_negative.cmake
  - tests/cmake/vectors_fixture.cmake
  - tests/cmake/vectors_full_run.cmake
  - tests/cmake/vectors_regen.cmake
  - tests/cmake/vectors_sample_match.cmake
  - tests/core/test_api.c
  - tests/core/test_profile.c
  - tests/cpu/test_bus.c
  - tests/cpu/test_cpu_unit.c
  - tests/cpu/test_vectors.c
  - tests/cpu/vector_bus.c
  - tests/cpu/vector_bus.h
  - tests/embed/CMakeLists.txt
  - tests/roms/manifest.txt
  - tests/vectors/65x02-sample.n65v
  - tests/vectors/fixtures/02-first3.json
  - tests/vectors/fixtures/README.md
  - tests/vectors/fixtures/a9-first3.json
  - tests/vectors/fixtures/a9-tail.json
  - tests/vectors/n65v.c
  - tests/vectors/n65v.h
  - tests/vectors/pins.txt
  - tests/vectors/test_n65v.c
  - tools/vecconv/CMakeLists.txt
  - tools/vecconv/vecconv.c
covered_digest: "v2:sha256:56a2096ce4636d77f7b0c7327107ff79869bbd91530e011df217a4ddd180da0e"
behavior_unverified: 0
overrides_applied: 0
human_verification:
  - test: "After the owner merges PR #5: `gh release view v0.1.1 --json tagName,isDraft,assets` and `gh run list --workflow release.yml --branch main --limit 2`"
    expected: "release.yml succeeded on the merge (and on the release-please PR merge); v0.1.1 is published (not draft) with the per-platform library/runner/libretro archives and SHA256SUMS"
    why_human: "Backstop (02-08 must-have, verification: backstop; SC 3 second clause). Can only happen after the merge; runnable by command, no visual judgement needed"
  - test: "The morning after merge: `gh run list --workflow nightly.yml --event schedule --limit 1` and `gh run view <id> --json jobs`"
    expected: "vectors-full success with 258 tests passed; report job ran (issues: write) and found/closed nothing or closed the rolling `nightly` issue"
    why_human: "Backstop (02-07 and 02-08 must-haves, verification: backstop). The schedule and the report job fire only on main"
  - test: "Decide on CR-01 before merge: fix `tests/cmake/fetch_vectors.cmake` (resolve the path, case-fold on macOS/Windows, delete only a marked directory) or record it deferred with a reason in 02-REVIEW-DISPOSITION.md"
    expected: "Either a fix commit with a test that the guard rejects a case-variant/symlinked DIR, or an explicit deferred disposition"
    why_human: "Escalation: a critical data-loss defect in shipped test tooling, reproduced by the verifier, outside the CPU must-haves; the owner decides whether it gates the merge"
  - test: "Acknowledge the flagged prohibitions (see Prohibitions table): five test-tier prohibitions have no wired enforcing test, one judgment-tier (clean room)"
    expected: "Owner accepts the verifier's directly-observed compliance, or asks for enforcing tests"
    why_human: "Fail-closed rule for test-tier prohibitions without wired enforcement; judgment-tier needs human resolution"
---

# Phase 2: The CPU matches the public vectors — Verification Report

**Phase Goal:** The 6502 behaves as the public 65x02 vectors say on every opcode and every bus cycle, and the release that merging publishes carries it.
**Verified:** 2026-10-03T16:40:00Z
**Status:** human_needed (all pre-merge must-haves verified by running them; what remains is post-merge backstops, one escalated review finding, and flagged prohibitions)
**Re-verification:** No — initial verification

## Commands run by the verifier (own process, HEAD 076f2c4)

| Command | Result |
| --- | --- |
| `cmake --workflow --preset ci` | exit 0, 100% tests passed out of 317; `ctest -N -L vectors` = 256 tests |
| `cmake --workflow --preset vectors-full` | exit 0, 100% tests passed out of 258 (fetch, sample-match, 256 x `0 of 10000 vectors failed`) |
| `cmake --workflow --preset hygiene` | exit 0, 6/6 |
| `cmake --workflow --preset nofp` | exit 0, 5/5 |
| `cmake --workflow --preset asan` | exit 0, 316/316 |
| Mutation: `index_base` page-cross dummy read removed, harness rebuilt by hand | bd 49/100, b9 57/100, b1 48/100 vectors failed (harness catches cycle-level errors) |
| Mutation: LDA #imm result xor 1 | a9 100/100 failed |
| Unmutated hand build of harness | bd, 9d, a9, 20, 6c, 8b: 0 of 100 failed |
| `fetch_vectors.cmake` with `GIT=/usr/bin/false` (simulated no network), fresh DIR | exit 1 (red, no skip) |
| `fetch_vectors.cmake` with DIR=`<repo>/tests` | exit 1, "inside the source tree but not under build/" |
| CR-01 repro on a scratch fake repo: DIR=`<scratch>/FakeRepo`, SOURCE_DIR=`<scratch>/fakerepo` | exit 1 but `<scratch>/fakerepo/src/cpu.c` deleted — CR-01 confirmed |
| `gh pr checks 5` / run 37136463308 (head 5b17fbb) | all 13 CI jobs success incl. `CI required`; 6 build legs 317/317, asan 316, both nofp 5/5, hygiene 6/6, hash-equality |
| run 37136463342 (nightly on PR, head 5b17fbb) | vectors-full success, 258 passed, fetch 64.6 s cold; report skipped (not schedule) |

Local HEAD is 2 commits ahead of the PR head; `git diff 5b17fbb HEAD` touches only 02-REVIEW.md and 02-REVIEW-DISPOSITION.md, so the CI evidence applies to the code at HEAD.

## Goal Achievement

### Roadmap Success Criteria

| # | Success criterion | Status | Evidence |
| --- | --- | --- | --- |
| SC1 | `ctest -L vectors` in `ci` passes the committed sample for all 256 opcodes, final state and every bus cycle | ✓ VERIFIED | 256 `cpu.vectors.<xx>` registered from an `00..ff` loop, no exclusion list; each checked by expect_output.cmake for exit 0 and exact `65x02/<xx>: 0 of 100 vectors failed`; harness compares pc,s,a,x,y,raw p, final RAM, cycle count and each cycle's addr/value/kind; mutation tests prove it is not vacuous; passes on all six CI legs |
| SC2 | `ctest -L vectors-full` fetches the full set at its pin, every test matches; CI runs it nightly | ✓ VERIFIED | Local run 258/258; each full test prints `vecconv: xx: 10000 tests written and re-read` and `65x02/xx: 0 of 10000 vectors failed`; fetch verifies 256 SHA-256+size against pins.txt (commit 2f6980a2…); nightly.yml cron `17 4 * * *`, green on the PR. The first scheduled run is a backstop (below) |
| SC3 | `ci`, `asan`, `nofp`, `hygiene` pass on all six platforms with the CPU in the library; merging publishes a release | ✓ VERIFIED (presets) / backstop (release) | CI run 37136463308 all green; `nm libnesturbator.a` shows `cpu.c.o` (U bus_read/bus_write only, T cpu_step) and `bus.c.o`. Release publication is the 02-08 backstop truth |

### Plan must-have truths

| Plan | Truth (abridged) | Status | Evidence |
| --- | --- | --- | --- |
| 01 | Branch `^phase/02`, plans tracked before first code commit | ✓ | branch phase/02-cpu-vectors; plan commit f23cca4 is ancestor of first code commit fef3fda |
| 01 | `vecconv.a9` converts and re-reads 3 tests | ✓ | passes in ci; message seen in full lane for 10000-test files |
| 01 | Reader rejects zero-length / truncated buffers with offset | ✓ | `vectors.n65v.crafted` passes |
| 01 | vecconv exits 1 with offset on every D-02 input | ✓ | 13 `vecconv.reject.*` + `vecconv.a9_tail.cut` pass |
| 01 | vecconv host tool, not installed, forbidden in embed | ✓ | tests/embed/CMakeLists.txt:17 lists vecconv; CMakeLists.txt:71 "never installed"; embed test passes |
| 01 | No `release-as`; `release.no_release_as` | ✓ | config has no key; test passes |
| 01/02 | runner.hash and write_hashes.content unchanged | ✓ | pass in ci; hash-equality green on six platforms |
| 02 | `vecconv.a9` through the CPU prints `0 of 3` | ✓ | vectors_fixture.cmake RUN path; test passes |
| 02 | cpu.c calls exactly two external functions; lib vs test bus never both | ✓ | `nm -u cpu.c.o` = bus_read, bus_write only; cpu.vectors links vector_bus.c + TARGET_OBJECTS, not the library |
| 02 | cpu.o, bus.o in libnesturbator.a; no new undefined/writable/FP | ✓ | nm output; nofp and global_symbols checks pass locally and in CI |
| 02 | `bus.unit` 24 ticks, mirrors, open bus | ✓ | passes |
| 03 | Sample blob 1,678,114 B, SHA-256 0c318cec… | ✓ | `stat`/`shasum` match; manifest line matches |
| 03 | `vectors_regen.cmake` from range prefixes | ✓ (artifact) | file exists with RANGE_END, `--first`; byte-identity independently proven by `cpu.vectors-full.sample-match` |
| 03 | `manifest.sha256` + selftest | ✓ | pass |
| 03 | `vectors.n65v` validates blob, crafted rejections | ✓ | pass |
| 03 | Empty-input edges for vecconv | ✓ | reject.empty_file/empty_array/closed_short/truncated pass |
| 03 | Ordering edges | ✓ | crafted repeated-index case; blob checked 00-ff |
| 03 | MIT notice verbatim, PROVENANCE lists blob | ✓ | THIRD-PARTY-NOTICES.md §"SingleStepTests 65x02" at 2f6980a2…; PROVENANCE.md:80 |
| 04 | 105 non-control official opcodes pass | ✓ | subsumed by 256/256 |
| 04 | Registers, raw p, RAM, every cycle incl. dummy reads | ✓ | harness code + mutation test |
| 04 | Page-crossing adjacency | ✓ | 256/256 sample + 2.56M full; mutation of page-cross dummy read fails bd/b9/b1 |
| 04 | SED/CLD store D; ADC/SBC binary | ✓ | 61/69/e9/f8/d8 sample and full pass |
| 04 | Flag writes touch only own bits | ✓ | raw p compare over full set |
| 05 | 151 official opcodes pass | ✓ | subsumed by 256/256 (NESTURBATOR_OFFICIAL_OPCODES list was superseded by NESTURBATOR_ALL_OPCODES in plan 06 — gsd artifact pattern check flags this; intentional) |
| 05 | RMW write-back, branch page dummy, JMP (ind) bug | ✓ | full set passes for these opcodes |
| 05 | D kept through PHP/PLP/RTI/BRK | ✓ | full set |
| 05 | P bits 4/5 rule | ✓ | raw p compare over full set |
| 05 | JSR order | ✓ | opcode 20 full set + cpu.unit |
| 06 | All 256 `cpu.vectors.xx` in ci (=SC1) | ✓ | see SC1 |
| 06 | 256 `case 0x`, no `default:` | ✓ | grep: 256, no default |
| 06 | JAM: 11 reads, jammed, loop | ✓ | `cpu.unit` test_jammed_loop + PC wrap test, passes |
| 06 | ANE/LXA magic, defaults 0xEE | ✓ | `core.profile` passes; instance.c:105-106 |
| 06 | SHY/SHX/SHA/TAS via one helper | ✓ | `store_sh`; 9c 9e 9f 93 9b pass full set |
| 06 | JSR stack-overlap issue #18 case | ✓ | cpu.unit (pc 0x017B → 0x0155) passes |
| 06 | 25 opcodes with P bits 4/5 set pass raw | ✓ | all 256 pass |
| 06 | `vecconv.02` compact layout, `0 of 3` | ✓ | passes |
| 06 | No skip list | ✓ | loop over 00..ff; no exclusion variable |
| 07 | vectors-full fetch + 2.56M tests (=SC2) | ✓ | see SC2 |
| 07 | sample-match provenance | ✓ | passes locally and on GitHub |
| 07 | Missing/altered/no network fails fetch; no 77/SKIP | ✓ | simulated offline → exit 1; grep finds no SKIP_RETURN_CODE/77; vectors_full_run requires exactly `0 of 10000` |
| 07 | `vectors.pins` offline ordering check | ✓ | passes with selftest |
| 07 | ci never touches network | ✓ | full tests only under `NESTURBATOR_VECTORS_FULL` (default OFF) |
| 07 | nightly.yml cron, dispatch, PR paths, `permissions: {}`, SHA pins, no cache/secrets | ✓ | file read; action_pins check passes |
| 07 | Report job rolling issue on schedule | ⚠️ backstop (insufficient_spec) | only runs on `schedule` |
| 08 | `CI required` success, six legs with 256 vector tests, asan, nofp, hygiene, hash-equality | ✓ | run 37136463308 |
| 08 | Nightly on PR success, 258 passed | ✓ | run 37136463342 |
| 08 | Nightly timeout = 2 × cold run, run ID in comment | ✓ | `timeout-minutes: 3`, 2×88 s; cites 37136095512 |
| 08 | ci.yml timeouts re-measured | ✓ (warning) | all jobs under half their timeout on measured run 37136095554; but on head run 37136463308 `nofp (ubuntu-24.04)` took 38 s against a 1-minute timeout (63%) |
| 08 | No `provisional` timeout comment survives | ✓ | grep finds none |
| 08 | hash-equality still passes | ✓ | green |
| 08 | Release 0.1.1 published after merge | ⚠️ backstop (insufficient_spec) | post-merge |
| 08 | First scheduled nightly on main passes | ⚠️ backstop (insufficient_spec) | post-merge |

**Score:** 50/53 truths verified (0 present-but-behavior-unverified; 3 post-merge backstops). The behaviour-dependent truths (JAM sequence and loop, JSR overlap, page-cross dummy reads) are each exercised by a passing named test, and the mutation runs show the harness detects cycle-level errors.

### Prohibitions

| Plan | Prohibition | Tier | Disposition |
| --- | --- | --- | --- |
| 01 | No JSON library, Python or `string(JSON)` parse of vector files | test | ⚠️ flagged: no wired enforcing test. Observed compliant: vecconv is owned C; `string(JSON)` appears only in release_config/release_markers on release-please-config.json |
| 02 | Public header gains no function/type/field | test | ⚠️ flagged: no API-freeze test. Observed compliant: `git diff main..HEAD -- include/nesturbator.h` adds 3 comment lines only |
| 02 | No GPL/LGPL source opened | judgment | ⚠️ unverified-prohibition, human review recommended (non-authoritative: nothing in the tree suggests copying) |
| 03 | Full 1.08 GB JSON never committed | test | ⚠️ flagged: hygiene blocks unlisted binaries, not large text JSON. Observed compliant: only the 3 fixtures are tracked JSON; .git is 6.7 MB |
| 06 | No exclusion/waiver list | test | ⚠️ flagged: no test asserts the count. Observed compliant: `ctest -N -L vectors` = 256, full lane 256 |
| 07 | Nightly never passes on skipped/cached fetch | test | ⚠️ flagged: no wired test. Observed compliant: no actions/cache, no SKIP_RETURN_CODE/77; simulated offline run exits 1 |
| 07 | Nightly no commit, no secret, minimal permissions | test | ⚠️ flagged: no wired test. Observed compliant: `permissions: {}`, contents: read / issues: write in report only, `persist-credentials: false` |

### Required Artifacts

`gsd-tools verify.artifacts` across all 8 plans: 27/28 pass. The one miss is `tests/CMakeLists.txt` missing pattern `NESTURBATOR_OFFICIAL_OPCODES` (plan 05), superseded by `NESTURBATOR_ALL_OPCODES` in plan 06; not a stub. All artifacts are substantive and wired (each is built and exercised by a passing ctest).

### Key Link Verification

`gsd-tools verify.key-links`: 13/13 verified across all plans (vecconv→n65v_next, add_subdirectory(tools/vecconv), TARGET_OBJECTS:nesturbator_cpu, cpu.vectors link set, manifest→blob, regen→`--first`, LABELS vectors, FIXTURES_SETUP vectors_full, nightly→`--preset vectors-full`, RP2A03G profile, ane_magic in harness, nightly.yml paths).

### Data-Flow Trace (Level 4)

| Artifact | Data | Source | Real data | Status |
| --- | --- | --- | --- | --- |
| cpu.vectors.<xx> | expected state/cycles | committed 65x02-sample.n65v (SHA-256 pinned in manifest, byte-equal to upstream per sample-match) | yes | ✓ FLOWING |
| cpu.vectors-full.<xx> | expected state/cycles | upstream JSON at pinned commit, SHA-256 checked per file | yes | ✓ FLOWING |

### Behavioral Spot-Checks / Probe Execution

See "Commands run by the verifier". No `scripts/*/tests/probe-*.sh` probes exist or are declared for this phase.

### Requirements Coverage

| Requirement | Source plans | Status | Evidence |
| --- | --- | --- | --- |
| CPU-01 | 02-01..02-06, 02-08 | ✓ SATISFIED | SC1 evidence; six platforms green |
| CPU-02 | 02-07, 02-08 | ✓ SATISFIED (nightly-on-main is backstop) | SC2 evidence; nightly green on the PR |

No orphaned requirements: REQUIREMENTS.md maps only CPU-01 and CPU-02 to Phase 2.

### Anti-Patterns and Review Findings

No TBD/FIXME/XXX in changed source, test or workflow files.

| Item | File | Severity | Undermines a must-have? |
| --- | --- | --- | --- |
| CR-01 lexical in-source guard; `file(REMOVE_RECURSE "${DIR}/src")` | tests/cmake/fetch_vectors.cmake:36-43,136 | ⚠️ Warning, escalated | No. The fetch, pin checks and full-set results are correct. Verifier reproduced the data loss on a scratch copy: a case-variant DIR deleted `<fake repo>/src/cpu.c`. It only fires when a user sets `NESTURBATOR_VECTORS_DIR`, and the README invites that. Not a CPU-correctness gap, but a critical safety defect in shipped tooling. Owner decision before merge |
| WR-01 nightly PR path filter misses its inputs | .github/workflows/nightly.yml:14-18 | ⚠️ Warning | No. CPU-02 "CI runs it nightly" holds via cron, but a change to cpu.c, vecconv or the run scripts is not full-set-tested before merge. The phase PR itself did trigger it |
| WR-02 3-minute nightly timeout with a network fetch | nightly.yml:25 | ⚠️ Warning | Not now: PR run took 90 s with a 64.6 s fetch, so it is already at 50% of the cap. It threatens the "first scheduled nightly passes" backstop if the fetch is slow |
| WR-03 harness clears only listed addresses | tests/cpu/test_vectors.c:187-197 | ⚠️ Warning | No. It can only inflate failure counts after a real failure; it cannot turn a failure into a pass, because a passing test's cycle log matches the expected writes exactly |
| nofp (ubuntu-24.04) 38 s on a 1-minute timeout | .github/workflows/ci.yml:134 | ⚠️ Warning | No (measured run was 12 s); a slow runner could cancel a required leg |
| IN-01..IN-06 | various | ℹ️ Info | No; dormant until the CPU drives frames (phase 3) |

### Human Verification Required

These are owner actions and post-merge commands, not manual UAT:

1. **Release after merge.** Run `gh release view v0.1.1` and `gh run list --workflow release.yml`. Expected: v0.1.1 is published with its archives and SHA256SUMS. This is a backstop and can happen only after merge.
2. **First scheduled nightly on main.** Run `gh run list --workflow nightly.yml --event schedule --limit 1`. Expected: vectors-full passes 258/258 and the report job runs. This is a backstop.
3. **CR-01 disposition.** Before merge, fix it or record it as deferred with a reason in 02-REVIEW-DISPOSITION.md. Escalated because it is critical.
4. **Flagged prohibitions.** Accept the observed compliance, or ask for enforcing tests. One is judgment-tier (clean room) and needs owner acknowledgement.

### Gaps Summary

No must-have failed. The CPU matches all 25,600 sample vectors and all 2,560,000 full vectors on final state and on every bus cycle. This holds locally, on six CI platforms, and in the nightly lane on the PR. Mutation runs show the harness catches single-cycle and single-value errors. What remains:
- the post-merge release and the scheduled nightly (both backstops);
- an owner decision on the reproduced CR-01 data-loss defect in `fetch_vectors.cmake`;
- six prohibitions flagged under the fail-closed rule (five test-tier, one judgment-tier), all observed compliant.

WR-02 and the nofp timeout headroom are the likeliest causes of a future false red.

---

_Verified: 2026-10-03T16:40:00Z_
_Verifier: Claude (gsd-verifier)_
