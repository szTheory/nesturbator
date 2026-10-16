---
phase: 16-host-evidence-and-roadmap
plan: "21"
status: full-passed-review-pending
date: 2026-10-16
runtime_acceptance: true
red_slot: passed-round5-with-retained-invalid-attempts
green_slot: passed-round5
full_slot: consumed-passed
review_slot: unconsumed
---

# Plan 21 source acceptance and retained history

Current result: Task1 causal RED/all8GREEN and Task2 full workflow/packages pass on committed candidate38e14fbc0104038a7eb9ad17d16499c7924c2b9b. Independent source review and mandatory parent gates are pending. The stop records below describe historical attempts; later accepted results do not erase them.

## Historical initial pre-runtime stop

Plan 21 stopped before any source edit or runtime slot was consumed. The task's required `already-quit` case has to drive the existing production session through all sixteen marker boundaries to the ordinary terminal `send_once QUIT` path. The same acceptance criteria cap that case's measured driver-plus-cleanup elapsed time at 5,000 ms.

The unchanged production path performs the complete sixteen-boundary progression, including two native anchor validations, before terminal QUIT. The checked-in README records the existing portable complete fake-host control at 53.2 seconds, before isolated case setup, lifecycle capture, or endpoint reacquisition. The driver also fixes its forty rewind delays at 50 ms each. This conflicts with the 5,000 ms per-case bound; the overall 60,000 ms matrix bound does not resolve the per-case conflict. Reporting a smaller elapsed value would misstate the actual invocation. Bypassing the marker path, extracting or directly mutating `QUIT_SENT`, or adding a production test mode would violate the plan's stated evidence contract.

## Evidence and disposition

- The recorded complete-path measurement is in `README.md` under the portable POSIX session-control description.
- `tests/retroarch/state_session.sh` sends ordinary QUIT only after the prescribed complete path; its fixed rewind gap is 50 ms and the loop sends forty rewinds.
- `16-21-PLAN.md` requires the existing complete fake-host fixture, ordinary terminal QUIT, and `elapsed_ms <= 5000` for every GREEN case.
- No RED, GREEN, preflight, FULL, or independent review command was launched. All five finite runtime allowances remain unconsumed; no result was adopted from Plan 20.
- `tests/retroarch/state_session.sh`, `tests/retroarch/test_state_session.sh`, `README.md`, `include/nesturbator.h`, `.planning/config.json`, and all historical Plan 20 evidence remain unchanged.

## Required next step

Revise and independently check Plan 21 so its already-quit observation can be both authentic and truthfully bounded. The amendment must reconcile the per-case and matrix limits with the measured complete-path cost, or specify an authorized, production-faithful bounded route to ordinary terminal QUIT. Then obtain a fresh Plan 21 scope check before dispatch. Until then, Plan 21 remains incomplete with no SUMMARY, requirement credit, FULL acceptance, review, or handoff to Plan 07. All mandatory parent, native, owner-consent, and release gates remain pending.

## Invalid fixture RED and scoped correction

The subsequent resumed execution consumed RED21 at `build/phase16-acceptance21-execute/red` but failed before fault injection: a precreated quit sentinel caused HOST_DIED with an empty transport log. Its reproduction banner and completed ledger were false; control and primary-result evidence were absent and finalization failed. All original raw files remain untouched. The disposition sidecar binds their digests. Production, README and header remain unchanged; only the regression harness is uncommitted. No causal RED, GREEN, PREFLIGHT, FULL, source review, SUMMARY or requirement credit exists. The substantive fixture amendment identifies the corrections and authorizes one distinct corrected RED root, preserving this failed allowance.

## Second invalid local RED

The corrected-fixture root ended READINESS_TRANSPORT before injection because fake_nc looked beneath an extra attempts directory. control.json/evidence.json were absent; ledger planned/consumed. The shell wrapper lost its own wait code by assigning zsh readonly status. No causal proof, production correction or GREEN/FULL/review is credited. Raw bytes and their disposition sidecar remain retained. The bounded development amendment now permits source-bound local fixture corrections in four distinct rounds; formal acceptance counts remain global and unchanged.

