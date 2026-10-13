# Phase 12: Save states in the library - Context

**Gathered:** 2026-10-11
**Status:** Ready for planning

<domain>
## Phase Boundary

The library gains `nesturbator_state_size`, `nesturbator_save_state` and `nesturbator_load_state`. These save the whole machine at a frame boundary into a fixed-size, canonical little-endian byte format and load it back all-or-nothing. A loaded run continues exactly as the uninterrupted run would, on all six boards and all six platforms.

The runner gains `--state-at-frame N:FILE`, `--load-state FILE` and `--hash-state N` as test tooling, so the state hash pins run on the six-platform CI matrix. A rejected state exits 5.

Not in this phase:
- libretro serialize and the `.info` claims (Phase 13).
- The state-loader fuzz target (Phase 13).
- Signal handling and the full exit table (Phase 14).
- Benchmarks (Phase 15).
- Movies that start from a state (STATE-06, future).

Requirements: STATE-01 to STATE-05 and RUN-01. Most of the design is already settled in `.planning/research/SUMMARY.md` (sections "Resolved disagreements" and "Phase 12"):
- one field-list visitor per struct, in SIZE, WRITE, VALIDATE and APPLY modes;
- a size that depends on the cartridge;
- a CRC-32 for the state and a CRC-32 identity for the ROM;
- a `state` op on each mapper;
- the battery-generation rule;
- hash pins only, with no committed state file.

This file adds the four decisions that were still open.

</domain>

<decisions>
## Implementation Decisions

### Function signatures and buffer contract (STATE-01)

- **D-01: Declarations.** — **Reversibility:** one-way. These signatures enter the public header, which only grows, and hosts compile against them.
  ```c
  size_t nesturbator_state_size(const nesturbator *inst);
  nesturbator_status nesturbator_save_state(const nesturbator *inst, void *buf, size_t capacity);
  nesturbator_status nesturbator_load_state(nesturbator *inst, const void *buf, size_t size);
  ```
  - `state_size` returns 0 for a NULL instance or when no cartridge is loaded. Otherwise it stays constant from a successful cartridge load until unload. It never changes the instance.
  - `save_state` takes a `const` instance, so the type itself says the save does not change it.
  - The functions take a plain byte blob, not a versioned struct, so there is no size tag. The magic, the format version and the cartridge identity are carried inside the blob.

- **D-02: Save accepts any capacity at or above the state size; load needs exactly the state size.**
  - Save writes exactly `nesturbator_state_size(inst)` bytes. It leaves any bytes beyond that untouched and never allocates.
  - Save writes bytes only when it returns `NESTURBATOR_OK`. On any error the buffer is untouched.
  - A NULL `buf` is never a size query; use `nesturbator_state_size` for that.
  - On load, any length other than the state size is refused, so truncated or padded input cannot pass.
  - This matches the libretro split: `retro_serialize` gets a buffer at least the state size, and `retro_unserialize` must match exactly. The Phase 13 fallback that accepts a zero-filled tail stays in the adapter, never in the library.

### Rejection status codes (STATE-04)

- **D-03: Five new statuses, one for each STATE-04 class, appended after `NESTURBATOR_ERR_CARTRIDGE = 7`.** — **Reversibility:** one-way. Enum values are fixed and only appended, and hosts branch on them.

  | Value | Name | Meaning |
  |---|---|---|
  | 8 | `NESTURBATOR_ERR_STATE_FORMAT` | Not a state this library reads: bad magic, or another format version. |
  | 9 | `NESTURBATOR_ERR_STATE_CARTRIDGE` | No cartridge is loaded, or the state's mapper, submapper, region or ROM CRC-32 differs from the loaded cartridge. |
  | 10 | `NESTURBATOR_ERR_STATE_SIZE` | The length is wrong: shorter than the header, unequal to `nesturbator_state_size`, or unequal to the header's total size. Covers truncation at any length. |
  | 11 | `NESTURBATOR_ERR_STATE_CHECKSUM` | The state CRC-32 does not match. |
  | 12 | `NESTURBATOR_ERR_STATE_INVALID` | The CRC matches, but a field is out of range, an enum value is unknown, or a reserved byte is non-zero. |

  - Research suggested three coarse classes. The owner's STATE-04 already requires distinct codes for each case, and five codes still form a small, closed set. Each code tells the host a different thing:
    - FORMAT: another version.
    - CARTRIDGE: the wrong game.
    - SIZE: the transfer or file was cut short.
    - CHECKSUM: the bytes were damaged.
    - INVALID: the state came from a bug or an attacker.
  - The enum does not grow with the format. A new field's range check returns INVALID.
  - `NESTURBATOR_ERR_CARTRIDGE` (7) is not reused, because it means a ROM image was rejected.

