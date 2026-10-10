/* Mapper 7, AxROM: a switchable 32 KiB PRG bank at $8000-$FFFF, 8 KiB of CHR
   RAM, and single-screen mirroring chosen by the register. Any CPU write to
   $8000-$FFFF sets the register, laid out xxxM xPPP: PPP selects the 32 KiB
   bank and M selects which CIRAM page all four nametables use. The header
   mirroring bit has no effect. Source: NESdev Wiki "AxROM", "Bus conflict",
   "Mirroring" and "NES 2.0 submappers".

   The power-on register is 0, which NESdev leaves unspecified, so bank 0 and
   page 0 are an implementation choice (D-16); the reset vector is therefore
   read from bank 0. */
#include "internal.h"

static void axrom_init(struct nesturbator *nes)
{
    nes->map.watch = NESTURBATOR_WATCH_CPU_WRITE;
    /* NESdev says submapper 0 should not emulate bus conflicts, because some
       games glitch under them; only submapper 2 asks for the AND. */
    if (nes->mapper.submapper == 2u)
        nes->map.watch |= NESTURBATOR_WATCH_BUS_CONFLICT;
}

static void axrom_rebuild(struct nesturbator *nes)
{
    struct nesturbator__map *map = &nes->map;
    const uint32_t bank = (uint32_t)(nes->mapper.reg.axrom.bank & 7u);
    const uint8_t page = (uint8_t)((nes->mapper.reg.axrom.bank >> 4) & 1u);
    nesturbator__map_prg_ram(nes);
    for (uint32_t i = 16u; i < 48u; ++i) {
        map->cpu_r[i] = nesturbator__map_prg(nes, bank * 32u + (i - 16u));
        map->cpu_w[i] = NULL;
    }
    nesturbator__map_chr_8k(nes, 0u);
    /* Single-screen: all four nametables on one CIRAM page; the header bit is ignored. */
    for (uint32_t k = 0u; k < 4u; ++k)
        map->nt[k] = page;
}

static void axrom_cpu_write(struct nesturbator *nes, uint16_t addr, uint8_t value,
                            uint64_t cpu_cycle)
{
    (void)cpu_cycle;
    if (addr < 0x8000u)
        return;
    nes->mapper.reg.axrom.bank = value;
    nes->map.ops.rebuild(nes);
}

void nesturbator__mapper_axrom_ops(struct nesturbator__mapper_ops *out)
{
    out->init = axrom_init;
    out->rebuild = axrom_rebuild;
    out->cpu_write = axrom_cpu_write;
    out->ppu_a12 = NULL;
    out->ppu_read = NULL;
}
