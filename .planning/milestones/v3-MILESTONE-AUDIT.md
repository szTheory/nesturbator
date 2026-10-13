---
milestone: v3
audited: 2026-10-13T00:00:00Z
status: tech_debt
scores:
  requirements: 18/18
  phases: 5/5
  integration: 11/12
  flows: 5/5
gaps:
  requirements: []
  integration: []
  flows: []
tech_debt:
  - phase: 11-tune-up-and-state-readiness
    items:
      - "Review IN-01..IN-03 open (info): power-on duplication in load_cartridge, zeroed av info on failed create, long comment line"
      - "11-01..11-03 SUMMARY frontmatter uses `requirements:` instead of `requirements-completed:` (TUNE-07, TUNE-09)"
  - phase: 13-rewind-and-run-ahead-in-retroarch
    items:
      - "Review WR-01 open: state-net frame-chain check cannot detect a dropped or doubled FRAMEADVANCE (tests/retroarch/run_retroarch.cmake)"
      - "Review WR-02 open: state_session.sh sends most commands once though a command can be missed"
      - "Review WR-03 open: special-case ROM selection matches a substring of the full path (libretro_host.c, test_state_runahead.c)"
      - "Review IN-01, IN-02, IN-04, IN-05 open (duplicate hash helpers, replay_main.h includes, unused allowlist entries unreported, .info description omits MMC3); IN-03 closed by quick task 261013-cvm (state-fuzz timeout measured)"
      - "13-08 SUMMARY has no requirements-completed key"
  - phase: 14-runner-process-contract
    items:
      - "Review IN-01..IN-04 deferred (cosmetic)"
  - phase: 15-speed-and-close-out
    items:
      - "Review IN-01..IN-03 open (info)"
      - "15-01..15-08 SUMMARY frontmatter has no requirements-completed key (PERF-02, PERF-03, TUNE-10)"
      - "actions/create-github-app-token v3.2.0 vs v3.3.0 left to Dependabot"
  - phase: integration
    items:
      - "runner/save.h:36 comment says state writes use the helper 'later'; they already do (runner/main.c:1020)"
      - "RetroArch state, rewind and run-ahead e2e runs only on macos-15; other platforms run libretro.serialize and libretro.runahead only"
      - "libretro adapter does not read nesturbator_save_generation; the libretro API has no such signal and the host owns .srm writes (README), so STATE-05 is met in the library and runner"
---

# Milestone v3 audit: Save states and rewind

**Status: tech_debt.** All 18 requirements are satisfied, all five phases passed verification, and main at b0ec7fb is green on CI, Nightly and Release. Nothing blocks completion. The open items below are review warnings and info findings that the phases deliberately left open, plus bookkeeping.

## Phases

| Phase | Verification | Notes |
|---|---|---|
| 11 Tune-up and state readiness | passed (7/8, the eighth was the hosted MMC3 round trip) | The `retroarch-e2e` MMC3 leg has since passed on every PR, including CI run 38407083206 on b0ec7fb |
| 12 Save states in the library | passed 5/5 | Six-platform hash equality confirmed by the hash-equality job on later heads |
| 13 Rewind and run-ahead in RetroArch | passed 4/4 | Three review warnings open (test robustness) |
| 14 Runner process contract | passed 4/4 | Warnings fixed in 15-01 |
| 15 Speed and close-out | passed 4/4 | WR-01..04 fixed after verification (15-REVIEW-FIX.md, quick task 261013-cvm) |

## Requirements (three-source cross-check)

| Requirement | Phase | Verification | SUMMARY `requirements-completed` | REQUIREMENTS.md | Final |
|---|---|---|---|---|---|
| TUNE-07 | 11 | SATISFIED | listed as `requirements:` (11-01) | [x] | satisfied |
| TUNE-08 | 11 | SATISFIED (hosted run since green) | 11-04 | [x] | satisfied |
| TUNE-09 | 11 | SATISFIED | listed as `requirements:` (11-02, 11-03) | [x] | satisfied |
| STATE-01 | 12 | SATISFIED | 12-01, 12-03 | [x] | satisfied |
| STATE-02 | 12 | SATISFIED | 12-01, 12-02, 12-10 | [x] | satisfied |
| STATE-03 | 12 | SATISFIED | 12-02, 12-10 | [x] | satisfied |
| STATE-04 | 12 | SATISFIED | 12-06 | [x] | satisfied |
| STATE-05 | 12 | SATISFIED | 12-07 | [x] | satisfied |
| RUN-01 | 12 | SATISFIED | 12-09 | [x] | satisfied |
| LR-01 | 13 | SATISFIED | 13-01 | [x] | satisfied |
| LR-02 | 13 | SATISFIED | 13-01, 13-02, 13-03 | [x] | satisfied |
| LR-03 | 13 | SATISFIED | 13-06, 13-07 | [x] | satisfied |
| LR-04 | 13 | SATISFIED | 13-04, 13-05 | [x] | satisfied |
| RUN-02 | 14 | SATISFIED | 14-02..14-08 | [x] | satisfied |
| RUN-03 | 14 | SATISFIED | 14-01..14-05, 14-07, 14-08 | [x] | satisfied |
| PERF-02 | 15 | SATISFIED | missing (plans 15-04..06 claim it) | [x] | satisfied, checked by hand |
| PERF-03 | 15 | SATISFIED | missing (plans 15-02, 15-03 claim it) | [x] | satisfied, checked by hand |
| TUNE-10 | 15 | SATISFIED | missing (plans 15-01, 15-07, 15-08 claim it) | [x] | satisfied, checked by hand |

The five requirements without a `requirements-completed` entry were checked by hand. Plan frontmatter claims each one, the verification cites evidence, and the hosted runs are green. No requirement is orphaned.

## Integration (gsd-integration-checker)

11 of 12 wiring checks pass. Battery-save and state-file writes go through the one atomic helper (`runner/main.c:1020`, `:1470` → `runner/save.c`). Exit status 5 and the stop path use the single table in `runner/exit_status.h`, with no duplicated literals. libretro serialize wraps the library state API (`libretro/libretro.c:256-268`). `--bench-state` uses the same API. Every v3 function and status code is in the public header and the README, and `runner.options.readme` holds `--help` and the README together.

The one failed check, that the adapter does not consume the battery generation, does not break a requirement. libretro has no generation signal, the adapter does no file I/O, and the README says the host writes the `.srm`. STATE-05's no-rewrite rule is met in the library and in the runner, which consumes the counter at `runner/main.c:530`. It is recorded as tech debt.

## E2E flows

| Flow | Status | Wired to |
|---|---|---|
| Host save, load and continue through the library | complete | `core.state*` tests, header C/C++ consumers |
| Runner `--state-at-frame` → `--load-state` with pins | complete | `runner.state.*`, `state.format_pin`, `hashes.txt` |
| RetroArch save, load, rewind and run-ahead | complete (macOS) | `retroarch-e2e` + `state_session.sh`, `libretro.serialize`, `libretro.runahead` |
| SIGTERM mid-run flushes `.sav` and exits per the table | complete | `runner.signal.term*` |
| Per-PR perf compare | complete | `perf` job, `perf_compare.cmake`, `perf.gate_selftest.*` |

## Nyquist

Skipped: no active validate-phase hook at verify:post.

## Tech debt

See the `tech_debt` frontmatter: 17 items across Phases 11, 13, 14 and 15 and the integration check. None changes released behaviour. The Phase 13 test-robustness warnings (WR-01..03) are the most substantive.
