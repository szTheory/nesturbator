# Nesturbator NTSC provider profile

This is the provider-side contract for the first qualified route: a public C
consumer running the permitted DABG NROM scenario through Nesturbator's
installed library package. The machine-readable facts are in
[provider-ntsc.json](provider-ntsc.json); its checker compares those facts with
the public header, NTSC clock/sample arithmetic, the DABG guide digest, and an
optional installed-runtime report.

**Qualification status: candidate only.** This profile is not bound to a
published package. The release version, exact source commit, archive digests,
and attestations remain pending in
[`provider-ntsc-release.json`](provider-ntsc-release.json), which is written
only after the later live release check succeeds. The current header's version
is not a promise of the next Release Please version. ABI, emulation behavior
revision, and native state format are separate compatibility fields.

## Route admission and lifecycle

Create an instance with a zeroed, size-tagged `nesturbator_config` and the
current ABI. All three allocator hooks set to NULL select the C library's
allocator; otherwise both allocation and free callbacks must be supplied.
`nesturbator_get_version()` reports the loaded library's semantic version, ABI
and behavior revision; compare it with the header when a shared library may
differ from the one used at compile time. Destroy the instance through the
matching API. Memory is allocated at create and cartridge load, not while
frames run.

For this route, call `nesturbator_load_cartridge_region()` with
`NESTURBATOR_REGION_PREFERENCE_NTSC`, then query both
`nesturbator_get_region()` and `nesturbator_get_info()` on that instance.
Explicit single-region NES 2.0 metadata wins over the preference. Legacy iNES
and multi-region images use the requested preference; `Auto` defaults to
NTSC. Explicit PAL stays PAL and is outside this route. Dendy and reserved
timing bits are refused. This is an admission rule for the qualified consumer,
not a change to core loader precedence.

A successful load establishes the cartridge and effective region. Unload
removes the cartridge; reset is a soft console reset and preserves the loaded
region and timing. A malformed or unsupported replacement returns
`NESTURBATOR_ERR_CARTRIDGE` and leaves the active session unchanged. A
successful later load replaces it. With no cartridge, the core supplies its
test pattern and silence using NTSC timing; this does not qualify a game.

All public structs are size-tagged. Initialize input structs by zeroing the
whole struct, then setting `size` and documented fields. Null pointers, bad
sizes, invalid arguments, unsupported content, or undersized output buffers
return their documented status before changing emulation state. The selected
scenario checks that a refused replacement leaves the already loaded DABG
instance able to continue with the expected state and output.

The read-only version and info queries write nothing for a NULL output or a
zero size tag; otherwise they write at most the struct size supported by that
library. `nesturbator_get_palette()` copies at most 512 entries; a NULL output
or zero count is a no-op, and NULL or unloaded instances select the NTSC
palette. Setting invalid input returns an error without changing the instance.
Reset is a soft reset between frames; it preserves the loaded region and does
not advance `frame_number`, though its seven CPU cycles contribute to
cumulative ticks. Unload releases the cartridge. The API's detailed status and
field contracts remain canonical in the [public header](../include/nesturbator.h)
and [API guide](api.md).

## Frame, input, video, and audio contract

The host supplies two independent logical controller masks, one per port; the
core does not map keyboards, gamepads, or physical devices. The eight button
bits are A, B, Select, Start, Up, Down, Left, and Right. The most recently set
pair is sampled at the beginning of each successful `nesturbator_run_frame()`
call. Calls are synchronous and ordered. Each successful call advances
`frame_number` once. Instructions finish across a requested frame boundary,
so a crossing instruction cannot mix input masks and the next input is
sampled only by the next call.

`ticks` is cumulative emulated time in half-master-clock periods. It includes
soft-reset cycles and instruction or DMA overshoot; it is not a fixed delta per
call. The nominal NTSC frame interval is 714732 ticks and its exact frame rate
is 39375000/655171 frames per second. Hosts can schedule against that fraction;
the profile does not qualify a fixed pacing policy.

The host provides storage for 256×240 `uint16_t` pixels, with a pitch of at
least 256 pixels. These are native palette values, not RGB: bits 0–5 hold the
palette index, bits 6–8 hold raw emphasis, and bits 9–15 are zero. The separate
`nesturbator_get_palette()` conversion returns up to 512 XRGB8888 entries for
the effective region. Hash native pixels before any host display conversion.

Audio is mono signed 16-bit PCM at exactly 48000/1 samples per second. For
NTSC, the source uses 352/315000 samples per tick and 714732 nominal ticks per
frame, or 10482736/13125 nominal samples per frame. The zero-phase integer
accumulator carries its remainder between calls, yielding 798 or 799 samples
per NTSC frame. Reserve capacity for 799 samples and read `audio_count` only
after `NESTURBATOR_OK`. A NULL audio pointer is accepted only with zero
capacity; zero capacity does not discard output needed for the frame. Explicit
PAL can require 959 or 960 samples and has a different clock, so refuse PAL
before advancing this NTSC route or qualify it separately.

Pitch below 256 or audio capacity below the next frame's exact sample count
returns `NESTURBATOR_ERR_BUFFER_TOO_SMALL` before state changes. On success,
video, audio count, frame number, and cumulative ticks describe that call. A
JAM returns `NESTURBATOR_STOP_JAM`, leaves the instance latched and does not
write frame output fields; neither caller buffer is a valid frame result.

## Scenario evidence and storage boundary

The DABG source, manifest ROM identity, rebuilt linker map, RAM predicate,
input recipe, native pixel response, and non-silent PCM window are documented
in [the gameplay qualification](nrom-gameplay.md). The recurring
`install.gameplay` check rebuilds a consumer from the public header, moves the
installed package, discards the old consumer build, and resolves and links
only against the moved prefix. The positive report is paired with no-input
and Start-only controls, refusal continuation, and buffer-boundary checks.
The profile checker binds its scenario field to the SHA-256 of that guide; the
final candidate record also binds the profile and scenario digests to one
source identity.

Battery-backed game RAM and machine snapshots are different interfaces. The
core owns the exposed save-RAM span; the host owns durable file I/O and content
identity. A returned RAM pointer remains valid through reset, state load, and a
refused replacement; it ends at unload, a successful later load, or destroy.
The save-generation counter tracks CPU writes reaching the enabled RAM span;
host writes through that pointer do not count. The selected NROM has no save
span. A save state is a whole-machine snapshot with its own format version;
the host owns the byte buffer and file storage. This DABG scenario establishes
neither battery restart behavior nor snapshot interoperability.

## Limits

This upstream package contract does not establish a LibFuturo installation,
host frontend, audio playback, physical pacing, PAL consumer route, browser,
device, Playstead, hardware, battery restart, machine-state interchange, or
broad mapper/title qualification. Existing downstream evidence remains bound
to the exact older source it observed. The profile says what this provider
offers and what the qualified DABG evidence exercises; the consumer owns its
host integration and play-session acceptance.