- **D-04: Load check order.** Each refusal leaves the instance byte-identical.
  1. A NULL `inst` or NULL `buf` gives `ERR_ARGUMENT`.
  2. A `size` smaller than the fixed header gives `ERR_STATE_SIZE`.
  3. A bad magic or another format version gives `ERR_STATE_FORMAT`.
  4. No cartridge loaded, or a mismatched mapper, submapper, region or ROM CRC, gives `ERR_STATE_CARTRIDGE`. Identity is checked before the full length, because the size depends on the cartridge, so another game's state reports "wrong game" and not "wrong length".
  5. A `size` different from `nesturbator_state_size(inst)`, or a header total size different from `size`, gives `ERR_STATE_SIZE`.
  6. A wrong CRC-32 gives `ERR_STATE_CHECKSUM`.
  7. Range, enum and reserved-zero validation gives `ERR_STATE_INVALID`.
  8. Apply. VALIDATE covers exactly the bytes APPLY reads, so APPLY cannot fail.
  - A flipped byte reports the first check it breaks. A flip in the magic or version gives FORMAT, a flip in an identity field gives CARTRIDGE, and a flip in the size field gives SIZE. A flip anywhere else, including the behaviour revision, gives CHECKSUM.
  - The flip-every-byte test asserts the expected class for each offset. The CRC-recomputed sweep asserts INVALID.
  - The header documents this order with the existing "Checks in order:" idiom.

- **D-05: Save check order.**
  1. A NULL `inst` or NULL `buf` gives `ERR_ARGUMENT`.
  2. No cartridge gives `ERR_STATE_CARTRIDGE`.
  3. A `capacity` smaller than the state size gives `ERR_BUFFER_TOO_SMALL`, the same meaning it has in `run_frame`.

- **D-06: JAM.** Save and load never return `NESTURBATOR_STOP_JAM`.
  - The `jammed` latch (`src/internal.h:48`) is part of the state and is validated as 0 or 1.
  - Saving a jammed instance succeeds.
  - Loading a clean state clears the jam, and loading a jammed state restores it, so the next `run_frame` returns `STOP_JAM` again.
  - A test covers both directions.

- **D-07: Runner mapping.** Every `ERR_STATE_*` status exits 5. stderr carries one line naming the class, for example `nesturbator-run: cannot load state <path> (wrong game)`.
  - A state file the runner cannot read (missing, or a directory) is an I/O failure and exits 1.
  - The README has a status table for the new codes, and a test checks that table against the enum, the same way the exit-code table is checked.

### Frame numbers and time after a load (STATE-03, RUN-01)

- **D-08: A state carries `frame_number` and `ticks`, and load restores them.** — **Reversibility:** costly. Changing this later changes every state hash and the runner's frame-index meaning.
  - A state is a point on one timeline, so save, load and continue equals the uninterrupted run, including the tick count STATE-03 compares.
  - The header comments for both fields change from "since create" to "since power-on, restored by a state load".
  - The frame buffer and audio are not in the state. The first frame after a load must equal the uninterrupted run's next frame, and the fork tests assert this.

- **D-09: Runner frame indices stay absolute, counted from power-on.** — **Reversibility:** one-way. This is part of the runner's published command line.
  - After `--load-state` restores frame K:
    - `--frames N` is the last frame, so the runner runs frames K+1 to N.
    - `--hash-frame N` and `--dump-frame N:FILE` need K < N <= `--frames`, because frame K's picture is not in the state.
    - `--state-at-frame N:FILE` and `--hash-state N` need K <= N <= `--frames`. N = K saves or hashes the state just loaded, which gives a cheap check that a load and a save round-trip.
    - An index out of range exits 2, and the message names K.
  - Without a load, K is 0, so `--state-at-frame 0:FILE` saves the power-on state, matching "including frame 0" in STATE-03.
  - The README says frame indices are absolute and that `--frames` is an end frame, not a count of more frames.

