/* Mapper 1, MMC1 (MMC1B): a serial port at $8000-$FFFF feeding Control, CHR
   bank 0, CHR bank 1 and PRG bank registers, and up to 32 KiB of PRG RAM at
   $6000-$7FFF. Source: NESdev Wiki "MMC1" and "SxROM". MMC1 has no bus
   conflict, so only the write watch is requested.

   Serial port (NESdev Wiki "MMC1"): "When the serial port is written to on
   consecutive cycles, it ignores every write after the first." The wiki adds
   that "the bit 7 reset is never ignored". On a 6502 only a read-modify-write
   instruction puts two writes on consecutive cycles (DMA steals read cycles
   only), so the board needs no opcode or CPU state: the write stamp from the
   hook is all it reads. The ignore applies even when the first write was not
   to the serial port, so a $6000 RAM write counts.

   Power-on is Control $0C (PRG mode 3, the last 16 KiB bank fixed at $C000,
   one-screen lower mirroring, 8 KiB CHR mode) with the PRG bank register 0 and
   RAM enabled. The registers are stored so that a zeroed block is that state
   (see mapper.h), which keeps the seam rule that the loader's memset is the
   power-on state and init writes no register. nesturbator_reset does not
   touch them: the cartridge sees no reset line.

   Variants come from the header sizes only, and the outer bits come from CHR
   bank register 0 (D-10). Source: NESdev Wiki "MMC1" and "SxROM".
   - SNROM (CHR-RAM, PRG <= 256 KiB, RAM <= 8 KiB): CHR0 bit 4 disables the
     PRG RAM.
   - SOROM (two 8 KiB RAM chips): CHR0 bit 3 picks the chip. The wiki says of
     it that "SOROM implements only this bit", so bit 2 is not read; bank 0 is
     the first chip, which does not retain data (the work half) and bank 1 the
     second, the battery half.
   - SUROM (512 KiB PRG): CHR0 bit 4 picks the 256 KiB half of PRG, for both
     windows including the normally fixed bank.
   - SXROM (32 KiB RAM, 512 KiB PRG): CHR0 bits 3-2 drive RAM A14 and A13, so
     they pick one of four 8 KiB banks; bit 4 is the PRG half as for SUROM.
   On every board the PRG register's bit 4 disables the RAM (MMC1B: "PRG-RAM
   is enabled by default but can by disabled by bit 4 of $E000"). */
#include "internal.h"

#define MMC1_PRG_256K (256u * 1024u)

static void mmc1_init(struct nesturbator *nes)
{
    nes->map.watch = NESTURBATOR_WATCH_CPU_WRITE;
}

