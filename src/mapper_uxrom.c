/* Mapper 2, UxROM: a switchable 16 KiB PRG bank at $8000-$BFFF, the last
   16 KiB bank fixed at $C000-$FFFF, and 8 KiB of CHR RAM or ROM. Any CPU write
   to $8000-$FFFF selects the bank. Source: NESdev Wiki "UxROM", "Bus conflict"
   and "NES 2 submappers".

   Submapper 1 has no bus conflicts and submapper 2 has AND conflicts. NESdev
   leaves submapper 0 open; the project default fixed by BOARD-01 is the AND.
   The power-on bank register is 0, which NESdev leaves unspecified, so that is
   an implementation choice (D-16). */
#include "internal.h"

static void uxrom_init(struct nesturbator *nes)
{
    nes->map.watch = NESTURBATOR_WATCH_CPU_WRITE;
    if (nes->mapper.submapper == 0u || nes->mapper.submapper == 2u)
        nes->map.watch |= NESTURBATOR_WATCH_BUS_CONFLICT;
}

static void uxrom_rebuild(struct nesturbator *nes)
{
    struct nesturbator__map *map = &nes->map;
    const uint32_t last_1k = (uint32_t)(nes->cart.prg_size / 1024u) - 16u;
    nesturbator__map_prg_ram(nes);
    for (uint32_t i = 16u; i < 32u; ++i) {
        map->cpu_r[i] =
            nesturbator__map_prg(nes, (uint32_t)nes->mapper.reg.uxrom.bank * 16u + (i - 16u));
        map->cpu_w[i] = NULL;
    }
    for (uint32_t i = 32u; i < 48u; ++i) {
        map->cpu_r[i] = nesturbator__map_prg(nes, last_1k + (i - 32u));
        map->cpu_w[i] = NULL;
    }
    nesturbator__map_chr_8k(nes, 0u);
    nesturbator__map_header_mirroring(nes);
}

static void uxrom_cpu_write(struct nesturbator *nes, uint16_t addr, uint8_t value,
                            uint64_t cpu_cycle)
{
    (void)cpu_cycle;
    if (addr < 0x8000u)
        return;
    /* bus.c has already ANDed the value when the watch bit asks for it, against
       the banking in force before this write. The full 8-bit value is kept
       (NESdev: treat it as a full register); the modulo wraps it. */
    nes->mapper.reg.uxrom.bank = value;
    nes->map.ops.rebuild(nes);
}

void nesturbator__mapper_uxrom_ops(struct nesturbator__mapper_ops *out)
{
    out->init = uxrom_init;
    out->rebuild = uxrom_rebuild;
    out->cpu_write = uxrom_cpu_write;
    out->ppu_a12 = NULL;
    out->ppu_read = NULL;
}
