# Mapper coverage and expansion priorities

Recorded 2026-10-30 after the owner asked how many mappers remain and requested
that the answer survive context clearing. This is planning context and a
recommendation, not approval of a mapper milestone or a compatibility claim.

## Current support and the size of the remaining work

The [public cartridge contract](../../../include/nesturbator.h) supports six
mapper IDs: **0 NROM, 1 MMC1, 2 UxROM, 3 CNROM, 4 MMC3 and 7 AxROM**, within
specified board/submapper, memory and NTSC limits. Supporting an ID does not
automatically support every board variant that shares it.

The community registry has hundreds of assigned IDs, including variants,
multicarts, bootlegs and homebrew. Original iNES has 256 numbering slots;
NES 2.0 has 4,096. Those capacities are not counts of distinct physical designs
or a remaining implementation checklist. Do not describe this project as
"six out of 4,096 complete" or promise exhaustive support from the ID count.
[Mapper registry](https://www.nesdev.org/wiki/Mapper),
[file-format specification](https://www.nesdev.org/wiki/INES),
[submapper specification](https://www.nesdev.org/wiki/NES_2.0_submappers).

The existing [cartridge survey](../../preparation/NES-HARDWARE-PPU-CARTRIDGE.md#5-cartridges-and-mappers)
used a **2020-04-19 database snapshot**, counting each dump/revision separately.
The six families account for **678/705 licensed North American entries (96.2%)**
and **1,870/2,216 licensed entries overall (84.4%)** in that snapshot. These are
historical mapper-prevalence figures, not current unique-game coverage or the
percentage that Nesturbator runs correctly. Region, board-profile restrictions
and remaining core accuracy issues still matter. Do not promote the survey to
a measured compatibility claim or silently treat it as a current database.

## Concrete demand and relative effort

The original requested games outside current support need **three additional
mapper IDs**, plus regional work:

| Requested game | Missing capability | Planning assessment |
|---|---|---|
| Punch-Out!! (USA) | MMC2, mapper 9 | Preferred next mapper candidate when expansion is selected. Moderate relative scope: graphics fetches trigger bank latches, needing precise observation and state/replay tests. |
| Castlevania III (USA) | MMC5, mapper 5 | Larger independent scope: banking, graphics extensions, RAM, IRQs, audio and continuation. One game's boot is not full MMC5 acceptance. |
| Gimmick! (Japan) | Sunsoft-5B, mapper 69 | More involved than a simple bank register: banking/IRQ work plus expansion sound, integer mixing and audio/state continuity. |
| Ufouria (Europe) | PAL timing/pitch and host metadata | The selected image is mapper 4, already a supported family. Its regional requirement does not call for a new mapper ID. |

Relative effort is engineering judgment from hardware features, not an estimate
of implementation time. Some remaining boards are simpler: mapper 206 is a
simplified MMC3 predecessor. MMC1/MMC3 already implemented here also have
nontrivial timing and variant requirements, so difficulty does not increase
monotonically as more IDs are added.
[Mapper designations/features](https://www.nesdev.org/wiki/List_of_mappers),
[MMC2 latch behavior](https://www.nesdev.org/wiki/MMC2),
[MMC5 hardware](https://www.nesdev.org/wiki/MMC5).

## Recommendation and promotion rule

Prioritize named games, hardware references and permitted test readiness over
the number of mapper IDs implemented. Favor **MMC2/Punch-Out!!** for the next
mapper expansion; keep MMC5 and expansion-audio boards independently scoped.
This does not displace the proposed player-readiness milestone or automatically
promote a mapper ahead of confirmed scene faults or DMA work.

The [product roadmap](../../PRODUCT-ROADMAP.md) keeps mapper expansion proposed
in Mid, MMC5/expansion audio in Long, and PAL as the regional alternative. The
owner must select a future milestone before any of these becomes committed.
No new implementation, fixture, benchmark or release qualification is claimed.
