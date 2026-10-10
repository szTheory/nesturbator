# Phase 5: Tune-up and v1 debt - Context

**Gathered:** 2026-10-10
**Status:** Ready for planning

<domain>
## Phase Boundary

CI runs its tests in parallel and finds flaky tests nightly. The release check
matches the publish job's conditions exactly. A trainer-bearing image gives the
same frame through the runner and through the libretro test program. No test
can report itself skipped. A player who presses reset in RetroArch gets the
console's soft reset on an NROM game. Mapper boards, battery saves and power-on
accuracy belong to later phases.

</domain>

<decisions>
## Implementation Decisions

### Local RetroArch tests (TUNE-05)
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

### Tests never skip (TUNE-05)
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

### Release-policy exactness (TUNE-03)
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

### Soft reset (TUNE-06)
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

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Phase scope
- `.planning/ROADMAP.md` (Phase 5): goal and success criteria
- `.planning/REQUIREMENTS.md`: TUNE-01 to TUNE-06, and the out-of-scope row on ccache
- `.planning/research/SUMMARY.md` (Phase 5 section, open questions)
- `.planning/research/STACK.md`: CI timings, ccache rule, parallel tests, flake hunt, action pins, labels
- `.planning/research/ARCHITECTURE.md`: Reset table, public header changes, tests and CI hooks
- `.planning/research/PITFALLS.md`: self-skips that hide rot; a reset that wipes saves
- `.planning/MILESTONES.md` and `.planning/milestones/v1-MILESTONE-AUDIT.md`: the v1 debt items

### Hardware
- `.planning/preparation/NES-HARDWARE-CPU-APU.md` §8 power-on state (HWC.05, HWC.13, HWC.22)
- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md`: PPU registers and the reset signal
- https://www.nesdev.org/wiki/PPU_power_up_state and https://www.nesdev.org/wiki/CPU_power_up_state

### CI and policy house style
- `.planning/preparation/ENGINEERING.md` §5: CI jobs, timeouts, caches and pins
- `tests/cmake/nightly_workflow_policy.cmake`: comment-stripping line scanner and mutation pattern
- `tests/cmake/vector_registration_policy.cmake` and `tests/cmake/vector_result_policy.cmake`: ctest JSON inventory pattern

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `nesturbator__ppu_run_until`: lazy PPU catch-up, which must run before reset changes PPU state.
- `nesturbator__apu_write(inst, 0x4015, 0)`: the existing silencing path.
- `tests/libretro/libretro_host.c`: already builds a direct-API reference instance (`check_input_frame_parity`); the reset parity check copies it.
- The `expect_rejected` and `check_policy` functions in the policy scripts.

### Established Patterns
- Policies are `cmake -P` scripts registered as CTest tests, each with a self-test that rejects mutated inputs.
- Between frames, `ppu.video_output` and `apu.sample_output` are NULL, so reset cycles write no pixels or samples.
- Vblank clears at scanline 261 dot 1 in `src/ppu.c`. The reset flag clears there.

### Integration Points
- `include/nesturbator.h`: append `nesturbator_reset`.
- `libretro/libretro.c` `retro_reset()`: currently empty, with a stale "test card" comment.
- `tests/retroarch/`, `CMakePresets.json` (asan filter), `.github/workflows/ci.yml` (`retroarch-e2e`), `tests/cmake/release_policy.cmake`, `.github/workflows/release.yml`.

</code_context>

<specifics>
## Specific Ideas

- Owner preference: another copy-paste is better than another dependency. No new actions, tools or parsers unless a measurement requires one.
- Recommendations were synthesized by multi-lens research per area and adopted as given.

</specifics>

<deferred>
## Deferred Ideas

- Power-on write-ignore window and a 7-cycle startup sequence on load. This changes v1 hashes, so it would need a behaviour-revision bump in a later accuracy phase (SEED-003).
- `retroarch-e2e` reusing build artifacts instead of rerunning the suite (STACK.md, about 80 s of runner time).
- `cpu_reset` and `apu_reset` test ROMs as scoreboard rows. They need a reset request the runner does not issue.
- Freezing the publish job's `permissions` and `environment` in the release policy.

</deferred>

---

*Phase: 05-tune-up-and-v1-debt*
*Context gathered: 2026-10-10*
