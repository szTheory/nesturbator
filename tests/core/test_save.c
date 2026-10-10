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
#define SOROM_CAP (16u + 131072u + 8192u)

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

/* D-11: argument errors write nothing; no cartridge and no span are OK with NULL and 0. */
static void test_arguments_and_empty_cases(void)
{
    static uint8_t image[IMAGE_CAP];
    nesturbator *inst = make_instance();
    uint8_t sentinel_byte = 0u;
    uint8_t *data = &sentinel_byte;
    size_t n = 77u;
    size_t size;
    CHECK_EQ_U64(nesturbator_save_generation(NULL), 0u);
    CHECK_EQ_U64(nesturbator_get_memory(NULL, NESTURBATOR_MEMORY_SAVE_RAM, &data, &n),
                 NESTURBATOR_ERR_ARGUMENT);
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, NULL, &n),
                 NESTURBATOR_ERR_ARGUMENT);
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &data, NULL),
                 NESTURBATOR_ERR_ARGUMENT);
    CHECK_EQ_U64(nesturbator_get_memory(inst, (nesturbator_memory)1, &data, &n),
                 NESTURBATOR_ERR_ARGUMENT);
    CHECK(data == &sentinel_byte);
    CHECK_EQ_U64(n, 77u);
    /* No cartridge. */
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &data, &n),
                 NESTURBATOR_OK);
    CHECK(data == NULL);
    CHECK_EQ_U64(n, 0u);
    /* 8 KiB of work RAM and no NVRAM: a battery-less cartridge has no span. */
    size = build_image(image, 7u, 0u, 0u);
    CHECK(size != 0u);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    data = &sentinel_byte;
    n = 77u;
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &data, &n),
                 NESTURBATOR_OK);
    CHECK(data == NULL);
    CHECK_EQ_U64(n, 0u);
    run_frame(inst);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 0u);
    nesturbator_destroy(inst);
}

/* D-11, D-12: host writes, a refused load, unload and reload. */
static void test_lifetime_and_generation(void)
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
    CHECK_EQ_U64(nesturbator_save_generation(inst), 0u);
    /* The host copies a save in after load; that is not a CPU write. */
    data[5] = 0x77u;
    CHECK_EQ_U64(nesturbator_save_generation(inst), 0u);
    run_frame(inst);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 2u);
    CHECK_EQ_HEX(data[5], 0x77u);
    /* A refused load keeps the pointer, the bytes and the count. */
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size - 1u), NESTURBATOR_ERR_CARTRIDGE);
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &again, &n),
                 NESTURBATOR_OK);
    CHECK(again == data);
    CHECK_EQ_U64(n, SPAN_8K);
    CHECK_EQ_HEX(data[0], 0xa5u);
    CHECK_EQ_HEX(data[5], 0x77u);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 2u);
    /* Unload: NULL and 0, count kept. */
    nesturbator_unload_cartridge(inst);
    again = &image[0];
    n = 99u;
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &again, &n),
                 NESTURBATOR_OK);
    CHECK(again == NULL);
    CHECK_EQ_U64(n, 0u);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 2u);
    /* A reload continues from the count, never from 0; load adds nothing. */
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 2u);
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &data, &n),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(n, SPAN_8K);
    CHECK_EQ_HEX(data[0], 0u);
    run_frame(inst);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 4u);
    nesturbator_destroy(inst);
}

/* D-06: in a NES 2.0 SOROM (8 KiB work RAM then 8 KiB battery RAM) the span is the second half
   (NESdev Wiki "MMC1", SOROM). */
static void test_sorom_span_offset(void)
{
    static uint8_t image[SOROM_CAP];
    struct ines_spec spec;
    size_t size;
    nesturbator *inst = make_instance();
    struct nesturbator *nes = inst;
    uint8_t *data = NULL;
    size_t n = 0u;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 8u;
    spec.chr_8k = 1u;
    spec.nes2 = 1u;
    spec.mapper = 1u;
    spec.battery = 1u;
    spec.prg_ram_shift = 7u;
    spec.prg_nvram_shift = 7u;
    spec.reset_vector = 0x8000u;
    size = ines_build(image, SOROM_CAP, &spec);
    CHECK(size != 0u);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nes->cart.prg_ram_size, 16384u);
    CHECK(nes->cart.save == nes->cart.prg_ram + 8192u);
    CHECK_EQ_U64(nes->cart.save_size, 8192u);
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &data, &n),
                 NESTURBATOR_OK);
    CHECK(data == nes->cart.prg_ram + 8192u);
    CHECK_EQ_U64(n, 8192u);
    nesturbator_destroy(inst);
}

/* Research open question 3: a trainer on a mapper 1 image with no declared RAM maps its 8 KiB
   at $6000 as work RAM (trainer at $7000), and there is no span. */
static void test_trainer_without_declared_ram(void)
{
    static uint8_t image[IMAGE_CAP];
    static uint8_t trainer[INES_TRAINER_SIZE];
    struct ines_spec spec;
    size_t size;
    nesturbator *inst = make_instance();
    struct nesturbator *nes = inst;
    uint8_t *data = &trainer[0];
    size_t n = 5u;
    for (size_t i = 0u; i < sizeof trainer; ++i)
        trainer[i] = (uint8_t)(i * 7u + 0x31u);
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 2u;
    spec.nes2 = 1u;
    spec.mapper = 1u;
    spec.trainer = trainer;
    spec.reset_vector = 0x8000u;
    size = ines_build(image, IMAGE_CAP, &spec);
    CHECK(size != 0u);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nes->cart.prg_ram_size, 8192u);
    CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x7000u), trainer[0]);
    CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x71ffu), trainer[511]);
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &data, &n),
                 NESTURBATOR_OK);
    CHECK(data == NULL);
    CHECK_EQ_U64(n, 0u);
    nesturbator_destroy(inst);
}

int main(void)
{
    test_span_and_generation();
    test_no_ram_is_open_bus();
    test_arguments_and_empty_cases();
    test_lifetime_and_generation();
    test_sorom_span_offset();
    test_trainer_without_declared_ram();
    CHECK_DONE();
}
