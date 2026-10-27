/* Owned NROM programs and cycle-aligned phase fixtures for Phase 20. */
#ifndef NESTURBATOR_TESTS_NMI_FIXTURE_H
#define NESTURBATOR_TESTS_NMI_FIXTURE_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "internal.h"
#include "check.h"
#include "ines.h"

#define NMI_FIXTURE_IMAGE_CAP 40000u

static inline struct nesturbator *nmi_fixture_build(const uint8_t *program, size_t program_len,
                                                    const uint8_t *handler, size_t handler_len,
                                                    uint16_t handler_address)
{
    static uint8_t image[NMI_FIXTURE_IMAGE_CAP];
    static const uint8_t idle_program[] = {0x4cu, 0x00u, 0x80u}; /* JMP $8000 */
    struct ines_segment handler_segment;
    struct ines_spec spec;
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    size_t size;

    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = 1u;
    spec.prg_code = program != NULL ? program : idle_program;
    spec.prg_code_len = program != NULL ? program_len : sizeof idle_program;
    spec.nmi_vector = handler_len != 0u ? handler_address : 0x8000u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    if (handler_len != 0u) {
        CHECK(handler != NULL);
        CHECK(handler_address >= 0x8000u);
        CHECK((size_t)(handler_address - 0x8000u) + handler_len <= INES_PRG_BANK);
        handler_segment.prg_offset = (uint32_t)(handler_address - 0x8000u);
        handler_segment.bytes = handler;
        handler_segment.len = handler_len;
        spec.segments = &handler_segment;
        spec.segment_count = 1u;
    }

    size = ines_build(image, sizeof image, &spec);
    CHECK(size != 0u);
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    CHECK(inst != NULL);
    if (inst != NULL)
        CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    return (struct nesturbator *)inst;
}

/* Start a cycle-aligned whitebox fixture at a named PPU point. One CPU cycle is
   24 half-master ticks; PPU dots take 8 ticks. */
static inline void nmi_fixture_at(struct nesturbator *nes, uint16_t scanline, uint16_t dot,
                                  uint8_t status, uint8_t control, uint8_t io_bus)
{
    nes->ticks = 0u;
    nes->cpu_cycle = 0u;
    nes->ppu.ppu_ticks = 0u;
    nes->ppu.scanline = scanline;
    nes->ppu.dot = dot;
    nes->ppu.status = status;
    nes->ppu.control = control;
    nes->ppu.io_bus = io_bus;
    nes->ppu.io_bus_age = 0u;
    nes->ppu.vblank_suppress = 0u;
    nes->ppu.address_latch = 0u;
    nes->ppu.reset_flag = 0u;
    nes->cpu.nmi_pending = 0u;
}

#endif /* NESTURBATOR_TESTS_NMI_FIXTURE_H */
