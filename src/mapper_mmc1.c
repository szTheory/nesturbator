/* Mapper 1, MMC1 (MMC1B): a serial port at $8000-$FFFF feeding Control, CHR
   bank 0, CHR bank 1 and PRG bank registers, and up to 32 KiB of PRG RAM at
   $6000-$7FFF. Source: NESdev Wiki "MMC1" and "SxROM". MMC1 has no bus
   conflict, so only the write watch is requested.

   This step has the power-on mapping only: Control $0C (PRG mode 3, the last
   16 KiB bank fixed at $C000, one-screen lower mirroring, 8 KiB CHR mode) with
   the PRG bank register 0 and RAM enabled. The registers are stored so that a
   zeroed block is that state (see mapper.h), which keeps the seam rule that
   the loader's memset is the power-on state and init writes no register. */
#include "internal.h"

static void mmc1_init(struct nesturbator *nes)
{
    nes->map.watch = NESTURBATOR_WATCH_CPU_WRITE;
}

static void mmc1_rebuild(struct nesturbator *nes)
{
    struct nesturbator__map *map = &nes->map;
    /* Control is stored XOR $0C; this step only has the power-on value. */
    const uint32_t control = (uint32_t)(nes->mapper.reg.mmc1.control_x ^ 0x0cu);
    const uint32_t bank = (uint32_t)(nes->mapper.reg.mmc1.prg & 0x0fu);
    (void)control;
    /* PRG mode 3: bank `prg` at $8000, the last bank at $C000. 0x0F wraps to
       the last bank through the true modulo in nesturbator__map_prg. */
    for (uint32_t i = 16u; i < 32u; ++i) {
        map->cpu_r[i] = nesturbator__map_prg(nes, bank * 16u + (i - 16u));
        map->cpu_w[i] = NULL;
    }
    for (uint32_t i = 32u; i < 48u; ++i) {
        map->cpu_r[i] = nesturbator__map_prg(nes, 0x0fu * 16u + (i - 32u));
        map->cpu_w[i] = NULL;
    }
    nesturbator__map_chr_8k(nes, 0u);
    map->nt[0] = 0u;
    map->nt[1] = 0u;
    map->nt[2] = 0u;
    map->nt[3] = 0u;
    nesturbator__map_prg_ram_8k(nes, nes->cart.prg_ram);
}

static void mmc1_cpu_write(struct nesturbator *nes, uint16_t addr, uint8_t value,
                           uint64_t cpu_cycle)
{
    /* D-15 step 1: a write on the cycle right after another hook write is
       ignored by the real chip. last_write holds the previous stamp plus one,
       so zero (never written) is never adjacent. */
    const int adjacent =
        nes->mapper.reg.mmc1.last_write != 0u && cpu_cycle == nes->mapper.reg.mmc1.last_write;
    (void)adjacent;
    (void)addr;
    (void)value;
    nes->mapper.reg.mmc1.last_write = cpu_cycle + 1u;
}

void nesturbator__mapper_mmc1_ops(struct nesturbator__mapper_ops *out)
{
    out->init = mmc1_init;
    out->rebuild = mmc1_rebuild;
    out->cpu_write = mmc1_cpu_write;
    out->ppu_a12 = NULL;
    out->ppu_read = NULL;
}
