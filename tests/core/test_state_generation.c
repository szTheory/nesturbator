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
#include "../state_test.h"

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
    spec.chr_8k = mapper == 3u ? 1u : 0u;
    spec.nes2 = 1u;
    spec.mapper = mapper;
    spec.battery = battery;
    spec.prg_ram_shift = ram_shift;
    spec.prg_nvram_shift = nvram_shift;
    spec.segments = &seg;
    spec.segment_count = 1u;
    spec.nmi_vector = 0x8134u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8200u;
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
    nesturbator_status status = nesturbator_load_cartridge(inst, image, size);
    if (status != NESTURBATOR_OK)
        fprintf(stderr, "synthetic mapper %u image rejected: header=%02x %02x %02x %02x %02x\n",
                (unsigned)((image[6] >> 4) | (image[7] & 0xf0u)), image[6], image[7], image[8],
                image[10], image[11]);
    CHECK_EQ_U64(status, NESTURBATOR_OK);
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

static int entry_cutpoint_variant(struct nesturbator *nes, enum nesturbator_entry_origin origin,
                                  uint8_t next_action, uint8_t select_nmi)
{
    uint8_t first = origin == NESTURBATOR_ENTRY_BRK ? 2u : 1u;
    if (next_action < first || next_action > 7u)
        return 0;
    nesturbator__cpu_entry_begin(nes, origin);
    while (nes->cpu.entry_origin != NESTURBATOR_ENTRY_NONE &&
           nes->cpu.entry_next_action < next_action) {
        if (select_nmi != 0u && nes->cpu.entry_next_action == 4u)
            st_assert_nmi_pin(nes);
        if (nesturbator__cpu_entry_action(nes) == 0u)
            return 0;
    }
    return (int)(nes->cpu.entry_origin == (uint8_t)origin &&
                 nes->cpu.entry_next_action == next_action);
}

/* Reset is compared with the same machine state after only its entry
   continuation is cleared. The active state itself is always made by the
   production entry actions and is loaded through the public state API. */
static void check_reset_cutpoint(const uint8_t *image, size_t image_size,
                                 enum nesturbator_entry_origin origin, uint8_t next_action,
                                 uint8_t select_nmi)
{
    nesturbator *source = make_loaded(image, image_size);
    nesturbator *control = NULL;
    uint8_t *active = NULL;
    uint8_t *source_after = NULL;
    uint8_t *control_after = NULL;
    uint8_t ram_before[2048];
    size_t state_size;
    uint8_t stack_before;
    uint8_t a_before, x_before, y_before;
    uint64_t ticks_before, cycles_before, generation_before;
    uint64_t reset_ticks, reset_cycles;

    CHECK(source != NULL);
    if (source == NULL)
        return;
    state_size = nesturbator_state_size(source);
    active = (uint8_t *)malloc(state_size);
    source_after = (uint8_t *)malloc(state_size);
    control_after = (uint8_t *)malloc(state_size);
    CHECK(state_size != 0u && active != NULL && source_after != NULL && control_after != NULL);
    if (state_size == 0u || active == NULL || source_after == NULL || control_after == NULL)
        goto done;

    CHECK_EQ_U64(nesturbator_reset(source), NESTURBATOR_OK);
    CHECK(entry_cutpoint_variant((struct nesturbator *)source, origin, next_action, select_nmi));
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu.entry_origin, origin);
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu.entry_next_action, next_action);
    CHECK_EQ_U64(nesturbator_save_state(source, active, state_size), NESTURBATOR_OK);

    control = make_loaded(image, image_size);
    CHECK(control != NULL);
    if (control == NULL)
        goto done;
    CHECK_EQ_U64(nesturbator_load_state(control, active, state_size), NESTURBATOR_OK);
    ((struct nesturbator *)control)->cpu.entry_origin = NESTURBATOR_ENTRY_NONE;
    ((struct nesturbator *)control)->cpu.entry_next_action = 0u;
    ((struct nesturbator *)control)->cpu.entry_vector = NESTURBATOR_ENTRY_VECTOR_NONE;
    ((struct nesturbator *)control)->cpu.entry_vector_low = 0u;

    memcpy(ram_before, ((struct nesturbator *)source)->bus.ram, sizeof ram_before);
    stack_before = ((struct nesturbator *)source)->cpu.s;
    a_before = ((struct nesturbator *)source)->cpu.a;
    x_before = ((struct nesturbator *)source)->cpu.x;
    y_before = ((struct nesturbator *)source)->cpu.y;
    ticks_before = ((struct nesturbator *)source)->ticks;
    cycles_before = ((struct nesturbator *)source)->cpu_cycle;
    generation_before = nesturbator_save_generation(source);

    CHECK_EQ_U64(nesturbator_reset(source), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_reset(control), NESTURBATOR_OK);
    reset_ticks = ((struct nesturbator *)source)->ticks;
    reset_cycles = ((struct nesturbator *)source)->cpu_cycle;
    CHECK_EQ_U64(reset_ticks - ticks_before, 7u * 24u);
    CHECK_EQ_U64(reset_cycles - cycles_before, 7u);
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu.pc, 0x8000u);
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu.s, (uint8_t)(stack_before - 3u));
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu.a, a_before);
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu.x, x_before);
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu.y, y_before);
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu.p & 0x04u, 0x04u);
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu.entry_origin, NESTURBATOR_ENTRY_NONE);
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu.entry_next_action, 0u);
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu.entry_vector, NESTURBATOR_ENTRY_VECTOR_NONE);
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu.entry_vector_low, 0u);
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu.jammed, 0u);
    CHECK(memcmp(ram_before, ((struct nesturbator *)source)->bus.ram, sizeof ram_before) == 0);
    CHECK_EQ_U64(nesturbator_save_generation(source), generation_before);
    CHECK_EQ_U64(nesturbator_save_generation(control), generation_before);
    CHECK_EQ_U64(nesturbator_save_state(source, source_after, state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(control, control_after, state_size), NESTURBATOR_OK);
    CHECK(memcmp(source_after, control_after, state_size) == 0);

    /* Reset-vector code is LDA #$A5; an ordinary two-cycle instruction must
       run next, with no entry push/service resuming and no stack write. */
    nesturbator__cpu_step((struct nesturbator *)source);
    nesturbator__cpu_step((struct nesturbator *)control);
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu.pc, 0x8002u);
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu.a, 0xa5u);
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu.s, (uint8_t)(stack_before - 3u));
    CHECK_EQ_U64(((struct nesturbator *)source)->ticks - reset_ticks, 2u * 24u);
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu_cycle - reset_cycles, 2u);
    CHECK_EQ_U64(((struct nesturbator *)source)->cpu.entry_origin, NESTURBATOR_ENTRY_NONE);
    CHECK(memcmp(ram_before, ((struct nesturbator *)source)->bus.ram, sizeof ram_before) == 0);
    CHECK_EQ_U64(nesturbator_save_state(source, source_after, state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(control, control_after, state_size), NESTURBATOR_OK);
    CHECK(memcmp(source_after, control_after, state_size) == 0);

done:
    if (control != NULL)
        nesturbator_destroy(control);
    if (source != NULL)
        nesturbator_destroy(source);
    free(control_after);
    free(source_after);
    free(active);
}

