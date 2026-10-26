/* Owned NROM programs and named expected-red bookkeeping for Phase 20. */
#ifndef NESTURBATOR_TESTS_NMI_FIXTURE_H
#define NESTURBATOR_TESTS_NMI_FIXTURE_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
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

typedef void (*nmi_expected_red_case)(void);

static const char *nmi_red_case_name;
static const char *nmi_red_assertion_name;
static unsigned nmi_red_assertion_count;

static inline int nmi_red_result_matches(const char *expected, const char *observed,
                                         unsigned named_failures, unsigned total_failures)
{
    return expected != NULL && observed != NULL && strcmp(expected, observed) == 0 &&
           named_failures == 1u && total_failures == 1u;
}

static inline void nmi_expected_assertion(const char *identity, int condition)
{
    if (condition)
        return;
    fprintf(stderr, "NMI expected assertion failed: %s (%s)\n",
            nmi_red_case_name != NULL ? nmi_red_case_name : "outside expected-red case", identity);
    if (nmi_red_case_name != NULL) {
        nmi_red_assertion_name = identity;
        nmi_red_assertion_count++;
    }
    check_failures++;
}

static inline void nmi_expect_red(const char *case_name, const char *expected_assertion,
                                  nmi_expected_red_case run_case)
{
    int before = check_failures;
    unsigned named_before = nmi_red_assertion_count;
    nmi_red_case_name = case_name;
    nmi_red_assertion_name = NULL;
    run_case();
    nmi_red_case_name = NULL;

    unsigned named_failures = nmi_red_assertion_count - named_before;
    unsigned total_failures = (unsigned)(check_failures - before);
    if (!nmi_red_result_matches(expected_assertion, nmi_red_assertion_name, named_failures,
                                total_failures)) {
        fprintf(stderr,
                "NMI expected-red rejected: case=%s expected=%s observed=%s named=%u total=%u\n",
                case_name, expected_assertion != NULL ? expected_assertion : "<none>",
                nmi_red_assertion_name != NULL ? nmi_red_assertion_name : "<none>", named_failures,
                total_failures);
        CHECK(0);
        return;
    }

    check_failures--;
    printf("CASE expected-red %s assertion=%s count=1\n", case_name, expected_assertion);
}

#endif /* NESTURBATOR_TESTS_NMI_FIXTURE_H */
