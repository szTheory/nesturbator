# Phase 10: Close-out and boot-to-play - Research

**Researched:** 2026-10-11
**Domain:** C emulator core (PPU/NMI), CMake/CTest test infrastructure, libretro host test, hash inventories, a real MIT NES game as a playability oracle
**Confidence:** HIGH. Every number below was measured this session on a scratch copy of `HEAD` plus the D-01 patch (outside the tracked tree). Items that rest on training knowledge or on CONTEXT.md alone are marked in the Assumptions Log.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
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

### Deferred Ideas (OUT OF SCOPE)
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
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| BOARD-04 | Frame and audio hashes for every board's committed ROMs are identical on all six platforms, and the libretro test program receives equal frames. | Section "Hash inventory: exact edit sites and row arithmetic" (what moves, what is added, the 07-WR-02 fix); section "libretro parity mode" (host structure, sentinel limits, save-file ordering race); "CI wiring". The 24 rows that move after D-01 are listed exactly. |
| PLAY-01 | A CI test proves nes-runner plays from boot through interactive play: a movie reaches gameplay, a meaningful RAM byte differs with and without input, the scene visibly changes, audio keeps varying, and the three negative controls fail the same check. | Section "Measured values on the fixed core" (every pin: Start frame, jump list, `$3E`, score trajectory, save RAM, pixel and audio figures, control behaviour) and "Predicate design and expected masks". The research flag is answered: yes, after D-01 nes-runner reaches gameplay and writes its save. |
</phase_requirements>

## Project Constraints (from CLAUDE.md)

Extracted from `CLAUDE.md` and `.claude/CLAUDE.md` at the repository root. Treated as locked.

- **Own code.** All emulation and test code is written here. `libretro.h` is the only vendored file. The core library links only the C memory functions. No new dependency (owner: "another copy and paste is better than another dep").
- **Three deliverables only** (library, runner, libretro adapter). No window, audio-device or input-device code.
- **Clean tree.** No ROM or BIOS bytes except test ROMs listed in `tests/roms/manifest.txt`. No personal paths, email addresses or names. Commits use the GitHub noreply identity. `scripts/hygiene.sh` runs on every commit and push.
- **Clean room.** Do not open GPL or LGPL emulator source; no copy in the workspace; reference emulators run only as released binaries. (nes-runner is an MIT game, not an emulator; its source is readable. This research did not open any emulator source.)
- **Deterministic core.** Integer-only, all state in the instance, same hashes on every platform.
- **Tested and documented together.** Every behaviour change lands with a test run by `cmake --workflow --preset ci`, and the README and the public header comments change in the same change. Checks are automated; none waits on a person.
- **One step at a time.** Phase branch; merge by pull request with a Conventional Commit title.
- **Style.** Small modules, plain control flow, fixed-width integer types. A hardware comment says what the hardware does and cites its source. Follow the ponytail ladder: remove the need, reuse what is here, use the standard library, use the platform, only then write the minimum new code.
- **Language.** C17 without extensions. Warnings (`-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wstrict-prototypes -Wmissing-prototypes -Wvla`, `/W4` on MSVC) are errors in the `ci` and `asan` presets.
- **Delivery.** The owner creates each phase's branch; it merges by pull request into a green `main` and is released automatically by release-please from the Conventional Commit title.

## Summary

The phase is six small, mostly file-disjoint pieces of work, and the research confirms each is feasible with no new dependency. The decisive findings, each measured this session:

1. **D-01 is correct and sufficient.** On pristine `HEAD` nes-runner never leaves the title (`$3E` stays 0 after Start; the frame is an NMI storm). With only the D-01 patch applied in a scratch copy, Start moves `$3E` to 2 on the next frame, the game plays, and it writes its save (`5A 6F` in the first two battery bytes, save generation 2). The patch compiles clean under the `ci` preset (`-Werror`), passes 445 of 446 `dev` and `ci` tests, and the single failure is `runner.write_hashes.content`, as CONTEXT said. The nightly `mmc3-oracle` lane (13 tests) still passes. There is no `ppu_vbl_nmi` lane in this repository, so nothing else moves.
2. **Exactly 24 rows of `tests/runner/hashes.txt` move** after D-01: 4 nesteroids rows change hash and ticks; 20 rows change only their `ticks` column (9 DABG movie rows, 4 RHDE boot rows, 7 Holy Mapperel rows). No audio row and no frame-1 row moves. Full list below.
3. **Revision bump touches four sites, not one.** `include/nesturbator.h:53`, `tests/core/test_api.c:96`, the `PHASE8_DECLARATIONS_SHA256` constant in `tests/cmake/vector_api_policy.cmake` (the revision macro is inside the hashed declaration stream; the new value is `ae22000086cf5a9a6ef97fe36fdfe65cf57b8637b9601e09b6e2d65566be8ebc` if no other public declaration changes), and `README.md:25`.
4. **05-CR-01 does not reproduce at `HEAD`.** I pre-filled the video buffer with an impossible pixel value and ran the first frames after `nesturbator_reset` on 6 ROMs, 121 pre-reset frame counts each, plus a sentinel-A versus sentinel-B frame comparison. Zero unwritten pixels, zero differing frames, on both pristine `HEAD` and the patched copy. The 21 top-left pixels are written, deterministically, by the end of the same `run_frame`. D-19 should therefore become "add the sentinel test; close CR-01 in the disposition file with this evidence; change the core only if the new test fails". That also removes one possible behaviour-revision change.
5. **The expected failure mask of the Start-only control is `P1|P2`, not `P1` alone** (D-11 table). After the player dies at frame 265 the screen is static (0 or 189 changed pixels per 30 frames), so P2 fails too. Likewise the "25% of pixels differ from N" figure (D-12) does not separate the attract stand-in (about 30%) from the movie (about 70%); 50% does. Both are in the "exact thresholds are pinned by procedure" discretion area.
6. **The libretro sentinel cannot reach the core's pixel buffer.** The adapter owns a static `video[]` buffer and a static `xrgb[]` buffer; a host-side sentinel only proves the callback delivered a full frame. The CR-01 sentinel test must sit at library level (`tests/core`).
7. **`--movie` mode prints a hash line for every frame**, and `write_hashes.cmake` selects frames 1, 30, 60, 120, 180 with a regex. With a 600-frame nes-runner movie those five rows only cover the title and the 84-frame transition. Gameplay starts at frame 145; pin extra rows (300, 450, 600) or the `nesrunner/play` rows prove nothing about play.
8. **Timeout headroom is tighter than CONTEXT assumes.** Under ASan the new boot-to-play executable needs about 2,400 emulated frames (about 55 s on this Mac, roughly 3.5x that on the CI image going by the M4 figures in `ci.yml`). `TIMEOUT 120` will not hold under ASan; use 300 and add a COST so it starts early. The `asan` job's 10-minute cap probably still holds because `runner.write_hashes` (184 s here under ASan) is the long pole and the suite averages only 1.8-way parallelism.

**Primary recommendation:** Execute in this order: (1) D-01 fix + regression tests + revision bump (4 sites) + re-pin the 24 rows; (2) nes-runner intake; (3) `hash_keys.cmake` + Holy Mapperel audio rows + nes-runner boot rows + 07-WR-02 + drift guard; (4) movie writer + `test_boot_to_play` + `nesrunner/play` rows; (5) libretro `--rom-parity` mode and per-key parity CTests; (6) CR-01 sentinel test and disposition; then README/header and the `feat:` PR title.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| `$2000` NMI edge rule | Core library (`src/ppu.c`) | - | Hardware behaviour of the PPU register file; the CPU only consumes `nmi_pending`. |
| Behaviour revision, version macros | Public header (`include/nesturbator.h`) | Tests (`test_api.c`, `vector_api_policy.cmake`) | The header owns the constant; two tests pin it. |
| Movie byte generation | Test tooling (`tests/runner/game_movie.c`, build-time) | - | No movie bytes are committed (D-05). |
| Movie replay in the proof | Test program (`tests/runner/test_boot_to_play.c`) via the public API | Runner binary (`--movie`) for the hash rows | D-08 and D-09: the library does the semantic check; the binary is covered by pinned hashes and libretro parity. |
| RAM observation | Public API `nesturbator_peek_cpu_ram`, `nesturbator_get_memory`, `nesturbator_save_generation` | - | No new API (D-08). |
| Hash key list (single source) | CMake script `tests/cmake/hash_keys.cmake` | `write_hashes.cmake`, `hash_inventory.cmake` | D-16: defined once. |
| Six-platform equality | CI `build` matrix + `hash-equality` job | `hash_inventory.cmake` | Existing mechanism; no workflow change planned. |
| libretro frame parity | libretro host test program (`tests/libretro/libretro_host.c`) | Runner `--dump-frame` P6 images | The adapter converts through the same `nesturbator_host_convert` as the runner (`libretro/libretro.c:210`). |
| Drift guard | CMake policy test | `tests/roms/*.nes` glob | D-17. |
| Release | release-please on the `feat:` PR title | `release.yml` (unchanged) | D-21. |

## Standard Stack

No external package is installed by this phase. Everything is in-repo or the build tools already required.

