/* Mapper 0, NROM: 16 or 32 KiB of PRG ROM at $8000-$FFFF, 8 KiB of CHR ROM
   or RAM, no registers. Source: NESdev Wiki "NROM" and "INES" [HWP.14]. */
#include "internal.h"

static void nrom_init(struct nesturbator *nes)
{
    /* No registers, so nothing is watched. */
    nes->map.watch = 0u;
}

static void nrom_rebuild(struct nesturbator *nes)
{
    struct nesturbator__map *map = &nes->map;
    nesturbator__map_prg_ram(nes);
    for (uint32_t i = 16u; i < 48u; ++i) {
        /* A 16 KiB ROM appears at both $8000 and $C000: the bank number wraps. */
        map->cpu_r[i] = nesturbator__map_prg(nes, i - 16u);
        map->cpu_w[i] = NULL;
    }
    nesturbator__map_chr_8k(nes, 0u);
    nesturbator__map_header_mirroring(nes);
}

void nesturbator__mapper_nrom_ops(struct nesturbator__mapper_ops *out)
{
    out->init = nrom_init;
    out->rebuild = nrom_rebuild;
    out->cpu_write = NULL;
    out->ppu_a12 = NULL;
    out->ppu_read = NULL;
}