- **D-10: Order of operations.**
  1. Parse and syntax-check the options (exit 2).
  2. Create the instance and load the ROM.
  3. Load the battery save.
  4. Apply `--load-state` (exit 5 if rejected, exit 1 on an I/O failure).
  5. Range-check the frame indices against K (exit 2).
  6. Run the frame loop.
  - State outputs for frame N are written after frame N runs. A state at K is taken before the loop.

- **D-11: Repetition.**
  - `--state-at-frame` and `--hash-state` may each repeat, like `--dump-frame`. The same N given twice to one flag exits 2.
  - `--load-state` may be given once.
  - State files are written only to the given path, through `nesturbator_run_file_replace` (Phase 11 D-01).
  - `--hash-state` prints the SHA-256 of the state bytes in the existing `hashes.txt` line style.

- **D-12: `--movie` together with `--load-state` exits 2** with `--movie cannot be combined with --load-state`. A movie indexed from frame 1 would silently go out of sync. Refusing now leaves STATE-06 free to define the meaning later.

### Format version policy (STATE-02)

- **D-13: A public `NESTURBATOR_STATE_FORMAT_VERSION 1` sits next to `NESTURBATOR_BEHAVIOUR_REVISION` in `include/nesturbator.h`.** — **Reversibility:** one-way. Once a release ships version 1, released state files and host rewind policies depend on it.
  - The header carries an exact version, and any mismatch is refused with `ERR_STATE_FORMAT`. There are no tagged or skippable sections and no converter, since conversion is out of scope.
  - Before the first release that contains states (Phases 12 to 15), the version stays 1 and layout changes simply regenerate the pins.
  - After that release, any change to the state bytes bumps the version in the same commit: a field added, removed, reordered or resized, or a field's meaning changed.
  - A fix that changes only emulation output bumps `NESTURBATOR_BEHAVIOUR_REVISION`, which is recorded in the state but not enforced. A fix that adds a field bumps both.

- **D-14: Semver mapping.**
  - A format bump does not change `NESTURBATOR_ABI_VERSION` and is not a breaking change on its own.
  - It ships under a `feat:` title, and the changelog body says older states are refused.
  - `!` or a BREAKING footer is used only when the C API changes.

- **D-15: The CI guard reuses the existing pin file. No new tool is added.**
  - `tests/runner/hashes.txt` gains a `state_format <N>` line, the state hashes, and the state size for each board.
  - A ci test requires the `state_format` line to equal the macro.
  - A layout change breaks the size or hash pins. Re-pinning them makes the author look at the `state_format` line in the same edit, and after the first release a version mismatch fails the test.
  - The remaining gap is someone editing both the pins and the line without bumping the macro. That gap is accepted.

- **D-16: Header evolution points.**
  - The header has a fixed size and carries a header-size field. Its reserved bytes are written as zero and validated as zero (D-04 step 7).
  - The state CRC-32 covers every byte except the CRC field itself.
  - The magic is 8 bytes and includes a non-text byte, as PNG does, so text-mode mangling is caught.

- **D-17: README "Save states" wording.**
  - A state is an exact snapshot for the same format version, ROM and board, and it is byte-identical on every platform.
  - Another format version is refused, never converted. Hosts should keep rewind buffers in memory and expect user states to need re-creating after a release that bumps the format.
  - The behaviour revision is recorded, not enforced, so after an emulation fix an old state loads but may diverge.
  - A table lists each format version and its first release.
  - Rewinding past an in-game save reverts the save RAM (STATE-05).

