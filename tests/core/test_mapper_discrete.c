/* BOARD-01: the discrete-logic boards on synthetic images. One function per behaviour. Each 16 KiB
   PRG bank b carries the marker 0x10 + b at bank offset 0x0010 and an AND seed at offset 0x0000, so
   a write to $8000 ANDs with the seed and a read of $8010 names the bank. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"

#define IMAGE_CAP (16u + INES_TRAINER_SIZE + 16u * INES_PRG_BANK + 4u * INES_CHR_BANK)
#define RESET_VECTOR 0xc123u

/* Builds and loads a board image; seeds[b] is bank b's byte at offset 0 (NULL means zeros). */
static struct nesturbator *load_board_ex(uint16_t mapper, uint8_t submapper, int nes2,
                                         uint8_t prg_16k, uint8_t chr_8k, int vertical, int trainer,
                                         const uint8_t *seeds, uint16_t bank0_vector)
{
    static uint8_t image[IMAGE_CAP];
    static const uint8_t trainer_bytes[INES_TRAINER_SIZE] = {0x6du};
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    struct ines_spec spec;
    size_t size, base;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = prg_16k;
    spec.chr_8k = chr_8k;
    spec.mirroring_vertical = (uint8_t)(vertical != 0);
    spec.nes2 = (uint8_t)(nes2 != 0);
    spec.mapper = mapper;
    spec.submapper = submapper;
    spec.trainer = trainer != 0 ? trainer_bytes : NULL;
    spec.reset_vector = RESET_VECTOR;
    size = ines_build(image, sizeof image, &spec);
    CHECK(size != 0u);
    base = 16u + (trainer != 0 ? INES_TRAINER_SIZE : 0u);
    for (uint8_t b = 0u; b < prg_16k; ++b) {
        image[base + (size_t)b * INES_PRG_BANK + 0u] = seeds != NULL ? seeds[b] : 0u;
        image[base + (size_t)b * INES_PRG_BANK + 0x10u] = (uint8_t)(0x10u + b);
    }
    /* CHR ROM bank k starts with the marker 0x20 + k. */
    for (uint8_t k = 0u; k < chr_8k; ++k)
        image[base + (size_t)prg_16k * INES_PRG_BANK + (size_t)k * INES_CHR_BANK] =
            (uint8_t)(0x20u + k);
    /* A reset vector at the end of the first 32 KiB, where AxROM bank 0 keeps it. */
    if (bank0_vector != 0u) {
        image[base + 0x7ffcu] = (uint8_t)(bank0_vector & 0xffu);
        image[base + 0x7ffdu] = (uint8_t)(bank0_vector >> 8);
    }
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    return (struct nesturbator *)inst;
}

static struct nesturbator *load_board(uint16_t mapper, uint8_t submapper, int nes2, uint8_t prg_16k,
                                      uint8_t chr_8k, int vertical, int trainer,
                                      const uint8_t *seeds)
{
    return load_board_ex(mapper, submapper, nes2, prg_16k, chr_8k, vertical, trainer, seeds, 0u);
}

static void release(struct nesturbator *nes)
{
    nesturbator_destroy((nesturbator *)nes);
}

static uint8_t peek(struct nesturbator *nes, uint16_t addr)
{
    return nesturbator__map_cpu_read(nes, addr);
}

/* One write path to the PPU: the CPU bus, $2006 high, $2006 low, then $2007. */
static void ppu_poke(struct nesturbator *nes, uint16_t addr, uint8_t value)
{
    nesturbator__bus_write(nes, 0x2006u, (uint8_t)(addr >> 8));
    nesturbator__bus_write(nes, 0x2006u, (uint8_t)(addr & 0xffu));
    nesturbator__bus_write(nes, 0x2007u, value);
}

static void test_uxrom_power_on(void)
{
    struct nesturbator *nes = load_board(2u, 0u, 0, 8u, 0u, 0, 0, NULL);
    CHECK_EQ_U64(nes->mapper.reg.uxrom.bank, 0u);
    CHECK_EQ_HEX(peek(nes, 0x8010u), 0x10u);
    CHECK_EQ_HEX(peek(nes, 0xc010u), 0x17u);
    CHECK_EQ_HEX(nes->cpu.pc, RESET_VECTOR);
    release(nes);
}

