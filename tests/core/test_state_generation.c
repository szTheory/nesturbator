/* STATE-05 (D-04 step 8): a state load raises nesturbator_save_generation exactly once when the
   state's battery span differs from the instance's span, and never otherwise. Images are
   synthetic (header, vectors and a few instructions; no ROM bytes). White box: reads the
   cartridge layout through internal.h. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"

#define IMAGE_CAP 70000u
#define SPAN_8K 8192u
#define LOADS 600u

/* LDA #$A5; STA $6000; LDA #$5A; STA $7FFF; JMP * (at $800A). */
static const uint8_t write_code[] = {0xa9u, 0xa5u, 0x8du, 0x00u, 0x60u, 0xa9u, 0x5au,
                                     0x8du, 0xffu, 0x7fu, 0x4cu, 0x0au, 0x80u};

static size_t build_image(uint8_t *out, uint8_t mapper, uint8_t ram_shift, uint8_t nvram_shift,
                          uint8_t battery)
{
    struct ines_segment seg;
    struct ines_spec spec;
    seg.prg_offset = 0u;
    seg.bytes = write_code;
    seg.len = sizeof write_code;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 2u;
    spec.nes2 = 1u;
    spec.mapper = mapper;
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

static nesturbator *make_loaded(const uint8_t *image, size_t size)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    CHECK(size != 0u);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
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

static uint8_t *save_of(nesturbator *inst, size_t *len)
{
    size_t n = nesturbator_state_size(inst);
    uint8_t *s = (uint8_t *)malloc(n);
    CHECK(s != NULL);
    CHECK_EQ_U64(nesturbator_save_state(inst, s, n), NESTURBATOR_OK);
    *len = n;
    return s;
}

/* Tracer (b): changing one span byte and loading S counts once; loading S again counts nothing. */
static void test_changed_span_counts_once(void)
{
    static uint8_t image[IMAGE_CAP];
    size_t size = build_image(image, 1u, 0u, 7u, 1u);
    nesturbator *inst = make_loaded(image, size);
    uint8_t *data = NULL, *s;
    size_t n = 0u, len;
    uint64_t g;
    run_frame(inst);
    s = save_of(inst, &len);
    CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &data, &n),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(n, SPAN_8K);
    data[5] ^= 0xffu;
    g = nesturbator_save_generation(inst);
    CHECK_EQ_U64(nesturbator_load_state(inst, s, len), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_generation(inst), g + 1u);
    CHECK_EQ_HEX(data[5], 0x00u);
    CHECK_EQ_U64(nesturbator_load_state(inst, s, len), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_generation(inst), g + 1u);
    free(s);
    nesturbator_destroy(inst);
}

/* (a) Success criterion 5: 600 loads of a state whose span matches the instance's leave the
   generation unchanged after every load, the first included. */
static void test_matching_span_600_loads(void)
{
    static uint8_t image[IMAGE_CAP];
    size_t size = build_image(image, 1u, 0u, 7u, 1u);
    nesturbator *inst = make_loaded(image, size);
    uint8_t *s;
    size_t len;
    uint64_t g;
    run_frame(inst);
    s = save_of(inst, &len);
    g = nesturbator_save_generation(inst);
    CHECK(g != 0u);
    for (uint32_t i = 0u; i < LOADS; ++i) {
        CHECK_EQ_U64(nesturbator_load_state(inst, s, len), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_save_generation(inst), g);
    }
    free(s);
    nesturbator_destroy(inst);
}

/* (c) Work RAM outside the span differs, the span matches: no count. The span, not the PRG-RAM
   allocation, is compared. */
static void test_work_ram_outside_span(void)
{
    static uint8_t image[IMAGE_CAP];
    size_t size = build_image(image, 1u, 7u, 7u, 1u);
    nesturbator *inst = make_loaded(image, size);
    struct nesturbator *nes = inst;
    uint8_t *s;
    size_t len;
    uint64_t g;
    CHECK_EQ_U64((uint64_t)(nes->cart.save - nes->cart.prg_ram), SPAN_8K);
    CHECK_EQ_U64(nes->cart.save_size, SPAN_8K);
    s = save_of(inst, &len);
    g = nesturbator_save_generation(inst);
    nes->cart.prg_ram[3] = 0x77u;
    CHECK_EQ_U64(nesturbator_load_state(inst, s, len), NESTURBATOR_OK);
    CHECK_EQ_HEX(nes->cart.prg_ram[3], 0x00u);
    CHECK_EQ_U64(nesturbator_save_generation(inst), g);
    free(s);
    nesturbator_destroy(inst);
}

/* (d) A battery-less NROM cartridge has no span: loads never count. */
static void test_batteryless(void)
{
    static uint8_t image[IMAGE_CAP];
    size_t size = build_image(image, 0u, 0u, 0u, 0u);
    nesturbator *inst = make_loaded(image, size);
    uint8_t *s;
    size_t len;
    run_frame(inst);
    s = save_of(inst, &len);
    run_frame(inst);
    CHECK_EQ_U64(nesturbator_load_state(inst, s, len), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_generation(inst), 0u);
    free(s);
    nesturbator_destroy(inst);
}

/* (e) A refused load never counts, even when its span differs from the instance's. */
static void test_refused_load(void)
{
    static uint8_t image[IMAGE_CAP];
    size_t size = build_image(image, 1u, 0u, 7u, 1u);
    nesturbator *inst = make_loaded(image, size);
    struct nesturbator *nes = inst;
    uint8_t *s;
    size_t len;
    uint64_t g;
    s = save_of(inst, &len);
    nes->cart.save[0] = 0x42u;
    g = nesturbator_save_generation(inst);
    s[len - 1u] ^= 0x01u; /* the checksum no longer matches */
    CHECK_EQ_U64(nesturbator_load_state(inst, s, len), NESTURBATOR_ERR_STATE_CHECKSUM);
    CHECK_EQ_U64(nesturbator_save_generation(inst), g);
    CHECK_EQ_HEX(nes->cart.save[0], 0x42u);
    free(s);
    nesturbator_destroy(inst);
}

/* (f) The instance's own writes raise the generation; a later changed-span load keeps counting
   up from there and never lowers it. */
static void test_counts_up_from_own_writes(void)
{
    static uint8_t image[IMAGE_CAP];
    size_t size = build_image(image, 1u, 0u, 7u, 1u);
    nesturbator *inst = make_loaded(image, size);
    uint8_t *s;
    size_t len;
    uint64_t g;
    s = save_of(inst, &len); /* before the program runs: span all zero */
    run_frame(inst);
    g = nesturbator_save_generation(inst);
    CHECK_EQ_U64(g, 2u);
    CHECK_EQ_U64(nesturbator_load_state(inst, s, len), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_generation(inst), g + 1u);
    free(s);
    nesturbator_destroy(inst);
}

int main(void)
{
    test_changed_span_counts_once();
    test_matching_span_600_loads();
    test_work_ram_outside_span();
    test_batteryless();
    test_refused_load();
    test_counts_up_from_own_writes();
    CHECK_DONE();
}
