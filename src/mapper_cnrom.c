/* Mapper 3, CNROM: fixed 16 or 32 KiB of PRG ROM as on NROM and a switchable
   8 KiB CHR ROM bank. Any CPU write to $8000-$FFFF selects the CHR bank.
   Writes to CHR ROM do nothing: the board has no CHR RAM. Source: NESdev Wiki
   "CNROM", "INES Mapper 003", "Bus conflict" and "NES 2.0 submappers".

   Submapper 1 has no bus conflicts and submapper 2 has AND conflicts. NESdev
   says CNROM is always subject to AND-type conflicts, so the project default
   for submapper 0 (BOARD-01) is the AND. The power-on bank register is 0,
   which NESdev leaves unspecified, so that is an implementation choice (D-16). */
#include "internal.h"

static void cnrom_init(struct nesturbator *nes)
{
    nes->map.watch = NESTURBATOR_WATCH_CPU_WRITE;
    if (nes->mapper.submapper == 0u || nes->mapper.submapper == 2u)
        nes->map.watch |= NESTURBATOR_WATCH_BUS_CONFLICT;
}

static void cnrom_rebuild(struct nesturbator *nes)
{
    struct nesturbator__map *map = &nes->map;
    nesturbator__map_prg_ram(nes);
    for (uint32_t i = 16u; i < 48u; ++i) {
        map->cpu_r[i] = nesturbator__map_prg(nes, i - 16u);
        map->cpu_w[i] = NULL;
    }
    /* chr_w stays NULL for CHR ROM, so the seam drops pattern-table writes. */
    nesturbator__map_chr_8k(nes, nes->mapper.reg.cnrom.bank);
    nesturbator__map_header_mirroring(nes);
}

static void cnrom_cpu_write(struct nesturbator *nes, uint16_t addr, uint8_t value,
                            uint64_t cpu_cycle)
{
    (void)cpu_cycle;
    if (addr < 0x8000u)
        return;
    /* bus.c has already ANDed the value when the watch bit asks for it. The raw
       byte is kept: bits 4-5 (the mapper 185 diode bits) are not interpreted,
       and the modulo in the CHR mapping wraps any value. */
    nes->mapper.reg.cnrom.bank = value;
    nes->map.ops.rebuild(nes);
}

void nesturbator__mapper_cnrom_ops(struct nesturbator__mapper_ops *out)
{
    out->init = cnrom_init;
    out->rebuild = cnrom_rebuild;
    out->cpu_write = cnrom_cpu_write;
    out->ppu_a12 = NULL;
    out->ppu_read = NULL;
}