static void test_uxrom_and_submapper0(void)
{
    static const uint8_t seeds[8] = {0x03u};
    struct nesturbator *nes = load_board(2u, 0u, 0, 8u, 0u, 0, 0, seeds);
    nesturbator__bus_write(nes, 0x8000u, 0x07u);
    CHECK_EQ_HEX(peek(nes, 0x8010u), 0x13u);
    release(nes);
    nes = load_board(2u, 0u, 1, 8u, 0u, 0, 0, seeds);
    nesturbator__bus_write(nes, 0x8000u, 0x07u);
    CHECK_EQ_HEX(peek(nes, 0x8010u), 0x13u);
    release(nes);
}

static void test_uxrom_and_submapper2(void)
{
    static const uint8_t seeds[8] = {0x03u};
    struct nesturbator *nes = load_board(2u, 2u, 1, 8u, 0u, 0, 0, seeds);
    nesturbator__bus_write(nes, 0x8000u, 0x07u);
    CHECK_EQ_HEX(peek(nes, 0x8010u), 0x13u);
    release(nes);
}

static void test_uxrom_no_and_submapper1(void)
{
    static const uint8_t seeds[8] = {0x03u};
    struct nesturbator *nes = load_board(2u, 1u, 1, 8u, 0u, 0, 0, seeds);
    nesturbator__bus_write(nes, 0x8000u, 0x07u);
    CHECK_EQ_HEX(peek(nes, 0x8010u), 0x17u);
    release(nes);
}

static void test_uxrom_and_against_pre_write_bank(void)
{
    static const uint8_t seeds[8] = {0x03u, 0x00u, 0x00u, 0x05u};
    struct nesturbator *nes = load_board(2u, 0u, 0, 8u, 0u, 0, 0, seeds);
    nesturbator__bus_write(nes, 0x8000u, 0x07u);
    CHECK_EQ_HEX(peek(nes, 0x8010u), 0x13u);
    nesturbator__bus_write(nes, 0x8000u, 0x07u);
    CHECK_EQ_HEX(peek(nes, 0x8010u), 0x15u);
    release(nes);
}

static void test_uxrom_fixed_last_bank_and_wrap(void)
{
    struct nesturbator *nes = load_board(2u, 1u, 1, 8u, 0u, 0, 0, NULL);
    nesturbator__bus_write(nes, 0x8000u, 0x09u);
    CHECK_EQ_HEX(peek(nes, 0x8010u), 0x11u);
    CHECK_EQ_HEX(peek(nes, 0xc010u), 0x17u);
    release(nes);
    nes = load_board(2u, 1u, 1, 16u, 0u, 0, 0, NULL);
    nesturbator__bus_write(nes, 0x8000u, 0x11u);
    CHECK_EQ_HEX(peek(nes, 0x8010u), 0x11u);
    CHECK_EQ_HEX(peek(nes, 0xc010u), 0x1fu);
    release(nes);
}

static void test_uxrom_write_boundary(void)
{
    struct nesturbator *nes = load_board(2u, 1u, 1, 8u, 0u, 0, 0, NULL);
    nesturbator__bus_write(nes, 0x7fffu, 0x02u);
    CHECK_EQ_U64(nes->mapper.reg.uxrom.bank, 0u);
    nesturbator__bus_write(nes, 0xffffu, 0x02u);
    CHECK_EQ_HEX(peek(nes, 0x8010u), 0x12u);
    release(nes);
}

static void test_uxrom_chr_ram_write_sticks(void)
{
    struct nesturbator *nes = load_board(2u, 0u, 0, 2u, 0u, 0, 0, NULL);
    CHECK_EQ_U64(nes->ppu.reset_flag, 0u);
    ppu_poke(nes, 0x0000u, 0x5au);
    CHECK_EQ_HEX(nes->cart.chr[0], 0x5au);
    CHECK(nes->map.chr_w[0] != NULL);
    release(nes);
}