## Four-round local development outcome — not accepted

All four permitted roots under `build/phase16-acceptance21-development` are retained. Round1 RED exited2 because the non-foreign case referenced an uninitialized foreign_path. Round2 RED exited2 because proof finalization lacked driver/fixture/frame environment, after directly observing READINESS_REPLY and a surviving owned host/lock. Round3 RED exited1 with causal observations; its GREEN exited1 because the complete-case override variable did not match run_fault, placing the real invocation outside its canonical case root. Each subsequent round changed the specifically diagnosed harness defect; no unchanged retry occurred.

Round4 RED exited1 and recorded actual request-directory injection after genuine readiness, with the owned host and endpoint lock surviving before emergency cleanup. Its GREEN exited0 and printed the eight-case banner. Primary observations were EXIT1/READINESS_REPLY879ms, INT130/INTERRUPTED_INT941ms, TERM130/INTERRUPTED_TERM894ms, HUP130/INTERRUPTED_HUP858ms, writable1/READINESS_REPLY829ms, already-quit1/QUIT_DEADLINE57560ms, foreign-endpoint1/ENDPOINT_BUSY52ms and foreign-owner-replacement1/READINESS_REPLY892ms. The complete case reached all sixteen boundaries and recorded ordinary quit/OK then quit/QUIT_DEADLINE with one physical QUIT. These are raw observations, not accepted matrix evidence.

The extracted read-only saved-proof gate rejects round4: the matrix duration is negative because Perl CLOCK_MONOTONIC and Python time.monotonic use different epochs on this host; the stdout digest predates the final banner; and four zero-send cleanup rows retain the original reason instead of CLEANUP_REQUEST_WRITE_FAILED. Individual assertion diagnostics found no additional rejection categories in the case identities, raw roles, lifecycle, foreign preservation or send contracts. No formal Task2 command, preflight, FULL or review ran.

After the invocation ended, the local repair sets the cleanup row's distinct reason while leaving FIRST_FAILURE intact. The harness now uses the same Perl monotonic clock for start/end, rejects nonpositive/negative elapsed accounting and enforces the exact zero-send diagnostic row before accepting the four write-failure cases. The foreign fixture installs actual INT/TERM/HUP logging traps before PID publication. Shell syntax, twelve embedded Python compilations and diff whitespace checks pass. Full stream/status hashes still require outer finalization after actual process wait. Four rounds are consumed; no further driver was launched, no source commit/candidate/SUMMARY exists, and no Task1 acceptance is credited. A new bounded continuation must be reconciled by the parent before another coherent unchanged-harness RED/GREEN pair.

## Task1 accepted coherent round5 source proof

The independently checked continuation at0a44a5b authorized one additional changed-source pair. The selected root is `build/phase16-acceptance21-development/round-5`; use `P16_REGRESSION_ROOT=build/phase16-acceptance21-development/round-5` for later gates. Earlier invalid roots remain retained and unused. The four paired source/test/comment paths were committed with normal hooks and the configured GitHub noreply identity as `38e14fbc0104038a7eb9ad17d16499c7924c2b9b`.

Actual RED used the exact reviewed helper blob435a0f5fd1c4444e3b45ca668bd5af0e1a87d7e0 and exited1/EXPECTED_CR01_FAILURE after3166ms. Genuine readiness consumed its request before request.txt became a directory; READINESS_REPLY remained original, and the owned host and exact endpoint lock survived before emergency cleanup. Actual GREEN used repaired helper blob8801e202a67936b25219ee9e93b9c6effae6ee4d and exited0 after105367ms. Both bind identical frozen harness blobf9f0d488d0f18a3193b8aa084bad5b11c7411757, SHA2563baae0839d710c53cce26ec70658da052fc2438af910a1e8fb371cb6814a7f60. Neither outer watchdog intervened. Each outer invocation records actual argv/PID, UTC and same-Perl-clock start/end, wait, complete stdout/stderr SHA256 and final elapsed after process exit. The local batch from round1 exclusive-directory creation to round5 GREEN process exit was696455ms, below3600000ms.

