# Library API and native state

This guide describes the public C API, battery-RAM view, status values, and native save-state buffer. It documents the current source tree; identify a published binary from its own release artifacts.

## Save states

A state is an exact snapshot of the whole machine at a frame boundary. It is
valid for the same format version, ROM and board, and it is byte-identical on
every platform. The library does no file I/O and never allocates for a state;
the host owns the buffer and decides where states live.

The current state format is 3 and the behavior revision is 9. Format 3 stores
the sampled NMI line, captured request, last instruction poll, retained
close-edge IRQ level and accepted IRQ decision independently of the live IRQ
line. It also stores an active BRK/IRQ/NMI entry's origin, next unperformed bus
action, selected vector pair and partial low-vector byte. Only reachable
completed-action stages load. The CPU records IRQ acceptance at the
instruction-specific hardware poll point, using old I for CLI, SEI and PLP and
retaining the first branch poll through later cycles (with the page-cross poll
added). A source cleared after acceptance does not revoke the pending IRQ before
entry arbitration. NMI wins that shared arbitration; an IRQ still asserted after
RTI restores I can be accepted again. State distinguishes a recognized NMI
request from an inhibited candidate that still requires the held input. Normal
instruction execution drains a restored entry
before continuing at the next instruction boundary. Public save/load uses
frame-boundary host calls and exposes no exact mid-entry stop; the core remains
synchronous. Exact race and loaded-entry cutpoints are exercised by internal
tests, while core, runner and libretro replay tests cover public frame-boundary
continuation.

1. `nesturbator_state_size(inst)` returns the exact length of a state of the
   loaded cartridge. It is 0 for a NULL instance or with no cartridge, and the
   same value from a successful load until unload.
2. `nesturbator_save_state(inst, buf, capacity)` writes exactly that many
   bytes. A larger buffer is fine and the bytes past the size are not touched;
   a smaller one returns `NESTURBATOR_ERR_BUFFER_TOO_SMALL` and writes nothing.
   A NULL `buf` is never a size query. Saving changes nothing in the instance,
   and saving a jammed instance succeeds.
3. `nesturbator_load_state(inst, buf, size)` needs `size` to equal the state
   size exactly. A load is all or nothing: every refusal leaves the instance
   byte for byte as it was. Loading a state saved after a JAM opcode makes the
   next frame return `NESTURBATOR_STOP_JAM`; loading a clean state clears the
   latch.

Load checks in this order and reports the first failure: arguments, a length
below the 64-byte header, the magic and format version, the cartridge identity
(mapper, submapper, region and ROM CRC-32), the length against the state size
and the header's total size, the CRC-32, and then the contents. A header size
other than 64, a reserved byte other than zero and any field out of range are
content checks and report `NESTURBATOR_ERR_STATE_INVALID`. So does a PPU
position ahead of the header's tick count or one whole frame or more behind it,
and so does a tick count above 2^62 (counters near 2^64 would wrap and never finish),
which keeps the first frame after a load to about one frame of work.

The header is 64 bytes, every multi-byte field little-endian:

| Offset | Bytes | Field |
|---|---|---|
| 0 | 8 | magic `89 4E 54 53 0D 0A 1A 0A` |
| 8 | 2 | format version |
| 10 | 2 | header size, 64 |
| 12 | 4 | total size |
| 16 | 4 | behaviour revision of the saving library |
| 20 | 2 | mapper id |
| 22 | 1 | submapper |
| 23 | 1 | region, 0 for NTSC |
| 24 | 4 | ROM CRC-32 of the 16 header bytes, PRG ROM and CHR ROM |
| 28 | 4 | state CRC-32 of every byte except these four |
| 32 | 8 | frame number |
| 40 | 8 | ticks |
| 48 | 16 | reserved, zero |

The machine fields, the whole PRG RAM (battery RAM included) and the 8 KiB CHR
RAM (when the board has it) follow. The video frame buffer, caller's PCM output
and transient staged PCM are not in a state; APU sample phase and synthesizer
and filter history are machine state and are serialized. This distinction also
holds in an internal snapshot before the first output frame: `core.nmi_state`'s
`test_frame_zero_empty_pcm_preserves_history` saves at frame 0 with no PCM
staged and nonzero audio history, restores it canonically, and checks empty
drain plus future continuation. Frame numbers and ticks continue from the saved
point: the first frame after a load reports the number and ticks of the
uninterrupted run.

