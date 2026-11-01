# Phase24 resume — local acceptance complete, hosted evidence pending

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

The owner acts next only to authorize the hosted PR/check path. Exact next
OpenGSD command after that authorization: `$gsd-ship` — prepare the PR and
hosted review/check run. Context can be cleared; read STATE and this resume
file before continuing.
