# Phase 5: Tune-up and v1 debt - Research

**Researched:** 2026-10-10
**Domain:** CTest/CMake presets and GitHub Actions hygiene, CMake-script policy tests, a synthetic iNES builder, and an NES soft reset (6502 reset sequence, 2A03 APU reset, 2C02 reset flag) in a deterministic C17 core
**Confidence:** HIGH for CI, policy and code-seam findings (read in this session, or measured); MEDIUM for the hardware reset details (NESdev wiki, no hardware or test ROM run here); one hardware conflict is flagged in Open Questions.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

#### Local RetroArch tests (TUNE-05)
- **D-01:** Remove the `retroarch.testframe` and `retroarch.game` CTest
  registrations and the `retroarch` label. Keep `compare_frame`,
  `retroarch.compare` and `retroarch.compare.cli`; they run on every platform.
- **D-02:** `tests/retroarch/run_retroarch.cmake` runs only in the hosted
  `retroarch-e2e` job. Delete the skip macro, the `nesturbator-skip:` line, the
  exit-77 branch, the `NESTURBATOR_RETROARCH` fallback and the `REQUIRED`
  switch. `RETROARCH`, `ROM`, `FRAME`, `EXPECTED_VERSION` and `VERSION_PLIST`
  become mandatory, and a non-macOS host or a missing app fails.
- **D-03:** The hosted step stays a plain `cmake -P` step, minus
  `-DREQUIRED=ON`. Drop the `NESTURBATOR_RETROARCH: /tmp/...` env workaround
  from `ci.yml`. The job still runs `cmake --workflow --preset ci`.
- **D-04:** Remove the asan test preset's `exclude label retroarch` filter.
  In `tests/cmake/vector_result_policy.cmake`, rename the self-test fixture
  that borrows the `retroarch.testframe` name to a synthetic name. Rewrite the
  README's "Try it in RetroArch" section to keep the manual `RetroArch -L ...`
  line and to say that the hosted job is where RetroArch is checked.

#### Tests never skip (TUNE-05)
- **D-05:** New `tests/cmake/skip_policy.cmake`, registered as `policy.no-skip`
  with no label, so every unfiltered preset runs it. It reads
  `ctest --test-dir <build> --show-only=json-v1` with `string(JSON)` and fails,
  naming the test, when any test has `SKIP_RETURN_CODE`,
  `SKIP_REGULAR_EXPRESSION` or `DISABLED`. A ctest error, invalid JSON or an
  empty test list also fail. There is no allowlist.
- **D-06:** Its self-test follows the house mutation pattern. It must reject
  each of the three properties, invalid JSON and an empty list, and accept the
  clean inventory. One case builds a throwaway one-test project with
  `SKIP_RETURN_CODE 77` and runs real `ctest` on it, so the property names are
  checked against CTest's actual output on every runner.
- **D-07:** A test that cannot run somewhere is not registered there (`if()`
  in CMake). A check that needs a host app lives in a required CI job, as
  `retroarch-e2e` does. Add one README line and one `tests/CMakeLists.txt`
  comment: tests never skip, and unrunnable tests are not registered.

#### Release-policy exactness (TUNE-03)
- **D-08:** `tests/cmake/release_policy.cmake` strips comments, then scopes the
  check to the `publish` job's own lines: from `^  publish:$` to the next job
  key. Copy the line scanner from `nightly_workflow_policy.cmake`. This also
  fixes the current `SUBSTRING ... -1`, which lets any later text in the file
  count as the publish job.
- **D-09:** The `publish` job's `needs:` and `if:` lines must each appear
  exactly once and equal canonical strings. So must the `ci` job's, because a
  skipped `ci` is what holds publish back. The canonical strings are CMake
  variables at the top of the script, with a comment to edit them together
  with `release.yml`. Write no YAML parser, no golden file and no hash.
  Equivalent spellings (`${{ }}`, quoting, `>-`) are rejected on purpose.
- **D-10:** Inside the publish job, reject `continue-on-error`, any step-level
  `if:`, and YAML anchors, aliases or merge keys. Exactly one job may run
  `gh release edit ... --draft=false`, and it must be `publish`.
- **D-11:** Mutation cases include at least: appended `|| always()`,
  `&& !cancelled()`, `|| failure()`, a `${{ }}`-wrapped form, a duplicate
  `if:`, a dropped or reordered `needs`, the `ci` job's `if:` widened, a
  step-level `if: always()`, `continue-on-error: true`, a second publishing
  job, the conditions present only in comments, and an anchor or alias. The
  real workflow passes. A replace helper fails when a mutation changes nothing.

#### Soft reset (TUNE-06)
- **D-12:** New `nesturbator_status nesturbator_reset(nesturbator *inst)`,
  appended to the header with ABI unchanged. NULL gives
  `NESTURBATOR_ERR_ARGUMENT`. With no cartridge it is a no-op that returns
  `NESTURBATOR_OK`. It is called between frames. It never allocates, frees or
  memsets the instance. `retro_reset()` calls it when an instance exists. —
  **Reversibility:** one-way — new public symbol in a released MIT library;
  removing or changing it breaks embedders.
- **D-13:** Kept: CPU RAM, PRG-RAM, CHR-RAM, nametable and palette RAM, OAM,
  A/X/Y, the PPU's scanline and dot, and host input state. Cleared: controller
  strobe and shift, pending OAM DMA, and the DMC DMA latches.
- **D-14:** CPU: `I` set, then a 7-cycle reset sequence through the bus. Five
  side-effect-free stack-page reads take S down by 3, then `$FFFC`/`$FFFD` are
  read through `nesturbator__bus_read`. `jammed` is cleared. There is no dummy
  read at PC. `ticks` and `frame_number` stay monotonic since create.
