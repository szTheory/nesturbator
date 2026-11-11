# Public C package usage

The installed `nesturbator::nesturbator` CMake target and
[public header](../include/nesturbator.h) are canonical. Zero each size-tagged
struct before assigning `size` and fields. The package-only NTSC profile uses
`nesturbator_load_cartridge`, `nesturbator_get_info`, `nesturbator_set_input`
and `nesturbator_run_frame`; it has no runtime region query or preference API.
See the [checked profile](provider-ntsc.md) for source and runtime evidence.

Supply two logical input masks before each synchronous frame call. Native
video is 256×240 `uint16_t` palette/emphasis values; provide pitch at least
256 pixels. Audio is mono signed `int16_t` PCM at 48000/1 samples per second,
with capacity for 799 samples and 798 or 799 valid samples per successful
NTSC call. Consume output fields and buffers only after `NESTURBATOR_OK`.
A latched JAM stops the instance and does not establish valid frame output.

A refused malformed replacement preserves the active cartridge and session.
A successful replacement, unload or destroy ends cartridge-owned spans;
reset preserves the loaded cartridge. No-content output is the NTSC test
pattern and silence. The gameplay test verifies refused-load continuation
against an untouched twin, alongside input and buffer edge controls.

Battery game memory is a core-owned span whose persistence and content
identity the host manages. Native states are whole-machine snapshots with
separate format and behavior fields. This source uses ABI 1, behavior 9 and
state format 3; the DABG scenario does not qualify battery restart or snapshot
interchange, including states from later unreleased branches.
