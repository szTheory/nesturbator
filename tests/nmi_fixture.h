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
#define NMI_FIXTURE_HOST_IMAGE_SIZE (16u + INES_PRG_BANK + INES_CHR_BANK)
#define NMI_FIXTURE_HOST_COUNT 0x30u
#define NMI_FIXTURE_HOST_EVENTS 0x40u

/* A host-replay image with one independently visible NMI per frame. The handler
   records a one-based event sequence in CPU RAM, changes the backdrop, and leaves
   pulse 1 running so public frame/audio output carries useful history. */
static inline size_t nmi_fixture_host_image(uint8_t *image, size_t capacity)
{
    static const uint8_t main_code[] = {
        0x78u,                             /* SEI */
        0xa9u, 0x40u, 0x8du, 0x17u, 0x40u, /* inhibit frame IRQ */
        0xa9u, 0x01u, 0x8du, 0x15u, 0x40u, /* enable pulse 1 */
        0xa9u, 0xbfu, 0x8du, 0x00u, 0x40u, /* pulse 1: duty and constant volume */
        0xa9u, 0xfdu, 0x8du, 0x02u, 0x40u, /* pulse 1: timer low */
        0xa9u, 0x08u, 0x8du, 0x03u, 0x40u, /* pulse 1: length and timer high */
        0xa9u, 0x3fu, 0x8du, 0x06u, 0x20u, /* palette address high */
        0xa9u, 0x00u, 0x8du, 0x06u, 0x20u, /* palette address low */
        0xa9u, 0x01u, 0x8du, 0x07u, 0x20u, /* visible backdrop */
        0xa9u, 0x0au, 0x8du, 0x01u, 0x20u, /* background, including left edge */
        0xa9u, 0x80u, 0x8du, 0x00u, 0x20u, /* enable NMI */
        0x58u,                             /* CLI */
        0x4cu, 0x34u, 0x80u};              /* JMP $8034 */
    static const uint8_t nmi_code[] = {0xe6u,
                                       NMI_FIXTURE_HOST_COUNT, /* INC guest NMI count */
                                       0xa5u,
                                       NMI_FIXTURE_HOST_COUNT, /* LDA guest NMI count */
                                       0xaau,
                                       0x95u,
                                       NMI_FIXTURE_HOST_EVENTS, /* STA $40,X: event sequence */
                                       0x29u,
                                       0x0fu,
                                       0x09u,
                                       0x01u,
                                       0x85u,
                                       0x31u, /* changing backdrop */
                                       0xa9u,
                                       0x3fu,
                                       0x8du,
                                       0x06u,
                                       0x20u,
                                       0xa9u,
                                       0x00u,
                                       0x8du,
                                       0x06u,
                                       0x20u,
                                       0xa5u,
                                       0x31u,
                                       0x8du,
                                       0x07u,
                                       0x20u,
                                       0xa9u,
                                       0x3fu,
                                       0x8du,
                                       0x06u,
                                       0x20u,
                                       0xa9u,
                                       0x00u,
                                       0x8du,
                                       0x06u,
                                       0x20u,
                                       0x40u}; /* RTI */
    if (image == NULL || capacity < NMI_FIXTURE_HOST_IMAGE_SIZE)
        return 0u;
    {
        struct ines_segment handler_segment;
        struct ines_spec spec;
        size_t size;
        memset(&spec, 0, sizeof spec);
        handler_segment.prg_offset = 0x100u;
        handler_segment.bytes = nmi_code;
        handler_segment.len = sizeof nmi_code;
        spec.prg_16k = 1u;
        spec.chr_8k = 1u;
        spec.prg_code = main_code;
        spec.prg_code_len = sizeof main_code;
        spec.segments = &handler_segment;
        spec.segment_count = 1u;
        spec.nmi_vector = 0x8100u;
        spec.reset_vector = 0x8000u;
        spec.irq_vector = 0x8000u;
        size = ines_build(image, capacity, &spec);
        return size;
    }
}

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
