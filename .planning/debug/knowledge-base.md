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

