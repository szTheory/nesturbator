/* SAVE-01 (D-06, D-11, D-12): the battery span of a synthetic mapper 1 image,
   its address, its write counter and the empty edges. Images are built from
   zeros, a header, a reset vector and a few instructions (no ROM bytes). */
#include <stdint.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"

#define IMAGE_CAP 70000u
#define SPAN_8K 8192u

/* LDA #$A5; STA $6000; LDA #$5A; STA $7FFF; JMP * (at $800A). */
static const uint8_t write_code[] = {0xa9u, 0xa5u, 0x8du, 0x00u, 0x60u, 0xa9u, 0x5au,
                                     0x8du, 0xffu, 0x7fu, 0x4cu, 0x0au, 0x80u};

/* NES 2.0 mapper 1, 32 KiB PRG, CHR RAM, with the given byte 10 nibbles. */
static size_t build_image(uint8_t *out, uint8_t ram_shift, uint8_t nvram_shift, uint8_t battery)
{
    struct ines_segment seg;
    struct ines_spec spec;
    seg.prg_offset = 0u;
    seg.bytes = write_code;
    seg.len = sizeof write_code;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 2u;
    spec.nes2 = 1u;
    spec.mapper = 1u;
    spec.battery = battery;
    spec.prg_ram_shift = ram_shift;
    spec.prg_nvram_shift = nvram_shift;
    spec.segments = &seg;
    spec.segment_count = 1u;
    spec.nmi_vector = 0x8000u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    return ines_build(out, IMAGE_CAP, &spec);
}

static nesturbator *make_instance(void)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    return inst;
}

static void run_frame(nesturbator *inst)
{
    static uint16_t video[256u * 240u];
    static int16_t audio[2048];
    nesturbator_frame io;
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = video;
    io.video_pitch = 256u;
    io.audio = audio;
    io.audio_capacity = 2048u;
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
}

/* Tracer: the CPU's writes to $6000 and $7FFF land in the span and are counted. */
static void test_span_and_generation(void)
{
    static uint8_t image[IMAGE_CAP];
    size_t size = build_image(image, 0u, 7u, 1u);
    nesturbator *inst = make_instance();
    uint8_t *data = NULL, *again = NULL;
    size_t n = 0u;
    CHECK(size != 0u);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &data, &n),
                 NESTURBATOR_OK);
    CHECK(data != NULL);
    CHECK_EQ_U64(n, SPAN_8K);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 0u);
    run_frame(inst);
    CHECK_EQ_HEX(data[0], 0xa5u);
    CHECK_EQ_HEX(data[0x1fffu], 0x5au);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 2u);
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &again, &n),
                 NESTURBATOR_OK);
    CHECK(again == data);
    CHECK_EQ_U64(nesturbator_reset(inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &again, &n),
                 NESTURBATOR_OK);
    CHECK(again == data);
    CHECK_EQ_U64(n, SPAN_8K);
    CHECK_EQ_HEX(data[0], 0xa5u);
    CHECK_EQ_HEX(data[0x1fffu], 0x5au);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 2u);
    nesturbator_destroy(inst);
}

/* BOARD-02 empty edge: a mapper 1 image with no PRG RAM reads $6000 as open bus. */
static void test_no_ram_is_open_bus(void)
{
    static uint8_t image[IMAGE_CAP];
    size_t size = build_image(image, 0u, 0u, 0u);
    nesturbator *inst = make_instance();
    struct nesturbator *nes = inst;
    uint8_t *data = NULL;
    size_t n = 1u;
    CHECK(size != 0u);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &data, &n),
                 NESTURBATOR_OK);
    CHECK(data == NULL);
    CHECK_EQ_U64(n, 0u);
    nes->bus.open_bus = 0x5cu;
    CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x6000u), 0x5cu);
    nesturbator_destroy(inst);
}

int main(void)
{
    test_span_and_generation();
    test_no_ram_is_open_bus();
    CHECK_DONE();
}
