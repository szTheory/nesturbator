# Phase 30 discussion log

**Date:** 2026-11-19
**Mode:** Autonomous smart discuss; inherited v11 decisions retained.
**Result:** Ready for planning; no owner decision remains open.

## Revalidated facts

- Live Nesturbator `main` was confirmed independently through `git ls-remote`,
  GitHub's branch/API view, and the pinned public header: commit
  `8923b66cca16739c5b402643798515cf5537ae22`, release 0.1.13, ABI 1, behavior
  revision 9 and state format 3. The isolated phase branch has that exact HEAD.
- The live tree has no MMC2 source yet. `src/mapper.h` reserves a watch-gated
  PPU-read hook; `src/cartridge.c` dispatches boards and `src/ppu.c` owns PPU
  memory reads. Existing board/state/test infrastructure is the direct port
  seam. This supports a small integration rather than wholesale file reuse.
- V10's completed implementation is evidence for its bounded mapper profile,
  but its rev18/format7 values are incompatible with this mainline. The v11
  target's rev10/format4/ABI1 values remain the selected minimal compatibility
  change. Re-derive if the exact upstream head changes.
- The current LibFuturo public `main` remains at
  `fa1908a9831354cb325f9480be0559d7c5dca687`. Its README says its current
  source-tree consumer uses `synthetic-v1`; its seam map is pinned to
  Nesturbator `2338608ab184f040620a30d17756f5e06a42d4e3`, rev9/state3, and
  describes a create/destroy probe only. No current LibFuturo roadmap file is
  present in that public tree. The owner-supplied 2026-11-19 report continues
  to identify the publisher receipt as missing. These facts leave its consumer,
  host, gameplay and receipt gates independent of MMC2.
- The target's full AccuracyCoin campaign is not enabled by the `ci` preset's
  default configure. Local acceptance must enable
  `NESTURBATOR_ACCURACYCOIN_FULL=ON` before invoking the unchanged
  `cmake --workflow --preset ci`. The campaign's historical 56-path source
  closure and existing test-registration pins must be preserved; any new MMC2
  admission is limited to the reviewed current source/test delta and tied to
  the same-run fingerprint.
- Current target CI/package timings showed no measured recurring inefficiency
  that justifies dropping coverage, adding a lane, or adding a dependency. The
  local full workflow remains under the existing 1500-second plan gate and
  30-minute workflow ceiling. Official CMake 3.25 documentation confirms
  `-D` cache-variable configuration and workflow preset execution semantics.

## Decision comparison

| Area | Recommended and retained | Rejected alternative and reason |
|---|---|---|
| Port route | Port the bounded v10 behavior onto exact live main; keep ABI1 and set behavior10/state4. | Cherry-pick the whole v10 branch: it conflicts across 11 files and embeds rev18/format7 from a different base. |
| Mapper evidence | Keep NES 2.0 mapper9/submapper0, marker-based actual reads, cross-deliverable parity and secondary-reference latch claim. | Add title/hardware claims or broader boards: no independent evidence or authority in this milestone. |
| AccuracyCoin | Preserve historical campaign and baseline pins; minimally admit exact new mapper/test files using current-candidate fingerprint checks. | Re-anchor the baseline or relax the historical checker: would mask a mismatch and violate protected evidence. Skip full campaign: hosted target runs it, so that would omit required acceptance. |
| Consumer route | Proceed independently with MMC2; retain the missing receipt and LibFuturo acceptance as consumer/provider gates. | Treat MMC2, the old source seam or synthetic-v1 proof as receipt/gameplay evidence: none exercises the installed NES consumer. |
| CI efficiency | Keep the existing workflow, with full campaign enabled locally; reuse existing focused/automated checks. | Add cache, lane, dependency or performance capture without measured recurring value; expand beyond this approved milestone. |

## Bounded review synthesis

Three parallel read-only reviews covered compatibility/architecture, port
assumptions and verification/release/consumer scope. They agreed that the exact
mainline baseline requires re-deriving compatibility rather than copying v10's
version values, the 8 added MMC2 state bytes require the format bump, and the
current source-closure gate is the main implementation risk. The verification
review identified the disabled-by-default local AccuracyCoin campaign and the
minimal checker adaptation needed to admit reviewed MMC2 source/test entries
without changing protected metadata. The consumer review confirmed that the
LibFuturo receipt and installed-consumer journey are separately owned and have
no current acceptance evidence. An adversarial comparison favored this narrow
port over whole-branch reuse, checker re-anchoring, or a broader cleanup.

## Sources

- Nesturbator base header and source:
  <https://github.com/szTheory/nesturbator/tree/8923b66cca16739c5b402643798515cf5537ae22>
- LibFuturo README and seam map:
  <https://github.com/szTheory/libfuturo/blob/fa1908a9831354cb325f9480be0559d7c5dca687/README.md>
  · <https://github.com/szTheory/libfuturo/blob/fa1908a9831354cb325f9480be0559d7c5dca687/docs/nesturbator-seams.md>
- Official CMake 3.25 command-line reference:
  <https://cmake.org/cmake/help/v3.25/manual/cmake.1.html>
- Local source files: `.planning/workstreams/mmc2/STATE.md`, `V10-HANDOFF.md`,
  `V11-HANDOFF.md`, `ROADMAP.md`, `REQUIREMENTS.md`, the preparation index,
  `src/mapper.h`, `src/cartridge.c`, `src/ppu.c`, and existing AccuracyCoin
  checks. The broader verification and discussion policies were read in the
  originating workstream checkout; their Phase 30 constraints are preserved in
  `30-CONTEXT.md` and this research record rather than copied wholesale.