static void test_active_entry_reset_matrix(void)
{
    static uint8_t image[IMAGE_CAP];
    size_t size = build_image(image, 0u, 0u, 0u, 0u);
    CHECK(size != 0u);
    if (size == 0u)
        return;

    for (enum nesturbator_entry_origin origin = NESTURBATOR_ENTRY_BRK;
         origin <= NESTURBATOR_ENTRY_NMI; ++origin) {
        uint8_t first = origin == NESTURBATOR_ENTRY_BRK ? 2u : 1u;
        for (uint8_t select_nmi = 0u; select_nmi <= 1u; ++select_nmi) {
            if (origin == NESTURBATOR_ENTRY_NMI && select_nmi != 0u)
                continue;
            for (uint8_t next = first; next <= 7u; ++next) {
                if (select_nmi != 0u && next < 5u)
                    continue;
                check_reset_cutpoint(image, size, origin, next, select_nmi);
            }
        }
    }
    printf("CASE reset aborts every reachable BRK/IRQ/NMI entry cutpoint and selected pair\n");
}

static void test_six_family_generation_controls(void)
{
    static uint8_t image[IMAGE_CAP];
    static const uint8_t mappers[] = {0u, 1u, 2u, 3u, 4u, 7u};
    for (size_t i = 0u; i < sizeof mappers / sizeof mappers[0]; ++i) {
        size_t image_size = build_image(image, mappers[i], 0u, 0u, 0u);
        nesturbator *inst;
        uint8_t *state;
        uint8_t *bad;
        size_t state_size;
        uint64_t generation;
        struct st_snap snap;

        CHECK(image_size != 0u);
        if (image_size == 0u)
            continue;
        inst = make_loaded(image, image_size);
        CHECK(inst != NULL);
        if (inst == NULL)
            continue;
        state_size = nesturbator_state_size(inst);
        state = (uint8_t *)malloc(state_size);
        bad = (uint8_t *)malloc(state_size);
        CHECK(state != NULL && bad != NULL);
        if (state == NULL || bad == NULL) {
            free(state);
            free(bad);
            nesturbator_destroy(inst);
            continue;
        }
        CHECK_EQ_U64(nesturbator_save_state(inst, state, state_size), NESTURBATOR_OK);
        generation = nesturbator_save_generation(inst);
        CHECK_EQ_U64(nesturbator_load_state(inst, state, state_size), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_save_generation(inst), generation);
        memcpy(bad, state, state_size);
        bad[state_size - 1u] ^= 1u;
        st_snapshot(inst, &snap);
        CHECK(st_refused_with(inst, &snap, generation, bad, state_size,
                              NESTURBATOR_ERR_STATE_CHECKSUM, "six-family refused load"));
        st_snap_free(&snap);
        CHECK_EQ_U64(nesturbator_save_generation(inst), generation);
        printf("row generation mapper=%u same-state=unchanged refused=unchanged\n",
               (unsigned)mappers[i]);
        free(bad);
        free(state);
        nesturbator_destroy(inst);
    }
}

int main(void)
{
    test_changed_span_counts_once();
    test_matching_span_600_loads();
    test_work_ram_outside_span();
    test_batteryless();
    test_refused_load();
    test_counts_up_from_own_writes();
    test_six_family_generation_controls();
    test_active_entry_reset_matrix();
    CHECK_DONE();
}