static void test_uxrom_trainer_visible(void)
{
    struct nesturbator *nes = load_board(2u, 0u, 0, 2u, 1u, 0, 1, NULL);
    CHECK_EQ_HEX(peek(nes, 0x7000u), 0x6du);
    release(nes);
}

static void test_uxrom_mirroring_follows_header(void)
{
    static const uint8_t vertical[4] = {0u, 1u, 0u, 1u};
    static const uint8_t horizontal[4] = {0u, 0u, 1u, 1u};
    struct nesturbator *nes = load_board(2u, 0u, 0, 2u, 0u, 1, 0, NULL);
    CHECK(memcmp(nes->map.nt, vertical, 4u) == 0);
    release(nes);
    nes = load_board(2u, 0u, 0, 2u, 0u, 0, 0, NULL);
    CHECK(memcmp(nes->map.nt, horizontal, 4u) == 0);
    release(nes);
}

/* CNROM: PRG at $8000 holds the AND seed 0x01; CHR bank k starts with 0x20 + k. */
static uint8_t chr_marker(struct nesturbator *nes)
{
    return nes->map.chr_r[0][0];
}

static void test_cnrom_and_submapper0_and_2(void)
{
    static const uint8_t seeds[2] = {0x01u};
    for (uint8_t sub = 0u; sub <= 2u; sub = (uint8_t)(sub + 2u)) {
        struct nesturbator *nes = load_board(3u, sub, 1, 2u, 4u, 0, 0, seeds);
        nesturbator__bus_write(nes, 0x8000u, 0x03u);
        CHECK_EQ_U64(nes->mapper.reg.cnrom.bank, 0x03u & 0x01u);
        CHECK(nes->map.chr_r[0] == nes->cart.chr + 8192u);
        CHECK_EQ_HEX(chr_marker(nes), 0x21u);
        release(nes);
    }
}

static void test_cnrom_no_and_submapper1(void)
{
    static const uint8_t seeds[2] = {0x01u};
    struct nesturbator *nes = load_board(3u, 1u, 1, 2u, 4u, 0, 0, seeds);
    nesturbator__bus_write(nes, 0x8000u, 0x03u);
    CHECK(nes->map.chr_r[0] == nes->cart.chr + 3u * 8192u);
    CHECK_EQ_HEX(chr_marker(nes), 0x23u);
    release(nes);
}

static void test_cnrom_chr_size_and_wrap(void)
{
    struct nesturbator *nes = load_board(3u, 1u, 1, 2u, 4u, 0, 0, NULL);
    for (uint8_t v = 0u; v < 4u; ++v) {
        nesturbator__bus_write(nes, 0x8000u, v);
        CHECK_EQ_HEX(chr_marker(nes), 0x20u + v);
    }
    nesturbator__bus_write(nes, 0x8000u, 4u);
    CHECK_EQ_HEX(chr_marker(nes), 0x20u);
    release(nes);
    nes = load_board(3u, 1u, 1, 2u, 2u, 0, 0, NULL);
    nesturbator__bus_write(nes, 0x8000u, 2u);
    CHECK_EQ_HEX(chr_marker(nes), 0x20u);
    nesturbator__bus_write(nes, 0x8000u, 0x30u);
    CHECK_EQ_HEX(chr_marker(nes), 0x20u);
    release(nes);
}

static void test_cnrom_write_boundary(void)
{
    struct nesturbator *nes = load_board(3u, 1u, 1, 2u, 2u, 0, 0, NULL);
    nesturbator__bus_write(nes, 0x7fffu, 0x01u);
    CHECK_EQ_U64(nes->mapper.reg.cnrom.bank, 0u);
    nesturbator__bus_write(nes, 0xffffu, 0x01u);
    CHECK_EQ_HEX(chr_marker(nes), 0x21u);
    release(nes);
}

/* The control proves the $2006/$2007 write path reaches pattern memory when a writable page
   exists; the CNROM case then proves the drop with the identical sequence. */