### Core
| Tool | Version | Purpose | Why Standard |
|------|---------|---------|--------------|
| C | C17, extensions off | Core, test programs | Project constraint |
| CMake / CTest | 3.25 minimum; 4.4.3 installed here | Build, `add_test`, fixtures, `-P` scripts | All tests are CTests; every `tests/cmake/*.cmake` script must carry `cmake_minimum_required(VERSION 3.25)` (`cmake.script_policy`) |
| Ninja | 1.13.2 here | Generator | Project preset base |
| `tests/check.h` | in-repo | `CHECK`, `CHECK_EQ_U64`, `CHECK_DONE` | No framework dependency |
| `runner/sha256.c`, `runner/audio_hash.c`, `runner/movie.c` | in-repo | Window hashes, NMOVIE1 reader | Reuse (D-12, D-05); `runner.movie.test` already compiles `runner/` sources into a test with `target_include_directories(... ${CMAKE_CURRENT_SOURCE_DIR}/runner)` |
| clang-format | 18.1.8 at `/opt/homebrew/opt/llvm@18/bin/clang-format` here (CI image 18.1.3) | `hygiene.format` check of every tracked `.c/.h` | `.clang-format`: LLVM base, 4 spaces, 100 columns, Linux braces, `SortIncludes: Never` |

### Supporting
| Tool | Purpose | When to Use |
|------|---------|-------------|
| `nesturbator-run` | Hash rows, P6 dumps, save files | `write_hashes.cmake`, parity dump fixtures |
| `runner.game_movie` | Movie writer | Extend with nes-runner scenarios |
| `holymapperel-derive` | Derived ROM copies | Unchanged; parity reads `build/tests/holymapperel/derived/<key>.nes` |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Library-linked test (D-08) | Runner `--peek-ram` flag | Rejected in CONTEXT (public CLI surface). Deferred. |
| Fixed jump list (D-06) | Closed-loop bot | Rejected in CONTEXT; the 600-frame score threshold catches any desync. |

**Installation:** none.

**Version verification:** Not applicable (no registry packages). Tool versions above were read from the installed binaries this session.

## Package Legitimacy Audit

Not applicable: this phase installs no external package. The only new external artefact is the nes-runner ROM, covered by the manifest sha256 guard (see "nes-runner intake").

**Packages removed due to [SLOP] verdict:** none
**Packages flagged as suspicious [SUS]:** none

## Architecture Patterns

### System Architecture Diagram

```
                    BUILD TIME                                         TEST TIME (every CTest leg)
 tests/runner/game_movie.c --writes--> *.nmovie ----+--> nesturbator-run --movie --rom runner.nes
   (constants: Start idx 60,                        |        |--> hashes.txt rows (nesrunner/play/...)  --> hash-equality (6 legs)
    jump list, N, attract, start-only)              |        '--> --dump-frame N:ppm  --------------------+
                                                    |                                                     |
                                                    +--> test_boot_to_play (library, in-process)          v
                                                    |      load ROM (sha256 pin first)             libretro.host --rom-parity
                                                    |      for scenario in {pos, none, attract,     (retro_load_game, retro_run xN,
                                                    |        start-only, truncated-ROM}:             host_buttons from movie,
                                                    |        set_input -> run_frame -> peek RAM      compare_with_ppm, sentinel fill)
                                                    |        $3E, $5D..$60, pixels/30f, PCM/30f,
                                                    |        save generation, SRAM[0..1]
                                                    |      judge(trace, N) -> failed-predicate mask
                                                    |      main: judge(pos)==0, judge(ctl_k)==expected_k
                                                    |      judge(synthetic mutant traces) != 0
 tests/roms/runner.nes --sha256 pinned in manifest.txt, THIRD-PARTY-NOTICES.md, drift guard (D-17)
 src/ppu.c $2000 write --D-01--> nmi_pending only on 0->1 with vblank set
```

### Recommended Project Structure
```
src/ppu.c                              # D-01 edit (lines 629-636)
include/nesturbator.h                  # revision 5 -> 6 (+ comment text)
tests/ppu/test_registers.c             # D-02 register-level cases (existing file, existing target ppu.registers)
tests/core/test_nmi_edge.c             # D-02 end-to-end synthetic ROM (new target core.nmi_edge)
tests/core/test_reset.c                # CR-01 sentinel test (existing target core.reset) -- or a new small file
tests/roms/runner.nes                  # D-04 (git add -f)
tests/roms/manifest.txt                # one new line
THIRD-PARTY-NOTICES.md                 # MIT text + provenance note
tests/runner/game_movie.c              # new scenarios + per-scenario frame count + pinning-procedure header comment
tests/runner/test_boot_to_play.c       # D-08/D-11/D-12/D-13 (new target runner.boot_to_play)
tests/cmake/hash_keys.cmake            # D-16 single key list (new; needs cmake_minimum_required(VERSION 3.25))
tests/cmake/write_hashes.cmake         # consume hash_keys; HM --hash-audio; nesrunner boot/play
tests/cmake/hash_inventory.cmake       # consume hash_keys; 07-WR-02; new self-test case
tests/cmake/rom_coverage.cmake         # D-17 drift guard (new policy test)
tests/runner/hashes.txt                # re-pinned + new rows
tests/libretro/libretro_host.c         # --rom-parity mode
tests/CMakeLists.txt                   # registrations
README.md, include/nesturbator.h       # same-change docs (CLAUDE.md rule 6)
.planning/phases/05-.../05-REVIEW-DISPOSITION.md, 07-.../07-REVIEW-DISPOSITION.md   # CR-01, WR-02 rows
```

### Pattern 1: One pure `judge` plus exact expected masks (D-11)
**What:** Collect a `struct trace` per run (per-frame `$3E`, score bytes, save generation; 30-frame sampled pixel-change counts and PCM peak-to-peak and window hashes). `judge(const trace *t, const trace *n) -> unsigned` returns a bitmask `P1|P2|P3`. `main` asserts `judge(pos) == 0` and `judge(ctl) == EXPECTED_MASK` exactly, then feeds hand-built mutant traces to `judge`.
**When to use:** All of PLAY-01.
**Example (shape only):**
```c
/* Source: pattern from D-11; thresholds are pinned from the measurements in this file. */
enum { P1_RAM = 1u, P2_SCENE = 2u, P3_AUDIO = 4u };
static unsigned judge(const struct trace *t, const struct trace *n);
/* main */
CHECK_EQ_U64(judge(&pos, &none), 0u);
CHECK_EQ_U64(judge(&none, &none), P1_RAM | P2_SCENE | P3_AUDIO);
CHECK_EQ_U64(judge(&attract, &none), P1_RAM | P2_SCENE);
CHECK_EQ_U64(judge(&trunc, &none), P1_RAM | P2_SCENE | P3_AUDIO);
CHECK_EQ_U64(judge(&startonly, &none), P1_RAM | P2_SCENE);   /* measured; see "Predicate design" */
```

### Pattern 2: Build-time movie writer, scenario by name (D-05)
`tests/runner/game_movie.c` today takes `<path> <scenario>`, has a fixed `FRAME_COUNT 180u`, and writes bytes `16 + 4*frame` (port 0) and `18 + 4*frame` (port 1). Make the frame count per scenario, keep DABG's 180, add `nesrunner-pos`, `nesrunner-none`, `nesrunner-attract`, `nesrunner-startonly` (600 each). One constants table, used by both `write_hashes.cmake` and the test.

### Pattern 3: One CTest per key with fixtures (D-18)
Existing pattern in `tests/CMakeLists.txt` lines 236-340: a `.dump` test (`FIXTURES_SETUP holymapperel_<key>`) then dependent tests with `FIXTURES_REQUIRED`. Parity tests copy it: `libretro.parity.<key>` with `FIXTURES_REQUIRED` on the matching dump fixture.

### Anti-Patterns to Avoid
- **Judging play by "frames changed" or "audio non-silent" alone.** On the broken core the NMI-storm frames change by about 40,000 pixels per 30 frames and the title music is loud (peak-to-peak 12-16k). Only the `$3E`/score bytes and the A/B comparison separate title from play.
- **Pinning only frames 1-180 of the play movie.** They are title and transition; frames 30 and 60 hash identically to the boot rows.
- **Reading RAM the game changes without input** (`$B2`, `$20`, `$1D`, `$27`, `$4E`, `$4F`, stack, `rng_seed`; CONTEXT D-10).
- **Hand-writing key lists in three places** (the reason for D-16).

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| SHA-256 of PCM windows | A second hash | `runner/sha256.c` + the LE serialisation loop from `runner/audio_hash.c` | Same bytes as the pinned audio rows (D-12) |
| Movie file parsing in the test or libretro host | Own reader | `runner/movie.c` (`nesturbator_movie_read`) | Validates magic, version 1, count, exact length |
| Synthetic cartridges | Own iNES builder | `tests/ines.h` (`ines_build`, `struct ines_spec`) | Already used by `ppu_fixture.h`, `test_reset.c`, `test_save.c` |
| Per-frame PPM compare | Own comparer | `compare_with_ppm` in `libretro_host.c:841` | Checks the exact `P6\n256 240\n255\n` header, then every pixel |
| Process spawning in the host | `system()` | `test_process_run` from `tests/test_process.h` (already used in `libretro_host.c`) | Portable across the six legs |
| Save-file check | New byte comparer | `tests/cmake/check_sav.cmake` (`-DFILE -DSIZE -DOFFSET -DHEX`) | Already used by the Holy Mapperel `:save` chains |
| Level-solving bot | Closed-loop bot | Offline search, documented procedure, fixed jump list | D-06 |

