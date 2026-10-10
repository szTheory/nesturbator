/* A loaded NROM instance for the PPU pipeline tests. Header-only, tests only: it builds a
   one-bank image with tests/ines.h and loads it through the public loader, so the page tables
   exist as they do in a real run. Tests edit pattern data through nes->cart.chr, which is
   writable for CHR ROM and CHR RAM alike because the loader keeps its own copy. */
#ifndef NESTURBATOR_TESTS_PPU_FIXTURE_H
#define NESTURBATOR_TESTS_PPU_FIXTURE_H

#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"

#define PPU_FIXTURE_IMAGE_CAP 40000u

/* PRG is JMP $8000 at $8000 with every vector pointing there. chr_ram selects declared CHR RAM
   instead of 8 KiB of CHR ROM. */
static inline struct nesturbator *ppu_fixture_load(uint8_t mirroring_vertical, uint8_t chr_ram)
{
    static uint8_t image[PPU_FIXTURE_IMAGE_CAP];
    static const uint8_t jmp_self[3] = {0x4cu, 0x00u, 0x80u};
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    struct ines_spec spec;
    size_t size;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = (uint8_t)(chr_ram != 0u ? 0u : 1u);
    spec.mirroring_vertical = mirroring_vertical;
    spec.prg_code = jmp_self;
    spec.prg_code_len = sizeof jmp_self;
    spec.nmi_vector = 0x8000u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    size = ines_build(image, PPU_FIXTURE_IMAGE_CAP, &spec);
    CHECK(size != 0u);
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    return (struct nesturbator *)inst;
}

static inline void ppu_fixture_free(struct nesturbator *nes)
{
    nesturbator_destroy((nesturbator *)nes);
}

#endif