static void test_chr_ram_write_lands_control(void)
{
    struct nesturbator *nes = load_board(2u, 0u, 0, 2u, 0u, 0, 0, NULL);
    CHECK_EQ_U64(nes->ppu.reset_flag, 0u);
    CHECK_EQ_HEX(nes->cart.chr[0], 0x00u);
    ppu_poke(nes, 0x0000u, 0xa5u);
    CHECK_EQ_HEX(nes->cart.chr[0], 0xa5u);
    release(nes);
}

static void test_cnrom_chr_rom_write_dropped(void)
{
    struct nesturbator *nes = load_board(3u, 1u, 1, 2u, 1u, 0, 0, NULL);
    CHECK_EQ_U64(nes->ppu.reset_flag, 0u);
    CHECK(nes->map.chr_w[0] == NULL);
    ppu_poke(nes, 0x0000u, 0xa5u);
    CHECK_EQ_HEX(nes->cart.chr[0], 0x20u);
    CHECK_EQ_HEX(nes->map.chr_r[0][0], 0x20u);
    release(nes);
}

static void test_cnrom_prg_as_nrom(void)
{
    struct nesturbator *nes = load_board(3u, 1u, 1, 1u, 1u, 0, 0, NULL);
    CHECK_EQ_HEX(peek(nes, 0x8010u), 0x10u);
    CHECK_EQ_HEX(peek(nes, 0xc010u), 0x10u);
    release(nes);
    nes = load_board(3u, 1u, 1, 2u, 1u, 0, 0, NULL);
    CHECK_EQ_HEX(peek(nes, 0x8010u), 0x10u);
    CHECK_EQ_HEX(peek(nes, 0xc010u), 0x11u);
    release(nes);
}

static void test_cnrom_power_on(void)
{
    struct nesturbator *nes = load_board(3u, 0u, 1, 2u, 2u, 0, 0, NULL);
    CHECK_EQ_U64(nes->mapper.reg.cnrom.bank, 0u);
    CHECK(nes->map.chr_r[0] == nes->cart.chr);
    release(nes);
}

/* AxROM: 32 KiB bank b begins at 16 KiB bank 2b, so $8010 reads 0x10 + 2b. */
static void test_axrom_no_and_submapper0_and_1(void)
{
    static const uint8_t seeds[16] = {0x03u};
    for (uint8_t sub = 0u; sub <= 1u; ++sub) {
        struct nesturbator *nes = load_board(7u, sub, 1, 16u, 0u, 0, 0, seeds);
        static const uint8_t ones[4] = {1u, 1u, 1u, 1u};
        nesturbator__bus_write(nes, 0x8000u, 0x17u);
        CHECK_EQ_HEX(peek(nes, 0x8010u), 0x1eu);
        CHECK(memcmp(nes->map.nt, ones, 4u) == 0);
        release(nes);
    }
}

static void test_axrom_and_submapper2(void)
{
    static const uint8_t seeds[16] = {0x03u};
    static const uint8_t zeros[4] = {0u, 0u, 0u, 0u};
    struct nesturbator *nes = load_board(7u, 2u, 1, 16u, 0u, 0, 0, seeds);
    nesturbator__bus_write(nes, 0x8000u, 0x17u);
    CHECK_EQ_HEX(peek(nes, 0x8010u), 0x16u);
    CHECK(memcmp(nes->map.nt, zeros, 4u) == 0);
    release(nes);
}

