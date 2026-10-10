/* Per-board image validation through the public API (D-01, D-09, D-10, D-13). */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"

typedef struct counts {
    unsigned allocs, frees;
    int fail_alloc;
} counts;
static void *count_alloc(void *user, size_t size)
{
    counts *c = (counts *)user;
    c->allocs++;
    if (c->fail_alloc) {
        c->fail_alloc = 0;
        return NULL;
    }
    return malloc(size);
}
static void count_free(void *user, void *ptr, size_t size)
{
    (void)size;
    ((counts *)user)->frees++;
    free(ptr);
}
static void make_image(uint8_t *p, size_t size, int nes2, int chr_ram)
{
    memset(p, 0, size);
    memcpy(p, "NES\032", 4);
    p[4] = 1u;
    p[5] = (uint8_t)(chr_ram ? 0u : 1u);
    if (nes2) {
        p[7] = 8u;
        if (chr_ram)
            p[11] = 7u;
    }
    p[16u + 0x3ffcu] = 0u;
    p[16u + 0x3ffdu] = 0x80u;
}
static void make_trainer_image(uint8_t *p, size_t prg_size, int nes2)
{
    const size_t prg_offset = 16u + 512u;
    const size_t size = prg_offset + prg_size + 8192u;
    memset(p, 0, size);
    memcpy(p, "NES\032", 4u);
    p[4] = (uint8_t)(prg_size / 16384u);
    p[5] = 1u;
    p[6] = 4u;
    if (nes2)
        p[7] = 8u;
    for (size_t i = 0u; i < 512u; ++i)
        p[16u + i] = (uint8_t)(i * 73u + 0x2du);
    p[prg_offset + 0x1234u] = 0x5au;
    p[prg_offset + 0x3ffcu] = 0x34u;
    p[prg_offset + 0x3ffdu] = 0x81u;
    if (prg_size == 32768u) {
        p[prg_offset + 0x0123u] = 0x39u;
        p[prg_offset + 0x4123u] = 0x6cu;
        p[prg_offset + 0x7ffcu] = 0x78u;
        p[prg_offset + 0x7ffdu] = 0x82u;
    }
}
static void test_formats_and_lifetime(void)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    counts c = {0, 0, 0};
    uint8_t *rom = malloc(16u + 16384u + 8192u);
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    cfg.allocator.alloc = count_alloc;
    cfg.allocator.free = count_free;
    cfg.allocator.user = &c;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    unsigned base = c.allocs;
    make_image(rom, 16u + 16384u + 8192u, 0, 0);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, rom, 16u + 16384u + 8192u), NESTURBATOR_OK);
    CHECK_EQ_U64(c.allocs, base + 1u);
    uint8_t bad[16] = {0};
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, bad, sizeof bad), NESTURBATOR_ERR_CARTRIDGE);
    CHECK_EQ_U64(c.allocs, base + 1u);
    make_image(rom, 16u + 16384u + 8192u, 1, 0);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, rom, 16u + 16384u + 8192u), NESTURBATOR_OK);
    make_image(rom, 16u + 16384u, 1, 1);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, rom, 16u + 16384u), NESTURBATOR_OK);
    uint8_t *large = malloc(16u + 32768u + 8192u + 512u);
    make_image(large, 16u + 32768u + 8192u + 512u, 1, 0);
    large[4] = 2u;
    large[6] = 4u;
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, large, 16u + 32768u + 8192u + 512u),
                 NESTURBATOR_OK);
    nesturbator_unload_cartridge(inst);
    CHECK_EQ_U64(c.allocs, c.frees + 1u);
    nesturbator_destroy(inst);
    CHECK_EQ_U64(c.allocs, c.frees);
    free(rom);
    free(large);
}
static void test_reject_before_allocation(void)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    counts c = {0, 0, 0};
    uint8_t *rom = malloc(16u + 512u + 16384u + 8192u + 1u);
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    cfg.allocator.alloc = count_alloc;
    cfg.allocator.free = count_free;
    cfg.allocator.user = &c;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    const size_t trainer_size = 16u + 512u + 16384u + 8192u;
    make_trainer_image(rom, 16384u, 0);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, rom, trainer_size), NESTURBATOR_OK);
    unsigned base = c.allocs;
    size_t full = 16u + 16384u + 8192u;
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, rom, 0u), NESTURBATOR_ERR_CARTRIDGE);
    CHECK_EQ_U64(c.allocs, base);
    CHECK_EQ_HEX(nesturbator__bus_read((struct nesturbator *)inst, 0x7000u), 0x2du);
    make_trainer_image(rom, 16384u, 0);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, rom, trainer_size - 1u),
                 NESTURBATOR_ERR_CARTRIDGE);
    CHECK_EQ_U64(c.allocs, base);
    CHECK_EQ_HEX(nesturbator__bus_read((struct nesturbator *)inst, 0x7000u), 0x2du);
    make_image(rom, full + 1u, 1, 0);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, rom, full + 1u), NESTURBATOR_ERR_CARTRIDGE);
    CHECK_EQ_U64(c.allocs, base);
    CHECK_EQ_HEX(nesturbator__bus_read((struct nesturbator *)inst, 0x7000u), 0x2du);
    make_image(rom, full, 1, 0);
    rom[9] = 0x0fu;
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, rom, full), NESTURBATOR_ERR_CARTRIDGE);
    CHECK_EQ_U64(c.allocs, base);
    CHECK_EQ_HEX(nesturbator__bus_read((struct nesturbator *)inst, 0x7000u), 0x2du);
    make_image(rom, full, 1, 0);
    rom[7] |= 0x10u;
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, rom, full), NESTURBATOR_ERR_CARTRIDGE);
    CHECK_EQ_U64(c.allocs, base);
    CHECK_EQ_HEX(nesturbator__bus_read((struct nesturbator *)inst, 0x7000u), 0x2du);
    make_trainer_image(rom, 16384u, 0);
    c.fail_alloc = 1;
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, rom, trainer_size), NESTURBATOR_ERR_NO_MEMORY);
    CHECK_EQ_U64(c.allocs, base + 1u);
    CHECK_EQ_HEX(nesturbator__bus_read((struct nesturbator *)inst, 0x7000u), 0x2du);
    nesturbator_destroy(inst);
    free(rom);
}
static void test_trainerless_ram_stays_unmapped(void)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    uint8_t rom[16u + 16384u + 8192u];
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    make_image(rom, sizeof rom, 0, 0);
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, rom, sizeof rom), NESTURBATOR_OK);
    if (inst != NULL) {
        struct nesturbator *nes = (struct nesturbator *)inst;
        nes->bus.open_bus = 0xa6u;
        CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x6000u), 0xa6u);
        nesturbator__bus_write(nes, 0x6000u, 0x5cu);
        CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x6000u), 0x5cu);
        nesturbator_destroy(inst);
    }
}
static void test_32k_reset_vector_comes_from_upper_prg_bank(void)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    uint8_t *rom = calloc(1u, 16u + 32768u + 8192u);
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK(rom != NULL);
    if (rom == NULL)
        return;
    memcpy(rom, "NES\032", 4u);
    rom[4] = 2u;
    rom[5] = 1u;
    rom[16u + 32768u - 4u] = 0x23u;
    rom[16u + 32768u - 3u] = 0x81u;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, rom, 16u + 32768u + 8192u), NESTURBATOR_OK);
    if (inst != NULL) {
        CHECK_EQ_HEX(((struct nesturbator *)inst)->cpu.pc, 0x8123u);
        nesturbator_destroy(inst);
    }
    free(rom);
}
static void test_trainer_is_visible_through_cpu_bus(void)
{
    for (int nes2 = 0; nes2 <= 1; ++nes2) {
        const size_t prg_size = nes2 ? 32768u : 16384u;
        const size_t size = 16u + 512u + prg_size + 8192u;
        uint8_t *rom = malloc(size);
        nesturbator_config cfg;
        nesturbator *first = NULL, *second = NULL;
        CHECK(rom != NULL);
        if (rom == NULL)
            continue;
        make_trainer_image(rom, prg_size, nes2);
        memset(&cfg, 0, sizeof cfg);
        cfg.size = (uint32_t)sizeof cfg;
        cfg.abi = NESTURBATOR_ABI_VERSION;
        CHECK_EQ_U64(nesturbator_create(&cfg, &first), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_create(&cfg, &second), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_load_cartridge(first, rom, size), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_load_cartridge(second, rom, size), NESTURBATOR_OK);
        if (first != NULL && second != NULL) {
            struct nesturbator *a = (struct nesturbator *)first;
            struct nesturbator *b = (struct nesturbator *)second;
            for (size_t i = 0u; i < 512u; ++i)
                CHECK_EQ_HEX(nesturbator__bus_read(a, (uint16_t)(0x7000u + i)), rom[16u + i]);
            CHECK_EQ_HEX(a->cpu.pc, nes2 ? 0x8278u : 0x8134u);
            CHECK_EQ_HEX(nesturbator__bus_read(a, 0x9234u), 0x5au);
            if (nes2)
                CHECK_EQ_HEX(nesturbator__bus_read(a, 0x8123u), 0x39u);
            if (!nes2)
                CHECK_EQ_HEX(nesturbator__bus_read(a, 0xd234u), 0x5au);
            else
                CHECK_EQ_HEX(nesturbator__bus_read(a, 0xc123u), 0x6cu);
            const uint16_t boundaries[] = {0x6000u, 0x6fffu, 0x7200u, 0x7fffu};
            for (size_t i = 0u; i < sizeof boundaries / sizeof boundaries[0]; ++i)
                CHECK_EQ_HEX(nesturbator__bus_read(a, boundaries[i]), 0u);
            nesturbator__bus_write(a, 0x6000u, 0x11u);
            nesturbator__bus_write(a, 0x6fffu, 0x22u);
            nesturbator__bus_write(a, 0x7200u, 0x33u);
            nesturbator__bus_write(a, 0x7fffu, 0x44u);
            for (size_t i = 0u; i < sizeof boundaries / sizeof boundaries[0]; ++i)
                CHECK_EQ_HEX(nesturbator__bus_read(a, boundaries[i]), (i + 1u) * 0x11u);
            nesturbator__bus_write(b, 0x6000u, 0x99u);
            CHECK_EQ_HEX(nesturbator__bus_read(b, 0x6000u), 0x99u);
            CHECK_EQ_HEX(nesturbator__bus_read(a, 0x6000u), 0x11u);
            CHECK_EQ_U64(nesturbator_load_cartridge(first, rom, size), NESTURBATOR_OK);
            CHECK_EQ_HEX(nesturbator__bus_read(a, 0x6000u), 0u);
            CHECK_EQ_HEX(nesturbator__bus_read(a, 0x7000u), rom[16u]);
            nesturbator_unload_cartridge(first);
            CHECK_EQ_U64(nesturbator_load_cartridge(first, rom, size), NESTURBATOR_OK);
            CHECK_EQ_HEX(nesturbator__bus_read(a, 0x71ffu), rom[16u + 511u]);
        }
        if (first != NULL)
            nesturbator_destroy(first);
        if (second != NULL)
            nesturbator_destroy(second);
        free(rom);
    }
}
/* D-10: an image whose mapper has no board is refused before any allocation
   and leaves the loaded cartridge, the CPU and the allocator count alone. */