**Key insight:** every piece this phase needs already exists as a pattern in the repo; the work is wiring and pinning, not new machinery.

## Measured values on the fixed core

All figures: scratch copy of `HEAD` plus the D-01 patch only, `dev` (Debug) build; the `ci` Release build produces identical hashes (the same single test failure and the same `write_hashes` output). Frame numbers are 1-based; movie "index" is 0-based, the loop variable in `game_movie.c` (index `k` is the input for frame `k + 1`). `nes-runner` sha256 re-verified: `1a67bfabdde2d96eacb04eb7f9d17e8da6c082d4976c318315fe31e2d19b4384`, 81,936 bytes, header `4e45531a 04 02 13 08 00 00 70` [VERIFIED: downloaded the release asset and hashed it this session].

### Baseline versus fix (the research flag)
| Core | Start held at index 60 | Result |
|------|------------------------|--------|
| pristine `HEAD` | any | `$3E` stays 0, save generation 0, frames churn about 40,000 px per 30 frames (NMI storm) |
| `HEAD` + D-01 | index 60..63 | `$3E` = 2 from frame 61; plays |

Start works for any index from 4 upward (`$3E` = 2 on the following frame); indexes 0 and 2 do not register (title not initialised yet / input ignored). `$4D == 4` and `$50..$53 == 02 05 04 01` hold from frame 4 on.

### Positive run (Start index 60, hold 4; 6 jumps)
Jump list as `{start index, hold}`: `{251,21} {303,21} {351,21} {408,23} {463,21} {558,25}`; A mask `0x01`, Start mask `0x08`. Survives the full 600 frames and beyond (checked alive to frame 640).

| Fact | Value |
|------|-------|
| `$3E` becomes 2 | frame 61 |
| `$61..$68` all `$F0` (layout canary at game start) | from frame 135 (`$68` later changes with play) |
| Save generation reaches 2 | frame 137; stays 2 through frame 600 |
| First score > 0 | frame 145 |
| SRAM bytes 0-1 | `5A 6F` |
| Score at checkpoints (30-frame grid, `$60`) | f150:1 f180:5 f210:9 f240:12 f270:16 f300:19 f330:22 f360:25 f390:29 f420:33 f450:37 f480:37 f510:41 f540:45 f570:49 f600:52 |
| Final score at frame 600 | 52 (so K = 36 is "about 70%") |

Note the plateau f450 to f480 (37 to 37): "strictly increasing between consecutive 30-frame checkpoints" would fail on the correct run. Compare checkpoints 60 frames apart, or require non-decreasing plus a minimum total.

### Pixels (changed native pixels between frames 30 apart, sampled every 30 frames)
| Run | f60 | f90 | f120 | f150 | f180 | f210 | f240 | f270 | f300 ... f600 |
|-----|-----|-----|------|------|------|------|------|------|---------------|
| positive | 735 | 29374 | 41190 | 33144 | 11209 | 11272 | 11354 | 11724 | 11782, 12417, 12956, 12672, 12712, 12122, 12167, 12796, 12683, 13297, 13043 |
| no input (N) | 735 | 735 | 735 | 735 | 735 ... all 735 |
| attract stand-in | 735 | 767 | 18845 | 0 | 0 | 0 | 3625 | 6211 | 6260, 6328, 6148, 6027, 5621, 5877, 5885, 5936, 5757, 5584, 5271 |
| Start-only | as positive to f270 (10720) | | | | | | | | f300 61440, f330 61440, f360 0, then 189, 189, 189, 0, 189, 189, 189, 0 |
| truncated ROM | 0 at every sample (the test card is static) |

Minimum over the play window (f180 to f600) of the positive run: 11209. Idle title: 735. So F = 4096 (largest power of two at or below 11209/2 = 5604), as CONTEXT suggested. Record both numbers beside the constant.

### Pixels versus N at the same frame
Positive 69.2% to 70.9% (f150 to f600). Attract 29.6% to 30.8%. Start-only 70.6% until death, 100% at f300, 27.4% from f420. A threshold of 25% passes the attract control; **use 50%** (the positive run's minimum is 69.2%, the controls' maximum below the death frame is 30.8%).

### Audio (peak-to-peak per 30-frame window, sampled every 30 frames; first value is the window ending f30)
- Positive: 14322 13651 14516 15859 13743 13506 13762 11689 14445 11981 13860 13955 10771 14470 15645 10364 16401 12025 15095 13180. Minimum from Start onward: 10364 (window ending f480). A 2048 floor has a 5x margin.
- N: minimum 12301 (title music is loud, so peak-to-peak never separates title from play).
- Truncated ROM: 0 in every window. Start-only after death: minimum 4141 (still above 2048).
- Window PCM hashes: positive windows are pairwise distinct and differ from N's from the third window (f90) on, 18 of 18 post-Start windows. Attract differs from N from the fourth window on. N's own windows are pairwise distinct. The truncated-ROM windows alternate between exactly two hashes (798 and 799 samples per frame), so the pairwise-distinct predicate fails there.
- Test-card frame hash (anti-vacuity sentinel for the failed-load control): `b49e9be44573a4de82d845179d4389a0a6e28e934bd9ab31516a40db2c0b0453` [VERIFIED: tests/CMakeLists.txt:29 and a runner call]; the test can equally compare against a cartridge-less instance's frame. With the test card and silence the audio hashes are `transitions e3b0c442...` (empty) and PCM `a9f2a202...e01e18` for 2 frames.

### Control behaviour (what each does on the fixed core)
| Control | Construction (indexes) | `$3E` | Score | Save gen | SRAM[0..1] |
|---------|------------------------|-------|-------|----------|-----------|
| No input | all zero | 0 | 0 | 0 | `00 00` |
| Attract stand-in | Down `0x20` @60 (2 frames), Up `0x10` @72 (2), Up @84 (2), Start `0x08` @110 (4) | 1 from frame 111 (Credits; cursor `$4C` = 3) | 0 | 0 | `00 00` |
| Start-only | Start @60 (4), no jumps | 2 from f61; **3 (dead) from frame 265**, score frozen at 16 | 16 | 42 at f265, 120 by f600 | `5A 6F` |
| Failed load | ROM truncated to 16 + 16384 bytes | `nesturbator_load_cartridge` returns 7 (`NESTURBATOR_ERR_CARTRIDGE`); RAM all zero | 0 | 0 (span size 0) | n/a |

Menu facts: `$4C` is the title cursor (0 Start Game; Down gives 1, which is the Seed screen and sets `$3E` = 5; Up from 0 wraps to 3, which is Credits). The runner binary exits 1 on the truncated ROM with "malformed or unsupported cartridge (status 7)" [VERIFIED: ran it]; the library control must therefore stay in-process.

### First differing RAM
Hashing all 2 KiB of CPU RAM per frame, the positive and N runs first differ at frame 61, which is the frame that consumes the first Start byte. The causality predicate (first differing frame at or after the first Start frame) holds with equality.

### Runner-binary rows (dev build, for cross-checking the executor's re-pin; Release is identical)
Boot (`--frames 180`): frame 1 `ticks 714816` hash `6d7bd1fa...0625`; frame 30 `ac8b18b7...0820`; frame 60 and frame 180 hash alike `d407ddd6...a055` (title palette cycle, period divides 120); frame 120 `65dbd823...d7d4`; audio transitions `3c74022b...f98a`, PCM `7418d671...f0e4`.
Play (`--movie`, 600 frames): frame 120 `b53d3a80...ccf3c`; 180 `1a9c0d42...8fd9`; 300 `8478ee93...3fc0`; 450 `d70d35de...4570`; 600 `6bf066cf...7025`; audio transitions `470ec17a...6873`, PCM `d21d8de6...e854`. Frames 1, 30 and 60 of the play run equal the boot run's.
The runner writes `runner.sav`, 8192 bytes, starting `5a 6f` [VERIFIED: ran `--movie ... --save-dir`].

### Timing (for TIMEOUT decisions)
| Measure | Value |
|---------|-------|
| One 600-frame scenario, Release | about 0.9 s (includes my frame buffering) |
| One 600-frame scenario, `-O0` ASan+UBSan, this Mac | about 14 s |
| Whole `ci` preset suite (Release, jobs 4) | 42 s; `runner.write_hashes` 14 s |
| Whole `dev` suite | 95 s; `runner.write_hashes` 83-93 s |
| Whole `asan` preset suite (this Mac, jobs 4) | 185 s wall, 340 s user; `runner.write_hashes` 184 s; M4 600-frame dumps 12-13 s each; `vectors.registration_policy` 38 s |

