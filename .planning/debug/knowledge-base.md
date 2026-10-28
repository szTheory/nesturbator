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