static void test_unboarded_id_leaves_instance(void)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    counts c = {0, 0, 0};
    uint8_t good[16u + 16384u + 8192u];
    uint8_t other[16u + 16384u + 8192u];
    struct ines_spec spec;
    struct nesturbator *nes;
    const uint8_t *bytes;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    cfg.allocator.alloc = count_alloc;
    cfg.allocator.free = count_free;
    cfg.allocator.user = &c;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    nes = (struct nesturbator *)inst;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = 1u;
    spec.reset_vector = 0x8123u;
    CHECK_EQ_U64(ines_build(good, sizeof good, &spec), sizeof good);
    spec.mapper = 1u;
    CHECK_EQ_U64(ines_build(other, sizeof other, &spec), sizeof other);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, good, sizeof good), NESTURBATOR_OK);
    bytes = nes->cart.bytes;
    {
        unsigned allocs = c.allocs;
        uint16_t pc = nes->cpu.pc;
        CHECK_EQ_HEX(pc, 0x8123u);
        CHECK_EQ_U64(nesturbator_load_cartridge(inst, other, sizeof other),
                     NESTURBATOR_ERR_CARTRIDGE);
        CHECK(nes->cart.bytes == bytes);
        CHECK_EQ_HEX(nes->cpu.pc, pc);
        CHECK_EQ_U64(c.allocs, allocs);
    }
    nesturbator_destroy(inst);
}
/* A NES 2.0 mapper 0 image with 8 KiB declared CHR-RAM loads. */
static void test_nes2_chr_ram_nrom_loads(void)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    uint8_t img[16u + 16384u];
    struct ines_spec spec;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.nes2 = 1u;
    CHECK_EQ_U64(ines_build(img, sizeof img, &spec), sizeof img);
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, img, sizeof img), NESTURBATOR_OK);
    CHECK_EQ_U64(((struct nesturbator *)inst)->cart.chr_size, 8192u);
    nesturbator_destroy(inst);
}

