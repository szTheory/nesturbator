# Publication formatting continuation, 2026-10-29

The owner authorized publishing a draft PR and running CI/Nightly to prepare real Plan 21-09 delivery facts. The first push was rejected by the pre-push clang-format 18 check on historical outgoing blobs. An isolated publication checkout formats 19 owned paths and publishes a clean commit on protected main, preserving the original execution history locally. This changes effective source bytes and the campaign fingerprint, so earlier candidate manifests and workflow results below are historical. The refreshed campaign remains 119 PASS / 27 FAIL with all 146 scored rows terminal and five DRAW inventory rows; its fingerprint is `cb1f1ce5de0bfe13b87e6754a8631a3a8d07365c7df1a62677ab4ef60257906c` and report SHA-256 is `56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`. The publication handoff must supply its actual candidate, tested merge, and new source manifests; no earlier manifest is substituted. Plan 21-09 remains pending until completed actual hosted attempts and artifact facts are available.

---

# Phase 21 evidence preparation

**Status:** Current post-review source is locally qualified; hosted acceptance
and Plan 21-09 remain pending separately delivered CI/Nightly evidence.

## Current post-review qualification

The current source candidate is `43d260a914ba76a130809524fba53ad58aca1298`
(`fix(21): honor post-choice NMI recognition inhibition`). The full CI, ASAN,
vectors, MMC3, and campaign runs used the dirty working source at
`7d5fbe18f367`; the source commit contains those same source, test, and public
documentation bytes. After the commit, the 427-path Git mode/blob manifest
verified both the current checkout and an absolute clean clone at the candidate.
The candidate manifest is
`build/phase21-hosted-evidence/source-modes-blobs-candidate.tsv`, SHA-256
`20460339ae9f742d3906ebe7ef6d5c4a2fff53c5c4b54cf59bcbb7b99cb9a1b2`. It
records committed/index/worktree mode and blob IDs, excluding only
`.planning/**` and `tests/accuracy/measurements.md`. The full CI build is not
described as a clean-clone build.

`cmake --workflow --preset ci` passed **695/695** tests in 421.60 seconds and
created all three packages. The full CI command ran before the explicit fresh
authority setup because the local environment lacked `RUNNER_TEMP` and
`GITHUB_ENV`; it therefore is not claimed to have consumed that newly fetched
authority. The authority was then prepared with the existing
`prepare_scoreboard_baseline.cmake` script from current `origin/main` and passed
to a focused CTest invocation on the unchanged source. That invocation included
its campaign fixture and passed **5/5**: campaign, same-invocation protected
observation, protected verdict, no-lost-pass, and all-eleven target gate. The
fresh authority is protected revision
`a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`, 114 PASS keys, SHA-256
`b814f7524d15e0f389e97cb1e8567efbbce601cc8b4c8902ef3471ab965991c4`; it has
exactly 114 PASS keys and no additional main PASS keys to preserve.

The current AccuracyCoin run uses ROM pin
`6bee919529289c555803b612b8c9e4f7eaaeeabb`, library `0.1.11`, behaviour
revision 9, and unchanged `32/5/1800` caps. It reports **119 PASS / 27 FAIL**,
146/146 terminal scored rows, zero skip/not-run/timeout, and five inventory-only
DRAW rows. All eleven named targets pass and all 114 fresh protected-main keys
remain PASS. The final campaign metadata records `source_revision=7d5fbe18f367`,
`source_dirty=1`, and 5,517,533,000 guest nanoseconds. Report SHA-256 remains
`56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`; scoreboard
SHA-256 is `059d7c7558bd9f4785ab384c589f04e16f330ef3ba477393a8492b59e086dbbf`.
The 56-input build fingerprint is
`d96efe4a88bdaa0680398334a3c84a0e04faef405016172e06ef2bb9c4999e57`.

