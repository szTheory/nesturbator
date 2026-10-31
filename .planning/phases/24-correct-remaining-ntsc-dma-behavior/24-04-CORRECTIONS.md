# Full-workflow divergence corrections

## Trigger and scope

The single authorized local workflow completed normally. Its complete
behavior13 campaign retained every119 previously passing key, but three
selected DMA rows still failed. [Actual result](24-CI-RUN.md). That report
remains attributable to its source. No later owned check relabels it as a
new ROM observation; no second campaign or full workflow ran.

The owned repairs below follow the already approved phase and compatibility
policy. State format6, ABI1 and all serialized fields/meanings stay unchanged;
output/timing changes raise behavior13 to14. No new field is required.

## Corrected bus oracle

Only an active internal4015 read disconnects the external bus. When select
ends, an undriven4000 read samples the external held byte again. The pinned
answer key excludes16/17, not15. The old owned oracle expecting retained
status50 after deselection was wrong. Its corrected matrix covers samples
00/A5/C2/FF and keeps the status/IRQ acknowledgment and separate external
latch assertions. A fail-first old-core run rejects the new deselected-read
assertion; the corrected production and observed paths agree.

## Full explicit sweep and armed requests

The prior isolated abort fixtures omitted offsets6/7. A genuine CPU guest
now covers all16 offsets and the exact key
`4,4,4,4,4,4,3,4,1,1,0,0,0,0,0,0`.

Offset6 retains a request already armed by the APU before disable. Offset7
writes disable one clock before the output edge: the trace has counter1,
GET half, bits0, full buffer and remaining1, but pending0. The narrow explicit
qualification excludes a waiting first load and pre-arms an ordinary PUT request before clearing the reader.
The accepted GET fills its buffer without underflowing remaining0, looping
or raising a sample-end IRQ anew. An enable-delayed request still cancels.
The separate early-G implicit late-stop case remains unchanged.

This supersedes Plan03's old no-GET oracle at stop433/434 and its unsupported
claim that every unaccepted request is canceled by disable. Those records
remain historical; current standalone probes independently require normal
GETs and4/3 extra clocks at these two stops. Two/three-clock aborts still
require exactly one parked halt and no GET, and writes suppress that halt.

## Looping sweep and inferred byte visibility

All three implicit16-offset guests now preserve the pinned helper instruction
order, including three NOPs and500/37+A order in loop3. Review caught an extra
500-cycle slide common to the initial emitter and a separate loop3 layout
mismatch; neither was used as a core oracle until corrected. The350/560 word
slides and relevant branch-page timing were independently checked.

The corrected guest reproduced the actual failing section: first two keys
pass; loop3 offsets8/9 incorrectly measured4 instead of0. A load GET is
immediately followed by an output edge, which consumed the newly fetched
byte and armed an adjacent looping refill.

The selected compatibility model keeps that fresh byte unavailable for the
immediately following output edge. The output unit retains the buffered
byte and starts a silent byte cycle, then consumes it at the next eligible
boundary. The synchronous bus passes this readiness only on the immediate
post-GET clock. OAM carries it through its next clock or, for a final GET,
to the resumed CPU read. These are local arguments/stages; no public API
returns before that read, so no readiness history needs serialization.

This is an inference from the pinned early-G keys, not a universal silicon
fact or a general-purpose DMA cooldown. The test author also distinguishes
measured console answer keys from a fully established internal cause
([primary discussion](https://github.com/100thCoin/AccuracyCoin/discussions/2)).
The declared early-G model is asserted explicitly; the fixture does not
silently choose a different accepted profile to pass. The existing ROM
classifier continues accepting both historical outcomes.

All64 explicit/implicit entries now equal their source-contract keys. A
test-local wrong model forces immediate sample visibility and independently
reproduces the extra looping reloads at8/9. The linked production and observed
source guests agree at each instruction and on whole final machine state.
The owned header is registered through default `bus.unit`; it is not a new
external ROM or dependency.

## Revision consumers and regression scope

README, public header, API/reproduction docs, API declaration policy, current
revision controls and required hash consumers identify behavior14 coherently.
The complete licensed-homebrew hash writer produces exactly the prior key
inventory and every non-state row, size and tick unchanged. Twelve native
hashes differ only because behavior14 and its CRC are recorded: restoring
revision13 and recomputing CRC recovers all12 exact prior hashes. This is
field/layout evidence; it is not cross-platform or performance qualification.

The focused byte-edge fixture reaches counter4/GET half/bits0 with real
bus reads, fetches77 at577 GET, and checks that the resumed output edge keeps
it full, silent and without pending refill. The DAC stays unchanged through
seven further bit clocks, then the next boundary consumes the byte, changes
the DAC and completes exactly one looping refill. The wrong visibility model
has the same first-GET/resume timing but empties the buffer, queues a refill
and changes the DAC immediately; it is independently rejected.

Native frame1 cuts cover an armed stopped request at29784 and an imminent
stopped request at29783. Existing pending state survives save/load and the
next actual INC takes8/9 clocks respectively versus5 without that history;
remaining stays0, with no underflow/restart. A fresh-buffer cut at29784 has
bits7/silence1/full77/remaining1/no pending. Poisoned native restore agrees
with the never-loaded reference through complete frame2/3 pixels, PCM,
metadata, field-wise cycle/level transitions, RAM and final bytes. Removing
buffer history produces a distinct continuation. Old format5 and invalid
field values refuse atomically. These use genuine guest-produced public
cuts, not a mid-DMA API or injected positive timer/CPU history.

Fresh local verification: full Release warnings-as-errors build; focused
`dev-check` workflow (20 tests,2.36s CTest); all64 sweep entries and named
wrong models in default bus.unit; native short stress with NROM/MMC3
OAM/DMC/IRQ witnesses and all new native cuts; documentation/policy controls
including mutation tests; licensed-homebrew hashes and all current revision
consumers. Consumer selection was inspected and contains no campaign setup.
All selected checks pass. Final logs live under ignored
`build/phase24-owned/revision14-*` and `phase24-*-final.txt`.

Independent reviews found no readiness-routing/state-boundary defect. Their
remaining concern was precisely the unsupported universal-hardware claim and
missing focused buffer/audio fixture; both are addressed by the explicit
inference label and the focused positive/wrong-model test above. Full-ROM acceptance of behavior14, fresh protected-main
authority, hosted matrix/vector/hash checks, performance and inherited
branch-wide whitespace remain unresolved. Canonical campaign metadata and
scoreboard are unchanged until attributable qualifying observations exist.
