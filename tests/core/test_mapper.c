/* MAP-01 (D-03 to D-08): the mapper seam on a synthetic NROM image with a test board
   installed. One function per behaviour. */
#include <stdint.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"
#include "../mapper_test.h"

#define IMAGE_CAP 40000u

/* Builds an NROM image (16 KiB PRG, CHR ROM) and loads it. code is placed at $8000 and the
   reset vector points there. */
static struct nesturbator *make_instance(const uint8_t *code, size_t code_len, int trainer,
                                         int vertical)
{
    static uint8_t image[IMAGE_CAP];
    static const uint8_t trainer_bytes[INES_TRAINER_SIZE] = {0x6du};
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    struct ines_spec spec;
    size_t size;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = 1u;
    spec.mirroring_vertical = (uint8_t)(vertical != 0);
    spec.trainer = trainer != 0 ? trainer_bytes : NULL;
    spec.prg_code = code;
    spec.prg_code_len = code_len;
    spec.nmi_vector = 0x8000u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    size = ines_build(image, IMAGE_CAP, &spec);
    CHECK(size != 0u);
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    return (struct nesturbator *)inst;
}

static struct nesturbator *make_plain(void)
{
    static const uint8_t jmp_self[3] = {0x4cu, 0x00u, 0x80u};
    return make_instance(jmp_self, sizeof jmp_self, 0, 0);
}

/* Writes to $8000, $8001 and $FFFF land on NULL write pages (ROM) and still reach the hook, in
   order, each stamped one cycle after the last. */
static void check_write_stamps(void)
{
    struct nesturbator *nes = make_plain();
    CHECK(nes->map.cpu_w[16] == NULL);
    CHECK(nes->map.cpu_w[47] == NULL);
    mapper_test_install(nes, NESTURBATOR_WATCH_CPU_WRITE);
    nesturbator__bus_write(nes, 0x8000u, 0x11u);
    uint64_t c0 = nes->cpu_cycle;
    nesturbator__bus_write(nes, 0x8001u, 0x22u);
    uint64_t c1 = nes->cpu_cycle;
    nesturbator__bus_write(nes, 0xffffu, 0x33u);
    uint64_t c2 = nes->cpu_cycle;
    CHECK_EQ_U64(mapper_test_write_count, 3u);
    CHECK_EQ_HEX(mapper_test_writes[0].addr, 0x8000u);
    CHECK_EQ_HEX(mapper_test_writes[1].addr, 0x8001u);
    CHECK_EQ_HEX(mapper_test_writes[2].addr, 0xffffu);
    CHECK_EQ_HEX(mapper_test_writes[0].value, 0x11u);
    CHECK_EQ_HEX(mapper_test_writes[1].value, 0x22u);
    CHECK_EQ_HEX(mapper_test_writes[2].value, 0x33u);
    CHECK_EQ_U64(mapper_test_writes[0].cpu_cycle, c0);
    CHECK_EQ_U64(mapper_test_writes[1].cpu_cycle, c1);
    CHECK_EQ_U64(mapper_test_writes[2].cpu_cycle, c2);
    CHECK_EQ_U64(c1, c0 + 1u);
    CHECK_EQ_U64(c2, c1 + 1u);
    nesturbator_destroy((nesturbator *)nes);
}

/* A trainer image has PRG RAM at $6000: the write both stores and reaches the hook. */
static void check_hook_on_writable_page(void)
{
    static const uint8_t jmp_self[3] = {0x4cu, 0x00u, 0x80u};
    struct nesturbator *nes = make_instance(jmp_self, sizeof jmp_self, 1, 0);
    mapper_test_install(nes, NESTURBATOR_WATCH_CPU_WRITE);
    nesturbator__bus_write(nes, 0x6000u, 0xa5u);
    CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x6000u), 0xa5u);
    CHECK_EQ_U64(mapper_test_write_count, 1u);
    CHECK_EQ_HEX(mapper_test_writes[0].addr, 0x6000u);
    nesturbator_destroy((nesturbator *)nes);
}

/* The oracle cpu_cycle * 24 == ticks holds for NTSC only (24 ticks per CPU cycle). D-04 keeps
   cpu_cycle because that ratio is not true on other regions. */