| Check | Current local result |
|---|---|
| `cmake --workflow --preset ci` | 695/695; 421.60 seconds; one campaign lane; packages produced |
| `cmake --workflow --preset asan` | 686/686; 192.01 seconds |
| `cmake --workflow --preset vectors-full` | 258/258; 17.05 seconds |
| `cmake --workflow --preset mmc3-oracle` | 13/13; 2.47 seconds |
| `cmake --workflow --preset nofp` on macOS arm64 | Float scan covers 23 files and passes; 4/5 ABI tests pass. `abi.undefined_symbols` still rejects Apple `stack_chk_fail` and `stack_chk_guard`; the allowlist is unchanged. |
| Fresh-authority CTest invocation | 5/5, including the one campaign fixture, 114-key observation/verdict, no-lost-pass and all-eleven target gate |

The runner regenerated and matched all 114 pinned frame, PCM, APU-transition,
and state-hash rows byte-for-byte. Both `runner.write_hashes` and
`runner.write_hashes.content` passed; the generated and pinned inventory SHA-256
is `76f999668c5bf584074a37f050a19cb4c31f9c63b6671b06b85388e9950be850`. The
six 0-tick and six 60-tick state hashes, six state-size rows, and
`state_format 3` row did not move. State format 3, behaviour revision 9, ABI 1,
and serialized state sizes remain unchanged; the existing NMI pending byte now
uses value 2 for an inhibited candidate, with no new serialized byte. The state
corpus remains 17 files / 23 bytes (aggregate path/content SHA-256
`3b6b71143ce28de6c88b02a4ac65bb73b322edeab950564db705abf2b568bc09`); the ROM
manifest remains 97 rows, SHA-256
`e56a6b2cde320a3c4b5315904b9d4dd99cefc7edf80292ed3f81fc5c008b850d`.

The current package ZIPs are retained in
`build/phase21-hosted-evidence/packages/post-review-source/`:

| Package | SHA-256 | Contents |
|---|---|---|
| `nesturbator-0.1.11-library-macos-arm64.zip` | `5e91fefcdbda743bd9e0c26dec3417e1cf5908f9f585340c6b0f548096995112` | Public header, static library, pkg-config and CMake package files |
| `nesturbator-0.1.11-libretro-macos-arm64.zip` | `c095033d872d2f3ea89ac87468359ec83a3f6c36ab1e6132231017ded4f23a7c` | Libretro core and `.info` file |
| `nesturbator-0.1.11-runner-macos-arm64.zip` | `a975a24b213895d5d86774785d2c4486bc20c992a7d18659f8459a7e845588c1` | `bin/nesturbator-run` |

The first full-CI attempt exposed a metadata parser issue: my explanatory
`source_dirty_reason` contained an extra equals sign. I removed it; the five
affected collector/baseline-fetch controls passed, followed by the complete
695/695 CI rerun. No source gate or parser was weakened. Logs for the current
refresh are `ci-review-final.log`, `asan-review-final.log`,
`vectors-review-final.log`, `mmc3-review-final.log`, and `nofp-review-final.log`
under `build/phase21-hosted-evidence/`. The fresh authority and its selected
current-source CTest output are retained under
`build/phase21-hosted-evidence/fresh-protected-main/`.

`build/phase21-hosted-evidence/candidate.txt` now contains only
`candidate_head_sha=43d260a914ba76a130809524fba53ad58aca1298`. The seven-field
`delivery.schema.json` remains available; `delivery.json` remains absent. No
hosted run ID/attempt, delivered PR, tested-merge SHA, immutable hosted artifact,
or external acceptance fact was invented. The pre-existing
`.planning/config.json` change remains uncommitted. CR-01 mode enforcement,
WR-03 relative source scanning, and WR-04 duplicate validation remain deferred.

## Historical pre-review candidate `1d27a0f` (superseded)

The local candidate source is `1d27a0f12542a235d8d79b9c97591168fbd77d8e`.
The fresh protected-main baseline used in the full CI workflow is
`a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`; its authority contains 114 PASS
keys and has SHA-256
`b814f7524d15e0f389e97cb1e8567efbbce601cc8c4b8902ef3471ab965991c4`. The
candidate campaign used AccuracyCoin pin `6bee919529289c555803b612b8c9e4f7eaaeeabb`,
library version `0.1.11`, behaviour revision 9, and unchanged boot/navigation/
execution caps `32/5/1800`. The result is 119 PASS, 27 FAIL, 146/146 scored rows
terminal, zero skip/not-run/timeout, and five inventory-only DRAW entries. All
114 protected PASS keys and each of the eleven exact target rows passed:

