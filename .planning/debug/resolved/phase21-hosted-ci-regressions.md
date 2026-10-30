---
status: resolved
trigger: "Phase 21 hosted CI performance regression and evidence-control timeouts. Read .planning/phases/21-interrupt-latency-and-hijack/21-DELIVERY.md"
created: 2026-10-29
updated: 2026-10-29T18:41:17Z
---

## Current Focus

bug_class: Bohrbug (deterministic instruction overhead); environment-amplified deterministic harness work
hypothesis: Confirmed compiler hot-path overhead, per-path Git launch amplification and native-newline fixture defect are repaired without changing their original contracts.
test: Final local workflows, actual same-source CI18jobs/Nightly7workjobs, all16rawZIP digests, six native hash files, independent428-input inventories and canonical read-only availability all pass.
expecting: Resolved debug handoff; Plan21-09 acceptance remains a separate command.
next_action: No further debug checks. Separately run $gsd-execute-phase 21 using the promoted canonical delivery and clean source.
fixture_reasoning_checkpoint:
  hypothesis: CMake native text output gives Windows Git CR-suffixed index-info paths, so Git ignores the intended conflict rows and the negative fixture is not constructed.
  confirming_evidence: [actual Windowsarm64 log reports Ignoring path target.txt three times then identity write succeeds, old KB nightly-intel-session-controls-portability documents this same native text conversion mechanism]
  falsification_test: Explicit LF bytes still produce ignored paths or fail to construct exact target.txt stages1and2 on fresh Windows CI.
  fix_rationale: Write the line-oriented Git protocol as explicit LF and assert both raw bytes and actual constructed index before exercising the identity helper.
  blind_spots: Actual failed fixture bytes were not uploaded; raw error and known CMake/Windows mechanism identify the candidate, final Windows jobs must confirm.
  candidate_causes: [data native newline bytes, environment Windows Git path validity, code fixture writer mode]
  and_gate: yes; platform newline conversion plus Windows Git rejecting CR-bearing paths prevented fixture construction.
performance_reasoning_checkpoint:
  hypothesis: Rechecking entry and inhibited-state conditions on every ordinary bus cycle accounts for most measured additional Ir/frame.
  confirming_evidence: [hosted bus_read_cycle delta +556327 Ir/frame, staged entry_action only +243 Ir/frame, code diff adds entry/inhibited tests to every cycle and line sample]
  falsification_test: Equivalent condition regrouping fails to materially lower bus_read_cycle instruction counts on the same ROM/frame interval.
  fix_rationale: Branch on the rare inhibited state and input transitions before inspecting entry fields; preserve pending recognized history and every phi2 close/open endpoint.
  blind_spots: GCC generated code effects and valid loaded histories need measurement and the staged-entry/state/held-pulse suite.
  candidate_causes: [code repeated state classification, environment compiler instruction selection, data changed guest cycle counts]
  and_gate: no; hosted function-level counts locate deterministic code overhead, guest hash and suite comparisons will check changed execution as an alternative.
reasoning_checkpoint:
  hypothesis: The new identity helper's 3843 Git launches multiply host process cost and exhaust the full controls budget.
  confirming_evidence:
    - Source read shows 3 Git subprocesses per 427 effective paths on each of three full identity invocations.
    - Isolated identity verify takes 5.65s locally; inherited all controls alone take 16.71s, with all named rejections passing.
    - Actual macOS Intel and both Windows jobs fail this combined control at 120s; other inherited control tests complete.
  falsification_test: A batched helper producing the identical manifest with bounded process count still spending comparable time would refute process amplification as the repaired cost.
  fix_rationale: Query the whole committed tree once and hash all regular working files in one Git invocation, retaining independent committed/index/worktree fields and symlink handling.
  blind_spots: Exact old hosted per-process timings were not retained; final hosted platforms must confirm the unchanged 120s cap.
  candidate_causes: [code repeated Git invocations, environment platform process cost, data effective input count, config combined 120s cap]
  and_gate: yes; per-input process multiplication and platform launch overhead combine, so reduce unnecessary work without changing the cap.