CI reference points in `.github/workflows/ci.yml`: `build` timeout 7 min (slowest observed 198 s on macOS Intel including packaging); `asan` timeout 10 min (a 4m14s run, then a 5-minute cap hit at 444/446 after Phase 9, per the comment).

## Predicate design and expected masks

Values pinned from the tables above. The planner should write the constants with the measured evidence in a comment, as D-12 asks.

| Constant | Pin | Evidence |
|----------|-----|----------|
| Start | index 60, hold 4, mask `0x08` | `$3E` = 2 at frame 61 |
| Movie length | 600 frames | survives to 640 |
| Checkpoint grid | frames 30, 60, ... 600 | D-12 |
| First play checkpoint | f150 (score 1) | first score at f145 |
| K (final score floor) | 36 | 70% of 52 |
| Score rule | score at checkpoint `i` > score at checkpoint `i - 2` (60-frame span), final >= K | plateau at f450-f480 |
| F (pixels per 30 frames) | 4096 | min 11209 (play), idle 735 |
| A/B pixel share vs N | 50% at frames 300 and 600 | positive >= 69%, attract <= 31% |
| Audio floor | 2048 peak-to-peak | min 10364 |
| Audio vs N | >= 75% of post-transition windows differ | positive 18/18 |

Expected failure masks, as measured (P1 = RAM, P2 = scene, P3 = audio):

| Run | P1 | P2 | P3 | Why |
|-----|----|----|----|-----|
| Positive | pass | pass | pass | |
| No input | fail | fail | fail (A/B part) | `$3E` 0; 735 px; identical windows to N |
| Attract stand-in | fail | fail | pass | `$3E` 1; zero changed pixels at f150-f210 and 3625 at f240; audio differs from N and is loud |
| Failed load | fail | fail | fail | all zero |
| Start-only | fail | **fail** | pass | dead at f265, score 16 < K; static after death. D-11 lists "P1" only; the measured mask is `P1|P2` |

If the owner prefers D-11's table literally, P2's window would have to end before frame 265, which also shortens the window for the positive run and weakens P2. Recommendation: keep the full window and record `P1|P2` as the expected mask. This stays inside D-11's rule that `judge(control_k) == expected_mask_k` exactly.

Window plan for the attract control: pairs inside the play window are taken from the same absolute window as the positive run (f150 to f600). Do not shift it per run.

### Jump-list pinning procedure (D-06; goes in the writer's header comment)
Tool used (not committed): a scratch program that links the library, snapshots the instance with `memcpy` of `struct nesturbator` plus `cart.prg_ram`, and replays.
1. Fix Start at index 60, hold 4.
2. With the list so far, find the first frame `d` at which `$3E` leaves 2.
3. Sweep the next jump's start `s` downward from `d - 1` and hold `h` from 3 to 28; keep the pairs for which the run is still alive 40 frames past `d`. With no jumps the player dies at frame 265.
4. The latest feasible start is the edge (a start one frame later dies at `d`). Pin the **middle** of the feasible window.
5. Repeat for the next obstacle; verify the finished list by shifting each jump start and hold by -2..+2 and requiring survival to frame 600.

Results. Latest-feasible list: `{261,21} {309,21} {357,21} {416,23} {469,21} {561,25}`. Survival windows around the pinned list (start shifts that keep the run alive, hold fixed): jump 1 start -11..+10, jump 2 -13..+5, jump 3 -10..+6, jump 4 -7..+8, jump 5 -11..+6, jump 6 -3..+3. Hold changes of +-2 did not change survival. So the pinned list tolerates +-2 on every jump with margin; jump 6 is the narrowest (+-3).

The list is specific to Start index 60. Another Start index shifts the first obstacle (death is always about Start + 205), so re-run the search if it changes.

## Exact edit sites

### D-01 (the NMI fix) - `src/ppu.c`
[VERIFIED: src/ppu.c:629-636] current text, read this session:
```c
    if (reg == 0x2000u) {
        nes->ppu.control = value;
        nes->ppu.t = (uint16_t)((nes->ppu.t & 0xf3ffu) | ((value & 3u) << 10));
        if ((value & 0x80u) != 0u && (nes->ppu.status & 0x80u) != 0u)
            nes->cpu.nmi_pending = 1u;
        else
            nes->cpu.nmi_pending = 0u;
    } else if (reg == 0x2001u) {
```
The patch I applied and tested in the scratch copy (builds under `-Werror`, `clang-format` clean):
```c
    if (reg == 0x2000u) {
        uint8_t old = nes->ppu.control;
        nes->ppu.control = value;
        nes->ppu.t = (uint16_t)((nes->ppu.t & 0xf3ffu) | ((value & 3u) << 10));
        if ((value & 0x80u) != 0u && (old & 0x80u) == 0u && (nes->ppu.status & 0x80u) != 0u)
            nes->cpu.nmi_pending = 1u;
        else if ((value & 0x80u) == 0u)
            nes->cpu.nmi_pending = 0u;
```
Add the hardware comment citing the page: `[NESdev Wiki, PPU registers: "Changing NMI enable from 0 to 1 while the vblank flag in PPUSTATUS is 1 will immediately trigger an NMI."]` [CITED: https://www.nesdev.org/wiki/PPU_registers, fetched this session; the page does not address the 1-to-1 rewrite, which follows from the "from 0 to 1" wording].

Other `nmi_pending` writers, unchanged and not to be touched: set at vblank start (`src/ppu.c:488`, only if bit 7 is already set), cleared at pre-render dot 1 (`:568`), cleared by a `$2002` read (`:494`), cleared by `ppu_reset` (`:614`), cleared by the CPU at NMI entry (`src/cpu.c:506`).

### Behaviour revision (four sites)
- [VERIFIED: include/nesturbator.h:53] `#define NESTURBATOR_BEHAVIOUR_REVISION 5` to 6. The comment above it reads "Raised whenever emulated behaviour changes the frame or audio output for the same inputs."
- [VERIFIED: tests/core/test_api.c:96] `CHECK_EQ_U64(v.behaviour_revision, 5);` to 6.
- [VERIFIED: tests/cmake/vector_api_policy.cmake:4-7] `set(PHASE8_DECLARATIONS_SHA256 "7ec4ca9a124042f9a64d090c4362af019c2b9773cb2aaf901683c0daade2d7f0")`. The normaliser strips comments and the three `NESTURBATOR_VERSION_*` defines only, so the revision define is in the hash. With the macro at 6 and no other declaration change, the script reports `ae22000086cf5a9a6ef97fe36fdfe65cf57b8637b9601e09b6e2d65566be8ebc` (measured by running the script against the scratch header). Update the constant and its comment ("behaviour revision 5 (the PPU fetch pipeline)"). Phase 6 did the same in `c902dc8` (12 lines changed in that file). Any other public-declaration change (none planned) would change this value again.
- [VERIFIED: README.md:25] "The behaviour revision is 5: the PPU fetch pipeline changed the frames of games that write the scroll or PPUCTRL while rendering." Rewrite for 6 and state the NMI edge rule. README line 149 area documents the vblank NMI edge; add the `$2000` rule there.

### `tests/runner/hashes.txt` rows that move (24 of 50)
Ran `runner.write_hashes` on pristine `HEAD` (identical to the committed file) and on the patched copy, then diffed. Unchanged: every audio row (all four games, both transitions and PCM), every `frame 1` row, `dabg/boot/*`, `dabg/*/frame 30`, `rhde` is ticks-only, Holy Mapperel m1sgrom, m1sjrom, m1skrom, m2, m3, m7.

| Rows | What changes | Why |
|------|--------------|-----|
| `nesteroids/boot/frame 30, 60, 120, 180` | hash and ticks | nesteroids was in the NMI storm; frames 120 and 180 no longer hash alike |
| `dabg/p0`, `dabg/p1`, `dabg/both` at frames 60, 120, 180 (9 rows) | ticks only | extra spurious NMIs removed (ticks shift by 24 to 336) |
| `rhde/boot/frame 30, 60, 120, 180` | ticks only | same |
| `holymapperel/m1surom/frame 200`, `m1sxrom/frame 400`, `m1sxrom.saved/frame 400`, `m4tnrom/frame 600`, `m4tkrom/frame 600`, `m4tkrom.saved/frame 600`, `m4w8k/frame 600` | ticks only | same; the Holy Mapperel result screens (hash column) are unchanged, and `holymapperel.*.decode` still reports code 0000 |

New `nesteroids` values on the patched dev build, for cross-checking the executor: frame 30 `ticks 21443256 ... f504aa7f...5c25`; frame 60 `ticks 42886752 ... d695d84c...b938`; frame 120 `ticks 85773792 ... 92cc9679...e0f6`; frame 180 `ticks 128661144 ... 6adc68fe...f528`.

Re-pin by running `runner.write_hashes` (or the bare `write_hashes.cmake`) and copying its output over `tests/runner/hashes.txt`; record the 24 rows and the reason in the plan summary (D-03). A frame hash that stays equal while `ticks` moves means the screen is the same but the CPU spent different cycles; that is the expected signature of removing a redundant NMI.

### 05-CR-01 (where it lives, and why it no longer fires)
[VERIFIED: .planning/phases/05-tune-up-and-v1-debt/05-REVIEW.md:52-61] "**File:** `src/instance.c:166-172` (with `src/cpu.c:1363-1380`, `src/ppu.c:297-304`, `src/frame.c:55-62`)" and "Scanline 0, dots 1 to 21 (x = 0 to 20, y = 0) are therefore never written." The disposition file marks it `open`.
Current code: `nesturbator_reset` (`src/instance.c:166-172`) calls `nesturbator__ppu_run_until`, `__ppu_reset`, `__apu_reset`, `__cpu_reset`; `cpu_reset` runs 7 bus cycles and every bus cycle catches the PPU up (`src/bus.c:11-31`) while `ppu.video_output` is NULL (`src/ppu.c:535`).
Measured: with a 0xFFFF sentinel (never a valid 9-bit pixel) pre-filled before each of the first two post-reset frames, 0 sentinel pixels remain for every pre-reset frame count 0..120 on nesteroids, DABG, RHDE, Holy Mapperel M4 and M1, and nes-runner; a run into a 0x0000-filled buffer equals a run into a 0xFFFF-filled buffer for 3 frames after reset on three games. The same holds on pristine `HEAD`. The pixels are not left stale: the frame's tail (the next PPU frame's first 20 or so dots) is rendered into the same buffer before `run_frame` returns, so the values are deterministic. Whether that is the hardware-correct content of those 21 pixels is a different question; it is outside the "hash depends on the host buffer" claim.
Recommendation for the planner: implement the sentinel test (library level, next to `test_reset.c`) as D-19 requires; if it passes, mark CR-01 `fixed` (or `skipped`) in `05-REVIEW-DISPOSITION.md` with this evidence and make no core change. Only a failing sentinel test justifies a fix; the fix options in the review stay valid (a PPU position that ends the 7 cycles at dot 0, or a small stash of the 21 pixels), but deferring the reset cycles into `run_frame` would break the `ticks + 168` assertions at `tests/core/test_reset.c:156` and `:162`.