static void test_axrom_single_screen_ciram(void)
{
    static const uint8_t zeros[4] = {0u, 0u, 0u, 0u};
    static const uint8_t ones[4] = {1u, 1u, 1u, 1u};
    struct nesturbator *nes = load_board(7u, 1u, 1, 4u, 0u, 0, 0, NULL);
    /* (a) the write lands: the control for the page-selection steps below */
    CHECK_EQ_U64(nes->ppu.reset_flag, 0u);
    CHECK(memcmp(nes->map.nt, zeros, 4u) == 0);
    ppu_poke(nes, 0x2000u, 0x5au);
    CHECK_EQ_HEX(nes->ppu.nametable[0x000], 0x5au);
    /* (b) page selection */
    nesturbator__bus_write(nes, 0x8000u, 0x10u);
    CHECK(memcmp(nes->map.nt, ones, 4u) == 0);
    ppu_poke(nes, 0x2000u, 0xc3u);
    CHECK_EQ_HEX(nes->ppu.nametable[0x400], 0xc3u);
    CHECK_EQ_HEX(nes->ppu.nametable[0x000], 0x5au);
    nesturbator__bus_write(nes, 0x2006u, 0x2cu);
    nesturbator__bus_write(nes, 0x2006u, 0x00u);
    (void)nesturbator__bus_read(nes, 0x2007u);
    CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x2007u), 0xc3u);
    /* (c) back to page 0 */
    nesturbator__bus_write(nes, 0x8000u, 0x00u);
    CHECK(memcmp(nes->map.nt, zeros, 4u) == 0);
    nesturbator__bus_write(nes, 0x2006u, 0x24u);
    nesturbator__bus_write(nes, 0x2006u, 0x00u);
    (void)nesturbator__bus_read(nes, 0x2007u);
    CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x2007u), 0x5au);
    release(nes);
}

static void test_axrom_ignores_header_mirroring(void)
{
    static const uint8_t zeros[4] = {0u, 0u, 0u, 0u};
    struct nesturbator *nes = load_board(7u, 1u, 1, 4u, 0u, 1, 0, NULL);
    CHECK(memcmp(nes->map.nt, zeros, 4u) == 0);
    release(nes);
}

static void test_axrom_prg_wrap(void)
{
    struct nesturbator *nes = load_board(7u, 1u, 1, 4u, 0u, 0, 0, NULL);
    nesturbator__bus_write(nes, 0x8000u, 0x03u);
    CHECK_EQ_HEX(peek(nes, 0x8010u), 0x12u);
    release(nes);
}

static void test_board_power_on_registers(void)
{
    struct nesturbator *nes = load_board(2u, 0u, 0, 2u, 0u, 0, 0, NULL);
    CHECK_EQ_U64(nes->mapper.reg.uxrom.bank, 0u);
    release(nes);
    nes = load_board(3u, 0u, 1, 2u, 2u, 0, 0, NULL);
    CHECK_EQ_U64(nes->mapper.reg.cnrom.bank, 0u);
    release(nes);
    nes = load_board(7u, 0u, 1, 4u, 0u, 0, 0, NULL);
    CHECK_EQ_U64(nes->mapper.reg.axrom.bank, 0u);
    release(nes);
}

static void test_axrom_power_on_pc(void)
{
    struct nesturbator *nes = load_board_ex(7u, 0u, 1, 8u, 0u, 0, 0, NULL, 0x8123u);
    CHECK_EQ_HEX(nes->cpu.pc, 0x8123u);
    release(nes);
}

int main(void)
{
    test_uxrom_power_on();
    test_uxrom_and_submapper0();
    test_uxrom_and_submapper2();
    test_uxrom_no_and_submapper1();
    test_uxrom_and_against_pre_write_bank();
    test_uxrom_fixed_last_bank_and_wrap();
    test_uxrom_write_boundary();
    test_uxrom_chr_ram_write_sticks();
    test_uxrom_trainer_visible();
    test_uxrom_mirroring_follows_header();
    test_cnrom_and_submapper0_and_2();
    test_cnrom_no_and_submapper1();
    test_cnrom_chr_size_and_wrap();
    test_cnrom_write_boundary();
    test_chr_ram_write_lands_control();
    test_cnrom_chr_rom_write_dropped();
    test_cnrom_prg_as_nrom();
    test_cnrom_power_on();
    test_axrom_no_and_submapper0_and_1();
    test_axrom_and_submapper2();
    test_axrom_single_screen_ciram();
    test_axrom_ignores_header_mirroring();
    test_axrom_prg_wrap();
    test_board_power_on_registers();
    test_axrom_power_on_pc();
    CHECK_DONE();
}