candidate_causes: [code per-cycle redundant conditions, config whole controls timeout, environment Git launch cost, data inventory size]
and_gate: yes for timeout (large fixed Git query count multiplied by platform launch cost); performance is independent of wall clock.
sbfl: skipped (no failing/passing per-test coverage spectrum for instruction-count gate)

## Symptoms

expected: Source-bound CI passes the +1% Ir/frame gate and completes every evidence control; same-source Nightly passes all seven work jobs.
actual: Candidate 5cf4c9c failed CI 40919233064/1; Nightly 40919233254/1 passed. Performance rose 2.88%, 3.10%, and 3.19%. The Phase21 evidence controls timed out on macOS x64 and both Windows architectures.
errors: phase21.hosted_evidence.controls exceeded 120.12s on macOS x64, 120.01s on Windows x64, and 120.02s on Windows arm64. Performance gate rejects all three measured ROMs above +1%.
timeline: First hosted qualification of Phase21 against protected main a64c23f; the publication formatting preflight passed hygiene 13/13 and focused tests 77/77. Earlier original-source full local suites remain historical.
reproduction: Use PR61 CI workflow on the actual candidate. Raw performance counts in build/phase21-hosted-evidence/publication/perf-artifact reproduce the failure with tests/cmake/perf_compare.cmake. Full logs, actual run/attempt identities, and artifact digests are indexed by 21-DELIVERY.md.

## Evidence

