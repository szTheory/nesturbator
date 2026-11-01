# Plan24-04 acceptance outcome — local gates passed; phase still open

The Plan24-04 local acceptance run is complete. On2026-11-01,
`cmake --workflow --preset ci` passed all707 CTest entries in517.82 seconds,
packaged the library/runner/libretro outputs, and produced a complete behavior14
AccuracyCoin campaign. All five selected DMA rows passed; all119 prior
current-pin protected PASS keys remain passing. The report observed146/146
scored rows terminal, with126 PASS,20 FAIL, no skipped/not-run/timeouts and5
inventory-only DRAW rows. See [dated run record](24-CI-RUN-20261101.md).

The source correction and owned wrong-model tests are complete at behavior14 /
format6 / ABI1. The local campaign measurements and canonical scoreboard now
reflect the actual report. This does not complete Phase24: local protected
tests used the checkout `scoreboard-main` fallback, and independent cold-main,
current-head hosted lanes, six-platform frame/PCM/transition equality, and the
unchanged Cachegrind `+1%` evidence remain outstanding. Existing branch-wide
planning whitespace debt also remains outside this local run.

The owner’s standing authority for local CMake commands is durable; no
per-run approval is needed. Hosted workflow triggers, pushes, and PR creation
have separate remote effects and were not authorized. The owner acts next if
they want those checks run. The exact next OpenGSD command after that
authorization is `$gsd-ship`; it prepares a PR and the hosted review/check
path. Context may be cleared after reading STATE and
[24-EXECUTION-RESUME.md](24-EXECUTION-RESUME.md).
