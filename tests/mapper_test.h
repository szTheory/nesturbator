/* A test board for the mapper seam (D-08). Header-only, tests only: it records
   every CPU write as (addr, value, cpu_cycle), exposes a settable IRQ bit, and
   logs PPU address bus bit-12 edges as (level, tick). The logs are owned by the
   test that includes this header. The hooks follow D-07: they read no ticks,
   touch no bus and allocate nothing.

   The board keeps NROM's pages (rebuild does nothing), so reads still see PRG. */
#ifndef NESTURBATOR_TESTS_MAPPER_TEST_H
#define NESTURBATOR_TESTS_MAPPER_TEST_H

#include <string.h>
#include "internal.h"

#define MAPPER_TEST_WRITE_CAP 64u
#define MAPPER_TEST_EDGE_CAP 256u

struct mapper_test_write {
    uint16_t addr;
    uint8_t value;
    uint64_t cpu_cycle;
};

struct mapper_test_edge {
    uint8_t level;
    uint64_t tick;
};

static struct mapper_test_write mapper_test_writes[MAPPER_TEST_WRITE_CAP];
static unsigned mapper_test_write_count;
static struct mapper_test_edge mapper_test_edges[MAPPER_TEST_EDGE_CAP];
static unsigned mapper_test_edge_count;
static uint8_t mapper_test_watch;

static inline void mapper_test_init(struct nesturbator *nes)
{
    nes->map.watch = mapper_test_watch;
}

static inline void mapper_test_rebuild(struct nesturbator *nes)
{
    (void)nes;
}

/* Bit 0 of a write to $E000-$FFFF is the board's IRQ line. */
static inline void mapper_test_cpu_write(struct nesturbator *nes, uint16_t addr, uint8_t value,
                                         uint64_t cpu_cycle)
{
    if (mapper_test_write_count < MAPPER_TEST_WRITE_CAP) {
        mapper_test_writes[mapper_test_write_count].addr = addr;
        mapper_test_writes[mapper_test_write_count].value = value;
        mapper_test_writes[mapper_test_write_count].cpu_cycle = cpu_cycle;
        mapper_test_write_count++;
    }
    if (addr >= 0xe000u) {
        nes->mapper.irq = (uint8_t)(value & 1u);
        nesturbator__irq_update(nes);
    }
}

static inline void mapper_test_ppu_a12(struct nesturbator *nes, uint8_t level, uint64_t tick)
{
    (void)nes;
    if (mapper_test_edge_count < MAPPER_TEST_EDGE_CAP) {
        mapper_test_edges[mapper_test_edge_count].level = level;
        mapper_test_edges[mapper_test_edge_count].tick = tick;
        mapper_test_edge_count++;
    }
}

static inline void mapper_test_fill(struct nesturbator__mapper_ops *out)
{
    out->init = mapper_test_init;
    out->rebuild = mapper_test_rebuild;
    out->cpu_write = mapper_test_cpu_write;
    out->ppu_a12 = mapper_test_ppu_a12;
    out->ppu_read = NULL;
}

/* Clears the logs and makes nes use the test board with the given watch mask. */
static inline void mapper_test_install(struct nesturbator *nes, uint8_t watch)
{
    mapper_test_write_count = 0u;
    mapper_test_edge_count = 0u;
    mapper_test_watch = watch;
    mapper_test_fill(&nes->map.ops);
    nes->map.ops.init(nes);
}

#endif /* NESTURBATOR_TESTS_MAPPER_TEST_H */
