# Phase 15 close-out record

Date: 2026-10-13

Review means confirm and record (D-21). Nothing here was bumped by hand.

## Pins

Every `uses:` line in `.github/workflows/*.yml`, compared with the latest release found with `gh api repos/<owner>/<repo>/releases/latest` on 2026-10-13.

| Action | Pinned version | Pinned SHA | Latest release | Status | Source |
|---|---|---|---|---|---|
| actions/attest | v4.2.2 | 1e69f48acb82 | v4.2.2 (2026-08-04) | latest | https://github.com/actions/attest/releases |
| actions/checkout | v7.0.1 | 3d3c42e5aac5 | v7.0.1 (2026-07-20) | latest | https://github.com/actions/checkout/releases |
| actions/create-github-app-token | v3.2.0 | bcd2ba492189 | v3.3.0 (2026-10-12) | behind | https://github.com/actions/create-github-app-token/releases |
| actions/download-artifact | v8.0.2 | 9000827ccba6 | v8.0.2 (2026-10-07) | latest | https://github.com/actions/download-artifact/releases |
| actions/upload-artifact | v7.0.2 | cf430e030ddb | v7.0.2 (2026-10-07) | latest | https://github.com/actions/upload-artifact/releases |
| googleapis/release-please-action | v5.0.0 | 45996ed1f6d0 | v5.0.0 (2026-04-22) | latest | https://github.com/googleapis/release-please-action/releases |

Behind: `actions/create-github-app-token` only. v3.3.0 was published the day before this review. `gh pr list --author app/dependabot --state open` shows no open Dependabot pull request yet, so there is nothing for Plan 08 to merge now; Dependabot owns the bump and its pull request is merged when it appears. Nothing is hand-bumped.

## Runner labels

| Label | Status | Next retirement | Source |
|---|---|---|---|
| ubuntu-24.04 | current | none announced | https://github.com/actions/runner-images |
| ubuntu-24.04-arm | current | none announced | https://github.com/actions/runner-images |
| macos-15 | current | leaves at the macOS 27 GA | https://github.com/actions/runner-images |
| macos-15-intel | current, the last x86_64 image | available until August 2027 | https://github.com/actions/runner-images/issues/13046 |
| windows-2025 | current, Visual Studio 2026 | none announced | https://github.com/actions/runner-images |
| windows-11-arm | current, now on the Visual Studio 2026 image `windows-11-vs2026-arm64` | none announced | https://github.com/actions/runner-images/issues/14602 |

Evidence for `windows-11-arm`: our own run 38329002566, job 115074198221, printed `Image: windows-11-vs2026-arm64`, `Version: 20261004.176.1` and a `DevEnvDir` under `Visual Studio\18`. Issue 14602 is still open without a closing notice, so the job log is the completion evidence.

## AccuracyCoin

Pinned: `673ef550db296136d52229961e7d39366116882a` (`tests/roms/manifest.txt`). Upstream is 9 commits ahead (`gh api repos/100thCoin/AccuracyCoin/compare/673ef550...main --jq .ahead_by`). Compare: https://github.com/100thCoin/AccuracyCoin/compare/673ef550...main

Kept. A bump re-baselines the ROM sha256, the scoreboard and the hashes, which does not serve closing the milestone. The bump is backlog item Phase 999.2 in ROADMAP.md.

## Timeouts

Filled in by Plan 08.

## Docs audit

One-off audit of README.md and include/nesturbator.h against the Phase 12 to 15 code (D-22). Comments only; no behaviour changed.

| Location | Claim | Evidence | Result |
|---|---|---|---|
| README.md:6-13 | status paragraph says v2 shipped and v3 is closing | all of Phases 12 to 15 are built; the text described v3 as future | fixed in a3a89c4 |
| README.md:1532 | section "What it will be" in the future tense | the three deliverables exist (library, runner, libretro core) | fixed in a3a89c4 |
| README.md:857-1000 (Save states) | state header is 64 bytes, fields at offsets 0 to 48 | include/nesturbator.h:481-491 layout comment lists the same offsets | matches |
| README.md "Status codes" | statuses 0 to 12, state refusals 8 to 12 | include/nesturbator.h:71-101 enum; test `state.status_table` | matches |
| include/nesturbator.h:436-495 | load checks in order, all or nothing | `core.state_reject` test; README lists the same order | matches |
| include/nesturbator.h:67 | NESTURBATOR_STATE_FORMAT_VERSION 1 | README format table "1: first release that contains save states"; `state_format_pin.cmake` | matches |
| include/nesturbator.h:56 and README.md:25 | behaviour revision is 6 | `#define NESTURBATOR_BEHAVIOUR_REVISION 6` | matches |
| include/nesturbator.h:43-45 | version 0.1.9 on release-please marker lines | `x-release-please-*` markers; test `version_consistency` | matches |
| README.md "The runner" `--state-at-frame`, `--hash-state`, `--load-state` | option semantics and exit 5 on a refused state | runner/main.c option parsing; runner/exit_status.h `RUNNER_EXIT_STATE_REJECTED`; `runner_state.cmake` | matches |
| README.md "The runner" `--bench`, `--bench-state` | range 1 to 1000000, conflicts exit 2, stderr line shapes | runner/main.c:396 bound; `runner_bench.cmake`; `runner.options.readme` | matches |
| README.md "Stopping the runner" | finish the frame, write the save, exit 0, second signal ends at once | runner/stop.c handler; `runner_stop.cmake`, `runner.signal.*` | matches |
| README.md "Stopping the runner" | an inherited ignore is honoured; errno preserved (WR-01) | runner/stop.c:5,23-34 `SIG_IGN` put back, errno saved | matches |
| README.md "Stopping the runner" | launcher must keep reading stdout (WR-03) | README paragraph present at "A launcher on Linux or macOS must keep reading" | matches |
| README.md "Exit status" | table 0, 1, 2, 4, 5; 3 and 77 reserved | runner/exit_status.h `RUNNER_EXIT_TABLE`; test `runner.exit.table` | matches |
| README.md "The libretro core" | serialize size, no quirks, `savestate_features = "deterministic"` | libretro/nesturbator_libretro.info; tests `libretro.serialize`, `libretro.no_quirks`, `libretro.info_claims` | matches |
| README.md "The libretro core" | module exports only the 25 `retro_*` functions | `RETRO_API` count in libretro/libretro.h is 25 | matches |
| README.md "Continuous integration" | `perf` job: Cachegrind, 60 and 360 frames, 1.00% gate, part of CI required | .github/workflows/ci.yml `perf` job, `needs: [... perf]` at line 417, `-DMODE=gate` at line 397 | matches |
| README.md "Performance allow file" | line format, expiry within 60 days, UNSTABLE fails | tests/cmake/perf_compare.cmake; `perf.gate_selftest.*` | matches |
| README.md "Continuous integration" | hash-equality inventory is 114 keys | `grep -c . tests/runner/hashes.txt` is 114 | matches |

Findings not fixed (outside README and header comments):

- `libretro/nesturbator_libretro.info` `description` lists "NROM, MMC1, UxROM, CNROM and AxROM" and omits MMC3 (mapper 4), which the core supports. Text only, no behaviour; left for a later change because this plan edits README and header comments only.
