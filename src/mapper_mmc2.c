/* Mapper 9, MMC2, within the explicit project allowlist in D-01. The board has
   one switchable 8 KiB PRG bank, three fixed final banks, four 4 KiB CHR
   selectors, and switchable horizontal/vertical mirroring. The register and
   bank organization follows NESdev Wiki "MMC2", pinned as HWP.15 revision
   22483 (T2); the selected full-row latch profile remains secondary-reference
   evidence, not hardware verification. */
#include "internal.h"
#include "state.h"

#define MMC2_PRG_PAGES 8u
#define MMC2_CHR_PAGES 4u

static void mmc2_init(struct nesturbator *nes)
{
    nes->map.watch = NESTURBATOR_WATCH_CPU_WRITE | NESTURBATOR_WATCH_PPU_READ;
}

static void map_prg_window(struct nesturbator *nes, uint32_t window, uint32_t bank_8k)
{
    for (uint32_t i = 0u; i < MMC2_PRG_PAGES; ++i) {
        const uint32_t page = 16u + window * MMC2_PRG_PAGES + i;
        nes->map.cpu_r[page] = nesturbator__map_prg(nes, bank_8k * MMC2_PRG_PAGES + i);
        nes->map.cpu_w[page] = NULL;
    }
}

static void map_chr_window(struct nesturbator *nes, uint32_t half, uint8_t bank_4k)
{
    for (uint32_t i = 0u; i < MMC2_CHR_PAGES; ++i) {
        const uint32_t page = half * MMC2_CHR_PAGES + i;
        nes->map.chr_r[page] = nesturbator__map_chr(nes, (uint32_t)bank_4k * MMC2_CHR_PAGES + i);
        nes->map.chr_w[page] = NULL;
    }
}

static void mmc2_rebuild(struct nesturbator *nes)
{
    const struct nesturbator__mapper *m = &nes->mapper;
    struct nesturbator__map *map = &nes->map;

    map_prg_window(nes, 0u, (uint32_t)(m->reg.mmc2.prg & 0x0fu));
    map_prg_window(nes, 1u, 13u);
    map_prg_window(nes, 2u, 14u);
    map_prg_window(nes, 3u, 15u);

    /* Before Plan 02 wires actual PPU reads, each half starts on its FE selector.
       Latch 0 maps $0000-$0FFF; latch 1 maps $1000-$1FFF. */
    map_chr_window(nes, 0u, m->reg.mmc2.chr[m->reg.mmc2.latch[0] != 0u ? 1u : 0u]);
    map_chr_window(nes, 1u, m->reg.mmc2.chr[m->reg.mmc2.latch[1] != 0u ? 3u : 2u]);

    /* $F000 bit 0 selects vertical (0) or horizontal (1) mirroring [HWP.15]. */
    if ((m->reg.mmc2.mirror & 1u) == 0u) {
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

static void mmc2_cpu_write(struct nesturbator *nes, uint16_t addr, uint8_t value,
                           uint64_t cpu_cycle)
{
    struct nesturbator__mapper *m = &nes->mapper;
    (void)cpu_cycle;
    if (addr < 0xa000u)
        return;

    switch (addr & 0xf000u) {
    case 0xa000u:
        m->reg.mmc2.prg = (uint8_t)(value & 0x0fu);
        break;
    case 0xb000u:
        m->reg.mmc2.chr[0] = (uint8_t)(value & 0x1fu);
        break;
    case 0xc000u:
        m->reg.mmc2.chr[1] = (uint8_t)(value & 0x1fu);
        break;
    case 0xd000u:
        m->reg.mmc2.chr[2] = (uint8_t)(value & 0x1fu);
        break;
    case 0xe000u:
        m->reg.mmc2.chr[3] = (uint8_t)(value & 0x1fu);
        break;
    case 0xf000u:
        m->reg.mmc2.mirror = (uint8_t)(value & 1u);
        break;
    default:
        return;
    }
    nes->map.ops.rebuild(nes);
}

/* A pattern read first observes the bank selected before this PPU access.
   The full 16-byte trigger rows are the adopted MMC2 T2 reference profile
   (NESdev MMC2 revision 22483, HWP.15), not hardware-verification evidence. */
static uint8_t mmc2_ppu_read(struct nesturbator *nes, uint16_t addr)
{
    const uint8_t *page = nes->map.chr_r[addr >> 10];
    const uint8_t value = page != NULL ? page[addr & 0x03ffu] : 0u;
    struct nesturbator__mapper *m = &nes->mapper;
    uint8_t *latch = NULL;
    uint8_t position = 0u;

    if (addr >= 0x0fd0u && addr <= 0x0fdfu) {
        latch = &m->reg.mmc2.latch[0];
    } else if (addr >= 0x0fe0u && addr <= 0x0fefu) {
        latch = &m->reg.mmc2.latch[0];
        position = 1u;
    } else if (addr >= 0x1fd0u && addr <= 0x1fdfu) {
        latch = &m->reg.mmc2.latch[1];
    } else if (addr >= 0x1fe0u && addr <= 0x1fefu) {
        latch = &m->reg.mmc2.latch[1];
        position = 1u;
    }

    if (latch != NULL && *latch != position) {
        *latch = position;
        nes->map.ops.rebuild(nes);
    }
    return value;
}

static void mmc2_state(struct nesturbator__mapper *m, struct nesturbator__sio *io)
{
    nesturbator__sio_u8(io, &m->reg.mmc2.prg, 15u);
    for (unsigned i = 0u; i < 4u; ++i)
        nesturbator__sio_u8(io, &m->reg.mmc2.chr[i], 31u);
    nesturbator__sio_u8(io, &m->reg.mmc2.mirror, 1u);
    nesturbator__sio_u8(io, &m->reg.mmc2.latch[0], 1u);
    nesturbator__sio_u8(io, &m->reg.mmc2.latch[1], 1u);
}

void nesturbator__mapper_mmc2_ops(struct nesturbator__mapper_ops *out)
{
    out->init = mmc2_init;
    out->rebuild = mmc2_rebuild;
    out->cpu_write = mmc2_cpu_write;
    out->ppu_a12 = NULL;
    out->ppu_read = mmc2_ppu_read;
    out->state = mmc2_state;
}