### Claude's Discretion
- The exact header field order and widths, the magic bytes, the number of reserved bytes, and the section layout after the header. The research suggests a 64-byte header with nine sections. Keep the header 64 bytes or smaller and every field explicitly little-endian.
- The stderr wording for each rejection class, as long as each class has its own word.
- Test file names, and how the fork-test matrix is split across executables.
- Whether `nesturbator_version` (or the struct it fills) also reports the format version. Add it only if appending is cheap under the ABI rules.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Requirements and design
- `.planning/REQUIREMENTS.md`: STATE-01 to STATE-05, RUN-01, and the Out of Scope table.
- `.planning/ROADMAP.md`, the "Phase 12" section: the success criteria and the research flag, which is a spike on instruction boundaries in `src/frame.c` and on the cost of saving and loading.
- `.planning/research/SUMMARY.md`: the resolved disagreements (size, CRC, position fields, flag names, hash pins), the state inventory, and the Phase 12 delivery list.
- `.planning/research/ARCHITECTURE.md`: the visitor design (SIZE, WRITE, VALIDATE, APPLY), the mapper `state` op, and the header and sections.
- `.planning/research/PITFALLS.md`: hidden state, struct dumps, cross-ROM loads, and the save generation.
- `.planning/research/FEATURES.md` and `.planning/research/STACK.md`: the state sections.
- `.planning/phases/11-tune-up-and-state-readiness/11-CONTEXT.md`: D-01 (`nesturbator_run_file_replace`), D-15 (`battery_save` rename), D-16 (frame-boundary pointers and the empty PCM ring), D-17 (status numbers).
- `.planning/preparation/README.md`: the index of hardware references. Open the files it points to for any field's meaning.

### Code contracts
- `include/nesturbator.h`: the status enum ("values are fixed and only appended"), the "Checks in order:" idiom, `nesturbator_save_generation`, `nesturbator_get_memory` and `NESTURBATOR_BEHAVIOUR_REVISION`.
- `libretro/libretro.h`: the serialize contract, for Phase 13 compatibility of the size rules.
- `tests/runner/hashes.txt`: the pin format to extend.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `runner/save.c` `nesturbator_run_file_replace`: an atomic write for state files.
- `runner/sha256.c`: the hash for `--hash-state`, the same one the existing frame pins use.
- `tests/check.h`, `tests/mapper_test.h` and `tests/runner/movie_fixture.h`: test scaffolding.
- `tests/holymapperel/roms.cmake`: derived ROMs per board for the fork test, with no new committed ROM bytes.
- The `--dump-frame N:FILE` parser in `runner/main.c`: the model for parsing `--state-at-frame N:FILE`.

### Established Patterns
- Status enum values are only appended, and every refusal leaves the instance unchanged.
- `frame_number` and `ticks` live in the instance (`src/internal.h:189,194`); create and cartridge load zero them, and `src/frame.c` advances them.
- The `jammed` latch is in the CPU struct (`src/internal.h:48`).
- The library does no file I/O. The runner owns every file.

### Integration Points
- New `src/state.c` with the visitor. Each mapper gains a `state` op (`src/mapper.h` and the six `src/mapper_*.c` files).
- The ROM CRC-32 is computed once at `nesturbator_load_cartridge` (`src/cartridge.c`).
- The runner option loop and frame loop in `runner/main.c`.
- The README gains a "Save states" section and the status table.

</code_context>

<specifics>
## Specific Ideas

- The owner asked for a broad, multi-lens fan-out on each decision followed by one coherent recommendation, adopted without further questions. The four research reports were reconciled into one set:
  - The buffer-contract report reused `ERR_ARGUMENT` for every refusal. That conflicts with STATE-04, so D-03 replaces it.
  - The status-code report proposed three classes. It was widened to five so that STATE-04 stays as written.
- Owner preference: a copy-paste beats another dependency. No new dependency or tool enters this phase.

</specifics>

<deferred>
## Deferred Ideas

- A movie that starts from a state (STATE-06). `--movie` with `--load-state` is refused until then (D-12).
- A hygiene check that fails when state pins change without the format macro changing (D-15). Add it only if drift actually happens.
- Accepting a zero-filled tail in `retro_unserialize` if RetroArch pads buffers. That belongs to Phase 13 and the adapter only.

</deferred>

---

*Phase: 12-save-states-in-the-library*
*Context gathered: 2026-10-11*