### 07-WR-02 and the Holy Mapperel audio gap
- [VERIFIED: tests/cmake/hash_inventory.cmake:57 and :97] `set(actual_keys)` runs once before the per-directory loop (line 57) and again inside it (line 97), so after the loop only the last artifact's keys are compared with `expected_keys`; the earlier artifacts are covered only by the byte-equality check against the first file (lines ~70-75).
- Hard-coded counts: `tests/cmake/hash_inventory.cmake:16` `math(EXPR expected_rows "36 + ${holymapperel_count} + ${holymapperel_saved_count}")` and `tests/cmake/write_hashes.cmake:216` `math(EXPR expected_count "36 + ${hm_count} + ${hm_saved_count}")`; the message at `write_hashes.cmake:218` repeats the arithmetic in prose.
- Key list is spelled out in `write_hashes.cmake:53` (the game loop), `hash_inventory.cmake:34` (`expected_keys`) and `hash_inventory.cmake:135` (the self-test fixture).
- [VERIFIED: tests/cmake/write_hashes.cmake:140-183] `holymapperel_hash_run` runs `"${runner}" --frames ${n2} --rom "${rom}" ${save_args} --hash-frame ${n} --hash-frame ${n2}` with no `--hash-audio`. Add `--hash-audio` to that one command, parse the two `audio ...` lines, return them as extra rows. The run already covers 2N frames, so there is no extra emulation.
- Row arithmetic with the planned inventory (the number `hash_keys.cmake` must produce): games boot 4 x (5 frame + 2 audio) = 28, DABG movies 15, nes-runner play (5 listed frames + 3 gameplay frames + 2 audio if you take the recommended list) = 10, Holy Mapperel 12 keys + 2 `.saved` = 14 frame rows plus 28 audio rows. Total 28 + 15 + 10 + 14 + 28 = 95 (today 50). Use `list(LENGTH)` of the single key list instead of any literal.
- Sorting: `list(SORT)` is byte order. `.` sorts before `/`, so `holymapperel/m1sxrom.saved/...` precedes `holymapperel/m1sxrom/...`; audio rows `.../audio/...` sort before `.../frame ...`. Both writer and validator use the same sort, so this is safe, but a hand-written expected order in a fixture would trip on it.

### Library and runner facts the new code relies on
- Runner `--movie` prints `frame N ticks T sha256 H` for **every** frame (`runner/main.c`: `if ((opt.movie_path != NULL || listed(...)))`). The filter in `write_hashes.cmake` is a regex over those lines.
- `--frames` with `--movie` must be omitted or equal the movie's frame count (otherwise exit 2).
- `--hash-audio` hashes transitions and PCM over the whole run and prints two lines at the end.
- `nesturbator_frame` fields: `video` (uint16_t, pitch >= 256), `audio` (int16_t, capacity >= 799), `audio_count`, `frame_number`, `ticks` [VERIFIED: include/nesturbator.h:195-212].
- Library NMOVIE1 layout is header 16 bytes (`NMOVIE1\0`, u32le version 1 at 8, u32le count at 12) then 4 bytes per frame: port 0, 0, port 1, 0 [VERIFIED: runner/movie.c:12-60 and tests/runner/game_movie.c].

## nes-runner intake (D-04)

- No local copy of the nes-runner source exists on this machine. Only the release asset was needed: downloaded `runner.nes` from the v1 release, sha256 and size match D-04, header `4e45531a 04 02 13 08 00 00 70 ...` (PRG 4 x 16 KiB, CHR 2 x 8 KiB, flags 6 `0x13` = mapper 1 low nibble, battery, vertical mirroring; flags 7 `0x08` = NES 2.0; byte 10 `0x70` = 8 KiB battery NVRAM) [VERIFIED: hashed and read the downloaded bytes].
- Tag `v1` resolves to commit `47537be0ccb8baeb8462547a1c08888ee2a6cfac` [VERIFIED: GitHub API `git/ref/tags/v1`]; the manifest pin check (40 lowercase hex) accepts it [VERIFIED: tests/cmake/manifest_sha256.cmake pin rule].
- GitHub reports the repository licence as MIT (`spdx_id: MIT`); the licence file is `LICENSE.md`, added in commit `e2d70cfcc64b947a26e7c05d3db55c240686169b` dated 2019-06-28 ("Update README.md and add LICENSE.md"), text begins "Copyright 2019 Zorchenhimer" [VERIFIED: GitHub API and raw file at `e2d70cf`]. The README at tag v1 has no licence statement, which matches D-04's "three days after the tag" note. Use the file name `LICENSE.md` in the notices entry.
- The FamiTracker driver attribution in D-04 was not re-checked (CONTEXT-sourced).
- `git add -f tests/roms/runner.nes`: `.gitignore` ignores `*.nes`. Manifest line is tab-separated: path, source `https://github.com/zorchenhimer/nes-runner/tree/47537be0ccb8baeb8462547a1c08888ee2a6cfac`, pin the 40-hex commit, `MIT`, sha256.
- `THIRD-PARTY-NOTICES.md` is currently structured as: an intro sentence naming what comes from elsewhere, then one `##` section per item with Upstream, Release or Commit, a SHA-256, then the licence in a fenced block (libretro.h, SingleStepTests 65x02, Holy Mapperel). Follow that shape. Update the intro sentence (it counts "Two things"). The committed games (Nesteroids, DABG, RHDE, AccuracyCoin) have no entry there today; adding them is optional and out of scope.
- `scripts/hygiene.sh` scans text only (`grep -I`), so the ROM bytes cannot trip the home-path or email scans; it only requires the manifest line [VERIFIED: scripts/hygiene.sh check_file]. The ROM's credits screen embeds third-party user handles as text (for example `SleepyMia`, `KimiMoons`); no `@`, URL or path strings (checked with `strings`). That is not an `ASSET_POLICY.md` "Never" item, but the owner may want to know.
- `manifest.sha256` (CTest `manifest.sha256`) verifies every listed file's hash on every leg. `.gitattributes` has `* text=auto eol=lf`; binary detection is by NUL bytes, as for the existing ROMs, so the Windows legs will not rewrite it.

## libretro parity mode (D-18)

[VERIFIED: tests/libretro/libretro_host.c:993-1152] `main` requires exactly 6 arguments (`argc != 6`) and dispatches by position; a new mode must be recognised before that check (for example `argc >= 2 && strcmp(argv[1], "--rom-parity") == 0`). The host already has: `video()` copying the XRGB frame into a static `frame[W*H]` and counting `video_calls`; `input()` mapping the libretro joypad ids to `NESTURBATOR_BUTTON_*` from `host_buttons[2]`; `compare_with_ppm` (line 841); `p_get_memory_data` / `p_get_memory_size`; the `_WIN32` `LoadLibrary` branch.

