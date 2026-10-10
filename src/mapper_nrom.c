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
    for (uint32_t i = 8u; i < 16u; ++i) {
        /* $6000-$7FFF is PRG RAM when the loader allocated it (a trainer image). */
        uint8_t *ram = nes->cart.prg_ram != NULL ? nes->cart.prg_ram + (i - 8u) * 1024u : NULL;
        map->cpu_r[i] = ram;
        map->cpu_w[i] = ram;
    }
    for (uint32_t i = 16u; i < 48u; ++i) {
        /* A 16 KiB ROM appears at both $8000 and $C000: the bank number wraps. */
        map->cpu_r[i] = nesturbator__map_prg(nes, i - 16u);
        map->cpu_w[i] = NULL;
    }
    for (uint32_t i = 0u; i < 8u; ++i) {
        map->chr_r[i] = nesturbator__map_chr(nes, i);
        map->chr_w[i] = nes->cart.chr_is_ram != 0u ? nesturbator__map_chr(nes, i) : NULL;
    }
    /* iNES flags 6 bit 0: vertical (1) or horizontal (0) mirroring. [HWP.14] */
    if ((nes->cart.bytes[6] & 1u) != 0u) {
        map->nt[0] = 0u;
        map->nt[1] = 1u;
        map->nt[2] = 0u;
        map->nt[3] = 1u;
    } else {
        map->nt[0] = 0u;
        map->nt[1] = 0u;
        map->nt[2] = 1u;
        map->nt[3] = 1u;
    }
}

void nesturbator__mapper_nrom_ops(struct nesturbator__mapper_ops *out)
{
    out->init = nrom_init;
    out->rebuild = nrom_rebuild;
    out->cpu_write = NULL;
    out->ppu_a12 = NULL;
    out->ppu_read = NULL;
}
