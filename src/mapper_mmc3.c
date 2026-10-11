/* Mapper 4, MMC3 (Sharp and NEC revisions): eight bank registers behind a
   select/data pair at $8000-$9FFF, mirroring and RAM control at $A000-$BFFF,
   and a scanline counter at $C000-$FFFF that is clocked by rises of PPU A12.
   Source: NESdev Wiki "MMC3" (oldid 24268), "NES 2.0 submappers" (mapper 004)
   and "Mirroring". MMC3 has no bus conflict, so the watch is the CPU write and
   the PPU A12 edge only.

   Registers decode by addr & $E001, so each 8 KiB range is an even/odd pair
   mirrored across the range. R0/R1 are 2 KiB CHR banks (low bit ignored), R2-R5
   are 1 KiB CHR banks, R6/R7 are 8 KiB PRG banks (top two bits ignored). Every
   bank number wraps by true modulo against the validated size, so a 16 KiB PRG
   aliases its two 8 KiB banks and no register value can point outside the
   allocation.

   Power-on is a zeroed block (see mapper.h): PRG mode 0, no CHR inversion,
   vertical mirroring, IRQ disabled, and RAM enabled and writable. */
#include "internal.h"

#define MMC3_PRG_PAGES 8u /* 1 KiB pages in one 8 KiB window */

static void mmc3_init(struct nesturbator *nes)
{
    nes->map.watch = NESTURBATOR_WATCH_CPU_WRITE | NESTURBATOR_WATCH_PPU_A12;
}

/* One 1 KiB CHR page; writable only for CHR-RAM, as the shared 4 KiB helper does. */
static void map_chr_1k(struct nesturbator *nes, uint32_t page, uint32_t bank_1k)
{
    nes->map.chr_r[page] = nesturbator__map_chr(nes, bank_1k);
    nes->map.chr_w[page] = nes->cart.chr_is_ram != 0u ? nesturbator__map_chr(nes, bank_1k) : NULL;
}

/* One 8 KiB PRG window (window 0 is $8000) from an 8 KiB bank number. */
static void map_prg_window(struct nesturbator *nes, uint32_t window, uint32_t bank_8k)
{
    for (uint32_t i = 0u; i < MMC3_PRG_PAGES; ++i) {
        nes->map.cpu_r[16u + window * MMC3_PRG_PAGES + i] =
            nesturbator__map_prg(nes, bank_8k * MMC3_PRG_PAGES + i);
        nes->map.cpu_w[16u + window * MMC3_PRG_PAGES + i] = NULL;
    }
}

static void mmc3_rebuild(struct nesturbator *nes)
{
    const struct nesturbator__mapper *m = &nes->mapper;
    struct nesturbator__map *map = &nes->map;
    const uint32_t banks_8k = (uint32_t)(nes->cart.prg_size / 8192u);
    const uint32_t second_last = banks_8k - 2u;
    const uint32_t last = banks_8k - 1u;
    const uint32_t r6 = (uint32_t)(m->reg.mmc3.r[6] & 0x3fu);
    const uint32_t r7 = (uint32_t)(m->reg.mmc3.r[7] & 0x3fu);
    /* PRG mode (select bit 6): 0 puts R6 at $8000 and the second-last bank at
       $C000; 1 swaps them. R7 at $A000 and the last bank at $E000 never move. */
    if ((m->reg.mmc3.select & 0x40u) == 0u) {
        map_prg_window(nes, 0u, r6);
        map_prg_window(nes, 2u, second_last);
    } else {
        map_prg_window(nes, 0u, second_last);
        map_prg_window(nes, 2u, r6);
    }
    map_prg_window(nes, 1u, r7);
    map_prg_window(nes, 3u, last);
    /* CHR (select bit 7): the two 2 KiB banks R0, R1 sit in the lower half
       ($0000) and the four 1 KiB banks R2-R5 in the upper half; inversion swaps
       the halves. */
    {
        const uint32_t two_kib = (m->reg.mmc3.select & 0x80u) != 0u ? 4u : 0u;
        const uint32_t one_kib = (m->reg.mmc3.select & 0x80u) != 0u ? 0u : 4u;
        const uint32_t r0 = (uint32_t)(m->reg.mmc3.r[0] & 0xfeu);
        const uint32_t r1 = (uint32_t)(m->reg.mmc3.r[1] & 0xfeu);
        map_chr_1k(nes, two_kib + 0u, r0);
        map_chr_1k(nes, two_kib + 1u, r0 | 1u);
        map_chr_1k(nes, two_kib + 2u, r1);
        map_chr_1k(nes, two_kib + 3u, r1 | 1u);
        for (uint32_t i = 0u; i < 4u; ++i)
            map_chr_1k(nes, one_kib + i, m->reg.mmc3.r[2u + i]);
    }
    /* $A000 bit 0 clear is vertical mirroring: a vertical arrangement of
       screens sharing columns, so nametables 0,1,0,1 (NESdev Wiki "Mirroring").
       Set is horizontal mirroring, nametables 0,0,1,1. NESdev now words the
       board's two settings as "arrangement", so both terms are given. */
    if ((m->reg.mmc3.mirror & 1u) == 0u) {
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
    /* PRG RAM at $6000-$7FFF. $A001 bit 7 enables it and bit 6 write-protects
       it; the field holds the byte XOR $80 (D-08), so here bit 7 set means
       disabled and bit 6 set means protected. A disabled RAM is NULL pages in
       both directions (open bus reads, dropped writes) and a protected RAM is
       NULL write pages only, so the bus needs no branch (NESdev Wiki "MMC3",
       "PRG RAM protect"). Bits 5-0 are not read. */
    nesturbator__map_prg_ram_8k(nes, (m->reg.mmc3.a001_x & 0x80u) == 0u ? nes->cart.prg_ram : NULL);
    if ((m->reg.mmc3.a001_x & 0x40u) != 0u) {
        for (uint32_t i = 8u; i < 16u; ++i)
            map->cpu_w[i] = NULL;
    }
}

static void mmc3_cpu_write(struct nesturbator *nes, uint16_t addr, uint8_t value,
                           uint64_t cpu_cycle)
{
    struct nesturbator__mapper *m = &nes->mapper;
    (void)cpu_cycle;
    if (addr < 0x8000u)
        return;
    switch (addr & 0xe001u) {
    case 0x8000u:
        m->reg.mmc3.select = value;
        break;
    case 0x8001u:
        m->reg.mmc3.r[m->reg.mmc3.select & 7u] = value;
        break;
    case 0xa000u:
        m->reg.mmc3.mirror = value;
        break;
    case 0xa001u:
        /* Stored XOR $80 so a zeroed block is RAM enabled and writable (D-08); NESdev
           Wiki "MMC3" leaves the $A001 power-on state unspecified. */
        m->reg.mmc3.a001_x = (uint8_t)(value ^ 0x80u);
        break;
    default:
        return;
    }
    nes->map.ops.rebuild(nes);
}

static void mmc3_ppu_a12(struct nesturbator *nes, uint8_t level, uint64_t tick)
{
    (void)tick;
    nes->mapper.reg.mmc3.a12 = level;
}

void nesturbator__mapper_mmc3_ops(struct nesturbator__mapper_ops *out)
{
    out->init = mmc3_init;
    out->rebuild = mmc3_rebuild;
    out->cpu_write = mmc3_cpu_write;
    out->ppu_a12 = mmc3_ppu_a12;
    out->ppu_read = NULL;
}