static void check_cpu_stamps(void)
{
    /* STA $8000; STA $8001; JMP $8006 */
    static const uint8_t code[] = {0x8du, 0x00u, 0x80u, 0x8du, 0x01u, 0x80u, 0x4cu, 0x06u, 0x80u};
    struct nesturbator *nes = make_instance(code, sizeof code, 0, 0);
    mapper_test_install(nes, NESTURBATOR_WATCH_CPU_WRITE);
    nesturbator__cpu_step(nes);
    CHECK_EQ_U64(nes->cpu_cycle * 24u, nes->ticks);
    nesturbator__cpu_step(nes);
    CHECK_EQ_U64(nes->cpu_cycle * 24u, nes->ticks);
    CHECK_EQ_U64(mapper_test_write_count, 2u);
    CHECK_EQ_U64(mapper_test_writes[1].cpu_cycle - mapper_test_writes[0].cpu_cycle, 4u);
    nesturbator_destroy((nesturbator *)nes);
}

/* An OAM DMA between two writes advances the stamp by the DMA's cycles: one write to $4014, 513
   or 514 DMA cycles, one read and the second write. The ticks ratio is an NTSC-only oracle. */
static void check_dma_stamps(void)
{
    struct nesturbator *nes = make_plain();
    mapper_test_install(nes, NESTURBATOR_WATCH_CPU_WRITE);
    nesturbator__bus_write(nes, 0x8000u, 0x01u);
    uint64_t ticks_before = nes->ticks;
    CHECK_EQ_U64(nes->cpu_cycle * 24u, nes->ticks);
    nesturbator__bus_write(nes, 0x4014u, 2u);
    (void)nesturbator__bus_read(nes, 0x0000u);
    nesturbator__bus_write(nes, 0x8001u, 0x02u);
    CHECK_EQ_U64(nes->cpu_cycle * 24u, nes->ticks);
    CHECK_EQ_U64(mapper_test_write_count, 2u);
    uint64_t delta = mapper_test_writes[1].cpu_cycle - mapper_test_writes[0].cpu_cycle;
    CHECK_EQ_U64(delta, (nes->ticks - ticks_before) / 24u);
    CHECK(delta == 516u || delta == 517u);
    nesturbator_destroy((nesturbator *)nes);
}

/* D-06: iNES flags 6 bit 0 clear is horizontal mirroring, set is vertical. */
static void check_nametable_map(void)
{
    static const uint8_t jmp_self[3] = {0x4cu, 0x00u, 0x80u};
    struct nesturbator *nes = make_instance(jmp_self, sizeof jmp_self, 0, 0);
    CHECK_EQ_U64(nes->map.nt[0], 0u);
    CHECK_EQ_U64(nes->map.nt[1], 0u);
    CHECK_EQ_U64(nes->map.nt[2], 1u);
    CHECK_EQ_U64(nes->map.nt[3], 1u);
    nesturbator_destroy((nesturbator *)nes);
    nes = make_instance(jmp_self, sizeof jmp_self, 0, 1);
    CHECK_EQ_U64(nes->map.nt[0], 0u);
    CHECK_EQ_U64(nes->map.nt[1], 1u);
    CHECK_EQ_U64(nes->map.nt[2], 0u);
    CHECK_EQ_U64(nes->map.nt[3], 1u);
    nesturbator_destroy((nesturbator *)nes);
}

/* NESdev "Bus conflict": the ROM drives the bus too, so the board sees the AND. */
static void check_bus_conflict(void)
{
    struct nesturbator *nes = make_plain();
    nes->cart.prg[0x10] = 0x5au;
    mapper_test_install(nes, NESTURBATOR_WATCH_CPU_WRITE | NESTURBATOR_WATCH_BUS_CONFLICT);
    nesturbator__bus_write(nes, 0x8010u, 0x0fu);
    CHECK_EQ_U64(mapper_test_write_count, 1u);
    CHECK_EQ_HEX(mapper_test_writes[0].value, 0x0au);
    mapper_test_install(nes, NESTURBATOR_WATCH_CPU_WRITE);
    nesturbator__bus_write(nes, 0x8010u, 0x0fu);
    CHECK_EQ_U64(mapper_test_write_count, 1u);
    CHECK_EQ_HEX(mapper_test_writes[0].value, 0x0fu);
    nesturbator_destroy((nesturbator *)nes);
}

static void check_watch_zero(void)
{
    struct nesturbator *nes = make_plain();
    mapper_test_install(nes, 0u);
    nesturbator__bus_write(nes, 0x8000u, 0x01u);
    CHECK_EQ_U64(mapper_test_write_count, 0u);
    nesturbator_destroy((nesturbator *)nes);
}

/* A zeroed instance has no cartridge: NULL pages, watch 0 (D-03, D-06). */
static void check_empty_instance(void)
{
    static struct nesturbator nes;
    memset(&nes, 0, sizeof nes);
    nes.bus.open_bus = 0xc3u;
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x5000u), 0xc3u);
    CHECK_EQ_HEX(nesturbator__bus_read(&nes, 0x9000u), 0xc3u);
    nesturbator__bus_write(&nes, 0x8000u, 0x12u); /* dropped; the NULL ops are never called */
    CHECK_EQ_U64(nes.map.watch, 0u);
    CHECK_EQ_U64(nesturbator__ppu_read(&nes, 0x0123u), 0u);
    nesturbator__ppu_write(&nes, 0x0123u, 0x55u); /* NULL CHR write page: dropped */
    CHECK_EQ_U64(nesturbator__ppu_read(&nes, 0x0123u), 0u);
}

