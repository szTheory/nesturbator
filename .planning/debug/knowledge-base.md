# GSD Debug Knowledge Base

Resolved debug sessions. Used by `gsd-debugger` to surface known-pattern hypotheses at the start of new investigations.

---

## nightly-vectors-fetch — bounded pinned blob acquisition
- **Date:** 2026-10-28
- **Error patterns:** curl 28, bad pack header, promisor remote, vectors-full.fetch, Windows CRLF fixture hash mismatch
- **Root cause(s):** Partial-clone checkout bulk-requested 260 blobs; its measured smart-HTTP response supplied no pack payload before the unchanged cutoff. The newly added test fixture also wrote native CRLF against an LF-only oracle on Windows.
- **Fix:** Prefetch pinned ancestor/v1 tree blobs in batches of at most 16 before checkout, preserving all pin/file/transport checks; use newline-free two-byte synthetic fixture payloads.
- **Files changed:** tests/cmake/fetch_vectors.cmake, tests/cmake/vectors_transport.cmake, tests/CMakeLists.txt, README.md, tests/accuracy/baseline-metadata.txt, tests/accuracy/measurements.md, Phase 20 delivery/state records
- **Why not caught:** No offline gate exercised checkout request shape; existing guards covered directory safety only. Windows CI caught the new fixture portability mismatch.
- **Recurrence guard:** tests/cmake/vectors_transport.cmake:vectors.fetch.transport, registered in ci, requires exact pinned initial-fetch policy, unchanged cutoff, bounded blob requests, all blobs before checkout, warm no-fetch reuse and strict corrupt/extra-file rejection. Original-code and batch/depth/CRLF mutant controls reject the bad behaviors; real six-platform CI and cold Nightly pass.
---

## nightly-intel-session-controls — interpreter overhead in bounded host polls
- **Date:** 2026-10-28
- **Error patterns:** PUBLICATION_UNSTABLE, load-probe_SEND_FAILED preempted, readiness full driver+cleanup exceeded 25000 ms despite valid exact reply/progression
- **Root cause(s):** Repeated clock interpreter launches amplify helper cost across the prefix; interpreted bytewise FNV can consume the publication window before a second stable observation. Controlled cost reproduces both mechanisms with immutable fixture bytes; exact failed-hosted per-process timing was not retained.
- **Fix:** Reuse the owned runner monotonic clock and bounded progression FNV reader through native frame_index modes after bootstrap. All publication/operation/attempt budgets, exact fault reasons, one-shot counts and 25000-ms total cap remain unchanged. Fresh local CI 688/688 plus packages passed; new-source hosted validation remains pending.
- **Files changed:** tests/retroarch/frame_index.c, tests/retroarch/state_session.sh, tests/retroarch/test_state_session.sh, tests/retroarch/CMakeLists.txt, tests/cmake/session_helpers.cmake, README.md, include/nesturbator.h, tests/accuracy/baseline-metadata.txt, tests/accuracy/measurements.md, .planning/STATE.md
- **Why not caught:** CI had semantic fault cases without deliberate helper-cost controls; Intel Nightly exposed timing amplification. The new script's policy omission and header-fingerprint drift were caught and corrected by the full workflow before acceptance.
- **Recurrence guard:** tests/retroarch/test_state_session.sh controlled 100-ms old-clock and 1050-ms old-hash paths, with active-shim usage assertions, run in retroarch.session.controls/readiness. retroarch.session_helpers freezes native binary FNV vectors, 4096-byte buffer and 1 MiB cap neighbors, missing/empty rejection and monotonic samples. Independent single-helper reverts restore each failure; current campaign evidence is measured, and prior hosted acceptance retains its historical source identity.
---

## nightly-intel-session-controls-portability — binary fixtures need binary writers
- **Date:** 2026-10-28
- **Error patterns:** Windows x64/arm64 retroarch.session_helpers expected 112184356382ae07, actual 8bb25531c6274ace
- **Root cause(s):** CMake text fixture output plus Windows newline conversion inserted CR before LF; independent intended/expanded byte models produce exactly the expected/actual hashes with the correct native reader.
- **Fix:** Reuse owned marker_write(wb) for a fixed binary vector and two isolated mutations, require exact HEX before hashing, keep frozen hash and CR/LF/high-byte coverage unchanged. Only two test files change; no deadline, emulation, header or fingerprint change.
- **Files changed:** tests/retroarch/frame_index.c, tests/cmake/session_helpers.cmake, .planning/STATE.md, resolved nightly-intel-session-controls session
- **Why not caught:** Initial local POSIX execution preserved LF and the fixture lacked a byte assertion; actual Windows CI caught the writer defect. First negative-control draft also used text output, so both negative vectors were moved to wb before acceptance to avoid confounded Windows mutations.
- **Recurrence guard:** retroarch.session_helpers exact HEX gates positive 010a0d7f80ff, CRLF mutant 010d0a0d7f80ff and high-byte-only mutant 010a0d7f80fe; fresh child processes must reject both mutants before hash comparison. Native-writer CR insertion mutant fails this gate; restoration and fresh full CI pass 688/688 plus packages. Prior repair Intel Nightly 40736138106 passes all three session tests; latest fixture source still requires fresh hosted validation.
---

## phase21-hosted-ci-regressions — hot-path specialization and bounded identity work
- **Date:** 2026-10-29
- **Error patterns:** Ir/frame +2.88/+3.10/+3.19 percent, +1% performance gate, phase21.hosted_evidence.controls 120s timeout, Windows Git Ignoring path target.txt
- **Root cause(s):** Enlarged cycle bookkeeping prevented GCC14 specializing dynamic PPUSTATUS work and folding an unobserved timestamp; ordinary opcodes carried generic exceptional IRQ bookkeeping. Three Git subprocesses per tracked path across three inventories amplified slow-host launch cost. The new conflict fixture also emitted native Windows CRLF into Git index-info, so its intended unresolved rows were ignored.
- **Fix:** Constant ordinary/status timing paths, coalesced unobserved ticks, transition-first NMI handling and operation-local IRQ acceptance preserve all histories. Batch independent Git inventories/raw hashes; add a measured 12-process budget and semantic neighbors. Write the conflict protocol with explicit LF and verify raw bytes plus exact actual stages. Original +1% and 120s gates remain intact.
- **Files changed:** src/bus.c, src/cpu.c, tests/cmake/phase21_source_identity.cmake, tests/cmake/phase21_source_identity_controls.cmake, tests/cmake/phase21_hosted_evidence_controls.cmake, README.md, include/nesturbator.h, tests/accuracy/baseline-metadata.txt, tests/accuracy/measurements.md, Phase 21 delivery/state/debug records
- **Why not caught:** Local macOS correctness tests did not measure GCC14 instruction counts or slow platform launch cost; no process work budget existed. The new fixture initially tested rejection without first proving protocol bytes and exact constructed stages. Actual hosted gates caught these defects.
- **Recurrence guard:** phase21.hosted_evidence.controls caps measured Git launches at 12, rejects the original 1285-launch helper, and retains every inherited negative. phase21_source_identity_controls.cmake covers independent committed/index/raw identities, modes, symlinks, missing/conflicted inputs and exact LF/stages; a CRLF mutant is killed. A scratch source-point revert fails M4 at +2.20%, reapplication restores +0.98857%. Final source 54c7db2 passes local CI695/ASAN686/vectors258/MMC313/hygiene13 and actual CI40929011059/1 all18 jobs plus Nightly40929010997/1 all7 work jobs. All six controls finish below120s and raw per-ROM +1% gates pass.
---