/* D-13: one row per accepted or refused image shape. Each row loads on a fresh instance and a
   mismatch prints the row's label. */
struct board_row {
    const char *label;
    uint16_t mapper;
    uint8_t nes2, submapper;
    uint16_t prg_banks; /* 16 KiB units; above 255 the NES 2.0 MSB nibble is used */
    uint8_t chr_8k;     /* 0 is CHR-RAM */
    uint8_t or_idx, or_val; /* header byte patch, OR-ed in; or_idx 0 means none */
    int size_delta;         /* +1 trailing byte, -1 missing byte */
    int diskdude;
    nesturbator_status want;
};
#define OK_ NESTURBATOR_OK
#define BAD_ NESTURBATOR_ERR_CARTRIDGE
static const struct board_row board_rows[] = {
    /* accept */
    {"uxrom-8-sub0", 2, 1, 0, 8, 0, 0, 0, 0, 0, OK_},
    {"uxrom-8-sub1", 2, 1, 1, 8, 0, 0, 0, 0, 0, OK_},
    {"uxrom-8-sub2", 2, 1, 2, 8, 0, 0, 0, 0, 0, OK_},
    {"uxrom-16-sub0", 2, 1, 0, 16, 0, 0, 0, 0, 0, OK_},
    {"uxrom-16-sub1", 2, 1, 1, 16, 0, 0, 0, 0, 0, OK_},
    {"uxrom-16-sub2", 2, 1, 2, 16, 0, 0, 0, 0, 0, OK_},
    {"uxrom-8-ines1", 2, 0, 0, 8, 0, 0, 0, 0, 0, OK_},
    {"uxrom-16-ines1", 2, 0, 0, 16, 0, 0, 0, 0, 0, OK_},
    {"uxrom-4mib", 2, 1, 0, 256, 0, 0, 0, 0, 0, OK_},
    {"cnrom-chr-1", 3, 0, 0, 1, 1, 0, 0, 0, 0, OK_},
    {"cnrom-chr-2", 3, 0, 0, 1, 2, 0, 0, 0, 0, OK_},
    {"cnrom-chr-4", 3, 0, 0, 2, 4, 0, 0, 0, 0, OK_},
    {"cnrom-sub2", 3, 1, 2, 1, 1, 0, 0, 0, 0, OK_},
    {"axrom-1", 7, 0, 0, 2, 0, 0, 0, 0, 0, OK_},
    {"axrom-2", 7, 0, 0, 4, 0, 0, 0, 0, 0, OK_},
    {"axrom-4", 7, 0, 0, 8, 0, 0, 0, 0, 0, OK_},
    {"axrom-8", 7, 0, 0, 16, 0, 0, 0, 0, 0, OK_},
    {"axrom-vertical", 7, 0, 0, 4, 0, 6, 1, 0, 0, OK_},
    {"axrom-sub2", 7, 1, 2, 2, 0, 0, 0, 0, 0, OK_},
    /* refuse: mappers without a board */
    {"mapper-1", 1, 0, 0, 1, 1, 0, 0, 0, 0, BAD_},
    {"mapper-4", 4, 0, 0, 1, 1, 0, 0, 0, 0, BAD_},
    {"mapper-5", 5, 0, 0, 1, 1, 0, 0, 0, 0, BAD_},
    {"mapper-6", 6, 0, 0, 1, 1, 0, 0, 0, 0, BAD_},
    {"mapper-8", 8, 0, 0, 1, 1, 0, 0, 0, 0, BAD_},
    {"nes2-mapper-256", 256, 1, 0, 1, 0, 0, 0, 0, 0, BAD_},
    {"nes2-mapper-4095", 4095, 1, 0, 1, 0, 0, 0, 0, 0, BAD_},
    /* refuse: submappers */
    {"uxrom-sub3", 2, 1, 3, 2, 0, 0, 0, 0, 0, BAD_},
    {"cnrom-sub3", 3, 1, 3, 1, 1, 0, 0, 0, 0, BAD_},
    {"axrom-sub3", 7, 1, 3, 2, 0, 0, 0, 0, 0, BAD_},
    {"nrom-sub1", 0, 1, 1, 1, 1, 0, 0, 0, 0, BAD_},
    /* refuse: sizes and CHR kinds */
    {"cnrom-chr-ram", 3, 0, 0, 1, 0, 0, 0, 0, 0, BAD_},
    {"cnrom-chr-64k", 3, 0, 0, 1, 8, 0, 0, 0, 0, BAD_},
    {"cnrom-prg-64k", 3, 0, 0, 4, 1, 0, 0, 0, 0, BAD_},
    {"uxrom-chr-rom-16k", 2, 0, 0, 2, 2, 0, 0, 0, 0, BAD_},
    {"uxrom-4mib-plus-16k", 2, 1, 0, 257, 0, 0, 0, 0, 0, BAD_},
    {"axrom-chr-rom", 7, 0, 0, 2, 1, 0, 0, 0, 0, BAD_},
    {"axrom-prg-48k", 7, 0, 0, 3, 0, 0, 0, 0, 0, BAD_},
    {"axrom-prg-16k", 7, 0, 0, 1, 0, 0, 0, 0, 0, BAD_},
    {"axrom-512k", 7, 0, 0, 32, 0, 0, 0, 0, 0, BAD_},
    {"prg-ram", 2, 1, 0, 2, 0, 10, 0x07, 0, 0, BAD_},
    /* refuse: four-screen, battery, trailing and missing bytes, DiskDude */
    {"uxrom-four-screen", 2, 0, 0, 2, 0, 6, 0x08, 0, 0, BAD_},
    {"cnrom-four-screen", 3, 0, 0, 1, 1, 6, 0x08, 0, 0, BAD_},
    {"axrom-four-screen", 7, 0, 0, 2, 0, 6, 0x08, 0, 0, BAD_},
    {"uxrom-battery", 2, 0, 0, 2, 0, 6, 0x02, 0, 0, BAD_},
    {"cnrom-battery", 3, 0, 0, 1, 1, 6, 0x02, 0, 0, BAD_},
    {"axrom-battery", 7, 0, 0, 2, 0, 6, 0x02, 0, 0, BAD_},
    {"uxrom-trailing", 2, 0, 0, 2, 0, 0, 0, 1, 0, BAD_},
    {"cnrom-trailing", 3, 0, 0, 1, 1, 0, 0, 1, 0, BAD_},
    {"axrom-trailing", 7, 0, 0, 2, 0, 0, 0, 1, 0, BAD_},
    {"uxrom-truncated", 2, 0, 0, 2, 0, 0, 0, -1, 0, BAD_},
    {"cnrom-truncated", 3, 0, 0, 1, 1, 0, 0, -1, 0, BAD_},
    {"axrom-truncated", 7, 0, 0, 2, 0, 0, 0, -1, 0, BAD_},
    {"diskdude", 2, 0, 0, 2, 0, 0, 0, 0, 1, BAD_},
};
static void test_board_profiles(void)
{
    const size_t cap = 16u + (size_t)257u * INES_PRG_BANK + 8u * INES_CHR_BANK + 8u;
    uint8_t *img = malloc(cap);
    CHECK(img != NULL);
    if (img == NULL)
        return;
    for (size_t i = 0u; i < sizeof board_rows / sizeof board_rows[0]; ++i) {
        const struct board_row *r = &board_rows[i];
        nesturbator_config cfg;
        nesturbator *inst = NULL;
        size_t size;
        nesturbator_status got;
        if (r->prg_banks > 255u) {
            size = 16u + (size_t)r->prg_banks * INES_PRG_BANK;
            memset(img, 0, size);
            memcpy(img, "NES\032", 4u);
            img[4] = (uint8_t)(r->prg_banks & 0xffu);
            img[7] = 0x08u | (uint8_t)(r->mapper & 0xf0u);
            img[6] = (uint8_t)((r->mapper & 0x0fu) << 4);
            img[8] = (uint8_t)(((unsigned)r->submapper << 4) | ((r->mapper >> 8) & 0x0fu));
            img[9] = (uint8_t)(r->prg_banks >> 8);
            img[11] = 0x07u;
        } else {
            struct ines_spec spec;
            memset(&spec, 0, sizeof spec);
            spec.prg_16k = (uint8_t)r->prg_banks;
            spec.chr_8k = r->chr_8k;
            spec.nes2 = r->nes2;
            spec.mapper = r->mapper;
            spec.submapper = r->submapper;
            spec.reset_vector = 0x8000u;
            size = ines_build(img, cap, &spec);
            CHECK(size != 0u);
        }
        if (r->or_idx != 0u)
            img[r->or_idx] |= r->or_val;
        if (r->diskdude)
            memcpy(img + 7, "DiskDude!", 9u);
        if (r->size_delta > 0)
            img[size++] = 0u;
        else if (r->size_delta < 0)
            size--;
        memset(&cfg, 0, sizeof cfg);
        cfg.size = (uint32_t)sizeof cfg;
        cfg.abi = NESTURBATOR_ABI_VERSION;
        CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
        got = nesturbator_load_cartridge(inst, img, size);
        if (got != r->want)
            fprintf(stderr, "board row %s: got %d want %d\n", r->label, (int)got, (int)r->want);
        CHECK_EQ_U64(got, r->want);
        nesturbator_destroy(inst);
    }
    free(img);
}
/* D-10: the board switch and the profiles agree on exactly the ids with a board. */
static void test_board_switch_matches_profiles(void)
{
    unsigned boarded = 0u;
    for (unsigned id = 0u; id <= 4095u; ++id) {
        struct nesturbator__mapper_ops ops;
        int has = nesturbator__mapper_ops_for((uint16_t)id, &ops);
        int want = id == 0u || id == 2u || id == 3u || id == 7u;
        CHECK_EQ_U64(has, want);
        if (has) {
            boarded++;
            CHECK(ops.init != NULL);
            CHECK(ops.rebuild != NULL);
        }
    }
    CHECK_EQ_U64(boarded, 4u);
}
int main(void)
{
    test_formats_and_lifetime();
    test_reject_before_allocation();
    test_32k_reset_vector_comes_from_upper_prg_bank();
    test_trainer_is_visible_through_cpu_bus();
    test_trainerless_ram_stays_unmapped();
    test_unboarded_id_leaves_instance();
    test_nes2_chr_ram_nrom_loads();
    test_board_profiles();
    test_board_switch_matches_profiles();
    CHECK_DONE();
}