- `Interrupt flag latency`
- `NMI Overlap BRK`
- `NMI Overlap IRQ`
- `$2002 flag timing`
- `NMI Control`
- `NMI Timing`
- `NMI Suppression`
- `NMI at VBlank end`
- `NMI disabled at VBlank`
- `Sprite 0 Hit behavior`
- `Arbitrary Sprite zero`

The final local campaign completed in 4,982,296,000 guest nanoseconds. Its
report SHA-256 remains
`56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`; the
56-input build fingerprint is
`566b32ce33b98b33277a516720f7073ec40f24cb11e67448436e7773e85d87b1`. This
fingerprint includes the corrected integer-only section citation in
`src/state.c`; the campaign result and scored row table did not change.

Candidate, tested merge, and protected base remain separate identities. The
candidate text file contains only:

```text
candidate_head_sha=1d27a0f12542a235d8d79b9c97591168fbd77d8e
```

`build/phase21-hosted-evidence/delivery.schema.json` defines exactly seven
required fields: `repository`, `ci_run_id`, `ci_run_attempt`,
`nightly_run_id`, `nightly_run_attempt`, `candidate_head_sha`, and `pr_number`.
There is no `delivery.json`: this local step has no hosted CI/Nightly run IDs,
attempts, PR number, or actual tested-merge SHA to enter. The candidate file
describes the tested local source; it does not claim a delivered PR candidate.

## Historical pre-review checks and packages (superseded)

The full `cmake --workflow --preset ci` command exited 0 from an absolute,
clean-equivalent checkout of the candidate. The fresh protected baseline was
prepared before candidate-controlled configure. The workflow ran one full
AccuracyCoin campaign lane, its same-invocation protected observation, all
695/695 CTest tests, and the package step. It produced:

| Package | SHA-256 | Contents checked |
|---|---|---|
| `nesturbator-0.1.11-library-macos-arm64.zip` | `f68cdd0c309ccff557e300d3e770add1fe363558045af25f919bd6caed961799` | Public header, static library, pkg-config and CMake package files |
| `nesturbator-0.1.11-runner-macos-arm64.zip` | `c82866961e4270ecbec58d7abf96dfcf969fad0f53ae8a7fea2c593eb82d7aa2` | `bin/nesturbator-run` |
| `nesturbator-0.1.11-libretro-macos-arm64.zip` | `9cf4734626f9c2a94636b2744209e26644582d91b72895bac869b100b7e8c3f8` | `cores/nesturbator_libretro.dylib` and its `.info` file |

The three ZIP files are retained under `build/phase21-hosted-evidence/packages/`.

The other configured local checks also passed from that clean checkout:

| Invocation | Result |
|---|---|
| `cmake --workflow --preset asan` | 686/686 tests passed; 230.04 seconds; configured short-run suite |
| `cmake --workflow --preset vectors-full` | 258/258 tests passed; 8.26 seconds |
| `cmake --workflow --preset mmc3-oracle` | 13/13 tests passed; 1.59 seconds |
| `cmake --workflow --preset nofp` on macOS arm64 | Float scan, its self-test, global-symbol check, and float fixture passed; undefined-symbol allowlist rejected Apple `stack_chk_fail` and `stack_chk_guard` |

The CI run also passed `runner.boot_to_play`, `holymapperel.m2.decode`, the
MMC3 oracle unit checks and the Phase21/Phase20 evidence controls. The vector
and MMC3 input trees were supplied from external scratch space and were not
added to the repository. Full command output is retained in
`build/phase21-hosted-evidence/ci-final.log`, `asan-final.log`,
`vectors-full-final.log`, `mmc3-oracle-final.log`, and `nofp-final.log`.

## Historical pre-review source identity (superseded)