- Initial evidence: .planning/phases/21-interrupt-latency-and-hijack/21-DELIVERY.md.
- Actual candidate: 5cf4c9cf056682f79daeafc7e35a38215903e930; tested merge: 28b1eefd1184a8d303f2bab7f03c4ec0fc31e199; protected base: a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562.
- Current branch HEAD before debug: 51ad35e39ef3ada335fbe2a062202a11fdf1dc38, a local delivery-metadata commit with all 427 effective source rows identical to the published candidate.
- Existing user edit: .planning/config.json only; preserve it unstaged and byte-for-byte.
- Knowledge base: prior nightly-intel-session-controls is a candidate pattern for interpreter/process overhead; no semantic recall tool was available. No prior CPU performance cause matches.
- Hosted runner Cachegrind 360-minus-60 function delta: bus_read_cycle +556327 Ir/frame; cpu_step +125801; bus_write +3117; bus_read +2560; entry_action only +243. Most +688455 Ir/frame is common bus work, not frequency of staged entry itself.
- Current source identity helper executes rev-parse and ls-tree and hash-object for every one of 427 paths. Three non-missing full controls produce 3843 per-path Git subprocesses. One isolated current verification takes 5.65s on local Apple Silicon; slow-platform causality still needs controlled launch-cost proof.
- Config SHA256 remains 62a22f6862efb7c74ee31baf2f22bae32d453e79aaec6726dbcf3b7213d47167.
- Isolated inherited Phase21 shared all-controls suite passes every rejection in 16.71s (13.32s user/3.04s sys), retained at build/phase21-hosted-evidence/shared-before.log. This excludes source identity work.
- Batched helper uses 7 Git launches instead of 1285 for the same 427-path manifest; original helper on the same changed checkout produces byte-for-byte identical output (cmp passes). Original helper 6.38s, batched helper with profiling 0.25s. Both dirty-file content and the CLAUDE.md symlink remain represented independently of committed/index blobs.
- Existing complete Phase21 controls pass in 17.51s after batching, unchanged 120s cap. A scratch copy using the original helper fails the new measured process budget with 1285 > 12 launches; current helper is 7/12. All existing positive/negative evidence controls are retained.
- Bus-only regrouping lowers runner 60-frame instructions from 1345885619 to 1336951349 with distinct binary digests; 15 focused IRQ/NMI/staged-entry/state/CPU tests pass. The first pair was discarded because hashes revealed identical binaries from stale Docker mount timestamps; a second explicit build compiled bus.c and produced distinct binaries.
- CPU hypothesis: storing old I, two exception flags and first poll on every opcode makes ordinary instructions classify rare CLI/SEI/PLP/branch/JSR exceptions afterward. Commit those exceptions at their existing legal poll points instead; retain accepted IRQ semantics and branch OR history. Falsification: same interval fails to lower cpu_step Ir or any poll/state test fails. Blind spot: compiler layout; measure independently before accepting.
- Linux GCC14 (same 14.2.0-4ubuntu2 package as hosted), runner 60→90 differential: original 22911431.9 Ir/frame; bus regrouping 22762529.7; CPU specialization 22734955.1. CPU gains only 27574.6; insufficient for the +1% target and a candidate for removal if a smaller fix suffices.
- Generated assembly comparison against independently archived protected base shows the grown cycle prevents GCC specializing dynamic PPUSTATUS dispatch: common reads now preserve status-only temporaries, zero two extra registers, test status again, push/pop an extra callee-saved register and adjust stack. The base kept that work on its status branch. New falsifiable hypothesis: explicit constant cycle arguments restore that specialization; same bus endpoints and unchanged DMC paths remain.
- Explicit constant-argument dispatch restores five-register hot-read code and removes 263637.4 Ir/frame on runner. Combining the unobserved +3/+12 tick updates removes another 59564.1 Ir/frame; no call, trace, sample, save or timed observer exists between those original increments (only CPU history assignments). 22 focused CPU/bus/PPU/NMI/entry/state tests pass.
- Final local Linux amd64 GCC14.2/Valgrind3.22, 60→360 differential: nesteroids base23914932.2267/head24103968.3533 (+0.79045%); runner base22203492.7433/head22392202.7133 (+0.84991%); M4 base21998505.29/head22215976.14 (+0.98857%). Protected-base counts independently match retained hosted measurements exactly. All +1% thresholds and allowances unchanged. CPU specialization is necessary for M4 margin, so retained.
- Synthetic identity semantics pass spaces/raw CRLF, independent committed/index/work content, executable index mode, staged-only path, raw symlink target text, missing tracked input and unresolved-index-stage rejection. Full root manifest equality versus old algorithm remains proven; dynamic budget kills old per-path query implementation.
- Fresh final campaign at dirty51ad35e39ef3 retains119PASS/27FAIL,146terminal,5DRAW, report56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3; refreshed56-input fingerprint bf09fe6ebb319fbb668abb65f70e5142bc1477a0dc70f391a305d7f3e97c136a. No state/hash/scoreboard revision changed.
- Independent read-only peer review returned no findings in CPU poll timing, valid loaded NMI histories, tick endpoints or independent committed/index/raw identity. Residual exotic-path and filesystem-mode limitations are inherited; CR-01/WR-03/WR-04 remain deferred.
- Causal source-point reversion compiled from final source with only constant-status dispatch reverted in a scratch bus object, preserving all other fixes and normal library/runner objects: M4 rises to22482530.8 Ir/frame (+2.200265%), failing the unchanged +1% gate. Reapplication is being measured; canonical source never changed during final CI.
- Current tracked inventory now428 inputs after adding the independent semantic control script. Current precommit manifest write/verify pass (39768c6caa47a7c332756bcecebaaddc5fbe70b3010a1c5ace0de7e6341ebd52). Final standalone controls including semantic neighbors pass19.52s under120s.
- Reapplication in the same isolated scratch archive restores M4 exactly to22215976.14 Ir/frame (+0.988571%), zero difference from final measured binary. The original helper also fails the new work budget1285>12 versus current7. Both root-cause repairs have causal red/green controls; canonical428-input manifest verifies unchanged across experiments.
- Final full local CI passes695/695 in380.80s and produces library/runner/libretro ZIPs, with freshly fetched protected-main114-key authority wired into that invocation. Its Phase21 controls pass19.69s. Final hygiene passes13/13 in34.68s. Logs under build/phase21-debug/ci-final.log and hygiene-final.log.
- Final ASAN passes686/686 in156.32s, vectors-full258/258 in10.03s, MMC3 oracle13/13 in1.86s. All final source workflows are complete; no full-suite rerun is needed for measurement/planning-only identity updates.
- Source repair committed and pushed normally as319916fb3868aa0343d887419a22ad96cd551e8a; pre-push hook verifies28 outgoing committed source blobs formatted. Existing draftPR61 remains draft. Actual new pull-request events are CI40927374281/1 and Nightly40927374013/1, both running on candidate319916f/basea64c23f. PR snapshot reports tested merge4b3f9e8521ef94675a7f226aae206ee7edcc30c9; completion and job logs must still confirm it.
- Independent no-hardlink absolute clean clone at build/phase21-repair-clean-source has empty Git status. Canonical and clean manifests match byte-for-byte across428 inputs, SHA791c6529b7975b27303839e2db8093f48477022c59ba9c7d37005c98f7dddae7. New immutable prep root build/phase21-repair-hosted-evidence contains bare candidate.txt, exact seven-field delivery.json and initial raw run/PR snapshots; earlier failed bundle remains intact.
- Actual hosted CI40927374281/1 perf job123636378599 succeeds with exact local-predicted counts: base/head nesteroids23914932/24103968, runner22203492/22392202, M421998505/22215976. Downloaded raw artifact12922150402 ZIP SHA8437af822a962f33ffd9c8a02d99e8ffad792fdd782c529a1d776c8d032c48f7 matches API digest; preserved raw job log and source-bound identity. The unchanged perf_compare gate also revalidates downloaded raw samples locally. Other hosted jobs remain running.
- New fixture portability failure: actual Windowsarm64 job123636480631 fails only phase21 controls in6.55s. Process budget7/12 and previous missing/altered-manifest controls pass. Git prints Ignoring path target.txt three times; intended unresolved stages are absent, so the last negative test fails to construct its input. All678 other tests pass. Raw log retained publication/windows-arm64-failure.log. This is a new test-data defect, not the old timeout or the CPU repair failing.
- Windowsx64 reproduces the same fixture defect in5.45s, all678 other tests pass; raw log retained. Fixture-only correction writes explicit LF, asserts exactly three LF/no-CR protocol rows and exact actual stages1/2 for target.txt. Corrected semantic controls pass macOS and Linux; explicit CRLF mutant fails the new byte guard. Full focused Phase21 controls pass18.15s with all inherited negatives. No CPU/bus/helper source, public header, baseline fingerprint, timeout or threshold changed in this followup.
- Fixture-only followup committed/pushed normally as54c7db29c73a66bb552e71b0887529249a41a072; read-only followup review reports no findings. Fresh actual pull-request attempts are CI40929011059/1 and Nightly40929010997/1. PR snapshot reports testedmergefaf1b9159ce8dae83cd2c08c155fc9b77fac32de/basea64c23f; current prep candidate/delivery files select these attempts, with previous319 selectors archived under publication/source-319916f. Clean clone updated through normal fetch/checkout, no recreation. Local full CI rerun on frozen54c is in progress. Old source319 hosted perf remains a valid historical success, while its Windows fixture failures prevent overall acceptance.
- Final54c actual hosted perf job123642609511 succeeds with exactly the same predicted counts. Final raw artifact12920863587 ZIP SHAe989d141819124cb704f65b1ffc267cf582506902275c002084d2595ab5933cf matches API digest; its identity binds CI40929011059/1, candidate54c and mergefaf1b915/basea64c23f. Unchanged gate revalidates final raw samples locally. Final54c local CI passes695/695 in392.75s, including Phase21 controls17.84s, and packages all three deliverables. Sanitizer/other final local checks and remaining hosted jobs are still running.
- Final54c canonical/clean manifests match428 inputs, SHAb11cb67a1a8e84c045a661cecfb7999bdeed3bcd25bed493cd49ed7762182e7b; independent candidate/merge tree mode/blob inventory SHAd62c2f7e3f5b27653f2edcd9f9fef77d272bef6465b8afb161f8b76186899fe3. Tested merge fulltree e27d0dd1cabf913f3c76c36c3fa59906ecb6e14d, exactparentsa64c23f and54c.
- Final54c actual Windowsx64 andarm64 build jobs succeed. Raw logs show Phase21 controls49.51s and63.31s respectively, below unchanged120s; all679tests pass each. Linuxx64 controls36.02s, Linuxarm64 32.96s and macOSarm64 46.03s also pass. Exact new-fixture Windows failures are eliminated; macOSIntel and remainingNightly still need completion.
- Final54c local ASAN passes686/686 in187.20s, vectors258/258 in8.65s, MMC313/13 in1.72s and hygiene13/13 in45.49s. All final local workflows are terminal on frozen corrected source. Canonical428-input manifest re-verifies unchanged.
- Fourteen of sixteen required actual ZIPs are retained under raw-artifacts with downloadSHA matching API digest, candidate54c workflow identity and safe unique archive members. Raw clean hosted campaign confirms119PASS/27FAIL/5DRAW, all114protected and11exacttargets, unchangedreport56ebd... and fingerprintbf09fe...; five native hash files are byte-identical pinned76f999668c5bf584074a37f050a19cb4c31f9c63b6671b06b85388e9950be850. Intel artifact pair/equality still pending. FinalNightly Intel session job now succeeds; only its suite-flake work job remains running.
- Final original-workflow verification is complete: CI40929011059/1 succeeds18/18jobs; Nightly40929010997/1 succeeds7/7workjobs, only declaredPRreport skips. MacIntelcontrols pass104.99s under120s; actualhash-equality andCIrequired succeed. All16source-bound rawZIPs have API-matchingdigests; all6nativehashfiles equalpinned76f999...be850. Primarycleanhostcampaign actualmetadata records119/27/5,146terminal,114protected,11targets and unchangedreport/fingerprint.
- Both unique and canonical production read-only availability probes returnavailable for final54c/mergefaf1b915/basea64, effective428-inputdigest004e963df11ad77f080e422ea0b87dab5c060117d38ff9b43d330116b974479e. Canonicalcandidate/delivery/effective-input selectors now select final54c attempts; original5cfselectors/availability/probe archived, originalrawbytes untouched. Existing canonicalcleanclone updated through normalfetch/detachedcheckout, clean and mode/blob-identical; originalexecutionref remains6c42607863c78a28b30abd2c053da6b429b3442d. Config remains exactSHA62a22f6862efb7c74ee31baf2f22bae32d453e79aaec6726dbcf3b7213d47167, unstaged. No collect/validate acceptance task, phase/requirement completion, PRready/merge or lifecycle command ran.