Battery RAM and the save generation: a state includes the battery RAM. Loading
a state whose battery bytes differ from the instance's raises `nesturbator_save_generation`
once, so the host writes the `.sav`. Loading a state whose battery bytes match,
however many times, never moves it, and neither does a refused load, so
repeated loads such as rewind never trigger a write.

Another format version is refused, never converted. Keep rewind buffers in
memory, and expect user-kept states to need re-creating after a release that
raises the format version. The behaviour revision is recorded and not
enforced: after an emulation fix an old state still loads, but its later frames
may differ from the run that saved it.

| Format version | Introduced |
|---|---|
| 1 | first release that contains save states |

A format bump ships under a `feat:` title with a changelog line saying older
states are refused. It does not change `NESTURBATOR_ABI_VERSION`, because the
C interface is the same.

Battery RAM is part of the PRG RAM, so a load replaces it. Rewinding past an in-game save reverts the save RAM.

The `core.state_fork.*` tests (one per committed game, plus
`core.state_fork.stress` and `core.state_fork.nmi`) fork games on all six boards,
a DMA and interrupt stress program (on NROM and on MMC3), and an owned NMI
program at public frame boundaries. The NMI program records a one-based guest
entry sequence, changes the visible backdrop, and keeps pulse audio active.
Each saved fork restores into a differently run and poisoned instance; later
native state, frame, tick, PCM, and independently expected guest events must
match the uninterrupted timeline. The board and DMA/interrupt matrices remain
registered, and the tests also show that saving every frame changes nothing.

The `core.state_reject` test shows the refusals. It loads each game's state into
every other game, flips every byte of a state and expects the status of the
first failing check, truncates the state to every shorter length, and sets every
bounded field just past each of its bounds (with the CRC-32 recomputed) and at
the type's limits, expecting `NESTURBATOR_ERR_STATE_INVALID`. Fields at their
bounds are loaded and run for a frame. After every refusal the instance and
`nesturbator_save_generation` are unchanged. The same test runs under the
address and undefined-behaviour sanitizers in the `asan` preset.

## Status codes

Every call returns one of these. An error changes no state.

| Value | Status | Meaning |
|---|---|---|
| 0 | `NESTURBATOR_OK` | The call succeeded. |
| 1 | `NESTURBATOR_ERR_ARGUMENT` | A required pointer is NULL, or an argument is out of range. |
| 2 | `NESTURBATOR_ERR_STRUCT_SIZE` | A struct's size field is 0, too small, or larger with a non-zero tail. |
| 3 | `NESTURBATOR_ERR_ABI` | The config's abi field differs from `NESTURBATOR_ABI_VERSION`. |
| 4 | `NESTURBATOR_ERR_NO_MEMORY` | The allocator returned NULL. |
| 5 | `NESTURBATOR_ERR_BUFFER_TOO_SMALL` | A caller-owned buffer is too small for the output. |
| 6 | `NESTURBATOR_STOP_JAM` | The cartridge executed a JAM opcode; the instance is latched. |
| 7 | `NESTURBATOR_ERR_CARTRIDGE` | Cartridge bytes are malformed or outside a supported board. |
| 8 | `NESTURBATOR_ERR_STATE_FORMAT` | The magic is wrong or the format version is not the library's. |
| 9 | `NESTURBATOR_ERR_STATE_CARTRIDGE` | No cartridge is loaded, or the state belongs to another game or board. |
| 10 | `NESTURBATOR_ERR_STATE_SIZE` | The length is wrong, including any truncation. |
| 11 | `NESTURBATOR_ERR_STATE_CHECKSUM` | The state's CRC-32 does not match: the bytes were damaged. |
| 12 | `NESTURBATOR_ERR_STATE_INVALID` | The CRC-32 matches but a field is out of range or a reserved byte is set. |