| GREEN case | Actual exit / first reason | Primary elapsed ms |
|---|---|---|
|exit-failure|1 / READINESS_REPLY|779|
|signal-INT|130 / INTERRUPTED_INT|969|
|signal-TERM|130 / INTERRUPTED_TERM|872|
|signal-HUP|130 / INTERRUPTED_HUP|841|
|writable-cleanup|1 / READINESS_REPLY|901|
|already-quit|1 / QUIT_DEADLINE|58461|
|foreign-endpoint|1 / ENDPOINT_BUSY|55|
|foreign-owner-replacement|1 / READINESS_REPLY|869|

All eight case bindings, original first reasons, exact statuses, raw role/inode isolation, request modes, actual foreign signal traps, send counts, bounded owned reap and lock preservation passed the extracted Task2 saved-proof assertions. All seven fresh reacquisitions passed. The four obstructed request cases each have zero physical QUIT sends, a CLEANUP_REQUEST_WRITE_FAILED diagnostic and one exact zero-send diagnostic journal row. The writable malformed-readiness cases send one QUIT. The complete already-quit case reaches sixteen boundaries and ordinary quit/OK then quit/QUIT_DEADLINE with one physical QUIT and no cleanup resend. Whole matrix105367ms minus complete primary58461ms is46906ms, within90000ms; the whole matrix is within420000ms.

The precommit working-byte proof passed before the paired source commit. After committing, `candidate.json` records the full candidate SHA, ten exact working-tree/committed source blobs and unchanged protected config SHA2560517388f5d422ae97afdc5dc1ae11bc43ebd481a757424f152ee8af61559ff5e. Only the four authorized paths differ from the baseline; six protected paths are unchanged. GREEN candidate identity was finalized only after verifying its actual saved driver/harness blobs equal the candidate and same frozen RED harness. `green/candidate-equivalence.json` preserves the original run commit and the byte-equivalence assertion; actual run facts were not replaced. The unmodified extracted read-only saved-proof assertions then printed READONLY SAVED PROOF PASS against the committed candidate. Shell syntax, twelve embedded Python compilations and diff checks pass.

Task1 source proof is accepted. Formal PREFLIGHT/FULL and independent source REVIEW remain unconsumed and shared globally. No full workflow, native observation, consent, release action, SUMMARY or whole-plan credit is claimed here. Parent owns Tasks2/3 and subsequent tracking.

## Task2 current committed-source FULL — PASS

The one shared formal launcher preflight passed runtime identity and the15-second version smoke using the same resolved installed launcher. The normal900-second workflow completed with exit0 on source candidate38e14fbc0104038a7eb9ad17d16499c7924c2b9b. Actual elapsed was293.375720seconds; CTest reports291.56seconds. Configure/build/test/package all completed,649 unique tests passed with no skips and jobs4, and registration/source/config identities remained exact. The metadata-bound parser accepted611 ordinary and38 registered-regex result forms, including2 WILL_FAIL outcomes; retained parser controls also passed. The normal CI invocation's new eight-case cleanup matrix independently recorded PASS in107999ms.

All three newly recreated macOS arm64 ZIPs (library, runner and libretro) passed inventory/CRC/digest/freshness checks and the existing archive checker. These are local packages, not published release proof. Full streams, registration metadata, original current CTest report and Temporary tree, package bytes/inventories, source-before/after, launcher/identity/smoke and actual run/PID/timestamps remain under the selected round's full directory. full-result.json binds both regression evidence digests and the complete per-case binding digest.

The formal FULL allowance is consumed/passed. No independent REVIEW has run yet; parent build/full-repeat and enabled gates remain pending. Native exact-final-head CI/perf/pinned-host, affirmative exact-head owner consent, normal publication and actual18released assets/checksums/attestations/sixmetadata remain07/08 obligations. No SUMMARY or phase completion is claimed by this acceptance record.

Full-result SHA256: 49ba951865a9574f0729bc51fff5a64a58bd2706cb71595f54ad2506a2f52020