Things the plan must handle:
1. **Games have no runner dumps today.** The existing `.ppm` dump tests are Holy Mapperel only (`tests/CMakeLists.txt` lines 236-340, writing `<key>.ppm`, `<key>-run1.ppm`, `<key>-run2.ppm` under `${NESTURBATOR_HM_DIR}`). Add runner dump fixtures for nesteroids, dabg, rhde at frame 180, and for nes-runner at its boot frame and its movie-end frame (`--movie F --rom runner.nes --dump-frame 600:...`; do not pass `--frames`).
2. **Movie replay needs `runner/movie.c` in `libretro.host`** (today it only has `${CMAKE_CURRENT_SOURCE_DIR}/runner` as an include dir for `ppm.h`/`host`; check what it links) and a `--movie FILE` option on the new mode, which D-18's argument list omits. Set `host_buttons[0]`/`[1]` from the movie before each `p_run()`.
3. **The sentinel does not test the core's pixels.** [VERIFIED: libretro/libretro.c:199-213] the adapter calls `nesturbator_run_frame` into its own static `video[]`, converts to its own `xrgb[]`, then calls `video_cb`. A host sentinel only proves `retro_run` delivered a full 256x240 frame. That is still worth having (anti-vacuity: `retro_load_game` true, `video_calls == frames`, exact ppm header, compared frame is not the test card), but the CR-01 check belongs at library level.
4. **Save-file ordering race.** `holymapperel.<key>.run1.sav` is a fixture and `run2.dump` rewrites the same `<stem>.sav` through an atomic rename (`runner/save.c`). If the run-1 parity test reads the `.sav` after run 2 started it can see run 2's file. Make `holymapperel.<key>.run2.dump` also depend on a fixture that `libretro.parity.<key>.run1` provides (or copy the run-1 `.sav` to a private name inside the run-1 chain). Run 2 parity copies the run-1 `.sav` into the span between `retro_load_game` and the first `retro_run` and compares with `--sav-expect`.
5. **Derived ROMs** (`m4w8k`, `m4tkrom`) live at `${NESTURBATOR_HM_DIR}/derived/<key>.nes`, built by the `holymapperel-derived` ALL target; the existing dump tests rely on that, so parity can too.
6. **Frames compared**: games at 180; Holy Mapperel at its N (`roms.cmake` field 3: 100, 100, 100, 600, 600, 100, 100, 100, 200, 400, 600, 600); nes-runner at its boot frame and movie end.
7. Generate the test list from the same lists as the dump tests (`NESTURBATOR_HOLYMAPPEREL_ROMS`, the game list in `hash_keys.cmake`) and fail configure if a list is empty (D-17).

The existing test registers `libretro.host` with `FIXTURES_REQUIRED testframe_ppm`. Do not rename it.

## CI wiring

[VERIFIED: .github/workflows/ci.yml, CMakePresets.json] How a new test reaches all six legs:
- `build` job: matrix `{ubuntu-24.04 ci x64, ubuntu-24.04-arm ci arm64, macos-15 ci arm64, macos-15-intel ci x64, windows-2025 ci-msvc x64, windows-11-arm ci-msvc arm64}`, each running `cmake --workflow --preset <preset>`. The workflow preset is configure, build, test, package. The test preset has `"noTestsAction": "error"` and `jobs: 4`. Any `add_test` in `tests/CMakeLists.txt` therefore runs on all six legs with no workflow edit. After the workflow, the leg runs the bare `write_hashes.cmake` with `-DMOVIE_WRITER=build/<preset>/tests/runner.game_movie[.exe]` and uploads `hashes.txt` as `hashes-<os>-<arch>`.
- `hash-equality` (needs `build`): downloads the six artifacts, runs `hash_inventory.cmake -DROOT=...`; requires exactly 6 `hashes-*` directories, identical bytes, sorted, and the key set. It does not build anything, so `hash_inventory.cmake` and `hash_keys.cmake` must be self-contained (relative `include(${CMAKE_CURRENT_LIST_DIR}/...)` like the existing `holymapperel/roms.cmake` include).
- `asan` (ubuntu-24.04, clang-18, `cmake --workflow --preset asan`): runs the same CTest set under `-fsanitize=address,undefined -fno-sanitize-recover=all`; timeout 10 min.
- `nofp`: only the `abi` label.
- `retroarch-e2e` (macos-15, 30 min): runs the full `ci` workflow again plus a RetroArch comparison; adds the new tests' time.
- CTest `TIMEOUT` values in use: 30 (save cases, `hash.inventory.self_test`, fuzz), 60 (spectral, accuracy pages, interval), 120 (Holy Mapperel dumps), 300 and 900 (nightly oracles). `runner.write_hashes` has none (default) and `COST 50` so it starts first.
- Windows: the movie writer needs `_CRT_SECURE_NO_WARNINGS` for `fopen` (existing pattern: `target_compile_definitions(... $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)`); `/W4` is fatal; pass paths with forward slashes from CMake.
- `release-please-config.json`: `"bump-minor-pre-major": true, "bump-patch-for-minor-pre-major": true`, so a `feat:` title at 0.1.9 produces 0.1.10. The `title` job requires `^(feat|fix|docs|...)(\([a-z0-9._-]+\))?!?: .+`.
- `.github/workflows/nightly.yml` has only the MMC3 blargg lane and the scoreboard; there is no `ppu_vbl_nmi` lane to move. I ran the `mmc3-oracle` preset locally with the fix: 13 of 13 pass.

Budget estimate (this Mac, ASan, jobs 4): new CPU about 200 s on top of 340 s user, so wall stays near the 185 s long pole; the CI image was roughly 3x slower on the M4 runs. Measure on the first CI run, record it in the `ci.yml` comment, and raise the 10-minute cap only if the measured run needs it (D-18 allows this). Give the boot-to-play test a `COST` so it starts with `runner.write_hashes`.

## Common Pitfalls

### Pitfall 1: Pinning timing before the NMI fix
**What goes wrong:** Every movie constant, hash row and score figure is measured on the wrong core.
**Why it happens:** On `HEAD` the game never leaves the title.
**How to avoid:** Plan 1 is the fix, its tests, the bump and the re-pin; everything else is measured after it. Re-run the measurements if any later change touches PPU/CPU timing.
**Warning signs:** `$3E` stays 0 after Start.

### Pitfall 2: Forgetting the revision macro is inside the API-policy hash
**What goes wrong:** `vectors.api_policy` fails with "public declaration stream changed".
**How to avoid:** Update `PHASE8_DECLARATIONS_SHA256` in the same commit; run `cmake -DSOURCE_DIR=. -P tests/cmake/vector_api_policy.cmake` to get the value.

### Pitfall 3: A re-pin that hides a regression
**What goes wrong:** Copying the new `hashes.txt` wholesale also accepts accidental changes.
**How to avoid:** Diff against the list of 24 rows above; any other moved row (especially an audio row, a `frame 1` row, or a Holy Mapperel hash column) is a defect to investigate.

### Pitfall 4: `--movie` prints every frame
**What goes wrong:** A new regex or a copy of the DABG loop selects only 1/30/60/120/180 and the play rows never cover gameplay; or the hash line count check (`found_count EQUAL 5`) fails when the frame list changes.
**How to avoid:** Give the nes-runner loop its own frame list and expected count; include frames after 145.

### Pitfall 5: Strictly increasing score at 30-frame spacing
**What goes wrong:** The correct run has a plateau (f450 = f480 = 37).
**How to avoid:** 60-frame comparison or non-decreasing plus a total.

### Pitfall 6: Assuming Start-only fails only P1
**What goes wrong:** `expected_mask == P1` assertion fails on the correct implementation because P2 also fails after death.
**How to avoid:** Use the measured mask `P1|P2`, or make P2's window end before frame 265 for that control only (not recommended).

### Pitfall 7: Race between libretro parity and the runner save chain
See "libretro parity mode" item 4.

### Pitfall 8: ASan time
`TIMEOUT 120` on the boot-to-play test is too tight under ASan on the CI image; 300 is safe. Keep the whole matrix of scenarios in one process to avoid reloading, and reuse N as the baseline for all controls (it is run once).

### Pitfall 9: `git add` of the ROM
`*.nes` is ignored; use `git add -f`. The pre-commit hook runs `scripts/hygiene.sh --staged`, which rejects an image that is not in the manifest, so add the manifest line in the same commit.

### Pitfall 10: Formatting
`hygiene.format` runs clang-format 18 over every tracked `.c/.h` file. Run `/opt/homebrew/opt/llvm@18/bin/clang-format --dry-run --Werror <files>` locally before committing. New `tests/cmake/*.cmake` files need the exact line `cmake_minimum_required(VERSION 3.25)` (`cmake.script_policy`).

### Pitfall 11: The truncated-ROM control and the runner binary
The runner exits 1 and never falls back, so the failed-load control must be in-process. Keep calling `run_frame` after `NESTURBATOR_ERR_CARTRIDGE`; the instance shows the test card (hash `b49e9be4...0453`) and silence. Assert the status code first so a future loader that accepts the truncated image fails loudly instead of passing the control for the wrong reason.

### Pitfall 12: Test warnings are errors
`-Wconversion -Wsign-conversion -Wmissing-prototypes -Wvla` are fatal in `ci` and `asan`. Mark helpers `static`, use fixed-width types, no VLAs, explicit casts for narrowing.

## Code Examples

