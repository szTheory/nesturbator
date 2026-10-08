/* D-01: bounded mapper-0 image validation through the public API. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"

typedef struct counts {
    unsigned allocs, frees;
} counts;
static void *count_alloc(void *user, size_t size)
{
    ((counts *)user)->allocs++;
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
static void test_formats_and_lifetime(void)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    counts c = {0, 0};
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
    counts c = {0, 0};
    uint8_t *rom = malloc(16u + 16384u + 8192u + 1u);
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    cfg.allocator.alloc = count_alloc;
    cfg.allocator.free = count_free;
    cfg.allocator.user = &c;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    unsigned base = c.allocs;
    size_t full = 16u + 16384u + 8192u;
    make_image(rom, full + 1u, 1, 0);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, rom, full + 1u), NESTURBATOR_ERR_CARTRIDGE);
    CHECK_EQ_U64(c.allocs, base);
    make_image(rom, full, 1, 0);
    rom[9] = 0x0fu;
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, rom, full), NESTURBATOR_ERR_CARTRIDGE);
    CHECK_EQ_U64(c.allocs, base);
    make_image(rom, full, 1, 0);
    rom[7] |= 0x10u;
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, rom, full), NESTURBATOR_ERR_CARTRIDGE);
    CHECK_EQ_U64(c.allocs, base);
    nesturbator_destroy(inst);
    free(rom);
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
int main(void)
{
    test_formats_and_lifetime();
    test_reject_before_allocation();
    test_32k_reset_vector_comes_from_upper_prg_bank();
    CHECK_DONE();
}