## Eliminated

- hypothesis: Staged entry iteration itself dominates the performance regression.
  evidence: Hosted runner entry_action adds243Ir/frame versus bus_read_cycle556327 and cpu_step125801; ordinary-cycle work dominates.
  timestamp: 2026-10-29
- hypothesis: Performance failure is host wall-clock noise or changed guest output.
  evidence: Instruction counts are deterministic with zero base-repeat spread, independent local protected-base counts match hosted exactly, and all final pinned hashes/campaign verdicts remain unchanged.
  timestamp: 2026-10-29

## Resolution

root_cause: Enlarged interrupt-cycle bookkeeping prevented GCC specializing dynamic PPUSTATUS cycle paths and folding an unobserved timestamp, while generic IRQ exception bookkeeping ran on ordinary opcodes; independent evidence controls launched Git three times per tracked path on three full passes, amplifying slower hosted process launch costs. The added semantic conflict fixture also needed explicit LF because native Windows CRLF made Git ignore its intended unresolved rows.
fix: Preserve semantics while specializing constant ordinary/status cycle arguments, combining unobserved ticks, checking NMI transitions/rare inhibited candidates first, and committing exceptional IRQ polls locally. Batch independent Git inventories/raw hashing and enforce a measured process-count budget plus independent semantic controls, with the original timeout intact. Write the conflict protocol with explicit LF and assert both bytes and actual stages before testing rejection.
verification:
  target_test: {result: pass, local_gate: all three ROMs below original1percent, hosted: CI40929011059/1success18jobs and Nightly40929010997/1success7workjobs, control_seconds: [36.02,32.96,104.99,46.03,49.51,63.31], original_cap: 120}
  mutation_check: {result: pass, source_point_mutant: reverted constant status dispatch yields M4plus2.200265percent and fails target gate; original per-path helper fails1285over12 process budget; explicitCRLFfixturemutant failsbyteguard, Stryker: not configured; owned causal source-point controls used}
  no_op_deletion: {result: pass, deletion_justified_by_rca: true, rationale: removes redundant timestamp and exception bookkeeping while retaining legal polls and independently tested NMI recognition; batches equivalent inventories}
  adjacent_tests: {result: pass, suites_run: [ci695, asan686, vectors258, mmc3_13, hygiene13, independent_review_no_findings]}
  revert_and_reconfirm: {result: pass, bug_returned_on_revert: true, fixed_on_reapply: true, scope: isolated scratch source/object/archive; canonical428-input manifest unchanged}
  guardrail_verdict: accepted
