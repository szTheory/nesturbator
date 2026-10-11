# Phase 10: Close-out and boot-to-play - Context

**Gathered:** 2026-10-11
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 10 closes milestone v2 with two proofs and ships its final release.

- **BOARD-04.** Frame and audio hashes for every board's committed ROMs match on
  all six CI platforms, and the libretro test program receives frames equal to
  the runner's for each of those ROMs.
- **PLAY-01.** A CI test proves that the MIT game nes-runner (zorchenhimer, MMC1)
  plays from boot into interactive play. The same test shows that each
  negative control fails.

Discussion found a core defect that stops PLAY-01. Fixing it is the first work
in this phase (D-01). New boards, new runner flags and new public API are out
of scope.

</domain>

<decisions>
## Implementation Decisions

The owner chose all four areas and asked, as in Phases 7–9, for a fan-out with
an adversarial pass followed by one coherent set of recommendations, applied
without further questions. Four researchers reported, and three of them ran
nes-runner v1 against this core.

The orchestrator checked the critical claims against the code:
- The `$2000` NMI defect is at `src/ppu.c:632`.
- 05-CR-01 is still open in `05-REVIEW-DISPOSITION.md`.
- `ASSET_POLICY.md` permits ROMs whose author states a licence.

The orchestrator then reconciled three conflicts:
- **Attract control.** "Title idle" would duplicate the no-input control, so
  D-11 uses menu navigation instead.
- **Failed-load control.** The runner exits 1 and never falls back, so D-11
  uses a library instance that keeps running after the load error.
- **Movie timing.** The two researchers' Start frames differed (60 against 100).
  Both were measured on scratch patches, so D-06 re-pins every timing after the
  real fix.

The owner made one decision directly: commit the nes-runner ROM (D-04).

### Core fix first: PPUCTRL NMI edge
- **D-01:** Fix `src/ppu.c` near line 629 so a `$2000` write raises an NMI only
  when bit 7 changes from 0 to 1 while the vblank flag is set.
  - Hardware rule, NESdev wiki PPU registers: "Changing NMI enable from 0 to 1
    while the vblank flag in PPUSTATUS is 1 will immediately trigger an NMI."
  - The code today sets `nmi_pending` whenever bit 7 is 1 during vblank. A
    game that rewrites `$2000` with bit 7 still set from its NMI handler
    re-enters NMI without end. nes-runner's `WritePalettes` does exactly
    this, so its main loop never runs and input is never read.
  - The fix:
    - Read the old control value before storing the new one.
    - Set pending on `(value & 0x80) && !(old & 0x80) && (status & 0x80)`.
    - Clear pending only when `(value & 0x80) == 0`. This keeps the existing
      NMI-suppression behaviour.
  - The hardware comment cites the page.
  - A scratch copy with this fix passed 445 of 446 `dev` tests. The failure
    was `runner.write_hashes.content`.
- **D-02:** Add a regression test with the fix. It is a synthetic `tests/ines.h`
  ROM or a `core.ppu` case that:
  - writes `$2000` with bit 7 set twice inside one vblank and expects exactly
    one NMI;
  - writes `0 -> 0x80` inside vblank and expects an immediate NMI;
  - writes bit 7 = 0 while an NMI is pending and expects it suppressed.

  After the fix, check whether the nightly blargg `ppu_vbl_nmi` lane, if
  present, moves.
- **D-03:** Bump the behaviour revision once, as Phase 6 did. Re-pin the hashes
  that moved in `tests/runner/hashes.txt`: nesteroids frame hashes and DABG
  milestone ticks moved in the scratch run. Holy Mapperel N values and blargg
  oracle frames must still pass. The executor records which rows moved, and
  why, in the plan summary. — **Reversibility:** costly — every pinned frame
  hash and every host's recorded hash downstream moves with the behaviour
  revision.