- **D-15:** PPU: catch up with `nesturbator__ppu_run_until` first. Clear
  `control`, `mask`, the w latch, `t`, `fine_x`, the read buffer and
  `odd_frame`, and the CPU's NMI latches. `v`, `status`, `oam_addr` and the
  memories are kept. A `reset_flag` drops `$2000`, `$2001`, `$2005` and `$2006`
  writes until scanline 261 dot 1, the same point where vblank clears. The
  window is 0 to about 29,780 CPU cycles depending on where reset lands. Other
  registers work normally.
  *(D-13 and D-15 revised at plan review 2026-10-10 by the owner: the PPU
  restarts at the top of the picture per NESdev, scanline 0 dot 0, so the
  window is the documented interval. See "Reset facts confirmed against
  NESdev" and Open Question 1.)*
- **D-16:** APU: write `$4015 = 0` through the existing path, clear the frame
  and DMC IRQs, `dmc.output &= 1`, and set triangle phase to 0. Re-apply the
  last `$4017` mode near the reset sequence. The synth and filter history are
  kept, so the output has no step. The phase researcher confirms the APU
  reset details and the `$4017` timing on nesdev's APU pages, because these
  came from memory.
- **D-17:** Load and power-on are unchanged. v1 hashes do not move and
  `NESTURBATOR_BEHAVIOUR_REVISION` stays 4. Power-on has no write-ignore
  window and no startup sequence. The README states this.
- **D-18:** The proof is `tests/core/test_reset.c` on a synthetic NROM image
  with a trainer, so PRG-RAM exists. It covers kept RAM, the vector, S-3, I,
  JAM cleared, `$4015` reading 0, writes ignored inside the window and landing
  after it, a reset placed in vblank, no cartridge, NULL, and equal hashes from
  two instances. `tests/libretro/libretro_host.c` compares frames after
  `retro_reset` with a direct-API instance that runs the same frames, reset
  and frames. No runner flag is added.

### Claude's Discretion
- The `execution.jobs` value in the test presets, and isolating any shared
  temporary paths that parallel runs expose.
- The `mappers` and `games` CTest labels, and whether the nightly flake job
  reuses `nightly.yml`.
- How compile share is measured, and the ccache call: research's 40% rule,
  with `actions/cache` SHA-pinned only if adopted.
- Action pin bumps (upload-artifact v7.0.2, download-artifact v8.0.2,
  resolved from tags) and the AccuracyCoin pin review.
- The synthetic iNES builder in C test code, built so Phases 7 to 9 reuse it.
  It serves the trainer image for TUNE-04 and the reset image for TUNE-06.
- Header, README and declaration-baseline updates that go with the new API
  (`tests/core/test_api.c`, `vector_api_policy.cmake`, `tests/header/*`,
  global-symbol checks).

### Deferred Ideas (OUT OF SCOPE)
- Power-on write-ignore window and a 7-cycle startup sequence on load. This changes v1 hashes, so it would need a behaviour-revision bump in a later accuracy phase (SEED-003).
- `retroarch-e2e` reusing build artifacts instead of rerunning the suite (STACK.md, about 80 s of runner time).
- `cpu_reset` and `apu_reset` test ROMs as scoreboard rows. They need a reset request the runner does not issue.
- Freezing the publish job's `permissions` and `environment` in the release policy.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| TUNE-01 | Test presets run CTest in parallel; nightly `--repeat until-fail:3 --schedule-random`; record slowest leg before/after; decide ccache from compile share | "CI measurements" below: before-numbers recorded, compile share is 4-22% (ccache: do not adopt), 70% of the slowest leg's test time is one test (`vectors.registration_policy`), so add `COST` as well as `execution.jobs`. Nightly job shape and the `nightly_workflow_policy.cmake` constraints it must satisfy are listed. |
| TUNE-02 | Action pins and runner labels current; AccuracyCoin pin recorded as reviewed; README and header comments describe released code | Resolved SHAs for upload-artifact v7.0.2 and download-artifact v8.0.2; the pin also lives in `tests/cmake/nightly_workflow_policy.cmake` (3 places). Runner labels checked against runner-images README. AccuracyCoin pin vs upstream HEAD recorded. Stale-comment list given. |
| TUNE-03 | Release-policy self-test fails on any extra condition in the publish gate | Exact current gate lines quoted; line-scanner pitfalls (`;` and `[` in list splitting) verified; mutation list from D-11 mapped to checks. |
| TUNE-04 | Synthetic trainer-bearing image: runner and libretro test program give equal frames | Existing `libretro_host.c` machinery (runner spawn, `compare_with_ppm`) and the `make_trainer_image` copy in `test_cartridge.c`; builder design and a trainer-executing program that makes the frame depend on the trainer. |
| TUNE-05 | Local RetroArch tests run or are removed; a CI job fails when an expected test skips | D-01 to D-07 mapped to exact files; the hosted job's suite run currently reports both retroarch tests `***Skipped`; `ctest --show-only=json-v1` property names verified by running ctest; a hand-written `CTestTestfile.cmake` is a valid "real ctest" fixture. |
| TUNE-06 | `nesturbator_reset()`; CPU RAM/cart RAM kept, vector with S-3, APU silenced, PPU write-ignore; `retro_reset()` calls it; docs | Exact code seams (`instance.c`, `cpu.c`, `ppu.c`, `apu.c`, `internal.h`, `libretro.c`), link-list traps (`bus.unit`, `cpu.vectors`), APU/CPU/PPU reset facts from NESdev, and one hardware conflict to settle (Open Question 1). |
</phase_requirements>

## Project Constraints (from CLAUDE.md)

Read from `CLAUDE.md` and `.claude/CLAUDE.md` this session.

- Clean room: never open GPL/LGPL emulator source; implement from hardware documentation (NESdev wiki) and test behaviour. Nothing in this research opened emulator source. The NESdev wiki and blargg readmes are documentation.
- Core is C17, extensions off, integer-only, all state in the instance, links only the C memory functions. No mutable static or global in the core (`abi.global_symbols`), no float (`nofp` preset).
- Every behaviour change lands with a test run by `cmake --workflow --preset ci`, and the README and the public header's comments are updated in the same change. Checks are automated; none waits on a person.
- No ROM bytes except `tests/roms/manifest.txt` entries. Synthetic images are built in C test code and need no manifest entry (REQUIREMENTS.md).
- No personal paths, emails or names in tracked files (`scripts/hygiene.sh`).
- Style: small modules, plain control flow, fixed-width integer types; a hardware comment says what the hardware does and cites its source; ponytail ladder (remove the need, reuse, standard library, platform, minimum new code). Owner preference (CONTEXT): another copy-paste beats another dependency.
- One GSD step at a time; work on a phase branch; merge by PR with a Conventional Commit title.
- Compiler flags: `-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wstrict-prototypes -Wmissing-prototypes -Wvla` (+ `/W4` MSVC), warnings are errors in presets [VERIFIED: CMakeLists.txt `nesturbator_warnings`]. Every new internal function needs a prototype in `src/internal.h`; every cast in test code must be explicit.
- C sources are checked with clang-format 18 (`hygiene.format`); `/opt/homebrew/opt/llvm@18/bin/clang-format` exists on this Mac [VERIFIED: ls this session].
- `workflow.nyquist_validation` is `false` and `security_enforcement` is `false` in `.planning/config.json` [VERIFIED: file read], so the Validation Architecture and Security Domain sections are omitted by design. The requirement-to-verification map is folded into the Phase Requirements table above and the per-area sections below.

## Summary

Phase 5 is six independent pieces of work on a library that already works. Four of them are policy/CI edits that touch only CMake scripts, presets and workflows (TUNE-01, -02, -03, -05); one is test-only plus a shared C helper (TUNE-04); one adds a public API and three small internal reset functions (TUNE-06). Nothing needs a new dependency: the answer to "should we add ccache" is no, by measurement.

**The measurement changes the TUNE-01 plan.** On the last green `main` run (38011892333) the slowest leg is `build (macos-15-intel)` at 181 s; its `cmake --workflow` step took 165 s and CTest's own "Total Test time (real)" was 152.2 s, so configure + compile + package is at most 13 s (8%). Across all legs the non-test share of the workflow step is 4 to 22%, far under the 40% rule, so no ccache and no `actions/cache`. But 107.2 s of the 152.2 s (70%) is one test, `vectors.registration_policy` (a full `cmake --preset vectors-full` configure inside CTest), plus 21.1 s for its `.self_test`. Plain `execution.jobs: 4` therefore saves at most `152 - 107 = 45` s on that leg, and only if the long test starts first. Add a `COST` property on the long tests (CTest runs descending COST first in parallel mode; with no cost data a cold CI would otherwise start test #50 after ~49 others). Locally on this Mac (18 cores) serial 37.8 s became 26.0 s at `-j4`, the floor being the 25 s test.

**The skip debt is visible in CI today:** the hosted `retroarch-e2e` job's suite run prints `Test #356: retroarch.testframe ... ***Skipped` and `#357: retroarch.game ... ***Skipped` (job log 114093520598). D-01 to D-07 remove that class of problem; `policy.no-skip` can be tested against real CTest cheaply with a hand-written `CTestTestfile.cmake` (verified: ctest 4.4.3 emits `SKIP_RETURN_CODE`, `SKIP_REGULAR_EXPRESSION` and `DISABLED` in `--show-only=json-v1`).

**Release-policy trap:** the "line scanner" D-08 says to copy splits text with `string(REPLACE "\n" ";" lines ...)`. Verified here: on `release.yml` that yields 190 list elements for a 172-line file because `for c in library runner libretro; do` contains a `;`. For exact-line equality checks, escape `;` (and `[`/`]`) first, or read with `file(STRINGS)`, which kept all 173 lines intact.

**Soft reset** fits cleanly: three internal functions (`nesturbator__cpu_reset` in `cpu.c`, `nesturbator__ppu_reset` in `ppu.c`, `nesturbator__apu_reset` in `apu.c`) orchestrated by `nesturbator_reset` in `instance.c`. Two link-list traps: `cpu.vectors`/`cpu.unit` link only `cpu.c` plus a test bus, and `bus.unit` compiles `bus.c apu.c synth.c cartridge.c ppu.c` directly, so `cpu.c` may call only `nesturbator__bus_read`, and `ppu.c`/`apu.c` may not call `cpu.c` functions. One hardware conflict needed the owner's eye (Open Question 1, now resolved): the NESdev wiki says the PPU "comes out of power and reset at the top of the picture" and gives the ignore window as ~29,658 CPU cycles, whereas the original D-13/D-15 kept the PPU position, giving a 0 to ~29,780 window. The owner revised D-13/D-15 at plan review: the PPU restarts at scanline 0, dot 0.

**Primary recommendation:** Use `execution.jobs: 4` plus `COST` on the three long tests; put the flake hunt in `nightly.yml` as a third job reported through the existing `report` job; reject ccache; write `skip_policy.cmake` and `release_policy.cmake` with `;`/`[`-safe line scanners and real-ctest / real-workflow self-tests; build one shared `tests/ines.h` builder and use it for the trainer and reset images; implement reset as three internal functions plus `nesturbator_reset` exactly per D-12 to D-18, with the PPU restarted at the top of the picture (D-13/D-15 as revised at plan review).

## Architectural Responsibility Map

The system has no browser/API tiers; the owning tiers are the three deliverables, the test harness and CI.

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Soft reset state transition | Core library (`src/`) | Public header (`include/nesturbator.h`) | Rule 5: all state in the instance; hosts only call the symbol. |
| Reset on RetroArch's Reset button | libretro adapter (`libretro/libretro.c`) | Core library | Adapter stays thin: `retro_reset()` calls `nesturbator_reset()` when `inst != NULL`. |
| Runner reset | none | n/a | D-18: no runner flag; the runner has no reset request (deferred idea). |
| Parallel CTest, labels, COST | Build system (`CMakePresets.json`, `tests/CMakeLists.txt`) | CI workflows | Presets own `execution.jobs`; the nightly consumes the preset. |
| Skip prohibition | Test harness (`tests/cmake/skip_policy.cmake`) | CI (runs in every unfiltered preset) | It inspects the registered inventory, so it needs no CI-specific step. |
| Release gate exactness | Test harness (`tests/cmake/release_policy.cmake`) | `release.yml` | The script reads the workflow text; the workflow stays the single source of the gate. |
| RetroArch evidence | Hosted CI (`retroarch-e2e`) | `tests/retroarch/run_retroarch.cmake`, `compare_frame` | A host app cannot be a registered unfiltered test (D-07). |
| Synthetic iNES images | Test code (`tests/ines.h`, new) | `tests/core`, `tests/libretro` | One builder reused by Phases 7 to 9. |
| Flake hunt | Nightly workflow (`nightly.yml`) | `report` rolling issue | Same issue channel as `vectors-full`. |

## Standard Stack

No library is added. The "stack" is the toolchain already in use.

### Core
| Tool | Version | Purpose | Why Standard |
|------|---------|---------|--------------|
| CMake / CTest | min 3.25 (local 4.4.3; runner images 3.31.x/4.x per STACK.md) | presets, `execution.jobs`, `--repeat`, `--schedule-random`, `--show-only=json-v1` | Already the single entry point (`cmake --workflow --preset <lane>`). [VERIFIED: ran `cmake --version`, `ctest --help` here] |
| `string(JSON)` | CMake 3.19+ | read the ctest inventory in policy scripts | Used by `vector_registration_policy.cmake` today. [VERIFIED: file read] |
| GitHub Actions pins | see below | CI | SHA pins required by `hygiene.action_pins`. |

### Action pins to set (resolved from tags this session)
| Action | Current pin | New pin | Evidence |
|--------|-------------|---------|----------|
| `actions/upload-artifact` | `043fb46d1a93c77aae656e7c1c64a875d1fc6a0a` # v7.0.1 | `cf430e030ddbb5b0abf93d22962f4752f3646cd9` # v7.0.2 | `gh api repos/actions/upload-artifact/git/ref/tags/v7.0.2` returned type `commit`, sha `cf430e03...`; the same call for `v7.0.1` returned the current pin, which validates the method. [VERIFIED: gh api] |
| `actions/download-artifact` | `3e5f45b2cfb9172054b4087a40e8e0b5a5461e7c` # v8.0.1 | `9000827ccba6bdab643e8b6fd33ac0654aef8333` # v8.0.2 | Same method; `v8.0.1` returned the current pin. [VERIFIED: gh api] |
| `actions/checkout` | `3d3c42e5aac5ba805825da76410c181273ba90b1` # v7.0.1 | unchanged | `releases/latest` = v7.0.1 (2026-07-20). [VERIFIED: gh api] |
| `actions/attest` | `1e69f48acb82d1966a394da916b4c1698aa569d6` # v4.2.2 | unchanged | latest = v4.2.2. [VERIFIED: gh api] |
| `actions/create-github-app-token` | `bcd2ba49...` # v3.2.0 | unchanged | latest = v3.2.0. [VERIFIED: gh api] |
| `googleapis/release-please-action` | `45996ed1...` # v5.0.0 | unchanged | latest = v5.0.0. The release policy pins this SHA by string too. [VERIFIED: gh api; `release_policy.cmake`] |

Both bumps are patch releases that only change artifact retry behaviour on HTTP 429 and update `@actions/artifact` to 6.3.1 [CITED: GitHub release notes for v7.0.2 and v8.0.2, fetched with `gh api`]. No Dependabot PR is open (`gh pr list` returned `[]`).

**Where each pin lives (the planner must touch all):**
- `upload-artifact@043fb46d...`: `.github/workflows/ci.yml` lines 113, 118, 224; `.github/workflows/nightly.yml` line 95; and, easy to miss, `tests/cmake/nightly_workflow_policy.cmake` line 133 (`string(FIND "${code}" "actions/upload-artifact@043fb46d...")`), plus its fixture on lines 152 and 155. Changing only the workflows makes `hygiene.nightly_workflow_policy` fail with `run-evidence`. [VERIFIED: grep]
- `download-artifact@3e5f45b2...`: `ci.yml` line 166, `release.yml` line 78.

### Runner labels
All six labels in `ci.yml` (`ubuntu-24.04`, `ubuntu-24.04-arm`, `macos-15`, `macos-15-intel`, `windows-2025`, `windows-11-arm`) are listed without a deprecation badge in the runner-images README; only `macos-14` is marked deprecated, and the page gives no retirement notice for `macos-15-intel`. Nothing to change. [CITED: raw.githubusercontent.com/actions/runner-images/main/README.md, fetched this session]. vCPU counts for public repos: 4 for ubuntu-24.04, ubuntu-24.04-arm, windows-2025, windows-11-arm, macos-15-intel; 3 (M1) for macos-15 [CITED: docs.github.com github-hosted-runners].

### AccuracyCoin pin review (record in the phase verification)
- Pin in `tests/roms/manifest.txt`: `673ef550db296136d52229961e7d39366116882a` (commit dated 2026-09-23T17:43:46Z).
- Upstream HEAD on 2026-10-10: `74613de3a7...` (2026-10-08), 7 commits ahead; files changed `AccuracyCoin.asm`, `AccuracyCoin.nes`, `README.md`. Commit subjects include "Added 2 new tests", "Fixed an issue with 'Misaligned OAM Behavior'", "Fixed a minor issue with BG Serial IN", "Prevented potential issues with the stack". [VERIFIED: `gh api repos/100thCoin/AccuracyCoin/compare/...` and `/commits`]
- Recommended disposition: reviewed, not moved. A pin move changes the ROM, `tests/accuracy/scoreboard.txt` and the hash inventory, and STACK.md says a pin move is its own change. Record the above two lines as the evidence.

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| `execution.jobs: 4` only | plus `COST` on long tests | COST is required to get the benefit on a cold CI (no cost data). |
| ccache / `actions/cache` | nothing | Compile share is 4-22% (measured); adding a cache is a new third-party `uses:` for no win. |
| A separate flake workflow file | third job in `nightly.yml` | One workflow means one `report` issue; the policy script already guards that file. |
| `retroarch-e2e` reusing build artifacts | rerun the suite (current) | Deferred by CONTEXT. |

**Installation:** none. **Package Legitimacy:** see the audit section.

## Package Legitimacy Audit

No external package (npm, PyPI, crates) is installed or recommended in this phase. The only third-party references are GitHub Actions already pinned by full SHA; two get patch bumps resolved from first-party `actions/*` repositories with `gh api` (see table above). No `gsd-tools package-legitimacy` run is needed.

| Package | Registry | Age | Downloads | Source Repo | Verdict | Disposition |
|---------|----------|-----|-----------|-------------|---------|-------------|
| (none) | n/a | n/a | n/a | n/a | n/a | n/a |

**Packages removed due to [SLOP] verdict:** none
**Packages flagged as suspicious [SUS]:** none
ccache and `hendrikmuhs/ccache-action` are not adopted, so they are not audited.

## Architecture Patterns

### CI measurements (the "before" record for TUNE-01)

Source: `gh run view 38011892333` (CI on `main`, commit "chore: archive v1 milestone (#24)", 2026-10-10) and its job logs fetched with `gh api --allow-escape-sequences .../jobs/<id>/logs`. [VERIFIED: gh]

| Leg | Job wall (s) | `cmake --workflow` step (s) | CTest "Total Test time (real)" (s) | Non-test remainder (s, share) | Longest test |
|-----|------|------|------|------|------|
| build macos-15-intel | **181** | 165 | 152.21 | 12.8 (8%) | `vectors.registration_policy` 107.21; `.self_test` 21.07 |
| asan (ubuntu-24.04) | 171 | 164 | 151.35 | 12.7 (8%) | `runner.write_hashes` 48.4; `vectors.registration_policy` 48.3 |
| build windows-11-arm | 144 | 110 | 93.12 | 16.9 (15%) | `vectors.registration_policy` 55.41 |
| build macos-15 | 113 | 98 | 87.77 | 10.2 (10%) | n/a |
| retroarch-e2e | 111 | 89 (suite) | 78.65 | n/a | same suite; both retroarch tests `***Skipped` |
| build windows-2025 | 92 | 72 | 58.63 | 13.4 (19%) | `vectors.registration_policy` 37.12 |
| build ubuntu-24.04 | 78 | 67 | 52.16 | 14.8 (22%) | n/a |
| build ubuntu-24.04-arm | 77 | 69 | 64.75 | 4.2 (6%) | n/a |

- **Slowest leg before: `build (macos-15-intel, ci, macos, x64)`, 181 s job wall.**
- **Compile share** (upper bound: configure + compile + package in the workflow step minus ctest time) is 4 to 22% on every leg. The 40% rule is not met anywhere. **Decision: no ccache, no `actions/cache`.** No change to `release-please` or pins for this.
- Local, this Mac (18 cores, build/ci, `-LE retroarch`, 355 tests): serial 37.75 s; `-j4 --schedule-random` 25.97 s; `-j4 --repeat until-fail:3 --schedule-random` 79.83 s with 1065 passes (355 x 3), no failure. Top local tests: `vectors.registration_policy` 25.0 s, `.self_test` 4.8 s, `runner.write_hashes` 1.3 s, `core.frame` 1.2 s. [VERIFIED: ran ctest]

**How to record the "after":** after the PR's CI run finishes, repeat `gh run view <run> --json jobs -q '.jobs[] | "\(.name)\t\((.completedAt|fromdateiso8601)-(.startedAt|fromdateiso8601))"'` and grep the job log for `Total Test time`. Expected, as arithmetic and not a measurement: Intel leg step falls from about 165 s to about 120 to 125 s (`max(107, rest/3)` plus ~13 s), job wall from 181 s to about 135 to 140 s, and the slowest leg may become asan or Intel. [ASSUMED: arithmetic from the table above; the recorded number must come from the real run.]

**Why `vectors.registration_policy` is slow:** the `else` branch of `tests/cmake/vector_registration_policy.cmake` runs `cmake --preset vectors-full` (a full project configure) via `execute_process`, then two `ctest --show-only=json-v1` calls. Its `.self_test` builds 5 synthetic 258-entry JSON inventories and reads them with hundreds of `string(JSON GET)` calls (quadratic in the inventory length), hence 21 s on the Intel runner. Both only need to start first. Rewriting them is out of scope and would touch the nightly's `FULL_ONLY` mode; do not.

### System Architecture Diagram

```
                       Phase 5 data and control flow

 RetroArch "Reset"                         CTest (preset: jobs=4, COST on long tests)
      |                                           |
      v                                           v
 retro_reset()  --inst!=NULL-->  nesturbator_reset(inst)        policy.no-skip ---> ctest --show-only=json-v1
 (libretro.c)                       (instance.c)                      |                  (this build dir)
                                        |                              +--> any of SKIP_RETURN_CODE /
              no cartridge ------------>+--> OK, no-op                      SKIP_REGULAR_EXPRESSION / DISABLED,
                                        |                                  empty list, bad JSON  ==> FAIL, names test
        cartridge loaded:               v
   1. ppu_run_until(ticks)  (catch up, ppu.c)         release_policy.cmake --reads--> release.yml
   2. bus: clear strobe/shift, oam_dma_pending                |
   3. nesturbator__ppu_reset  (ppu.c)                         +--> strip comments (;/[ safe)
        control,mask,w,t,fine_x,read_buffer,odd_frame=0       +--> cut `publish:` .. next job key
        cpu.nmi_* = 0, ppu.reset_flag = 1                     +--> needs:/if: exactly once, == canonical
   4. nesturbator__apu_reset  (apu.c)                         +--> no continue-on-error / step if / anchors
        $4015=0 via apu_write, frame_irq=0, dmc.irq=0,        +--> exactly one `release edit --draft=false`,
        dmc DMA latches=0, dmc.output&=1, triangle.phase=0,         and it is in publish
        re-apply last $4017 mode
   5. nesturbator__cpu_reset  (cpu.c)                  nightly.yml (new job) --> cmake --preset ci; build;
        p|=I; jammed=0; 7 bus cycles (5 stack-page reads,        ctest --preset ci --repeat until-fail:3
        S-=3, then $FFFC/$FFFD via nesturbator__bus_read)        --schedule-random  --> report job (issue)
   ticks += 7*24; frame_number unchanged
        |
        v
 next nesturbator_run_frame: PPU drops $2000/$2001/$2005/$2006 writes until scanline 261 dot 1,
 where status &= 0x1f already happens; reset_flag = 0 there too.

 tests/ines.h (builder) --> test_reset.c, libretro_host.c (trainer + reset parity), later Phases 7-9
```

### Recommended Project Structure (files the planner touches)

```
CMakePresets.json                         # execution.jobs on test presets ci, ci-msvc, dev, asan; drop asan label filter
tests/CMakeLists.txt                      # COST on 3 tests; register policy.no-skip(+self_test), core.reset; no-skip comment
tests/cmake/skip_policy.cmake             # NEW (D-05, D-06)
tests/cmake/release_policy.cmake          # rewrite publish-gate part (D-08..D-11)
tests/cmake/vector_result_policy.cmake    # line 106: rename fixture test name (D-04)
tests/cmake/vector_api_policy.cmake       # refresh PHASE4_DECLARATIONS_SHA256 after the header edit
tests/cmake/nightly_workflow_policy.cmake# upload-artifact SHA (3 places); must still pass with the new job
tests/retroarch/CMakeLists.txt            # delete the two add_test blocks and the label (D-01)
tests/retroarch/run_retroarch.cmake       # rewrite: mandatory inputs, no skip (D-02)
tests/retroarch/test.cfg.in               # first comment mentions retroarch.testframe: reword
tests/ines.h                              # NEW shared synthetic iNES builder
tests/core/test_reset.c                   # NEW (D-18)
tests/core/test_api.c                     # NULL / no-cartridge cases for nesturbator_reset
tests/libretro/libretro_host.c            # trainer parity + reset parity checks
include/nesturbator.h                     # append nesturbator_reset; fix stale create() comment
src/internal.h                            # reset_flag in ppu struct; 3 prototypes; fix stale cpu comments
src/instance.c                            # nesturbator_reset
src/cpu.c, src/ppu.c, src/apu.c           # nesturbator__{cpu,ppu,apu}_reset; PPU write gate and flag clear
libretro/libretro.c                       # retro_reset
.github/workflows/ci.yml                  # pins; drop env + -DREQUIRED=ON in retroarch-e2e
.github/workflows/nightly.yml             # pins; new flake job; report job gets it
.github/workflows/release.yml             # download-artifact pin only
README.md                                 # status, RetroArch section, nightly, reset, no-skip line
.planning/preparation/ENGINEERING.md      # section 5: nightly flake job, jobs=4, COST (keep prep docs current)
```

### Pattern 1: Parallel presets plus COST (TUNE-01)
**What:** add `"jobs": 4` to `execution` in the `ci`, `ci-msvc`, `dev` and `asan` test presets (CMake preset schema version 6 allows it; the file is already `"version": 6`) [VERIFIED: CMakePresets.json]. Add `set_tests_properties(<t> PROPERTIES COST <n>)` for `vectors.registration_policy`, `vectors.registration_policy.self_test` and `runner.write_hashes`. Leave `nofp`, `hygiene` and `vectors-full` alone.
**Why COST:** "When parallel testing is enabled, tests in the test set will be run in descending order of cost" and an undefined cost starts at 0 [CITED: `cmake --help-property COST`]. A fresh CI build directory has no cost data, so without COST the 107 s test starts around test #50.
**Shared paths:** inspected every test that writes under the build tree; each writes to its own name (`fixture.nmovie`, `movie-a.out`, `nrom-tracer.*`, `cmp/`, `stage/`, `consumer/`, `embed/`, `vectors/<name>`, `game-movies/`), and tests that share files already use FIXTURES (`testframe_ppm`, `hashes_txt`, `cmp_files`, `staged`). One local `-j4 --schedule-random` run and a three-fold `--repeat until-fail:3` run passed 355/355 on macOS arm64. Windows and slower runners are not exercised until CI runs; the nightly flake hunt is the safety net. [VERIFIED: ran ctest; ASSUMED for other platforms]
**Do not** use `RUN_SERIAL` or `RESOURCE_LOCK` unless CI shows a collision.

### Pattern 2: Nightly flake job (TUNE-01)
Add a third job to `.github/workflows/nightly.yml`, before `report`:

```yaml
  suite-flake:
    runs-on: ubuntu-24.04
    timeout-minutes: 10 # initial; set to twice the first measured cold run (ENGINEERING section 5)
    permissions:
      contents: read
    outputs:
      outcome: ${{ steps.flake.outcome }}
    steps:
      - uses: actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1 # v7.0.1
        with:
          persist-credentials: false
      - name: Linux toolchain
        run: |
          echo "CC=gcc-14" >> "$GITHUB_ENV"
          echo "CXX=g++-14" >> "$GITHUB_ENV"
          command -v ninja || { sudo apt-get update && sudo apt-get install -y ninja-build; }
      - name: Prepare protected-main scoreboard baseline
        run: cmake -P tests/cmake/prepare_scoreboard_baseline.cmake
      - run: cmake --preset ci
      - run: cmake --build --preset ci
      - id: flake
        run: ctest --preset ci --repeat until-fail:3 --schedule-random
```

- `ctest --preset ci --repeat until-fail:3 --schedule-random -N` is accepted (verified: lists 357 tests); the preset supplies `jobs`, `outputOnFailure` and `noTestsAction: error`. [VERIFIED: ran it]
- The `scoreboard` baseline step is needed because `accuracy.scoreboard` requires the CI-provided baseline when `CI=true` (see `accuracy.scoreboard.ci_missing_baseline` in `tests/CMakeLists.txt`) [VERIFIED: file read]; copy the step from `ci.yml`.
- `report` must list the new job: add it to `needs`, add `&& needs.suite-flake.result == 'success'` to the `RESULT` expression, add an env line and a body line for `outcome`. Keep exactly one `issues: write`.
- **Constraints from `tests/cmake/nightly_workflow_policy.cmake` (hygiene lane):** no `actions/cache`, `restore-keys:` or `cache:` anywhere in the file; no `SKIP_RETURN_CODE`, `exit 77`, `continue-on-error: true` or `|| true`; no write permissions other than `report`'s `issues: write`; no `secrets.`; no `git commit/push`; every `uses:` must be a full SHA. The new job satisfies all of these as written above. `nightly.yml` also runs on `pull_request` for the listed paths and on every push to `main`, so the flake job will run there too (about 3 minutes); that is acceptable and validates changes to the files that matter.
- Timeout rule: "twice a measured cold duration, with the run recorded in a comment" (ENGINEERING section 5). Start at 10 and tighten after the first run.

### Pattern 3: Skip policy as a ctest inventory check (TUNE-05, D-05/D-06)
**What:** `tests/cmake/skip_policy.cmake` runs `${CTEST} --test-dir ${BUILD_DIR} --show-only=json-v1` with `execute_process`, then loops `string(JSON ... GET ... tests i name)` and tests each `properties` entry's `name` for the three property names. Copy `property_exists` from `vector_registration_policy.cmake` (existence of the property is the right test; values differ in type).
**Verified output** (ctest 4.4.3, hand-written fixture):
```
a [('SKIP_RETURN_CODE', 77), ...]
b [('SKIP_REGULAR_EXPRESSION', ['skip:']), ...]
c [('DISABLED', True), ...]
```
`SKIP_RETURN_CODE` is a JSON number, `SKIP_REGULAR_EXPRESSION` an array, `DISABLED` a boolean. An empty `CTestTestfile.cmake` yields `"tests" : []` and exit 0 (so the empty list must be rejected explicitly); a missing directory exits 1. [VERIFIED: ran ctest]
**Real-ctest self-test without a configure step:** write a `CTestTestfile.cmake` into the work directory and run the real ctest on it:
```cmake
# Source: verified locally with ctest 4.4.3
file(WRITE "${WORK}/CTestTestfile.cmake"
  "add_test([=[t]=] \"${CMAKE_COMMAND}\" \"-E\" \"true\")\n"
  "set_tests_properties([=[t]=] PROPERTIES SKIP_RETURN_CODE \"77\")\n")
execute_process(COMMAND "${CTEST}" --test-dir "${WORK}" --show-only=json-v1
  RESULT_VARIABLE rc OUTPUT_VARIABLE json)
```
This needs no generator or compiler on any runner. [VERIFIED on 4.4.3; ASSUMED on CMake 3.25 to 3.31, the format is old and stable.]
**Placement:** register `policy.no-skip` and `policy.no-skip.self_test` in `tests/CMakeLists.txt` with no `LABELS`, so the `ci`, `ci-msvc`, `dev` and `asan` presets run them; `nofp` (label `abi`), `hygiene` and `vectors-full` filter by label and do not. The script must set `cmake_minimum_required(VERSION 3.25)` (`cmake.script_policy` checks every script in `tests/cmake`).
**What the policy cannot see:** a test that internally prints "skipped" and exits 0. That is outside D-05 by design. Note `skip` in `runner/main.c:372,539` and the scoreboard is AccuracyCoin's own 0xFF result code, unrelated to CTest.

### Pattern 4: Release policy with an exact publish gate (TUNE-03, D-08 to D-11)
**Current gate** (all quoted from `.github/workflows/release.yml`):
```
57:  ci:
58:    needs: release-please
59:    if: needs.release-please.outputs.release_created == 'true'
65:  publish:
66:    needs: [release-please, ci]
67:    if: needs.release-please.outputs.release_created == 'true'
134:        run: gh release edit "$TAG" --draft=false --repo "$GITHUB_REPOSITORY"
```
`publish` is the last job, so "to the next job key" means to end of file. `release-please` (line 22) legitimately has a step-level `if:` on line 48 (`if: steps.release.outputs.prs_created == 'true'`); the scope cut at `publish:` is what keeps D-10's "no step-level if" from rejecting it. [VERIFIED: grep -n]
**Current defect:** `release_policy.cmake` uses `string(FIND ...)` for `needs: [release-please, ci]` and `if: needs.release-please.outputs.release_created == 'true'` inside `SUBSTRING ... ${publish_marker} -1`. Appending `|| always()` keeps both substrings, so the current check accepts it. [VERIFIED: file read]
**Scanner pitfall (verified):** with `string(REPLACE "\n" ";" lines "${text}")`, `release.yml` splits into 190 elements for 172 lines, because `for c in library runner libretro; do` and similar shell lines contain `;`. `file(STRINGS ...)` gave 173 intact elements. `action_pins.cmake` documents the same hazard and neutralises `[`, `]` and `;` first. For exact-line equality use one of: (a) encode `;` as a placeholder before splitting, apply the same encoding to the canonical strings, or (b) read the file with `file(STRINGS)` and feed mutations through a temp file. Option (a) keeps the existing text-in/text-out `check_policy(config workflow out_errors)` shape, which the mutation helper needs. Add a mutation case containing `;` inside the gate line.
**Scanner details to copy and fix:** the nightly scanner treats `#` as a comment start anywhere in a line (`string(FIND "${line}" "#" ...)`), which truncates `VERSION="${TAG#v}"`; harmless here, but compare canonical strings after stripping trailing whitespace only. Job keys match `^  [A-Za-z0-9_-]+:$` (two-space indent).
**Checks to implement**, each with a mutation in the self-test (D-11): publish and ci `needs:` and `if:` lines appear exactly once each and equal the canonical variables; no `continue-on-error` in the publish block; no step-level `if:` in the publish block (a line matching `^ {6}- if:` or `^ {8}if:`); no `&`/`*` anchors or aliases and no `<<:` merge key; the count of `release edit` lines carrying `--draft=false` across the whole file is exactly one and it lies inside the publish block. Mutation helper: `string(REPLACE ...)` then fail if the result equals the input (D-11's "replace helper").
**Not in scope** (deferred): `permissions`/`environment` freezing.

### Pattern 5: Shared synthetic iNES builder (TUNE-04/-06, discretion)
**What:** new `tests/ines.h`, `static inline` functions only (no link unit), used by `tests/core/test_reset.c` and `tests/libretro/libretro_host.c`; `tests/core/test_cartridge.c` keeps its own helpers (no refactor needed).
**Design:** a small spec struct `{ prg_16k, chr_8k, flags6, flags7, mapper, trainer (ptr or NULL), prg_code (ptr, len), reset_vector }` and one `size_t ines_build(uint8_t *out, size_t cap, const struct ines_spec *)`. Header layout: bytes 0-3 `NES\032`, 4 PRG count, 5 CHR count, 6 flags (bit 2 = trainer, bit 0 = vertical mirroring, mapper low nibble in high bits), 7 flags (mapper high nibble), followed by 512 trainer bytes when flag bit 2 is set, then PRG, then CHR [VERIFIED: `validate_image` in `src/cartridge.c` lines 56-118 and `make_trainer_image` in `tests/core/test_cartridge.c`]. Include the mapper and submapper fields now so Phases 7 to 9 only add rows; the loader still rejects mapper != 0 today (`src/cartridge.c:75` for NES 2.0, `:94` for iNES 1).
**Trainer program that makes the frame depend on the trainer:** reset vector at `$8000` points to `JMP $7000` (`4C 00 70`); the 512 trainer bytes hold a short 6502 routine at offset 0 that sets palette `$3F00` to `$16`, enables the background (`$2001 = $0A`) and loops. If the trainer is not loaded, `$7000` is zero (`BRK`), so the frame differs from the hard-coded colour. Pixel (0,0) should then be `0xBA3100` (native `$16`), the value `libretro_host.c` already asserts for its other programs (`CHECK_EQ_HEX(frame[0], 0xBA3100u)`, line 357). The loader copies the trainer to `prg_ram + 0x1000`, i.e. CPU `$7000` [VERIFIED: `src/cartridge.c:177-179`].
**Host-path check:** extend `libretro_host.c` with `check_trainer_frame_parity(...)` modelled on `check_input_frame_parity` (lines 241-362): build the image, `p_load_game`, `p_run`, write the image to `argv[5]`, spawn the runner with `--rom <argv[5]> --frames 1 --dump-frame 1:<argv[3]>` through `test_process_run`, `compare_with_ppm(argv[3])`, and also assert the hard-coded pixel. It runs inside the existing `libretro.host` CTest case (one module load, its own build-dir files `nrom-tracer.*`, parallel-safe). If a distinct CTest name is wanted for traceability, add a second `add_test` for `libretro.host` with an extra argument selecting the check; not required by the requirement text.

### Pattern 6: Reset implementation (TUNE-06, D-12 to D-18)

**Where each piece goes (and why):**

| Piece | File | Constraint |
|-------|------|-----------|
| `nesturbator_status nesturbator_reset(nesturbator *inst)` | `src/instance.c` | Public; NULL -> `NESTURBATOR_ERR_ARGUMENT`; `cart.bytes == NULL` -> `NESTURBATOR_OK` and return before touching anything. |
| `nesturbator__cpu_reset(nes)` | `src/cpu.c` | `cpu.vectors` and `cpu.unit` link `cpu.c` with `tests/cpu/vector_bus.c`, which defines only `nesturbator__bus_read/_write` [VERIFIED: `tests/CMakeLists.txt` lines ~437-470]. So it may call only those two and touch `nes->cpu`. |
| `nesturbator__ppu_reset(nes)` | `src/ppu.c` | `bus.unit` compiles `bus.c apu.c synth.c cartridge.c ppu.c` and no `cpu.c`; `ppu.c` already writes `nes->cpu.nmi_pending` by field, so clear the NMI latches by field, not by calling cpu.c. |
| `nesturbator__apu_reset(nes)` | `src/apu.c` | Needs the file-static `update_irq_line`; calls `nesturbator__apu_write(nes, 0x4015, 0)`. |
| `reset_flag` | `struct nesturbator__ppu` in `src/internal.h` | Internal struct, public ABI unchanged. `load`/`unload` `memset` the PPU so the flag starts 0 (D-17). |
| Prototypes | `src/internal.h` | `-Wmissing-prototypes` is on. |

**Sequence in `nesturbator_reset`** (matches D-13 to D-16; order chosen so each step sees consistent state):
1. `nesturbator__ppu_run_until(inst, inst->ticks)`.
2. `bus.controller_strobe = 0; controller_shift[0..1] = 0; oam_dma_pending = 0` (keep `input_pending`, `input_buttons`, latch values per D-13: "host input state" kept).
3. `nesturbator__ppu_reset`: `control = mask = 0; address_latch = 0; t = 0; fine_x = 0; read_buffer = 0; odd_frame = 0; cpu.nmi_pending = 0; cpu.nmi_prev = 0; reset_flag = 1`. Keep `v`, `status`, `oam_addr`, memories, `io_bus`.
4. `nesturbator__apu_reset`: set `triangle.phase = 0` and `dmc.output &= 1` first, then `nesturbator__apu_write(nes, 0x4015, 0)` (it zeroes the length counters, `dmc.remaining`, `enable_delay`, `dma_load_waiting`, `dmc.irq`, and calls `update_mixed_level` so the level transition goes through the normal band-limited path) [VERIFIED: `src/apu.c:369-406`, `:472-476`]. Then clear `frame_irq`, `frame_irq_clear_pending`, `dmc.dma_pending`, `dmc.dma_halt_phase`; call `update_irq_line`. Re-apply the last `$4017` mode: `frame_pending_mode` already holds the last written mode; set `frame_mode = frame_pending_mode`, `frame_reset_delay = 0`, `frame_cycle = 0`, leave `frame_irq_inhibit` alone. *(Superseded 2026-10-10 by the exact form in "Reset facts confirmed against NESdev": keep `frame_pending_mode`, set `frame_reset_delay = bus.apu_get_put_phase != 0 ? 2 : 1` and let the existing expiry path apply the mode and reset `frame_cycle`.)*
5. `nesturbator__cpu_reset`: `p |= FLAG_I` (`FLAG_I` is `0x04u` in `cpu.c` line 18); `jammed = 0`; `poll_latch = 0`; `irq_line` recomputed by step 4; then 7 bus cycles: five `nesturbator__bus_read(nes, 0x0100 | s')` (stack page, RAM, no side effects), with `s` decremented three times across them (S ends at `s - 3`, wrapping as `uint8_t`), then `lo = nesturbator__bus_read(nes, 0xFFFCu); hi = nesturbator__bus_read(nes, 0xFFFDu); pc = hi << 8 | lo`.
6. Do not touch `frame_number`; `ticks` advances by 7 x 24 = 168 through the bus.

**Reset ticks and audio (check, no change needed):** `run_frame` computes `target = inst->ticks + NESTURBATOR_TICKS_PER_FRAME` at frame start (`src/frame.c:52`), so the 168 reset ticks sit before the next frame and are not deducted from it. `apu.sample_output` and `ppu.video_output` are NULL between frames (set NULL at `frame.c:60-61`), so reset cycles write no pixels or samples; `apu_begin_frame` resets `sample_phase = audio_rem` (`apu.c:180`). The header's `ticks` description ("emulated time ... since create") should say reset cycles are counted. [VERIFIED: file reads]

**PPU write gate:** in `nesturbator__ppu_register_write` (`src/ppu.c:340`), after the `io_bus` update, `if (nes->ppu.reset_flag && (reg == 0x2000u || reg == 0x2001u || reg == 0x2005u || reg == 0x2006u)) return;` (the latch must not toggle, per the wiki). Clear the flag in `nesturbator__ppu_run_until` inside the existing `scanline == 261 && dot == 1` block (`src/ppu.c:271-274`: `nes->ppu.status &= 0x1fu; nes->cpu.nmi_pending = 0u;`), the point where the wiki says the reset signal is "cleared at the end of VBlank, by the same signal that clears the VBlank, sprite 0, and overflow flags". `$2002`, `$2003`, `$2004`, `$2007` and `$4014` work immediately [CITED: wiki PPU power up state].

**libretro:** replace the body of `retro_reset` (`libretro/libretro.c:151-154`, currently empty under the stale comment "The test card has nothing to reset.") with `if (inst != NULL) { (void)nesturbator_reset(inst); }` and update the comment. A no-game (test card) instance returns OK from the core; no adapter branch needed.

**Tests (D-18):** `tests/core/test_reset.c` uses the internal header (like `core.cartridge`, `core.profile`) to read `cpu.s/p/pc/jammed`, `ppu.*` and `bus.ram` and to drive `nesturbator__bus_read/_write`. *(Revised: with D-13/D-15 as amended, reset itself puts the PPU at scanline 0 dot 0, so the test no longer places the position; it counts CPU cycles from the reset.)* Step single bus cycles around scanline 261 dot 1 to show a `$2000`/`$2005` write at the cycle before is dropped and the cycle after lands. Register as `core.reset` through `nesturbator_core_test` plus `target_include_directories(... src)`, like `core.cartridge`. `libretro_host.c`: build one instance via the public API, run N frames, `nesturbator_reset`, run M frames; run the module through `p_load_game`, N `p_run`, `p_reset`, M `p_run`; require equal frames (convert the direct instance's native pixels with `nesturbator_get_palette` as `check_input_frame_parity` does). The image needs RAM state that changes across the run (a frame counter in CPU RAM) so "kept" and "reset" differ observably.

**Header text to add** (append after `nesturbator_get_palette`, before the `extern "C"` close; keep all comments, which `vector_api_policy` strips before hashing):
```c
/* Soft reset, as the console's Reset button. Call between frames. NULL gives
 * NESTURBATOR_ERR_ARGUMENT; with no cartridge it does nothing and returns
 * NESTURBATOR_OK. Kept: CPU and cartridge RAM, ... Cleared: ... The CPU sets
 * I, lowers S by 3 and takes the reset vector in 7 bus cycles; ... Power-on
 * (load) is unchanged. */
nesturbator_status nesturbator_reset(nesturbator *inst);
```
Also fix the stale sentence in the `nesturbator_create` comment (header lines 223-226, "The CPU does not yet run during frames.") and the stale comment block in `src/internal.h` lines 34-47 ("Nothing runs the CPU during a frame yet", "nothing clears it yet" for `jammed`).

### Reset facts confirmed against NESdev (2026-10-10, checker issue 4)

Raw wiki text fetched with `action=raw` on 2026-10-10. Hardware documentation only; no emulator source was opened.

| Fact | Source (URL, quoted) | Effect on the plan |
|------|----------------------|--------------------|
| CPU after reset: A, X, Y unchanged; PC = ($FFFC); S -= 3; I = 1; C, Z, D, V, N unchanged; "RESET uses the logic shared with NMI, IRQ, and BRK that would push PC and P ... the 2A03 prohibits writes during reset." | https://www.nesdev.org/wiki/CPU_power_up_state ("After Reset" column and footnote) | Confirms D-14. |
| Internal RAM and cartridge RAM are "unchanged after a reset"; battery RAM "generally unchanged after subsequent resets". | https://www.nesdev.org/wiki/CPU_power_up_state | Confirms D-13 kept memories. |
| APU after reset: Status ($4015) "0 (all channels disabled)"; triangle phase "0 (output = 15)"; DMC direct load "[$4011] &= 1"; $4010, $4012, $4013 unchanged; pulse and noise registers "unchanged?"; Frame Counter ($4017) "unchanged"; frame-counter LFSR revision-dependent. | https://www.nesdev.org/wiki/CPU_power_up_state ("APU" table) | Confirms D-16: `$4015 = 0`, `triangle.phase = 0`, `dmc.output &= 1`; `$4017`'s value (mode and IRQ inhibit) is kept, not rewritten with a new value. |
| "Power-up and reset have the effect of writing $00, silencing all channels." "Writing to this register clears the DMC interrupt flag." | https://www.nesdev.org/wiki/APU (Status $4015) | Confirms routing through `nesturbator__apu_write(nes, 0x4015u, 0u)`, which also clears `dmc.irq`. |
| "Writing to this register resets the counter to 0. If the write occurs during an APU cycle, the effects occur 3 CPU cycles after the write cycle, and if the write occurs between APU cycles, the effects occurs 4 CPU cycles after the write cycle." With bit 7 set, "both Quarter frame and Half frame clocks are also generated". The page says nothing about reset. | https://www.nesdev.org/wiki/APU_Frame_Counter | This is what `src/apu.c` already models with `frame_reset_delay` = 3 or 4 by `apu_get_put_phase`. |
| "It is known that after power and reset, it is as if the APU's $4017 were written 10 clocks before the first code starts executing. ... The cause is likely the reset sequence of the 2A03, when it reads the reset vector." (This sentence is on the PPU page, not the APU pages.) | https://www.nesdev.org/wiki/PPU_power_up_state | Resolves A2 exactly, see below. |
| "The PPU comes out of power and reset at the top of the picture." Writes to PPUCTRL, PPUMASK, PPUSCROLL, PPUADDR "are ignored if earlier than ~29658 CPU clocks after reset", and the latch does not toggle; PPUSTATUS, OAMADDR, OAMDATA, PPUDATA and OAMDMA work immediately. The internal reset signal "is set on reset and cleared at the end of VBlank, by the same signal that clears the VBlank, sprite 0, and overflow flags". The VBL flag is "unchanged by reset", next set "around 27384". "On front-loading consoles (NES-001), the Reset button ... resets both the CPU and PPU." | https://www.nesdev.org/wiki/PPU_power_up_state | Basis of the revised D-13/D-15 (position 0/0) and of the write gate. |
| Writes to $2000, $2001, $2005 and $2006 are ignored "until reaching the pre-render scanline of the next frame; more specifically, for around 29658 NTSC CPU cycles ... assuming the CPU and PPU are reset at the same time." | https://www.nesdev.org/wiki/PPU_registers | Same. |

**$4017 re-apply timing, exact (replaces A2's approximation).** Let R0 be the first of the 7 reset cycles; the first instruction starts at R0 + 7. "Written 10 clocks before the first code" puts the virtual write in cycle R0 - 3. In `src/apu.c` a `$4017` write sets `frame_reset_delay` to 4 when `bus.apu_get_put_phase` is nonzero in the write cycle and 3 otherwise; the write lands after that cycle's `nesturbator__apu_clock`, so the delay counts down from cycle R0 - 2. The phase toggles once per cycle, so its value in cycle R0 - 3 equals its value now (after cycle R0 - 1). Two of the countdown cycles (R0 - 2, R0 - 1) have already passed, so `nesturbator__apu_reset` sets `frame_pending_mode` unchanged (it already holds the last written mode), and `frame_reset_delay = bus.apu_get_put_phase != 0 ? 2 : 1`. The existing expiry path then resets `frame_cycle`, applies the mode and, in 5-step mode, generates the quarter and half clocks, inside reset cycle R0 or R0 + 1: 6 or 7 cycles before the first instruction, as the wiki's 10-clock statement plus the documented 3-or-4-cycle delay give. `frame_irq_inhibit` is untouched ($4017 "unchanged").

**Still open, recorded (not confirmed on any wiki page fetched):** that reset clears the frame IRQ flag. None of the four pages states it; the source is the blargg `apu_reset` readme (`irq_flag_cleared`, Sources: Secondary). D-16 keeps the clear; the deferred `apu_reset` ROMs are the check.

**Write-ignore window, exact figure for this implementation.** Ticks advance 24 per CPU cycle and 8 per PPU dot, and `ticks` is a multiple of 24 between instructions, so after the catch-up in `nesturbator_reset` `ppu.ppu_ticks == ticks`. Reset sets the PPU to scanline 0, dot 0. Scanline 261 dot 1 is 261 x 341 + 1 = 89,002 dots later (`odd_frame` is cleared and the odd-frame skip is at 261/340, after 261/1), so 712,016 ticks. A bus write in the CPU cycle that starts k cycles after reset began sees the PPU caught up to k x 24 + 24 ticks before the access. It is dropped while (k + 1) x 24 < 712,016, so for k <= 29,666 (PPU at scanline 261 dot 0), and lands for k >= 29,667 (PPU at scanline 261 dot 3). The window is therefore 29,667 CPU cycles from the start of the reset, 29,660 from the first instruction (cycles 0 to 6 are the reset sequence), against the wiki's ~29,658. (An earlier draft of this file gave 29,674; that was an arithmetic slip: (261 x 341 + 1) / 3 = 29,667.3.)

### Anti-Patterns to Avoid
- **Rebuilding the instance in `retro_reset`.** Wipes RAM and cartridge RAM (PITFALLS.md "Reload instance to implement reset": Never). D-12 forbids allocate/free/memset of the instance.
- **Calling `nesturbator__cpu_reset` machinery from `ppu.c` or `apu.c`** (breaks `bus.unit` link) or `ppu`/`apu` functions from `cpu.c` (breaks `cpu.vectors` link).
- **Splitting workflow text on `;` for exact matching** (see Pattern 4).
- **Putting the new pin only in workflows.** The nightly policy hard-codes the upload-artifact SHA.
- **`RUN_SERIAL`/`RESOURCE_LOCK` to hide a collision.** Fix the path instead; the nightly exists to find these.
- **Counting reset cycles as part of the next frame.** `target` is computed from current ticks; the extra 168 ticks are expected.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Parallel CTest ordering | A custom test scheduler or sharding script | `execution.jobs` + `COST` | CTest already schedules by cost, honours FIXTURES and resource locks. |
| Reading the registered tests | Parsing `CTestTestfile.cmake` text | `ctest --show-only=json-v1` + `string(JSON)` | The machine-readable inventory is what `vector_registration_policy.cmake` already uses. |
| YAML for the release gate | A YAML parser, golden file or hash (D-09) | Exact-line equality of canonical strings on a comment-stripped, job-scoped line list | Locked by D-09; copy-paste beats a dependency. |
| Flake detection | A re-run wrapper | `ctest --repeat until-fail:3 --schedule-random` | Built in; verified to run here. |
| Compiler caching | ccache/`actions/cache` | nothing | Measured compile share <= 22%. |
| Synthetic ROMs | Per-test ad-hoc byte arrays | `tests/ines.h` | Phases 7 to 9 need bus-conflict, MMC1 and battery images; one builder. |
| Reset cycles | A second CPU "reset" micro-model | The existing `nesturbator__bus_read` for 7 cycles | Time, PPU/APU catch-up and open bus stay correct for free. |

**Key insight:** every piece of this phase is a thin layer over something the repo already has (CTest properties, the inventory-JSON pattern, `nesturbator__apu_write`, `nesturbator__ppu_run_until`, the direct-API reference instance in `libretro_host.c`). The risk is not missing capability but the traps listed below.

## Runtime State Inventory

Phase 5 is not a rename or migration. Two small "removal" items were checked so the planner does not have to:

| Category | Items Found | Action Required |
|----------|-------------|------------------|
| Stored data | None. No database or datastore holds `retroarch.*` names. | None. |
| Live service config | The GitHub ruleset requires only `CI required` (`.github/rulesets/main.json`); no test name is referenced. The rolling nightly issue is keyed by label `nightly`, not by job name. | None; the new flake job joins the existing issue. |
| OS-registered state | None. | None. |
| Secrets/env vars | `NESTURBATOR_RETROARCH` is read only by `run_retroarch.cmake` and set only in `ci.yml` (`retroarch-e2e`, line 186) and the README. | Remove all three (D-02, D-03, D-04). |
| Build artifacts | Existing `build/` directories on the owner's Mac hold stale `retroarch.*` test registrations and `build/ci/retroarch*` run dirs. `build/` is git-ignored. | None for the repo; a reconfigure drops them. |

## Common Pitfalls

### Pitfall 1: Parallelism that helps nothing
**What goes wrong:** `execution.jobs: 4` alone leaves the 107 s test starting late and the leg barely faster.
**Why:** CTest orders by COST then by definition order; a cold CI build has no cost history.
**How to avoid:** COST on `vectors.registration_policy` (largest), `.self_test`, `runner.write_hashes`.
**Warning signs:** the "after" job wall is within ~10 s of "before" on the Intel leg.

### Pitfall 2: `;` and `[` break CMake list splitting
**What goes wrong:** exact-once counting of `if:`/`needs:` lines goes wrong when a line contains `;` (real example: line 91 of `release.yml`) or when `[` ... `;` ... `]` group elements.
**How to avoid:** encode `;`, `[`, `]` before splitting (as `action_pins.cmake` does) or read with `file(STRINGS)`; include a mutation with `;` in the gate.
**Warning signs:** line counts differ from `wc -l`; a mutation containing `;` is silently accepted.

### Pitfall 3: The upload-artifact SHA is duplicated in a policy script
**What goes wrong:** bumping workflows only fails `hygiene.nightly_workflow_policy` with `run-evidence`.
**How to avoid:** update `tests/cmake/nightly_workflow_policy.cmake` lines 133, 152, 155 in the same change (also the fixture in 152/155, or the clean fixture will fail).

### Pitfall 4: The public declaration baseline
**What goes wrong:** `vectors.api_policy` (and `.self_test`) hash the comment-free, whitespace-free declaration stream of `include/nesturbator.h` against `PHASE4_DECLARATIONS_SHA256 = 300efd72...0952` [VERIFIED: `vector_api_policy.cmake` line 5]. Adding `nesturbator_reset` changes it.
**How to avoid:** make the header edit, run `cmake -DSOURCE_DIR=. -P tests/cmake/vector_api_policy.cmake`; the failure message prints `found <hash>`; paste it, rename the constant and comment to Phase 5. Comments, version macros and whitespace do not move the hash, so comment edits are free.

### Pitfall 5: Link-list traps for the reset functions
`cpu.vectors`, `cpu.unit` (cpu.c + test bus only) and `bus.unit` (bus, apu, synth, cartridge, ppu directly) restrict which symbols each file may reference (see Pattern 6 table). `tests/header/*` and `tests/consumer` also compile against the public header; a new declaration must be valid C17 and C++11 under `-pedantic -Werror`.

### Pitfall 6: Windows/MSVC and warnings in new test code
`-Wconversion -Wsign-conversion` and `/W4` are errors in presets: cast every `uint8_t`/`uint16_t` arithmetic result explicitly (the existing code does, e.g. `(uint8_t)(nes->cpu.s - 1u)`). Use `_CRT_SECURE_NO_WARNINGS` for `fopen` in any new test that opens files (as `libretro.host` does). Quote CMake arguments per the existing `-D` style.

### Pitfall 7: `policy.no-skip` run inside the ctest it inspects
It runs `ctest --test-dir <same build> --show-only=json-v1` from within a running ctest. `--show-only` does not run tests, and `vectors.registration_policy` already does the same on all six platforms, so this is safe. Pass `${CMAKE_CTEST_COMMAND}`, not a bare `ctest`.

### Pitfall 8: Removing the asan label filter changes what asan runs
D-04 removes `"filter": { "exclude": { "label": "retroarch" } }`. That filter is the only reason the retroarch tests never ran under asan; once D-01 deletes them, the filter is dead. Do both together or `noTestsAction`/filter validity is unaffected but the README claims drift.

### Pitfall 9: Reset when the PPU position and window interact
*(Revised at plan review 2026-10-10.)* With the original D-13 (position kept) the window length depended on where reset landed. With D-13/D-15 as revised, reset sets the PPU to scanline 0 dot 0, so the window is fixed at 29,667 CPU cycles from the start of the reset; the test counts cycles from the reset instead of placing the position. A reset taken while `status` bit 7 is set must keep bit 7 (the wiki: VBL flag "unchanged by reset"), and the flag must still clear at the next 261/1.

### Pitfall 10: Re-applied `$4017` in 5-step mode clocks the counters
With `frame_mode == 1` the existing delay-expiry path calls `frame_quarter_clock` and `length_clock` (`src/apu.c:301-304`). That is what a real `$4017` write does and is harmless right after `$4015 = 0` (all length counters are 0), but the reset test should not assert envelope/linear counter values after reset in 5-step mode.

### Pitfall 11: README/`x-release-please` markers
The install block in README carries `x-release-please` markers (one version per line, `release.one_version_per_line`). Edit around it, do not reflow it. Keep the README's `retroarch` section free of an inline version number.

## Code Examples

### Skip-property check over the ctest inventory
```cmake
# Source: pattern from tests/cmake/vector_registration_policy.cmake (property_exists);
# property names verified against ctest 4.4.3 --show-only=json-v1.
function(check_inventory json out_error)
  string(JSON count ERROR_VARIABLE jerr LENGTH "${json}" tests)
  if(jerr)
    set(${out_error} "invalid ctest inventory: ${jerr}" PARENT_SCOPE)
    return()
  endif()
  if(count EQUAL 0)
    set(${out_error} "ctest inventory is empty" PARENT_SCOPE)
    return()
  endif()
  math(EXPR last "${count} - 1")
  foreach(i RANGE 0 ${last})
    string(JSON name GET "${json}" tests ${i} name)
    string(JSON np ERROR_VARIABLE perr LENGTH "${json}" tests ${i} properties)
    if(perr OR np EQUAL 0)
      continue()
    endif()
    math(EXPR plast "${np} - 1")
    foreach(p RANGE 0 ${plast})
      string(JSON prop GET "${json}" tests ${i} properties ${p} name)
      if(prop STREQUAL "SKIP_RETURN_CODE" OR prop STREQUAL "SKIP_REGULAR_EXPRESSION"
         OR prop STREQUAL "DISABLED")
        set(${out_error} "test can skip or is disabled: ${name} (${prop})" PARENT_SCOPE)
        return()
      endif()
    endforeach()
  endforeach()
  set(${out_error} "" PARENT_SCOPE)
endfunction()
```

### `;`-safe line list for exact-line checks
```cmake
# Source: hazard documented in tests/cmake/action_pins.cmake and reproduced here on release.yml.
string(REPLACE ";" "<SEMI>" text "${workflow}")
string(REPLACE "[" "<LB>" text "${text}")
string(REPLACE "]" "<RB>" text "${text}")
string(REPLACE "\r\n" "\n" text "${text}")
string(REPLACE "\n" ";" lines "${text}")
# Apply the same three encodings to the canonical strings before comparing.
```

### Mutation helper that fails when nothing changed (D-11)
```cmake
function(mutate out label find replace)
  string(REPLACE "${find}" "${replace}" result "${WORKFLOW_TEXT}")
  if(result STREQUAL WORKFLOW_TEXT)
    message(FATAL_ERROR "release_policy self-test: mutation '${label}' changed nothing")
  endif()
  set(${out} "${result}" PARENT_SCOPE)
endfunction()
```

### CPU reset sequence (shape, not final code)
```c
/* The 2A03 runs the interrupt sequence for reset with the stack writes
   suppressed, so S falls by 3 and I is set; the vector is $FFFC/$FFFD.
   Source: NESdev Wiki "CPU power up state" and "CPU interrupts". */
void nesturbator__cpu_reset(struct nesturbator *nes)
{
    nes->cpu.p |= FLAG_I;
    nes->cpu.jammed = 0u;
    nes->cpu.poll_latch = 0u;
    for (unsigned i = 0u; i < 5u; i++) {
        (void)nesturbator__bus_read(nes, (uint16_t)(0x0100u | nes->cpu.s));
        if (i >= 2u)
            nes->cpu.s = (uint8_t)(nes->cpu.s - 1u);
    }
    uint8_t lo = nesturbator__bus_read(nes, 0xFFFCu);
    uint8_t hi = nesturbator__bus_read(nes, 0xFFFDu);
    nes->cpu.pc = (uint16_t)(((uint16_t)hi << 8) | lo);
}
```

### `retro_reset`
```c
/* L7680: the console's Reset button. The core keeps RAM and cartridge RAM. */
void retro_reset(void)
{
    if (inst != NULL) {
        (void)nesturbator_reset(inst);
    }
}
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Tests self-skip with exit 77 / `nesturbator-skip:` | Unrunnable tests are not registered; host-app checks live in a required CI job | This phase | `retroarch-e2e` no longer prints two `***Skipped` lines. |
| Substring match on the publish gate | Exact-line equality in a job-scoped, comment-stripped view | This phase | `|| always()` is rejected. |
| Serial CTest (357 tests) | `execution.jobs: 4` + COST | This phase | About 45 s off the Intel leg at best. |
| `retro_reset()` no-op | `nesturbator_reset()` soft reset | This phase | A player's Reset keeps saves/RAM. |

**Deprecated/outdated in this repo:** `macos-14` labels (none used); the stale header comment "The CPU does not yet run during frames"; README "Status: Phase 4, NTSC sound timing" (v1 shipped; update the status line to the current state).

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | ~~The NES-001 reset puts the PPU back at the top of the picture; D-13 keeps the position instead.~~ **Resolved at plan review 2026-10-10:** the owner revised D-13/D-15 to restart the PPU at scanline 0 dot 0 per NESdev; window 29,667 CPU cycles from the reset (29,660 from the first instruction) against the documented ~29,658. | Open Questions 1 | None remaining. |
| A2 | ~~Mapping the 10-clock `$4017` statement to `frame_cycle = 0` is close enough.~~ **Resolved 2026-10-10:** the statement is on the NESdev "PPU power up state" page; with the documented 3-or-4-cycle `$4017` delay it maps exactly to `frame_reset_delay = apu_get_put_phase ? 2 : 1` (see "Reset facts confirmed against NESdev"). | Reset facts confirmed | None remaining; the frame-IRQ clear stays recorded as open (test-ROM readme only). |
| A3 | Expected "after" times (Intel leg ~135 to 140 s job wall) are arithmetic from the table. | CI measurements | None for correctness; the phase must record the measured value. |
| A4 | Hand-written `CTestTestfile.cmake` works as a "real ctest" fixture on CMake 3.25 to 3.31 (verified on 4.4.3 only). | Pattern 3 | Self-test fails on one runner; fallback is a configure of a `LANGUAGES NONE` project with the parent's generator. |
| A5 | No collisions between tests under `-j4` on Windows and the slower runners (verified on macOS arm64 only). | Pattern 1 | A parallel-only flake on CI; fix the path, the nightly finds it. |
| A6 | blargg `apu_reset/len_ctrs_enabled` ("length counters should be enabled at power-on and reset") does not conflict with `$4015 = 0` clearing the enable bits; it was not interpreted further. | Pattern 6 | Only matters when the deferred reset ROMs are added. |
| A7 | `COST` ordering applies on all generators/CMake versions the runners use (documented behaviour since CTest 2.8). | Pattern 1 | If ignored, the parallel win shrinks to ~25 s on Intel. |

## Open Questions (RESOLVED)

1. **Does reset restart the PPU at the top of the picture (wiki) or keep the dot position (D-13)?**
   - RESOLVED (owner, plan review 2026-10-10): restart at the top of the picture, scanline 0 dot 0, per NESdev. D-13 and D-15 amended in 05-CONTEXT.md; implemented by plan 05-06 Task 1 (`nesturbator__ppu_reset` sets `scanline = 0`, `dot = 0`); window 29,667 CPU cycles from the reset (the 29,674 below was an arithmetic slip, see "Reset facts confirmed against NESdev").
   - What we know: NESdev "PPU power up state" line 41: "The PPU comes out of power and reset at the top of the picture"; it gives "~29658 CPU clocks" for ignored writes and says the VBL flag is "next set around 27384" after power or reset. "PPU registers" says writes are ignored "until reaching the pre-render scanline of the next frame ... around 29658 NTSC CPU cycles ... assuming the CPU and PPU are reset at the same time." With the position restarted at scanline 0 dot 0 and the flag clearing at 261/1, the window is (261 x 341 + 1) / 3 = 29,674 CPU cycles, within ~16 of the documented figure (`nesturbator_load_cartridge` already starts the PPU at scanline 0, dot 0 because it `memset`s the PPU). Also: on Famicom/NES-101 top-loaders "only the CPU is reset", and on the NES-001 both are.
   - What's unclear: whether "comes out of reset at the top" means the counters are reset. D-13 and D-15 (locked) say keep position; STATE/SUMMARY and the roadmap criterion say "documented interval" (about 29,658).
   - Recommendation: implement D-13/D-15 as locked, but write the position handling as one clearly commented spot (and size the test helper to take the position as an argument) so flipping to "set scanline = 0, dot = 0" is a one-line change plus the README sentence. Ask the owner at plan review whether to flip; if flipped, the test asserts a window of 29,674 +/- a few cycles and the README quotes the wiki figure.

2. **Where should the nightly flake job's wall-time budget start?**
   - What we know: serial ubuntu suite is 52 s; `-j4` x 3 repetitions; the critical path is `vectors.registration_policy` (about 37 to 55 s on the slower legs).
   - What's unclear: the cold nightly duration.
   - Recommendation: `timeout-minutes: 10` initially, tighten to twice the first observed run, per ENGINEERING section 5.
   - RESOLVED: plan 05-04 Task 2 sets `timeout-minutes: 10` on `suite-flake` with the comment to set it to twice the first measured cold run.

3. **Should timeouts in `ci.yml` be lowered after parallelisation?**
   - What we know: build 7 min, asan 5 min, e2e 30 min against observed 3 min, 171 s, 111 s.
   - Recommendation: leave unless the planner wants the "twice cold time" rule applied; if lowered, cite the run id in the comment as the existing comments do. Not required by TUNE-01.
   - RESOLVED: plan 05-04 leaves the `ci.yml` timeouts unchanged (its "Decisions taken under Claude's discretion").

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| cmake / ctest | everything | yes | 4.4.3 (min 3.25) | none needed |
| ninja | presets | yes (`/opt/homebrew/bin/ninja`) | present | n/a |
| gh (authenticated, `szTheory`) | action pin resolution, before/after timings | yes | logged in | WebFetch of the GitHub API |
| clang-format 18 | `hygiene.format` | yes (`/opt/homebrew/opt/llvm@18/bin/clang-format`) | 18 | hygiene preset stops at configure otherwise |
| Existing build tree `build/ci` | local measurement | yes | configured Oct 9 | rebuild |
| RetroArch on this Mac | not needed after D-01 | not probed (tests are removed) | n/a | hosted `retroarch-e2e` |
| Hosted runners (6 + macos-15 e2e) | the "after" measurement | CI | n/a | none; the number comes from the PR's CI run |

**Missing dependencies with no fallback:** none.
**Missing dependencies with fallback:** none.

## Sources

### Primary (HIGH confidence)
- Repository files read this session: `CMakePresets.json`, `CMakeLists.txt`, `tests/CMakeLists.txt`, `tests/retroarch/{CMakeLists.txt,run_retroarch.cmake}`, `tests/cmake/{release_policy,nightly_workflow_policy,action_pins,vector_api_policy,vector_registration_policy,vector_result_policy,global_symbols,undefined_symbols}.cmake`, `.github/workflows/{ci,nightly,release}.yml`, `include/nesturbator.h`, `src/{internal.h,instance.c,cartridge.c,bus.c,cpu.c (interrupt section),apu.c,ppu.c,frame.c,synth.c (transition)}`, `libretro/libretro.c`, `tests/libretro/libretro_host.c`, `tests/core/test_cartridge.c`, `tests/check.h`, `tests/test_process.h`, `README.md`, `.planning/{STATE,REQUIREMENTS}.md`, `.planning/research/{STACK,SUMMARY,ARCHITECTURE,PITFALLS}.md`.
- `gh run view 38011892333` and its job logs (timings, Total Test time, `***Skipped` lines).
- Local commands: `ctest --test-dir build/ci -LE retroarch` serial / `-j4 --schedule-random` / `--repeat until-fail:3`; `ctest --show-only=json-v1` on `build/ci` and on hand-written fixtures; `cmake --help-property COST`.
- `gh api` for action releases and tag-to-SHA resolution; `gh api repos/100thCoin/AccuracyCoin/...` for the pin review.

### Secondary (MEDIUM confidence)
- NESdev Wiki raw pages fetched this session: `PPU_power_up_state`, `CPU_power_up_state`, `PPU_registers`, `CPU_interrupts` (tick-by-tick, reset note), `APU`, `APU_Frame_Counter`.
- christopherpow/nes-test-roms `apu_reset/readme.txt` and `cpu_reset/readme.txt` (summarised by the fetch tool: `4015_cleared`, `4017_written`, `irq_flag_cleared`; CPU: S drops by 3 with nothing written, A/X/Y unchanged, I set).
- docs.github.com hosted-runner specs; actions/runner-images README (labels).

### Tertiary (LOW confidence)
- None used for recommendations. Items resting on the wiki's wording alone are listed in the Assumptions Log.

## Metadata

**Confidence breakdown:**
- Standard stack / CI findings: HIGH. Measured from real run logs and local runs; pins resolved by API.
- Policy-script design: HIGH. Files read; list-splitting hazard reproduced; CTest property names verified by running ctest.
- Architecture (reset seams, link lists): HIGH. Read the CMake link lines and sources.
- Hardware reset details: MEDIUM. NESdev wiki and blargg readmes, no hardware; the PPU-position question was resolved by the owner at plan review, and the CPU, APU and `$4017` facts were confirmed against the wiki pages on 2026-10-10 (the frame-IRQ clear rests on the blargg readme only).
- Pitfalls: HIGH for build/test traps (reproduced), MEDIUM for hardware ones.

**Research date:** 2026-10-10
**Valid until:** 2026-10-24 for pins (upstream releases move weekly), 30 days for the rest.