static void mmc1_rebuild(struct nesturbator *nes)
{
    struct nesturbator__map *map = &nes->map;
    /* Control is stored XOR $0C (D-16), so zero is the power-on Control $0C. */
    const uint32_t control = (uint32_t)(nes->mapper.reg.mmc1.control_x ^ 0x0cu);
    const uint32_t p = (uint32_t)(nes->mapper.reg.mmc1.prg & 0x0fu);
    /* CHR register 0 bit 4 picks the 256 KiB PRG half on 512 KiB boards, and
       the half applies to the "fixed" bank too (NESdev Wiki "MMC1", SUROM). */
    const uint32_t outer =
        nes->cart.prg_size > MMC1_PRG_256K ? (uint32_t)((nes->mapper.reg.mmc1.chr0 >> 4) & 1u) : 0u;
    uint32_t lo, hi;
    switch ((control >> 2) & 3u) {
    case 0u:
    case 1u: /* one 32 KiB bank; the low bit of the register is ignored */
        lo = (outer << 4) | (p & 0x0eu);
        hi = lo + 1u;
        break;
    case 2u: /* first bank fixed at $8000, switchable at $C000 */
        lo = outer << 4;
        hi = (outer << 4) | p;
        break;
    default: /* switchable at $8000, last bank fixed at $C000 */
        lo = (outer << 4) | p;
        hi = (outer << 4) | 0x0fu;
        break;
    }
    /* nesturbator__map_prg takes a true modulo, so 0x0F is the last bank. */
    for (uint32_t i = 0u; i < 16u; ++i) {
        map->cpu_r[16u + i] = nesturbator__map_prg(nes, lo * 16u + i);
        map->cpu_w[16u + i] = NULL;
        map->cpu_r[32u + i] = nesturbator__map_prg(nes, hi * 16u + i);
        map->cpu_w[32u + i] = NULL;
    }
    /* CHR: bit 4 of Control clear is one 8 KiB bank (CHR0 with its low bit
       ignored), set is two independent 4 KiB banks. */
    if ((control & 0x10u) == 0u) {
        const uint32_t pair = (uint32_t)(nes->mapper.reg.mmc1.chr0 & 0x1eu);
        nesturbator__map_chr_4k(nes, 0u, pair);
        nesturbator__map_chr_4k(nes, 1u, pair | 1u);
    } else {
        nesturbator__map_chr_4k(nes, 0u, nes->mapper.reg.mmc1.chr0);
        nesturbator__map_chr_4k(nes, 1u, nes->mapper.reg.mmc1.chr1);
    }
    /* Mirroring: 0 one-screen lower, 1 one-screen upper, 2 vertical, 3
       horizontal. The header's mirroring bit is not used. */
    switch (control & 3u) {
    case 0u:
        map->nt[0] = 0u;
        map->nt[1] = 0u;
        map->nt[2] = 0u;
        map->nt[3] = 0u;
        break;
    case 1u:
        map->nt[0] = 1u;
        map->nt[1] = 1u;
        map->nt[2] = 1u;
        map->nt[3] = 1u;
        break;
    case 2u:
        map->nt[0] = 0u;
        map->nt[1] = 1u;
        map->nt[2] = 0u;
        map->nt[3] = 1u;
        break;
    default:
        map->nt[0] = 0u;
        map->nt[1] = 0u;
        map->nt[2] = 1u;
        map->nt[3] = 1u;
        break;
    }
    /* PRG RAM: a NULL page reads as open bus and drops writes, so a disabled
       RAM needs no branch in the bus. */
    {
        const uint32_t chr0 = nes->mapper.reg.mmc1.chr0;
        const size_t ram_total = nes->cart.prg_ram_size;
        const int snrom_off = nes->cart.chr_is_ram != 0u && nes->cart.prg_size <= MMC1_PRG_256K &&
                              ram_total <= 8192u && (chr0 & 0x10u) != 0u;
        uint8_t *ram = NULL;
        if (nes->cart.prg_ram != NULL && (nes->mapper.reg.mmc1.prg & 0x10u) == 0u && !snrom_off) {
            /* The bank is chosen by the allocation size, not by a fixed bit
               pair: SOROM has two chips and only bit 3, SXROM has four. */
            uint32_t bank = 0u;
            if (ram_total > 16384u)
                bank = (chr0 >> 2) & 3u;
            else if (ram_total > 8192u)
                bank = (chr0 >> 3) & 1u;
            ram = nes->cart.prg_ram + (size_t)bank * 8192u;
        }
        nesturbator__map_prg_ram_8k(nes, ram);
    }
}

static void mmc1_cpu_write(struct nesturbator *nes, uint16_t addr, uint8_t value,
                           uint64_t cpu_cycle)
{
    struct nesturbator__mapper *m = &nes->mapper;
    /* D-15 step 1: a write on the cycle right after another hook write is
       ignored by the real chip. last_write holds the previous stamp plus one,
       so zero means never written: without that guard a first write at stamp 1
       would look adjacent to the zeroed field. The update happens on every
       call, including RAM writes and ignored writes. */
    const int adjacent = m->reg.mmc1.last_write != 0u && cpu_cycle == m->reg.mmc1.last_write;
    m->reg.mmc1.last_write = cpu_cycle + 1u;
    if (addr < 0x8000u)
        return;
    if ((value & 0x80u) != 0u) {
        /* Reset: clear the shift register and set Control |= $0C. Mirroring,
           CHR mode and the other registers stay. Never ignored. */
        m->reg.mmc1.shift = 0u;
        m->reg.mmc1.count = 0u;
        m->reg.mmc1.control_x = (uint8_t)(m->reg.mmc1.control_x & ~0x0cu);
        nes->map.ops.rebuild(nes);
        return;
    }
    if (adjacent)
        return;
    /* Bits arrive LSB first: each write enters at bit 4 and the register
       shifts right. */
    m->reg.mmc1.shift = (uint8_t)((m->reg.mmc1.shift >> 1) | ((value & 1u) << 4));
    m->reg.mmc1.count++;
    if (m->reg.mmc1.count < 5u)
        return;
    /* The fifth write's address bits 14-13 pick the register. */
    switch ((addr >> 13) & 3u) {
    case 0u:
        m->reg.mmc1.control_x = (uint8_t)(m->reg.mmc1.shift ^ 0x0cu);
        break;
    case 1u:
        m->reg.mmc1.chr0 = m->reg.mmc1.shift;
        break;
    case 2u:
        m->reg.mmc1.chr1 = m->reg.mmc1.shift;
        break;
    default:
        m->reg.mmc1.prg = m->reg.mmc1.shift;
        break;
    }
    m->reg.mmc1.shift = 0u;
    m->reg.mmc1.count = 0u;
    nes->map.ops.rebuild(nes);
}

void nesturbator__mapper_mmc1_ops(struct nesturbator__mapper_ops *out)
{
    out->init = mmc1_init;
    out->rebuild = mmc1_rebuild;
    out->cpu_write = mmc1_cpu_write;
    out->ppu_a12 = NULL;
    out->ppu_read = NULL;
}