oracle_type: specified (original instruction-count threshold, control timeout, source identity/protocol and protected target contracts)
source_commits: [319916fb3868aa0343d887419a22ad96cd551e8a, 54c7db29c73a66bb552e71b0887529249a41a072]
confirmation: Original real hosted workflow reproductions are now green; repository-authorized automated checks need no manual sign-off.
files_changed: [src/bus.c, src/cpu.c, tests/cmake/phase21_source_identity.cmake, tests/cmake/phase21_source_identity_controls.cmake, tests/cmake/phase21_hosted_evidence_controls.cmake, README.md, include/nesturbator.h, tests/accuracy/baseline-metadata.txt, tests/accuracy/measurements.md]

## Prevention

- Code/compiler branch: added per-cycle entry state made dynamic status-read timing expensive on ordinary reads; local correctness tests did not measure GCC14 instruction counts. Preserve constant cycle paths and the unchanged hosted per-ROM +1% gate; the isolated source-point revert demonstrates the gate catches this class.
- Code/environment/data branch: three Git processes per tracked path multiplied host launch costs across three full inventories. A measured maximum12-process budget now guards the complete inventory, with independent committed/index/raw/mode/symlink/stage controls retaining semantics. No timeout is enlarged.
- Data/environment branch: native CRLF broke the new Git line protocol only on Windows. Explicit LF, exact raw-byte assertions and exact constructed-stage assertions remove that ambiguity; a deliberate CRLF mutant is killed before the subject helper runs.
- Why not caught: local macOS suites did not exercise GCC14 Cachegrind or slow platform process launches, and no process work budget existed. The new fixture initially checked rejection without first proving exact protocol bytes/stages. Actual hosted gates caught both classes rather than being weakened.
- Recurrence guards: tests/cmake/phase21_hosted_evidence_controls.cmake measured12-process bound and all inherited negatives; tests/cmake/phase21_source_identity_controls.cmake exactLF/stage and dirty identity neighbors; unchanged .github/workflows/ci.yml perf gate, all six platform controls and same-source Nightly repetition/fuzz lanes.
- Semantic memory: MemPalace tool/CLI unavailable; append-only knowledge-base.md is the durable fallback.