### NMI regression ROM (D-02) - validated this session
Program built with `tests/ines.h` (NROM, 16 KiB PRG, 8 KiB CHR ROM, vectors reset `$8000`, NMI `$8010`):
```
reset  $8000:  A9 80        LDA #$80
               8D 00 20     STA $2000
               4C 05 80     JMP $8005
nmi    $8010:  E6 00        INC $00          ; 16-bit counter in $00/$01
               D0 02        BNE +2
               E6 01        INC $01
               A9 80        LDA #$80         ; rewrite $2000 with bit 7 still set
               8D 00 20     STA $2000
               40           RTI
```
```c
/* Source: scratch validation, this session. The NMI handler's own $2000 write must not re-enter NMI. */
static const uint8_t reset_code[] = {0xA9, 0x80, 0x8D, 0x00, 0x20, 0x4C, 0x05, 0x80};
static const uint8_t nmi_code[] = {0xE6, 0x00, 0xD0, 0x02, 0xE6, 0x01,
                                   0xA9, 0x80, 0x8D, 0x00, 0x20, 0x40};
static const struct ines_segment seg[] = {{0x10, nmi_code, sizeof nmi_code}};
/* spec: prg_16k = 1, chr_8k = 1, prg_code = reset_code, segments = seg, segment_count = 1,
   nmi_vector = 0x8010, reset_vector = 0x8000, irq_vector = 0x8000 */
```
Result after frame n: patched core counts exactly n NMIs (1, 2, ... 10); pristine `HEAD` counts 761 after 7 frames and 1086 after 10. Assert `count == frames_run` (allow `+/-1` if the executor prefers a tolerance, but equality held for 10 frames).

### Register-level cases (D-02) in `tests/ppu/test_registers.c`
The file's pattern is `static struct nesturbator nes; memset(&nes, 0, sizeof nes);` then direct calls to `nesturbator__ppu_register_write(&nes, 0x2000u, ...)` and checks of `nes.cpu.nmi_pending`.
```c
/* 0 -> 1 inside vblank: immediate NMI. 1 -> 1: no second NMI. Bit 7 cleared: suppressed. */
reset(); nes.ppu.status = 0x80u;
nesturbator__ppu_register_write(&nes, 0x2000u, 0x80u);
CHECK_EQ_U64(nes.cpu.nmi_pending, 1u);
nes.cpu.nmi_pending = 0u;                       /* the CPU took it */
nesturbator__ppu_register_write(&nes, 0x2000u, 0x80u);
CHECK_EQ_U64(nes.cpu.nmi_pending, 0u);          /* the storm case */
nes.cpu.nmi_pending = 1u;                       /* pending, not yet taken */
nesturbator__ppu_register_write(&nes, 0x2000u, 0x00u);
CHECK_EQ_U64(nes.cpu.nmi_pending, 0u);
```
Add the case "bit 7 set, flag clear, then a write that keeps bit 7: no NMI" and "pending stays 1 when a still-set write arrives before the CPU takes it".

### Movie writer extension (`tests/runner/game_movie.c`)
Keep `write_movie(path, scenario)`; replace `#define FRAME_COUNT 180u` and the fixed `MOVIE_SIZE` with a per-scenario count; for `nesrunner-*` fill `p0` per frame index from the tables: Start `0x08` at indexes 60..63; jumps `0x01` for `{start, hold}` in the list above; attract: `0x20` at 60..61, `0x10` at 72..73 and 84..85, `0x08` at 110..113; none: zero. Keep the existing `p0`/`p1`/`both` behaviour byte for byte (the DABG rows must not move).

### Hash key function (`tests/cmake/hash_keys.cmake`, shape)
```cmake
cmake_minimum_required(VERSION 3.25)
include("${CMAKE_CURRENT_LIST_DIR}/../holymapperel/roms.cmake")
function(nesturbator_expected_hash_keys out)
  set(keys)
  foreach(game IN ITEMS nesteroids dabg rhde nesrunner)
    list(APPEND keys "${game}/boot/audio/pcm sha256" "${game}/boot/audio/transitions sha256")
    foreach(frame IN ITEMS 1 30 60 120 180)
      list(APPEND keys "${game}/boot/frame ${frame}")
    endforeach()
  endforeach()
  # dabg/{p0,p1,both}/frame {1,30,60,120,180}; nesrunner/play/frame {...,300,450,600} + audio
  # holymapperel/<key>/frame N, .../audio/{pcm,transitions} sha256, and the .saved variants
  list(SORT keys)
  set(${out} "${keys}" PARENT_SCOPE)
endfunction()
```
`write_hashes.cmake` and `hash_inventory.cmake` both `include()` it; compare the sorted generated key list with the function's output instead of counting. For 07-WR-02, move the key comparison inside the `foreach(directory ...)` loop. The self-test in `hash_inventory.cmake` (`if(SELFTEST)`) gets a case that deletes one `holymapperel/<key>/audio/pcm` row from one artifact and expects rejection; the fixture writer (`write_hash_fixture`, line 133) must write the new rows (including the audio rows as `<key> sha256 <digest>`).

### Drift guard (D-17, shape)
```cmake
file(GLOB roms CONFIGURE_DEPENDS "${ROOT}/tests/roms/*.nes")
file(GLOB hm_roms CONFIGURE_DEPENDS "${ROOT}/tests/roms/hm/*.nes")
# fail if either list is empty; for each file require: in the game key map, in
# NESTURBATOR_HOLYMAPPEREL_ROMS, or in the explicit exemption list (accuracycoin.nes)
```
Map `runner.nes` to the hash key `nesrunner`. `tests/roms/none.nes` is referenced in usage tests as a path that does not exist; the glob will not see it.

### Libretro parity (shape)
```
libretro.host --rom-parity <module> <rom> <frames> <ppm> [--movie F] [--sav-in F] [--sav-expect F]
  load module; set_environment; init; set callbacks
  read ROM into memory; retro_load_game(data,size) must be true
  [--sav-in] copy file into retro_get_memory_data(RETRO_MEMORY_SAVE_RAM) before the first run
  for i in 0..frames-1: fill frame[] with a sentinel; set host_buttons from the movie; retro_run
  CHECK video_calls == frames; compare_with_ppm(ppm); frame hash != test-card hash
  [--sav-expect] compare memory data/size with the file
```

## State of the Art

| Old approach | Current approach | When changed | Impact |
|--------------|------------------|--------------|--------|
| `nmi_pending = bit7 && vblank` on every `$2000` write | Edge on 0 to 1 only; clear when bit 7 is cleared | this phase | Removes NMI storms in games that rewrite `$2000` inside NMI (nes-runner, nesteroids, DABG ticks) |
| Frame-hash-only Holy Mapperel rows | Frame and audio rows from the same 2N-frame run | this phase | Closes the "frame and audio" gap for boards 1, 2, 3, 4, 7 |
| Three hand-written key lists and a literal `36` | One function and no counts | this phase | Removes silent drift (07-WR-02) |

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | The CI image runs ASan about 3.5x slower than this Mac (from the "about 45 s per M4 ROM" comment in `ci.yml` versus 12-13 s measured here) | Timing, Pitfall 8 | If lower, `TIMEOUT 300` is merely generous; if higher, the `asan` job cap needs raising sooner |
| A2 | The FamiTracker NSF driver 2.11 attribution in D-04 is accurate (taken from CONTEXT; the README at tag v1 does not mention it) | nes-runner intake | A wrong attribution line in `THIRD-PARTY-NOTICES.md` |
| A3 | The ROM's credits-screen handles are acceptable to publish (not covered by `ASSET_POLICY.md` "Never") | nes-runner intake | Owner may prefer to know before the one-way D-04 commit |
| A4 | Pinned positive movie hashes and the jump list stay valid on all six platforms because the core is integer-only; only the Mac was exercised | Measured values | A platform difference would show as a `hash-equality` failure, not a silent pass |
| A5 | The hardware-correct content of the 21 top-left pixels after a soft reset is the first PPU frame's, not the following frame's tail | 05-CR-01 | Only matters if the owner wants hardware-exact reset pixels; outside the stated CR-01 claim |

## Open Questions (RESOLVED)