/* D-03: a bank number is reduced with a true modulo against the validated size. */
static void check_bank_modulo(void)
{
    struct nesturbator *nes = make_plain();
    CHECK_EQ_U64(nes->cart.prg_size, 16384u);
    CHECK(nesturbator__map_prg(nes, 16u) == nesturbator__map_prg(nes, 0u));
    CHECK(nesturbator__map_prg(nes, 47u) == nes->cart.prg + 15u * 1024u);
    CHECK(nesturbator__map_chr(nes, 8u) == nesturbator__map_chr(nes, 0u));
    nesturbator_destroy((nesturbator *)nes);
}

/* D-05: the CPU IRQ line is frame (not inhibited) OR DMC OR mapper. */
static void check_irq_or(void)
{
    struct nesturbator *nes = make_plain();
    for (unsigned c = 0u; c < 8u; ++c) {
        nes->apu.frame_irq_inhibit = 0u;
        nes->apu.frame_irq = (uint8_t)(c & 1u);
        nes->apu.dmc.irq = (uint8_t)((c >> 1) & 1u);
        nes->mapper.irq = (uint8_t)((c >> 2) & 1u);
        nesturbator__irq_update(nes);
        CHECK_EQ_U64(nes->cpu.irq_line, c != 0u ? 1u : 0u);
    }
    nes->apu.frame_irq = 1u;
    nes->apu.frame_irq_inhibit = 1u;
    nes->apu.dmc.irq = 0u;
    nes->mapper.irq = 0u;
    nesturbator__irq_update(nes);
    CHECK_EQ_U64(nes->cpu.irq_line, 0u);

    nes->apu.frame_irq = 0u;
    nes->apu.frame_irq_inhibit = 0u;
    nes->mapper.irq = 1u;
    nes->apu.dmc.irq = 1u;
    nesturbator__irq_update(nes);
    CHECK_EQ_U64(nes->cpu.irq_line, 1u);
    nes->mapper.irq = 0u;
    nesturbator__irq_update(nes);
    CHECK_EQ_U64(nes->cpu.irq_line, 1u);
    nes->apu.dmc.irq = 0u;
    nesturbator__irq_update(nes);
    CHECK_EQ_U64(nes->cpu.irq_line, 0u);
    nesturbator_destroy((nesturbator *)nes);
}

static void check_irq_through_hook(void)
{
    struct nesturbator *nes = make_plain();
    mapper_test_install(nes, NESTURBATOR_WATCH_CPU_WRITE);
    nesturbator__bus_write(nes, 0xe000u, 1u);
    CHECK_EQ_U64(nes->cpu.irq_line, 1u);
    nesturbator__bus_write(nes, 0xe000u, 0u);
    CHECK_EQ_U64(nes->cpu.irq_line, 0u);
    nes->apu.dmc.irq = 1u;
    nesturbator__bus_write(nes, 0xe000u, 1u);
    nesturbator__bus_write(nes, 0xe000u, 0u);
    CHECK_EQ_U64(nes->cpu.irq_line, 1u);
    nesturbator_destroy((nesturbator *)nes);
}

/* Mapper registers live in nes->mapper, which a soft reset keeps; the reset runs 7 CPU cycles. */
static void check_reset_keeps_board(void)
{
    struct nesturbator *nes = make_plain();
    struct nesturbator__mapper before;
    nes->mapper.irq = 1u;
    memcpy(&before, &nes->mapper, sizeof before);
    uint64_t cycles = nes->cpu_cycle;
    CHECK_EQ_U64(nesturbator_reset((nesturbator *)nes), NESTURBATOR_OK);
    CHECK_EQ_U64(memcmp(&before, &nes->mapper, sizeof before), 0);
    CHECK_EQ_U64(nes->cpu_cycle, cycles + 7u);
    nesturbator_destroy((nesturbator *)nes);
}

int main(void)
{
    check_write_stamps();
    check_hook_on_writable_page();
    check_cpu_stamps();
    check_dma_stamps();
    check_nametable_map();
    check_bus_conflict();
    check_watch_zero();
    check_empty_instance();
    check_bank_modulo();
    check_irq_or();
    check_irq_through_hook();
    check_reset_keeps_board();
    CHECK_DONE();
}
