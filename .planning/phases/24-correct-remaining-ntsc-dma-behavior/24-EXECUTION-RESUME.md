# Phase24 resume — local acceptance complete, verification gap planning next

Read `.planning/STATE.md` and
[24-ACCEPTANCE.md](24-ACCEPTANCE.md) first. The local Plan24-04 work is done;
do not repeat completed correction plans or ask again for local CMake
approval.

## Final local result

The behavior14 candidate (`a12a79a2cbf0d872150ada9c91754b5345ded38a`,
format6 / ABI1) passed all five selected AccuracyCoin rows at the unchanged
pin. All119 previous current-pin protected PASS keys remain passing; DMA +
`$4015` Read, Misaligned OAM DMA, and INC `$4014` also pass. The complete
campaign observed all146 scored rows terminal, with126 PASS,20 FAIL, no
skip/not-run/timeout, and5 inventory-only DRAW rows. All707 CTest entries
passed in517.82 seconds, and packaging generated library, runner and libretro
macOS arm64 archives. The exact provenance and raw evidence links are in
[24-CI-RUN-20261101.md](24-CI-RUN-20261101.md).

The local run used `scoreboard-main` fallback for protected observations. It
does not establish independent cold-main authority. Hosted current-head lanes,
Linux/macOS/Windows x64/arm64 frame/PCM/transition hash equality, and unchanged
Cachegrind `+1%` evidence remain outstanding. Branch-wide inherited planning
whitespace findings remain documented; most are outside Phase24 artifacts.

## Authority and next action

The owner granted durable authority for local CMake configure/build/test and
workflow commands, including full CI and its configured campaign fixtures.
No per-run approval is needed. Keep the existing39-minute ceiling and inner
caps; investigate a real timeout/interruption before any retry. Hosted
workflow triggers, pushes, PRs, merges, and releases are separate remote
actions and were not authorized by that local-CMake instruction.

Phase verification returned `gaps_found` (8/9). The next command is
`$gsd-plan-phase 24 --gaps`, which creates a gap-closure plan from the
verification report. This planning step does not trigger hosted checks. The
owner must still separately authorize remote PR/check actions before
`$gsd-ship`. Context can be cleared; read STATE and this resume file first.

## Latest phase review and verification — 2026-11-01

The configured code review recorded one open warning: `tests/cpu/test_bus.c`
can silently overflow its fixed trace buffer before final DMA assertions.
See [24-REVIEW.md](24-REVIEW.md) and [24-REVIEW-DISPOSITION.md](24-REVIEW-DISPOSITION.md).

Goal verification scored 8/9 and returned `gaps_found`. DMA-03 still needs
independent cold protected-main evidence, current-head hosted CI, Linux/macOS/
Windows x64/arm64 frame/PCM/transition hash equality, and the unchanged
Cachegrind `+1%` result. The verifier also flagged a stale README acceptance
sentence, inherited branch whitespace, and three plan prohibitions without
verification tiers. See [24-VERIFICATION.md](24-VERIFICATION.md).

The next OpenGSD command is `$gsd-plan-phase 24 --gaps`; it plans closure for
the verified gap and does not trigger hosted checks. Remote PR/check actions
remain outside authority until separately authorized. Context may be cleared;
read STATE, this resume file, and [24-ACCEPTANCE.md](24-ACCEPTANCE.md) first.