1. **Start-only expected mask (`P1|P2` versus D-11's `P1`).**
   - What we know: measured P2 fails after the death at frame 265.
   - What's unclear: whether the owner wants the table in CONTEXT changed or P2's window clipped.
   - Recommendation: use `P1|P2`; it is inside the stated rule and keeps the full window.
   - RESOLVED: the Start-only expected mask is `P1|P2`.
2. **A/B pixel threshold: 25% (D-12) versus 50%.**
   - Recommendation: 50%; D-12 says the exact values are pinned by procedure.
   - RESOLVED: the A/B pixel threshold is 50%.
3. **CR-01 disposition.**
   - What we know: not reproducible at `HEAD`.
   - Recommendation: sentinel test; close the finding with evidence; change the core only if the test fails.
   - RESOLVED: the CR-01 sentinel test comes first; the core changes only if it fails.
4. **Whether to keep `TIMEOUT 120`.**
   - Recommendation: 300 for the boot-to-play test; measure on the first CI run.
   - RESOLVED: TIMEOUT is 300.
5. **Extra `nesrunner/play` frame rows (300, 450, 600).**
   - Recommendation: add them; otherwise the rows pin only the title and transition.
   - RESOLVED: nes-runner play rows at frames 300, 450 and 600.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| CMake | everything | yes | 4.4.3 (project minimum 3.25) | - |
| Ninja | build | yes | 1.13.2 | - |
| C compiler | build | yes | Apple clang 21.0.0 (CI uses gcc-14, clang-18, MSVC) | - |
| clang-format 18 | hygiene.format | yes | 18.1.8 at `/opt/homebrew/opt/llvm@18/bin/clang-format` (not on PATH) | CI runs it on every PR |
| Python 3 | scratch tooling only | yes | 3.14.4 | not used by the repo |
| Network | fetching the ROM once | yes | - | The ROM is committed and never fetched at test time (D-04) |
| `gh` | PR creation | yes | - | - |

**Missing dependencies with no fallback:** none.

## Validation Architecture

> `.planning/config.json` has `workflow.nyquist_validation: false`; the orchestrator asked for this section anyway, so it is included.

### Test Framework
| Property | Value |
|----------|-------|
| Framework | CTest with in-repo `tests/check.h` and CMake `-P` scripts; no external framework |
| Config file | `CMakePresets.json` (presets `dev`, `ci`, `ci-msvc`, `asan`, `nofp`, `hygiene`, `vectors-full`, `mmc3-oracle`), `tests/CMakeLists.txt` |
| Quick run command | `ctest --preset ci -R '<regex>'` after `cmake --build --preset ci` |
| Full suite command | `cmake --workflow --preset ci` (`ci-msvc` on Windows) |
| ASan | `cmake --workflow --preset asan` |

### Phase Requirements to Test Map
| Req ID | Behaviour | Test Type | Automated Command | File Exists? |
|--------|-----------|-----------|-------------------|--------------|
| BOARD-04 / D-01 | `$2000` rewrite with bit 7 set raises no second NMI; 0 to 1 in vblank raises one; clearing bit 7 suppresses | unit | `ctest --preset ci -R '^ppu.registers$'` | file exists, add cases |
| BOARD-04 / D-01 | Synthetic ROM sees one NMI per frame | integration | `ctest --preset ci -R '^core.nmi_edge$'` | Wave 0 (new) |
| BOARD-04 / D-03 | Revision 6 reported; API policy hash updated | unit | `ctest --preset ci -R 'core.api\|vectors.api_policy'` (use the actual registered names from `tests/CMakeLists.txt`) | existing, edit constants |
| BOARD-04 / D-03 | 24 moved rows re-pinned; rest unchanged | integration | `ctest --preset ci -R '^runner.write_hashes'` | existing |
| BOARD-04 / D-15, D-16 | Single key list; Holy Mapperel audio rows present and validated on every artifact | script | `ctest --preset ci -R '^hash.inventory.self_test$'` plus `cmake -DROOT=<six artifacts> -P tests/cmake/hash_inventory.cmake` in CI | edit |
| BOARD-04 / D-17 | Every committed ROM is hashed or exempt; lists non-empty | policy | `ctest --preset ci -R '^policy.rom_hash_coverage'` | Wave 0 (new) |
| BOARD-04 / D-18 | Libretro frames equal the runner's for every ROM in scope; save round trip | integration | `ctest --preset ci -R '^libretro.parity.'` | Wave 0 (new) |
| BOARD-04 / six platforms | `hashes.txt` identical on six legs | CI | `hash-equality` job (`cmake -DROOT=... -P tests/cmake/hash_inventory.cmake`) | existing |
| PLAY-01 / D-04 | ROM is the pinned file | policy | `ctest --preset ci -R '^manifest.sha256'` | existing |
| PLAY-01 / D-08..D-13 | Positive passes; each control fails with its exact mask; synthetic mutants fail | integration | `ctest --preset ci -R '^runner.boot_to_play$'` | Wave 0 (new) |
| PLAY-01 / D-09 | Binary replay hashes pinned | integration | `ctest --preset ci -R '^runner.write_hashes'` | edit |
| PLAY-01 / D-07 | Save RAM `5A 6F`, generation >= 2 after game start, higher after Start-only death | integration | inside `runner.boot_to_play` | Wave 0 |
| CR-01 / D-19 | First frames after reset into a sentinel buffer leave no sentinel | unit | `ctest --preset ci -R '^core.reset'` | file exists, add case |
| 07-WR-02 | Validator checks keys on every artifact | script | `hash.inventory.self_test` (new "missing key in a non-last artifact" case) | edit |
| Docs | README and header updated with behaviour | policy | review plus existing doc-consistency tests if any; there is no automated README check for the "50 keys" sentence, so update it by hand | manual in-change |

Success criteria to commands:
1. SC1 (hashes equal on six platforms, libretro equals runner): CI `build` x6 then `hash-equality`; locally `ctest --preset ci -R '^(runner.write_hashes|libretro.parity)'`.
2. SC2 (movie reaches gameplay, RAM byte differs, scene changes, audio varies): `ctest --preset ci -R '^runner.boot_to_play$' --output-on-failure`.
3. SC3 (negative controls fail in the same test): the same test; it fails if `judge(control)` is not the expected mask, so a predicate that stops checking is reported.

### Sampling Rate
- **Per task commit:** `cmake --build --preset ci && ctest --preset ci -R '<tests the task touches>'`, plus `/opt/homebrew/opt/llvm@18/bin/clang-format --dry-run --Werror <changed .c/.h>`.
- **Per wave merge:** `cmake --workflow --preset ci`.
- **Phase gate:** `cmake --workflow --preset ci`, `cmake --workflow --preset asan`, `cmake --workflow --preset hygiene`, `cmake --workflow --preset nofp`, then green GitHub CI on all six legs before `/gsd-verify-work`.

### Wave 0 Gaps
- [ ] `tests/core/test_nmi_edge.c` and its CTest target (`core.nmi_edge`)
- [ ] `tests/runner/test_boot_to_play.c`, the movie scenarios in `game_movie.c`, and the `runner.boot_to_play` CTest target (with a fixture that writes the movies)
- [ ] `tests/cmake/hash_keys.cmake`
- [ ] `tests/cmake/rom_coverage.cmake` and its policy test (plus a self-test, following `manifest_sha256.cmake`)
- [ ] `--rom-parity` mode in `tests/libretro/libretro_host.c` and one `libretro.parity.<key>` test per key
- [ ] CR-01 sentinel case in `tests/core/test_reset.c`
- No framework install is needed.

## Security Domain

`workflow.security_enforcement` is `false` in `.planning/config.json`; the ASVS section is omitted. The phase has one supply-chain-relevant act (committing a third-party binary); it is controlled by the sha256-pinned manifest line, `manifest.sha256`, the hygiene scan and the drift guard, and nothing is fetched at test time.

## Sources

### Primary (HIGH confidence)
- Repository files read this session: `src/ppu.c` (lines 470-545, 560-640), `src/cpu.c` (488-510, 1355-1395), `src/instance.c` (140-180), `src/frame.c`, `src/bus.c`, `src/internal.h`, `include/nesturbator.h`, `runner/main.c`, `runner/movie.c`, `runner/audio_hash.c`, `runner/save.c`, `libretro/libretro.c`, `tests/CMakeLists.txt`, `tests/cmake/{write_hashes,hash_inventory,manifest_sha256,vector_api_policy,script_policy,release_policy}.cmake`, `tests/holymapperel/roms.cmake`, `tests/runner/game_movie.c`, `tests/runner/hashes.txt`, `tests/libretro/libretro_host.c`, `tests/ppu/{test_registers.c,ppu_fixture.h}`, `tests/core/{test_api.c,test_reset.c}`, `tests/ines.h`, `tests/check.h`, `.github/workflows/ci.yml`, `release-please-config.json`, `CMakePresets.json`, `scripts/hygiene.sh`, `ASSET_POLICY.md`, `THIRD-PARTY-NOTICES.md`, `.gitattributes`, `.gitignore`, `.clang-format`, `.planning/phases/05-*/05-REVIEW.md`, `07-*/07-REVIEW.md`, both disposition files.
- Scratch experiments (outside the tracked tree, in the session scratchpad): patched and pristine builds, `dev`, `ci` and `asan` full suites, the `mmc3-oracle` lane, probe programs for every figure in "Measured values".
- GitHub API for `zorchenhimer/nes-runner`: release v1 asset, tag v1 commit, licence key, commit `e2d70cf`, raw `LICENSE.md`.

### Secondary (MEDIUM confidence)
- NESdev Wiki, PPU registers: https://www.nesdev.org/wiki/PPU_registers (the 0-to-1 sentence quoted above).

### Tertiary (LOW confidence)
- CONTEXT.md's FamiTracker attribution (A2).

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH, nothing new is installed.
- Architecture and edit sites: HIGH, read from the files.
- Measured pins: HIGH on the dev build of one platform; determinism makes the others equal, to be confirmed by `hash-equality`.
- Pitfalls: HIGH for the ones reproduced (revision hash, per-frame movie output, Start-only mask, CR-01), MEDIUM for the ASan timing extrapolation (A1).

**Research date:** 2026-10-11
**Valid until:** until any PPU, CPU or APU timing change lands (the pins depend on timing); otherwise 30 days.