### nes-runner intake and the gameplay movie
- **D-04 (owner decision):** Commit the unmodified v1 release asset
  byte-for-byte as `tests/roms/runner.nes`.
  - Facts:
    - 81,936 bytes;
    - sha256 `1a67bfabdde2d96eacb04eb7f9d17e8da6c082d4976c318315fe31e2d19b4384`;
    - NES 2.0, mapper 1, 64 KiB PRG, 16 KiB CHR ROM, 8 KiB battery NVRAM.
  - Manifest line:
    - source `https://github.com/zorchenhimer/nes-runner/tree/47537be0ccb8baeb8462547a1c08888ee2a6cfac`;
    - pin `47537be0ccb8baeb8462547a1c08888ee2a6cfac` (tag v1);
    - licence `MIT`.
  - `THIRD-PARTY-NOTICES.md` gets:
    - the MIT text, "Copyright 2019 Zorchenhimer";
    - a note that the licence file arrived in commit `e2d70cf` (2019-06-28),
      three days after the tag, and that GitHub reports the repository as MIT;
    - an attribution line for the embedded FamiTracker NSF driver 2.11 by jsr
      (Jonathan Liss). It is source-available with no explicit licence, and
      its maintainers presume distribution is permitted.
  - Do not build from source: that needs cc65, Go and a gitignored CSV. Do not
    fetch at test time either: that is network-flaky and leaves a supply-chain
    gap. — **Reversibility:** one-way — removing a ROM from a public
    repository needs a history rewrite (`ASSET_POLICY.md` "If something gets
    in").
- **D-05:** Movies use the existing pattern: a small C writer at build time
  emits NMOVIE1 from constants, as `tests/runner/game_movie.c` does for DABG.
  - No movie bytes are committed.
  - The positive movie is: Start held about 4 frames on "Start Game" (the
    title ignores input for 5 frames and edge-detects buttons), then a fixed
    list of A jumps as `{start, hold}` pairs.
  - A held (mask `0x01`) jumps, and the hold length sets the height. Start
    (mask `0x08`) on the game-over screen returns to the title.
  - The writer emits every scenario the check needs (D-11).
- **D-06:** Re-measure all timing constants on the fixed core, and record the
  pinning procedure in the writer's header comment.
  - **Start frame:** pin it from the fixed core.
  - **Jump list:** found by an offline search over snapshots, choosing the
    latest feasible hop rather than the earliest to leave slack.
    - Before pinning, shift each jump ±2 frames and confirm the run still
      survives.
    - A scratch run on a patched core found a 31-jump list that survives to
      frame 899 with score 76. Treat it as a starting point only.
  - **The search tool is not committed.** The procedure is documented, and the
    test's score threshold makes any desync fail loudly.
  - A closed-loop bot that reads RAM was rejected: it is more code, and a bug
    in it could hide a regression.
- **D-07:** Save RAM, which answers the research flag: confirm in the same check
  that nes-runner writes its save. Run with no save file, from fresh zeroed
  SRAM.
  - The positive run's battery span (`nesturbator_get_memory`) has bytes 0–1 =
    `5A 6F` ("Zo", the first-start seed init), and `nesturbator_save_generation`
    is at least 2 after game start.
  - The Start-only control dies and inserts its score into the table, so its
    save generation rises further.
  - Because the seed is initialised from fresh SRAM, the level layout is
    deterministic.
  - No runner `--save-dir` chain is required for PLAY-01. Runner save
    round-trips are already covered by the Phase 8 and 9 `:save` chains.

### RAM observation and the semantic byte
- **D-08:** The check is one C test program, `tests/runner/test_boot_to_play.c`,
  linked against the library like `test_spectral.c`.
  - It loads the ROM, replays the generated movie through
    `nesturbator_set_input`, and reads RAM with `nesturbator_peek_cpu_ram`.
  - It adds no runner flag and no public API. A `--peek-ram` flag was rejected:
    it would add public CLI surface, a parser and an output format that nobody
    has asked for.
  - The roadmap says "on nes-runner", which names the game, not the binary.
- **D-09:** Cover the binary as well, with the same positive movie.
  - Replay it through `nesturbator-run --movie --rom tests/roms/runner.nes`.
  - Pin its frame and audio hashes as `nesrunner/...` rows in
    `tests/runner/hashes.txt` (D-13).
  - The game is then exercised end to end through the runner and through
    libretro parity (D-14).
- **D-10:** The semantic bytes come from the v1 source (MIT, readable) and were
  confirmed on the release ROM.

  | Address | Name | Meaning |
  |---|---|---|
  | `$3E` | `current_gamestate` | 0 title, 1 credits, 2 game, 3 dead (DED), 4 highscore, 5 seed |
  | `$5D`–`$60` | `PlayerScore3..0` | Base-100 digits, one binary byte each; `$60` is the lowest. The score rises by 1 per 8-pixel column crossed while alive in game |

  - Never use these, because they change without input:
    - `$B2`, the sound-driver frame counter;
    - `$20`, `$1D`, `$27` (`calc_scroll`), `$4E` and `$4F`;
    - the stack page;
    - `rng_seed`.
  - **Layout canary**, asserted first: after title init, `$4D == 4` and
    `$50..$53 == 02 05 04 01`. At game start, `$61..$68 == $F0`. A wrong ROM
    or a mistyped address then fails loudly.
  - The ROM's sha256 pin is checked before anything else.
  - The planner may re-derive the addresses from the source as a cross-check.

### Pass predicates and negative controls
- **D-11:** One executable runs the positive case and every control, and judges
  each with one pure function:
  `judge(trace, no_input_trace) -> bitmask of failed predicates`.
  - `main` asserts that `judge(positive) == 0` and that
    `judge(control_k) == expected_mask_k` exactly. A predicate that silently
    stops checking therefore shows up as "expected failure did not fail".
  - Failure messages name the predicate, the frame, the value and the threshold.

  The runs, about 600 frames each or as D-06 pins them:

  | Run | Construction | Must fail |
  |---|---|---|
  | Positive | Start, then the jump list | nothing |
  | 1. No input | An all-zero movie. This is also the baseline N for the A/B predicates | P1, P2, and the A/B parts of P3 |
  | 2. Attract stand-in | nes-runner has no attract or demo mode, so it uses menu navigation that is busy but never plays: Down/Up on the title, then enter Credits. `$3E` = 1, and frames and audio change | P1, P2 |
  | 3. Failed load | Truncate a copy of the ROM in memory to 16 + 16384 bytes. Assert `NESTURBATOR_ERR_CARTRIDGE`, then keep calling `run_frame` the way a host that ignores the error would. The test frame is static and the audio is all zeros | P1, P2, P3 |
  | 4. Start-only | The same Start, no jumps. The player dies at about frame 270–312, `$3E` = 3 and the score freezes near 16 | P1 |

  The runner binary exits 1 on a bad ROM and never falls back, so control 3
  must run in the library.
- **D-12:** Predicates are integer-only. Windows are 30 frames and sampling is
  every 30 frames.
  - **P1, semantic RAM.**
    - `$3E == 2` at every checkpoint from the first after Start + transition
      until the movie ends.
    - The score strictly increases between checkpoints and ends at or above K.
      K is pinned at about 70% of the observed final score.
    - In N, `$3E` stays 0 and the score stays 0.
    - Causality: the first frame where the RAM of the movie run and of N
      differ is at or after the first Start frame.
    - Assert the actual values, not just that they changed.
  - **P2, the scene changes.**
    - Every pair of frames 30 apart inside the play window differs in at least
      F changed native pixels.
    - At a fixed frame after the transition, the movie frame differs from N's
      frame in at least 25% of pixels.
    - Distinct frame hashes are logged but are not a predicate. The title's
      rainbow palette cycle makes every idle frame distinct.
    - Pin F as the largest power of two at or below half the measured minimum.
      The scratch measurements were a gameplay minimum of about 10.7k pixels
      and an idle title of about 735, which suggests 4096. Record both numbers
      in a comment next to the constant.
  - **P3, the audio keeps varying.**
    - Every window from Start to the end, the last one included, has a
      peak-to-peak of at least 2048. The measured minimum was about 11.9k.
    - The window PCM hashes are pairwise distinct, so the output is not stuck.
    - At least 75% of the windows after the transition hash differently from
      the same window in N. This catches a constant tone.
    - The title music loops loudly, so audio alone cannot separate title from
      play. P1 and the A/B parts carry that.
    - The window hash reuses `runner/sha256.c` with the little-endian
      serialisation from `runner/audio_hash.c`, compiled into the test.
- **D-13:** Synthetic mutant traces feed `judge` directly, and each must fail.
  They are hand-built traces, cheap and with no emulation:
  - silence;
  - a constant tone;
  - an APU click followed by silence;
  - a static picture with a ticking RAM counter;
  - a rainbow-only title;
  - an NMI-alive hang (`$3E` = 0 with live audio);
  - dying early.

  The test runs in a few seconds. Register it with `add_test` and
  `TIMEOUT 120`, under the `ci` and `ci-msvc` presets, on all six legs, and
  keep it under ASan.

### BOARD-04 close-out scope
- **D-14:** "Every board's committed ROMs" means:
  - the 10 Holy Mapperel files under `tests/roms/hm/`;
  - the two derived copies built at test time, `m4w8k` and `m4tkrom`;
  - the three mapper-0 games (nesteroids, dabg, rhde);
  - nes-runner (mapper 1).

  Exempt, recorded explicitly:
  - AccuracyCoin. It is mapper 0, NROM is already covered by three games, and
    its scoreboard runs on every leg.
  - The fuzz corpus seeds. They are malformed inputs run by `fuzz.regress`.
- **D-15:** Hash rows to add to `tests/runner/hashes.txt`:
  - **Holy Mapperel audio rows.** Holy Mapperel rows carry frame hashes only
    today: `holymapperel_hash_run` never passes `--hash-audio`, so the
    criterion's "frame and audio" is unmet for boards 1, 2, 3, 4 and 7. Add
    `holymapperel/<key>/audio/pcm sha256` and `.../audio/transitions sha256`
    for all 12 keys plus the two `.saved` keys, which is 28 rows. Take them
    from the runner call that already runs 2N frames.
  - **nes-runner rows.** `nesrunner/boot/frame {1,30,60,120,180}` and the boot
    audio rows, in the game loop pattern. Add `nesrunner/play/frame ...` rows
    and play audio rows from the positive movie, in the DABG movie-row
    pattern.
- **D-16:** Each hash key is defined once.
  - Today the key list is spelled out three times (`write_hashes.cmake:53`,
    `hash_inventory.cmake:34` and `:135`), and the counts `36 + ...` are
    hard-coded (`write_hashes.cmake:216`, `hash_inventory.cmake:16`).
  - Create `tests/cmake/hash_keys.cmake` with one function that returns the
    expected key list.
    - `write_hashes` asserts that the keys it generates equal that list,
      replacing the counts.
    - The validator and its self-test fixture use the same function.
    - `hash.inventory.self_test` gains a "missing Holy Mapperel audio key" case.
  - This folds in **07-WR-02**: the validator now checks keys on every
    platform artifact, not only the last.
- **D-17:** Add a drift guard: a policy test that globs `tests/roms/*.nes` and
  `tests/roms/hm/*.nes`. It fails if a committed ROM is neither hashed nor in
  the exemption list (D-14), and at configure time if any ROM list is empty.
- **D-18:** Libretro parity covers every ROM in D-14.
  - Add a mode to `tests/libretro/libretro_host.c`:
    `--rom-parity <module> <rom> <frames> <ppm> [--sav-in FILE] [--sav-expect FILE]`.
    - It reuses `compare_with_ppm` (`libretro_host.c:841`).
    - It compares against the runner's existing P6 dumps, so the runner is not
      run a second time.
  - Register one CTest per key, named `libretro.parity.<key>`, generated from
    the same lists as the dump tests. A single looping test was rejected: it
    hides which ROM failed, lets state bleed across ROMs and can pass on an
    empty list.
  - Compare frames as follows:
    - games at 180;
    - Holy Mapperel at its N;
    - nes-runner at its boot frame and at its movie-end frame (replaying the
      movie through `retro_set_input_state`).
  - For `m1sxrom` and `m4tkrom`:
    - run 1 compares `retro_get_memory_data` and `retro_get_memory_size` with
      the runner's `.sav`;
    - run 2 copies the `.sav` into the span after `retro_load_game` and before
      the first `retro_run`, as RetroArch loads a `.srm`.

    Each run is ordered behind the matching runner save fixture.
  - Anti-vacuity checks:
    - `retro_load_game` returns true;
    - `video_calls == frames`;
    - the ppm header is exact;
    - the compared frame is not the test-frame hash;
    - the host frame buffer is pre-filled with a sentinel before every
      `retro_run`, so unwritten pixels fail the compare.
  - The tests are ordinary CTests, so they run on all six legs and under ASan.
    - Measure the ASan cost, which is about 2 minutes extra for the 600-frame
      M4 ROMs.
    - If the 10-minute `asan` timeout in `ci.yml` is threatened, raise it,
      recording the measured run in a comment. Do not drop tests.
- **D-19:** Fold in 05-CR-01 as a small plan.
  - The defect: after `nesturbator_reset`, 21 top-left pixels of the first
    frame are never written. The frame hash then depends on the host's buffer,
    which contradicts "the same hashes everywhere".
  - The D-18 sentinel pre-fill would expose it on any reset path.
  - Fix as `05-REVIEW.md` suggests: run the 7 reset cycles inside the next
    `run_frame`, or place the PPU so they end at dot 0.
  - Add the sentinel-buffer test.
  - If the fix changes any pinned hash, bump the revision together with D-03,
    once for the phase.

  Other open review items stay deferred (see `<deferred>`).
- **D-20:** README and the public header comments change in the same change as
  the behaviour (CLAUDE.md rule 6):
  - NMI edge behaviour;
  - the coverage statement (BOARD-04 rows and parity);
  - nes-runner as the boot-to-play proof;
  - the README states plainly that six-platform equality proves consistency,
    and that correctness comes from Holy Mapperel codes, AccuracyCoin and the
    blargg nightly.

### Release
- **D-21:** The phase PR gets a Conventional Commit `feat:` title, for example
  `feat: NMI edge fix, board parity close-out and nes-runner boot-to-play`.
  release-please cuts the milestone's final release from it. No release
  workflow changes are planned.

### Plan ordering, at Claude's discretion within these constraints
- The NMI fix, its test and the re-pinned hashes come first. Everything else is
  measured on the fixed core.
- nes-runner intake (manifest, notices, ROM) lands before the drift guard and
  the parity plans, or the drift guard fails.
- The movie writer and the boot-to-play check come before the
  `nesrunner/play` hash rows and nes-runner parity.
- The 05-CR-01 fix lands before parity sentinel buffers are turned on.

### Claude's Discretion
- Exact file split for the check (one driver, or a driver plus a small trace
  helper).
- Exact checkpoint frames, K, F and window counts. They are pinned by the
  procedures above.
- Whether the libretro parity also compares game PCM hashes against the runner.
  This is optional extra coverage; skip it if it costs too much.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Scope and requirements
- `.planning/ROADMAP.md` §Phase 10: goal, success criteria 1–3, research flag
- `.planning/REQUIREMENTS.md`: BOARD-04, PLAY-01
- `.planning/research/SUMMARY.md` item 5, `.planning/research/PITFALLS.md`
  (false-passing boot-to-play, Pitfall 12): why the weak checks are banned
- `.planning/research/STACK.md`: the nes-runner v1 row (pin, sha256, header)

### Hardware sources
- NESdev wiki, PPU registers (PPUCTRL bit 7 and the NMI 0→1 rule), cited in the
  D-01 hardware comment
- `.planning/preparation/NES-HARDWARE-PPU-CARTRIDGE.md`: the PPU and NMI notes
  used by the project

### Asset rules
- `ASSET_POLICY.md`, `tests/roms/manifest.txt` (format header),
  `THIRD-PARTY-NOTICES.md`
- nes-runner v1 source (MIT): https://github.com/zorchenhimer/nes-runner/tree/v1
  (`runner.asm`, `title.asm`, `game.asm`, `utils.asm`, `ded.asm`,
  `scores.asm`); release asset https://github.com/zorchenhimer/nes-runner/releases/tag/v1;
  licence commit `e2d70cf`

### Open review items folded in
- `.planning/phases/05-tune-up-and-v1-debt/05-REVIEW.md` CR-01 (with its fix
  options) and `05-REVIEW-DISPOSITION.md`
- `.planning/phases/07-uxrom-cnrom-and-axrom/07-REVIEW-DISPOSITION.md` WR-02

### Prior phase decisions this phase builds on
- `.planning/phases/09-mmc3/09-CONTEXT.md` D-01/D-02: ROM intake and the derived
  `m4w8k` and `m4tkrom` copies
- `.planning/phases/08-mmc1-and-battery-saves/08-CONTEXT.md`: the `:save`
  two-run chain and the save-generation semantics
- `.planning/preparation/LIBRETRO-AND-RUNNER.md` §5: the runner exit-status
  contract and the movie options

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `nesturbator_peek_cpu_ram`, `nesturbator_get_memory`,
  `nesturbator_save_generation` (`include/nesturbator.h`): everything the
  check reads, with no new API.
- `runner/movie.c` (NMOVIE1 reader) and `tests/runner/game_movie.c`: the writer
  pattern for D-05.
- `runner/sha256.c`, `runner/audio_hash.c`: window hashes for P3.
- `tests/runner/test_spectral.c`: a library-linked test that reads audio, and
  the model for D-08.
- `tests/libretro/libretro_host.c` (`compare_with_ppm` at line 841; the
  `_WIN32` LoadLibrary branch): the base for parity mode.
- `tests/cmake/write_hashes.cmake`, `hash_inventory.cmake`,
  `tests/holymapperel/roms.cmake`, `hm_derive.cmake`, `check_sav.cmake`.
- `src/testcard.c`: the static test frame. Its hash is the anti-vacuity
  sentinel.

### Established Patterns
- ROM intake: raw bytes, a manifest line, a notices entry and a
  `manifest_sha256` guard (Phases 7–9 D-01).
- Behaviour-revision bump with re-pinned hashes in one commit (Phase 6).
- Tests use in-repo `tests/check.h` and CMake `-P` scripts. There is no test
  framework dependency.
- The `policy.no-skip` rule: unrunnable tests are not registered.

### Integration Points
- `src/ppu.c` `$2000` write handler (about lines 629–636): D-01.
- `src/instance.c:166-172`, `src/cpu.c` reset, `src/ppu.c` reset: D-19.
- `tests/CMakeLists.txt`: new targets, parity tests, the drift-guard policy
  test.
- `.github/workflows/ci.yml`: the `hash-equality` job (no change expected) and
  the `asan` timeout, which may change after measurement.

</code_context>

<specifics>
## Specific Ideas

- The predicates are designed against an adversarial list of broken cores that
  must each be caught:
  - an NMI storm or hang (the current bug);
  - input ignored or latched late;
  - test-frame fallback;
  - silence or a click;
  - a constant tone;
  - a static picture with a ticking counter;
  - a palette-only animation;
  - menu activity without play;
  - an early death;
  - a time-shifted wrong state.
- The current core would fail the PLAY-01 check, which is the intended
  behaviour. The phase must make it pass by fixing the core, never by weakening
  a predicate.

</specifics>

<deferred>
## Deferred Ideas

- A runner `--peek-ram` or `--dump-ram` flag. Add it only if Playstead or
  another host shows it needs out-of-process RAM watches.
- A closed-loop movie bot, if core timing churn makes pinned jump lists
  expensive to maintain.
- Stallar (MMC3, MIT) as a second real-game check.
- Open review items not folded in:
  - 05-WR-01 and 05-WR-03 (reset parity against hardware; APU reset phase);
  - 06-WR-02 and 06-IN-01;
  - 07-WR-01 (AxROM soft-reset wording), 07-IN-01 to 07-IN-03;
  - 08-WR-01 to 08-WR-03 and 08-IN-01 to 08-IN-02;
  - 09-WR-01 (accepted), 09-IN-01 and 09-IN-02.
- Backlog 999.1: the nightly `suite-flake` timeout tightening.

</deferred>

---

*Phase: 10-close-out-and-boot-to-play*
*Context gathered: 2026-10-11*
