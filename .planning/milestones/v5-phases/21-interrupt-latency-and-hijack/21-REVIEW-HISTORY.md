# Phase 21 source-review corroboration

The original `21-REVIEW.md` report and its history are preserved. Its current
working copy records the follow-up disposition and the reviewer confirmed no
active findings on the changed source and tests. Independent corroboration
rejects CR-01; CR-02 was fixed and locally qualified in `43d260a`. This document
records evidence and disposition, not hosted acceptance. The prior candidate
`1d27a0f` and review commit `7d5fbe1` remain historical.

## Primary sources

- **S1:** [Visual6502 State Machine](https://www.nesdev.org/wiki/Visual6502wiki/6502_State_Machine),
  transistor-level timing analysis, section on BRK/VEC1: interrupt sources share
  the forced-BRK entry mechanism and request bookkeeping is cleared after vector
  handling. This supports arbitration consumption rather than an independent IRQ
  queue surviving NMI entry.
- **S2:** [Visual6502 Interrupt Hijacking, revision 19268](https://www.nesdev.org/w/index.php?title=Visual6502wiki/6502_Interrupt_Hijacking&oldid=19268),
  based on node/transistor inspection and simulator experiments: stage1 NMI
  recognition is inhibited after vector choice; held input is recognized at the
  next T1, while a pulse released before that point is lost. Its IRQ-after-NMI
  discussion assumes a source holds the IRQ level until software acknowledges it.
- **S3:** [MOS MCS6500 hardware manual](https://ftpmirror.your.org/pub/misc/bitsavers/components/mosTechnology/6500-10A_MCS6500hwMan_Jan76.pdf),
  IRQ/NMI signal descriptions and interrupt handling: IRQ is level-sensitive and
  interrupt entry sets I; RTI restores the interrupted status.
- **S4:** [Pinned AccuracyCoin assembly](https://raw.githubusercontent.com/100thCoin/AccuracyCoin/6bee919529289c555803b612b8c9e4f7eaaeeabb/AccuracyCoin.asm),
  IFlagLatency and paired NMI/IRQ overlap source schedules. Ordinary accepted
  branch IRQ history survives a later source acknowledgement; this does not
  establish a second independent queued IRQ through NMI arbitration.

The existing annex's C/T crosswalk remains a source-derived finite-margin
inference. No equal-timestamp physical winner is claimed. Source S2's inhibited
window is not interchangeable with Phase20 ordinary edge capture.

## CR-01: rejected; preserve current arbitration behavior

The finding treats an accepted IRQ as an independently queued event after NMI
wins entry arbitration. Sources S1/S3 instead establish one shared forced-BRK
entry event, I masking during its handler, and later level-sensitive IRQ
recognition. S2's IRQ-after-NMI discussion assumes the source remains asserted;
it does not establish an acknowledged IRQ that must still be serviced later.

New owned production regressions schedule both sources at the same NOP poll.
They pass on the unchanged runtime: NMI wins, its first handler instruction
runs with I set, an NMI handler's actual `$4015` read suppresses subsequent IRQ,
and a held source is freshly accepted after RTI restores I. The ordinary
branch-accepted IRQ/source-acknowledgement controls remain passing separately.
Thus no IRQ queue-preservation runtime change is applied.

The review's exact proposed conditional-preservation change was exercised as a
temporary mutant. It fails the new regression: `irq_accepted` stays 1 and IRQ
entry runs before the first NMI-handler marker despite I being set. Logs and
CPU/bus/binary identities are retained in
`build/entry-review-evidence/cr01-proposed-queue-mutant.log` and
`cr01-mutant-identity.json`. The mutant was removed.

## CR-02: corroborated, fixed, re-reviewed, and locally qualified

Before-fix production cases assert an input for a complete sampled C5, C6 or C7
cycle, then release before the first handler call. All nine BRK/IRQ/NMI-origin
cases enter an unwanted NMI instead of completing the first handler's RTI.
A first-handler branch also incorrectly recognizes the released input.
The fresh baseline evidence is `build/entry-review-evidence/red.log` and
`red-identity.json`. C5/C6 cases have later entry cycles before T1 recognition.

The C7-close/next-T1 boundary has no positive whole-cycle gap in the current
bus-call crosswalk. A C7 close followed by release before the first handler call
can establish deterministic model behavior, but cannot alone prove a physical
finite-margin release. C5/C6 schedules provide that independent margin.

The repair distinguishes a recognized request from an inhibited candidate
using value 2 in the existing `nmi_pending` byte. Raw line sampling continues.
Candidate 2 is canceled on release and promoted only when held through the
first handler opening, before that instruction's C1 branch poll. Already
recognized history 1 stays sticky; consumed held inputs do not generate another
entry. C4 vector selection consumes only recognized history, preserving the
same coherent vector pair and ordinary Phase20 close/open timing.

No field or byte is added. State format 3, revision 9, ABI 1 and all 427 effective
source paths remain unchanged. The visitor bounds the pending byte to 0..2,
requires candidate2's sampled-high/no-poll/no-JAM history and its C6/C7 or idle
after-C7 location, and rejects recognized value1 in BRK/IRQ post-choice stages
where C4 already consumed it. Current PPU line equality is not required.
CRC-correct controls exercise these cross-field refusals before APPLY, complete
instance/generation preservation, native round-trips and future execution.
An explicit production C7 sibling reaches idle candidate 2 with the current
input released. CRC-correct accepted-IRQ, JAM, raw IRQ poll, NMI poll and
sampled-low neighbors are refused there: no instruction or poll can occur
between completed C7 and qualification at the first handler opening. Its
sampled-high/current-line-low positive sibling remains accepted.

The old fresh-edge test had not released the consumed NMI before its supposed
C3 edge: the first new sampled edge was actually its inhibited C5 pulse.
Releasing the old input before C1 makes C3 a real eligible edge; the same
original pending/second-service marker assertions remain. Its C5 pulse cannot
erase that recognized request. Additional siblings cover all three origins at
C5/C6/C7, held inputs, consumed levels, native idle-candidate reload, first
handler branch recognition, and active held/released candidate replay.

The conditional pending maximum moved from the unconstrained at-bound row to
the existing conditional-field test list, alongside the entry-stage fields.
Its maximum 2 is tested through production-reachable held/released cutpoints;
the range sweep still rejects 3/type-max, and new CRC-correct invalid-history
neighbors reject 2 with a false sampled line, accepted poll, JAM or pre-C5 stage.
This preserves field coverage without fabricating an impossible legal sibling.

Removing only the recognition correction restores the new pulse failures;
reapplying it restores the passing cases. The exact bus/CPU/test-binary identity
and output are in `cr02-revert-identity.json` and `cr02-revert.log`. No Stryker
configuration exists for this owned C project; explicit causal reverts and the
CR-01 proposal mutant provide the recorded negative controls.

## Fresh verification and identities

Fresh Release/Ninja build `build/entry-review-repair` uses warnings as errors
and `NESTURBATOR_ACCURACYCOIN_FULL=ON`. After both temporary mutants were removed,
the following command passed **19/19 in 68.11 seconds**:

```sh
ctest --test-dir build/entry-review-repair -j 1 -R '^(core\.(interrupt_entry|interrupt_state|nmi.*|irq_poll|state_generation|state_reject|state_runahead|state_fork.entry)|ppu\.nmi_races|cpu\.unit|bus\.unit|libretro\.runahead|fuzz\.state.*|accuracycoin\.campaign)$' --output-on-failure
```

This includes native and actual-adapter runahead, every existing entry cutpoint,
Phase20 ordinary NMI controls, reset/generation/rejection, native seeds, CPU/bus
and the full campaign. `final-focused.log` retains the output. The full campaign
remains **119 PASS/27 FAIL**, all 119 previous PASS keys retained, all 11 v5
targets PASS, 146 terminal scored rows and 5 DRAW. Every status/raw/frame row is
unchanged, so there is no campaign answer-table or row-pin adjustment.

The actual adapter was then extended and reverified specifically for the new
serialized semantic state. `rah_entry_inhibited_state` in
`tests/libretro/libretro_host.c` reaches candidate 2 through a real C5 or C7
sample and saves through the public visitor. Four histories cover active C6
and idle after C7, each with the input held or released. `rah_entry_replay`
loads each through the actual `retro_unserialize`, requires the initial
`retro_serialize` byte string to equal the candidate-2 cutpoint exactly, and
compares all committed/speculative video, stereo PCM, state, ticks, frame numbers
and both handler markers against native continuation. Absolute first-frame
marker assertions require IRQ=1 and NMI=1 only for held inputs, NMI=0 for release.
Both existing scripts and depths one through three run for each history.

`ctest --test-dir build/entry-review-repair -R '^libretro.runahead$' -V` passed
in 26.63 seconds; `adapter-candidate2.log` retains the four named histories and
`entry_inhibited_histories=4 entry_inhibited_script_cases=24`. The existing
`entry_cutpoints=20 entry_depth_cases=60 entry_script_cases=120` and all four
entry controls remain unchanged. No timeout, base coverage or assertion was
weakened. Runtime, module, runner and all prior tested source identities stayed
byte-identical during this test-only extension; the final identity file also
includes the extended host source and host binary.

Final source/binary/report digests and exact-key gates are in
`build/entry-review-evidence/final-identity.json` and `final-gates.json`.
Key SHA256 values:

- `src/bus.c`: `446079ddc3c479d5a9cb3e5899b0d95f3142deee07ef9e5d0f0ea88d686922a6`
- `src/cpu.c`: `036eb8943179edfdba7110f8857eee662c35abad102287f483ef31f89d6a32db`
- `src/state.c`: `c6926b8baef622700e9626ac5f814d6003f203049cdf0ea14a9e1c8b6208b753`
- Campaign 56-input fingerprint: `d96efe4a88bdaa0680398334a3c84a0e04faef405016172e06ef2bb9c4999e57`
- Unchanged campaign report: `56ebd01adb97b3916fde36f8739602284d951a1fd00d84285bb4ef441fb463a3`
- Dirty-source 427-input mode/blob manifest: `5b472e1930388bfe56fb2c795ecbe00766f95ba2d5d05dcf1ab436ad1a989305`

The original 427-path manifest records HEAD 7d5fbe1 and actual changed working
blobs; it remains historical. The post-review candidate is
`43d260a914ba76a130809524fba53ad58aca1298`, with a new 427-path mode/blob
manifest at `build/phase21-hosted-evidence/source-modes-blobs-candidate.tsv`
(SHA-256 `20460339ae9f742d3906ebe7ef6d5c4a2fff53c5c4b54cf59bcbb7b99cb9a1b2`).
The candidate manifest verified against the current checkout and an absolute
clean clone at that commit. DMC retry, controller upper bits, timer/write
ordering, pin/caps, paired tables, shared IRQ arbitration and global timestamp
ordering are preserved.

Post-review local qualification passed full CI 695/695, ASAN 686/686,
vectors-full 258/258, and MMC3 13/13. The no-float scan passes on 23 source
files. The macOS undefined-symbol check still rejects Apple
`stack_chk_fail`/`stack_chk_guard`; this known platform issue was not waived.
Fresh protected-main revision `a64c23fd3ca7caed7e3ad102ff1a9beaa29fa562`
provided 114 PASS keys (SHA-256
`b814f7524d15e0f389e97cb1e8567efbbce601cc8b4c8902ef3471ab965991c4`); an
explicit current-source CTest invocation passed the campaign fixture,
same-invocation protected observation/verdict, no-lost-pass and all-eleven
target gate (5/5).

## Hosted closeout remains pending

Candidate `1d27a0f` and its manifest remain historical and are superseded by the
post-review candidate. Plan 09 still requires actual delivery, PR, CI/Nightly
run and attempt IDs, tested-merge SHA, protected base, and immutable hosted
artifacts. This disposition does not claim hosted acceptance, requirement or
phase completion. No dispatch, push, PR, or merge occurred.