`build/phase21-hosted-evidence/source-modes-blobs.tsv` records the candidate
HEAD and tree plus 427 effective tracked paths. Each row records committed Git
mode/blob, index Git mode/blob, and the current worktree content blob. The
manifest excludes only `.planning/**` and `tests/accuracy/measurements.md`.
Its SHA-256 is
`daa659df9cd037d09c489f1795d7d1a8e564f82b6c2299d77428645f44e45c84`. It was
verified before and after the local workflows against both the absolute clean
source checkout and the current checkout. The only pre-existing current-tree
edit, `.planning/config.json`, is outside this effective input list and was
kept out of all commits.

The no-float probe first caught the decimal hardware section citation in the
state-validation comment. The comment now spells the reference as integer
section and subsection numbers; the scanner itself and its hard gate were not
changed. The final-source scan passes. The remaining macOS nofp failure is
only the undefined-symbol allowlist's `stack_chk_fail` and `stack_chk_guard`
entries. The configured CI nofp matrix uses Linux x64 and arm64, so this local
macOS failure is not a substitute for those hosted results; the Linux result
remains pending.

This is an independent, run-specific tracked Git identity record and
clean-equivalent-source check. It does not establish host filesystem
permission bits or repair collector debt. CR-01 mode enforcement, WR-03
relative source scanning, and WR-04 duplicate validation remain deferred.
Local synthetic controls prove that diagnostics reject their bounded fixture
cases; they are not actual hosted proof. No arbitrary marker or synthetic
self-comparison is credited as a mode check. A later source, test, pin,
workflow, or collector change requires fresh hosted runs.

The wrapper reuses existing Phase20 collector functions with a narrow Phase21
contract of 114 protected keys, the exact eleven target keys, behaviour
revision 9, and the current library version. Phase20 continues to use its
historical 108-key/six-target/revision-8/`0.1.11` defaults and existing
artifact names, including `phase20-perf`.

## Plan 21-09 external evidence still required

Plan 21-09 must first confirm availability of completed CI and Nightly runs
for the delivered candidate. These facts are absent here: actual candidate
delivery and PR number, CI run ID/attempt, Nightly run ID/attempt, tested merge
SHA, actual protected base for that merge, and immutable run/job/artifact
metadata and content digests. No hosted-acceptance verdict or publication
authorization is made by this preparation.

The later collector must validate the original workflow outputs under their
existing names:

- Six platform jobs: Linux x64, Linux arm64, macOS x64, macOS arm64, Windows
  x64, and Windows arm64. Each provides `hashes-${os}-${arch}` and
  `accuracycoin-evidence-${os}-${arch}-${ci_run_id}-${attempt}`. All complete
  frame, PCM, APU-transition, and state hash inventories must match exactly;
  each lane must retain the fresh 114-key protected authority and all eleven
  target passes.
- The actual base/head performance artifact remains
  `phase20-perf-${ci_run_id}-${attempt}`. It must contain measured results and
  meet the existing zero-spread / +1% slowdown limits; `n/a`, missing, or
  mixed-attempt results fail.
- Required CI jobs and the actual `retroarch-marker-progression` evidence
  must pass; the CI evidence must also include Holy Mapperel and boot-to-play.
  Local host-session controls do not replace that marker artifact.
- Nightly must supply `vectors-full-evidence-${nightly_run_id}` and
  `mmc3-oracle-evidence-${nightly_run_id}` from the same tested source, with
  successful full-vector and MMC3 inventories and JUnit results.
- Candidate, actual tested merge, protected base, every run/attempt, and each
  artifact identity must be separately bound. Current-checkout source
  revalidation and the independent tracked mode/blob comparison supplement
  deferred collector checks without resolving them.

`build/phase21-hosted-evidence/delivery.json` must be created only from the
actual separately delivered run and PR facts. Until those facts exist, Plan
21-09 remains pending and this local preparation is not a claim that INT-01,
INT-02, KEEP-01, the phase, or hosted acceptance is complete.

Publication preflight: hygiene 13/13 (including clang-format 18) and focused interrupt/state/actual-adapter suite 77/77 passed on the formatted source. The complete original execution history is preserved locally at `refs/phase21-history/execution-6c42607`. Hosted qualification remains pending the publication runs.
