/* Lifecycle qualification and negative controls for the exact replay verdict. */
#include "internal.h"
#include "../check.h"

int nt_exec_embedded_main(int argc, char **argv);
#define main nt_exec_embedded_main
#include "execution.c"
#undef main

#define GUARD_PIXELS 4u
#define GUARD_SAMPLES 4u
#define GUARD_PIXEL 0xcafeu
#define GUARD_SAMPLE 0x5aa5u

struct guarded_buffers {
    uint16_t video[GUARD_PIXELS + 256u * 240u + GUARD_PIXELS];
    int16_t audio[GUARD_SAMPLES + NT_EXEC_AUDIO_SAMPLES + GUARD_SAMPLES];
};

struct fixture {
    struct nt_exec_budget budget;
    struct nt_exec_metrics metrics;
    struct nt_exec_profile profile;
    uint8_t seed[NT_EXEC_RECIPE_SIZE];
    uint8_t *image;
    size_t image_size;
    nesturbator *inst;
};

enum verdict_control {
    VERDICT_VALID,
    VERDICT_SUPPRESSED_EXECUTION,
    VERDICT_SKIPPED_RESTORE,
    VERDICT_WRONG_ORIGIN,
    VERDICT_SHARED_INSTANCE
};

static void guard_reset(struct guarded_buffers *buffers)
{
    for (size_t i = 0u; i < GUARD_PIXELS; ++i) {
        buffers->video[i] = GUARD_PIXEL;
        buffers->video[GUARD_PIXELS + 256u * 240u + i] = GUARD_PIXEL;
    }
    for (size_t i = 0u; i < GUARD_SAMPLES; ++i) {
        buffers->audio[i] = GUARD_SAMPLE;
        buffers->audio[GUARD_SAMPLES + NT_EXEC_AUDIO_SAMPLES + i] = GUARD_SAMPLE;
    }
}

static int guard_intact(const struct guarded_buffers *buffers)
{
    for (size_t i = 0u; i < GUARD_PIXELS; ++i) {
        if (buffers->video[i] != GUARD_PIXEL ||
            buffers->video[GUARD_PIXELS + 256u * 240u + i] != GUARD_PIXEL)
            return 0;
    }
    for (size_t i = 0u; i < GUARD_SAMPLES; ++i) {
        if (buffers->audio[i] != GUARD_SAMPLE ||
            buffers->audio[GUARD_SAMPLES + NT_EXEC_AUDIO_SAMPLES + i] != GUARD_SAMPLE)
            return 0;
    }
    return 1;
}

static void fixture_make(struct fixture *fixture, uint8_t profile_code)
{
    struct nt_exec_recipe recipe;
    memset(fixture, 0, sizeof *fixture);
    nt_exec_seed(fixture->seed);
    fixture->seed[5] = profile_code;
    CHECK(nt_exec_decode(fixture->seed, sizeof fixture->seed, &recipe));
    CHECK(nt_exec_profile_for(profile_code, &fixture->profile));
    fixture->image = nt_exec_generate(&recipe, fixture->seed, sizeof fixture->seed,
                                      &fixture->profile, &fixture->image_size,
                                      &fixture->budget);
    CHECK(fixture->image != NULL);
    if (fixture->image != NULL)
        fixture->inst = nt_exec_instance(&fixture->budget, fixture->image,
                                         fixture->image_size, &fixture->metrics);
}

static void fixture_drop(struct fixture *fixture)
{
    if (fixture->inst != NULL) {
        nesturbator_unload_cartridge(fixture->inst);
        nesturbator_destroy(fixture->inst);
        fixture->inst = NULL;
    }
    nt_exec_free(&fixture->budget, fixture->image, fixture->image_size);
    fixture->image = NULL;
    CHECK_EQ_U64(fixture->budget.current, 0u);
}

static void save_bytes(nesturbator *inst, uint8_t *bytes, size_t size)
{
    CHECK_EQ_U64(nesturbator_state_size(inst), size);
    CHECK_EQ_U64(nesturbator_save_state(inst, bytes, size), NESTURBATOR_OK);
}

static int branch_equal(const struct nt_exec_branch *a, const struct nt_exec_branch *b)
{
    return a->frames == b->frames && a->stop_status == b->stop_status &&
           a->output_hash == b->output_hash && a->guest_ticks == b->guest_ticks &&
           a->prg_marker == b->prg_marker && a->chr_marker == b->chr_marker &&
           a->guest_counter == b->guest_counter && a->nametable_marker == b->nametable_marker &&
           a->controller_a == b->controller_a && a->visible_pixels == b->visible_pixels;
}

static int observation_equal(const struct nt_exec_observation *a,
                             const struct nt_exec_observation *b)
{
    if (a->state_hash_a != b->state_hash_a || a->state_hash_b != b->state_hash_b ||
        a->origin_state_hash != b->origin_state_hash || a->states_equal != b->states_equal ||
        a->mapper != b->mapper || a->accepted_status != b->accepted_status ||
        a->completed_frames != b->completed_frames || a->operation_count != b->operation_count ||
        a->bank_effects != b->bank_effects || !branch_equal(&a->continuation_a, &b->continuation_a) ||
        !branch_equal(&a->continuation_b, &b->continuation_b))
        return 0;
    if (a->metrics.accepted != b->metrics.accepted || a->metrics.frames != b->metrics.frames ||
        a->metrics.frame_ok != b->metrics.frame_ok || a->metrics.frame_jam != b->metrics.frame_jam ||
        a->metrics.operations != b->metrics.operations ||
        a->metrics.generated_bytes != b->metrics.generated_bytes ||
        a->metrics.state_bytes != b->metrics.state_bytes ||
        a->metrics.guest_ticks != b->metrics.guest_ticks ||
        a->metrics.stop_status != b->metrics.stop_status || a->metrics.mapper != b->metrics.mapper ||
        a->metrics.submapper != b->metrics.submapper ||
        a->metrics.qualified_bank_effects != b->metrics.qualified_bank_effects)
        return 0;
    for (uint32_t i = 0u; i < a->metrics.operations; ++i) {
        if (a->metrics.operation_indices[i] != b->metrics.operation_indices[i])
            return 0;
    }
    return 1;
}

static void run_guarded_frame(nesturbator *inst, uint8_t buttons, uint64_t *ticks,
                              struct nt_exec_branch *branch, struct nt_exec_metrics *metrics,
                              struct guarded_buffers *buffers)
{
    guard_reset(buffers);
    CHECK(nt_exec_run_phase(inst, buttons, 1u, buffers->video + GUARD_PIXELS,
                            buffers->audio + GUARD_SAMPLES, ticks, branch, metrics));
    CHECK(guard_intact(buffers));
}

static int mapper_fields_equal(uint16_t mapper, const struct nesturbator__mapper *a,
                               const struct nesturbator__mapper *b)
{
    if (a->id != b->id || a->submapper != b->submapper || a->irq != b->irq)
        return 0;
    switch (mapper) {
    case 1u:
        return a->reg.mmc1.shift == b->reg.mmc1.shift &&
               a->reg.mmc1.count == b->reg.mmc1.count &&
               a->reg.mmc1.control_x == b->reg.mmc1.control_x &&
               a->reg.mmc1.chr0 == b->reg.mmc1.chr0 && a->reg.mmc1.chr1 == b->reg.mmc1.chr1 &&
               a->reg.mmc1.prg == b->reg.mmc1.prg &&
               a->reg.mmc1.last_write == b->reg.mmc1.last_write;
    case 2u:
        return a->reg.uxrom.bank == b->reg.uxrom.bank;
    case 3u:
        return a->reg.cnrom.bank == b->reg.cnrom.bank;
    case 4u:
        return a->reg.mmc3.select == b->reg.mmc3.select &&
               memcmp(a->reg.mmc3.r, b->reg.mmc3.r, sizeof a->reg.mmc3.r) == 0 &&
               a->reg.mmc3.mirror == b->reg.mmc3.mirror &&
               a->reg.mmc3.a001_x == b->reg.mmc3.a001_x &&
               a->reg.mmc3.latch == b->reg.mmc3.latch &&
               a->reg.mmc3.counter == b->reg.mmc3.counter &&
               a->reg.mmc3.reload == b->reg.mmc3.reload &&
               a->reg.mmc3.irq_enable == b->reg.mmc3.irq_enable &&
               a->reg.mmc3.a12 == b->reg.mmc3.a12 &&
               a->reg.mmc3.low_cycle == b->reg.mmc3.low_cycle;
    case 7u:
        return a->reg.axrom.bank == b->reg.axrom.bank;
    default:
        return 1;
    }
}

static void alter_mapper(struct nesturbator *nes)
{
    switch (nes->mapper.id) {
    case 1u:
        nes->mapper.reg.mmc1.prg = 0u;
        nes->mapper.reg.mmc1.chr0 = 1u;
        break;
    case 2u:
        nes->mapper.reg.uxrom.bank = 2u;
        break;
    case 3u:
        nes->mapper.reg.cnrom.bank = 1u;
        break;
    case 4u:
        nes->mapper.reg.mmc3.r[6] = 2u;
        nes->mapper.reg.mmc3.r[2] = 2u;
        break;
    case 7u:
        nes->mapper.reg.axrom.bank = 0x11u; /* Bank 1, single-screen page 1. */
        break;
    default:
        return;
    }
    nes->map.ops.rebuild(nes);
}

static void test_lifecycle_profile(uint8_t profile_code)
{
    struct fixture fixture;
    struct guarded_buffers buffers;
    struct nt_exec_branch branch;
    struct nesturbator *nes;
    struct nesturbator__mapper mapper_before;
    uint8_t *state = NULL;
    uint8_t *state_after = NULL;
    uint8_t *bad_state = NULL;
    uint8_t *span = NULL;
    uint8_t *reported_span = NULL;
    size_t state_size = 0u;
    size_t span_size = 0u;
    size_t reported_span_size = 0u;
    uint64_t ticks;
    uint64_t frames;
    uint64_t generation;
    uint64_t generation_before_unload;
    uint64_t guest_ticks = 0u;
    uint8_t expected_ram = 0x6cu;
    uint8_t expected_prg_ram = 0x9bu;
    uint8_t expected_chr_ram = 0xa6u;
    nesturbator *fresh = NULL;
    uint8_t *fresh_state = NULL;
    size_t fresh_state_size = 0u;
    struct nt_exec_metrics fresh_metrics;

    memset(&fresh_metrics, 0, sizeof fresh_metrics);
    memset(&branch, 0, sizeof branch);
    fixture_make(&fixture, profile_code);
    CHECK(fixture.inst != NULL);
    if (fixture.inst == NULL || fixture.image == NULL)
        return;
    run_guarded_frame(fixture.inst, fixture.seed[10], &guest_ticks, &branch,
                      &fixture.metrics, &buffers);
    nes = (struct nesturbator *)fixture.inst;
    state_size = nesturbator_state_size(fixture.inst);
    state = (uint8_t *)malloc(state_size);
    state_after = (uint8_t *)malloc(state_size);
    bad_state = (uint8_t *)malloc(state_size);
    CHECK(state != NULL && state_after != NULL && bad_state != NULL);
    if (state == NULL || state_after == NULL || bad_state == NULL)
        goto done;
    save_bytes(fixture.inst, state, state_size);

    /* A refused cartridge load is atomic for both machine bytes and the live span. */
    CHECK_EQ_U64(nesturbator_get_memory(fixture.inst, NESTURBATOR_MEMORY_SAVE_RAM,
                                        &span, &span_size), NESTURBATOR_OK);
    save_bytes(fixture.inst, state_after, state_size);
    CHECK_EQ_U64(memcmp(state, state_after, state_size), 0u);
    {
        static const uint8_t invalid_header[16] = {0u};
        CHECK_EQ_U64(nesturbator_load_cartridge(fixture.inst, invalid_header,
                                                 sizeof invalid_header),
                     NESTURBATOR_ERR_CARTRIDGE);
    }
    save_bytes(fixture.inst, state_after, state_size);
    CHECK_EQ_U64(memcmp(state, state_after, state_size), 0u);
    reported_span = NULL;
    reported_span_size = 0u;
    CHECK_EQ_U64(nesturbator_get_memory(fixture.inst, NESTURBATOR_MEMORY_SAVE_RAM,
                                        &reported_span, &reported_span_size), NESTURBATOR_OK);
    CHECK(reported_span == span);
    CHECK_EQ_U64(reported_span_size, span_size);

    /* Checksum refusal leaves canonical machine state and battery generation untouched. */
    memcpy(bad_state, state, state_size);
    bad_state[state_size - 1u] ^= 1u;
    generation = nesturbator_save_generation(fixture.inst);
    CHECK_EQ_U64(nesturbator_load_state(fixture.inst, bad_state, state_size),
                 NESTURBATOR_ERR_STATE_CHECKSUM);
    save_bytes(fixture.inst, state_after, state_size);
    CHECK_EQ_U64(memcmp(state, state_after, state_size), 0u);
    CHECK_EQ_U64(nesturbator_save_generation(fixture.inst), generation);

    /* Battery generation is per-instance and changes only with writes/state-span changes. */
    if (span != NULL && span_size > 0u) {
        const uint64_t before_host_write = nesturbator_save_generation(fixture.inst);
        CHECK(before_host_write > 0u);
        span[0] ^= 0x5au;
        CHECK_EQ_U64(nesturbator_save_generation(fixture.inst), before_host_write);
        CHECK_EQ_U64(nesturbator_load_state(fixture.inst, state, state_size), NESTURBATOR_OK);
        generation = nesturbator_save_generation(fixture.inst);
        CHECK_EQ_U64(generation, before_host_write + 1u);
        CHECK_EQ_U64(nesturbator_load_state(fixture.inst, state, state_size), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_save_generation(fixture.inst), generation);
    } else {
        CHECK_EQ_U64(nesturbator_save_generation(fixture.inst), 0u);
    }
    save_bytes(fixture.inst, state_after, state_size);
    CHECK_EQ_U64(memcmp(state, state_after, state_size), 0u);
    CHECK_EQ_U64(nesturbator_load_state(fixture.inst, state, state_size), NESTURBATOR_OK);
    save_bytes(fixture.inst, state_after, state_size);
    CHECK_EQ_U64(memcmp(state, state_after, state_size), 0u);

    /* Alter retained RAM and board registers before soft reset. AxROM's selected
       bank has a valid reset vector so this checks current-bank vector lookup. */
    nes->bus.ram[0x0123u] = expected_ram;
    if (nes->cart.prg_ram_size > 0u)
        nes->cart.prg_ram[0] = expected_prg_ram;
    if (nes->cart.chr_is_ram != 0u)
        nes->cart.chr[17] = expected_chr_ram;
    alter_mapper(nes);
    memcpy(&mapper_before, &nes->mapper, sizeof mapper_before);
    nes->cpu.jammed = 1u;
    ticks = nes->ticks;
    frames = nes->frame_number;
    generation = nesturbator_save_generation(fixture.inst);
    CHECK_EQ_U64(nesturbator_reset(fixture.inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nes->ticks, ticks + 168u);
    CHECK_EQ_U64(nes->frame_number, frames);
    CHECK_EQ_U64(nes->cpu.pc, fixture.profile.code_address);
    CHECK_EQ_U64(nes->cpu.jammed, 0u);
    CHECK_EQ_HEX(nes->bus.ram[0x0123u], expected_ram);
    if (nes->cart.prg_ram_size > 0u)
        CHECK_EQ_HEX(nes->cart.prg_ram[0], expected_prg_ram);
    if (nes->cart.chr_is_ram != 0u)
        CHECK_EQ_HEX(nes->cart.chr[17], expected_chr_ram);
    CHECK(mapper_fields_equal(nes->mapper.id, &mapper_before, &nes->mapper));
    CHECK_EQ_U64(nesturbator_save_generation(fixture.inst), generation);

    /* Never retain or dereference `span` after unload. Reload must match a fresh
       instance's canonical load state and keep the instance-lifetime counter. */
    generation_before_unload = nesturbator_save_generation(fixture.inst);
    span = NULL;
    span_size = 0u;
    nesturbator_unload_cartridge(fixture.inst);
    CHECK_EQ_U64(nesturbator_state_size(fixture.inst), 0u);
    CHECK_EQ_U64(nes->ticks, 0u);
    CHECK_EQ_U64(nes->frame_number, 0u);
    reported_span = (uint8_t *)(uintptr_t)1u;
    reported_span_size = 99u;
    CHECK_EQ_U64(nesturbator_get_memory(fixture.inst, NESTURBATOR_MEMORY_SAVE_RAM,
                                        &reported_span, &reported_span_size), NESTURBATOR_OK);
    CHECK(reported_span == NULL);
    CHECK_EQ_U64(reported_span_size, 0u);
    CHECK_EQ_U64(nesturbator_load_cartridge(fixture.inst, fixture.image,
                                             fixture.image_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_generation(fixture.inst), generation_before_unload);
    reported_span = NULL;
    reported_span_size = 0u;
    CHECK_EQ_U64(nesturbator_get_memory(fixture.inst, NESTURBATOR_MEMORY_SAVE_RAM,
                                        &reported_span, &reported_span_size), NESTURBATOR_OK);
    if (fixture.profile.battery != 0u) {
        CHECK(reported_span != NULL);
        CHECK_EQ_U64(reported_span_size, 8192u);
    } else {
        CHECK(reported_span == NULL);
        CHECK_EQ_U64(reported_span_size, 0u);
    }
    fresh = nt_exec_instance(&fixture.budget, fixture.image, fixture.image_size, &fresh_metrics);
    fresh_state_size = nesturbator_state_size(fresh);
    fresh_state = (uint8_t *)malloc(fresh_state_size);
    CHECK(fresh_state != NULL);
    if (fresh_state != NULL) {
        uint8_t *reloaded_state = (uint8_t *)malloc(fresh_state_size);
        CHECK(reloaded_state != NULL);
        if (reloaded_state != NULL) {
            save_bytes(fixture.inst, reloaded_state, fresh_state_size);
            save_bytes(fresh, fresh_state, fresh_state_size);
            CHECK_EQ_U64(memcmp(reloaded_state, fresh_state, fresh_state_size), 0u);
        }
        free(reloaded_state);
    }
    CHECK_EQ_U64(nesturbator_save_generation(fresh), 0u);

done:
    free(state);
    free(state_after);
    free(bad_state);
    free(fresh_state);
    if (fresh != NULL) {
        nesturbator_unload_cartridge(fresh);
        nesturbator_destroy(fresh);
    }
    fixture_drop(&fixture);
}

static int verdict_control(enum verdict_control control)
{
    struct fixture fixture;
    struct nt_exec_metrics metrics;
    struct nt_exec_branch before;
    struct nt_exec_branch branch_a;
    struct nt_exec_branch branch_b;
    struct guarded_buffers buffers;
    struct nesturbator *a;
    struct nesturbator *b;
    uint8_t *origin;
    uint8_t *correct_origin = NULL;
    uint8_t *state_a;
    uint8_t *state_b;
    size_t state_size;
    uint64_t ticks_a = 0u;
    uint64_t ticks_b = 0u;
    int verdict;

    memset(&metrics, 0, sizeof metrics);
    memset(&before, 0, sizeof before);
    memset(&branch_a, 0, sizeof branch_a);
    memset(&branch_b, 0, sizeof branch_b);
    fixture_make(&fixture, NT_EXEC_PROFILE_NROM);
    a = fixture.inst;
    b = nt_exec_instance(&fixture.budget, fixture.image, fixture.image_size, &metrics);
    state_size = nesturbator_state_size(a);
    origin = (uint8_t *)malloc(state_size);
    if (control == VERDICT_WRONG_ORIGIN)
        correct_origin = (uint8_t *)malloc(state_size);
    state_a = (uint8_t *)malloc(state_size);
    state_b = (uint8_t *)malloc(state_size);
    CHECK(origin != NULL && state_a != NULL && state_b != NULL &&
          (control != VERDICT_WRONG_ORIGIN || correct_origin != NULL));
    if (origin == NULL || state_a == NULL || state_b == NULL ||
        (control == VERDICT_WRONG_ORIGIN && correct_origin == NULL)) {
        free(origin); free(correct_origin); free(state_a); free(state_b);
        nesturbator_destroy(b);
        fixture_drop(&fixture);
        return 0;
    }
    run_guarded_frame(a, 0x01u, &ticks_a, &before, &metrics, &buffers);
    save_bytes(a, origin, state_size);
    if (control == VERDICT_WRONG_ORIGIN) {
        memcpy(correct_origin, origin, state_size);
        run_guarded_frame(a, 0x01u, &ticks_a, &before, &metrics, &buffers);
        save_bytes(a, origin, state_size);
        CHECK_EQ_U64(nesturbator_load_state(a, correct_origin, state_size), NESTURBATOR_OK);
        ticks_a = ((struct nesturbator *)a)->ticks;
    }
    if (control != VERDICT_SKIPPED_RESTORE)
        CHECK_EQ_U64(nesturbator_load_state(b, origin, state_size), NESTURBATOR_OK);
    if (control == VERDICT_SUPPRESSED_EXECUTION) {
        ticks_b = ((struct nesturbator *)b)->ticks;
        run_guarded_frame(b, 0x80u, &ticks_b, &branch_b, &metrics, &buffers);
        memcpy(state_a, origin, state_size); /* A's guest continuation was deliberately omitted. */
        save_bytes(b, state_b, state_size);
    } else if (control == VERDICT_SHARED_INSTANCE) {
        ticks_b = ticks_a;
        run_guarded_frame(a, 0x80u, &ticks_a, &branch_a, &metrics, &buffers);
        ticks_b = ticks_a;
        run_guarded_frame(a, 0x80u, &ticks_b, &branch_b, &metrics, &buffers);
        save_bytes(a, state_a, state_size);
        save_bytes(a, state_b, state_size);
    } else {
        ticks_b = ((struct nesturbator *)b)->ticks;
        run_guarded_frame(a, 0x80u, &ticks_a, &branch_a, &metrics, &buffers);
        run_guarded_frame(b, 0x80u, &ticks_b, &branch_b, &metrics, &buffers);
        save_bytes(a, state_a, state_size);
        save_bytes(b, state_b, state_size);
    }
    verdict = nt_exec_verdict(&branch_a, &branch_b, state_a, state_b, state_size);
    free(origin); free(correct_origin); free(state_a); free(state_b);
    nesturbator_unload_cartridge(b);
    nesturbator_destroy(b);
    fixture_drop(&fixture);
    return verdict;
}

static void test_verdict_controls(void)
{
    CHECK(verdict_control(VERDICT_VALID));
    CHECK(!verdict_control(VERDICT_SUPPRESSED_EXECUTION));
    CHECK(!verdict_control(VERDICT_SKIPPED_RESTORE));
    CHECK(!verdict_control(VERDICT_WRONG_ORIGIN));
    CHECK(!verdict_control(VERDICT_SHARED_INSTANCE));
}

static void test_recipe_order_and_repeat(void)
{
    uint8_t seed[NT_EXEC_RECIPE_SIZE];
    struct nt_exec_recipe recipe;
    struct nt_exec_profile profile;
    struct nt_exec_budget budget = {0u, 0u};
    struct nt_exec_observation a_first;
    struct nt_exec_observation b_middle;
    struct nt_exec_observation a_last;
    uint8_t *first;
    uint8_t *second;
    size_t first_size = 0u;
    size_t second_size = 0u;

    nt_exec_seed(seed);
    CHECK(nt_exec_decode(seed, sizeof seed, &recipe));
    CHECK(nt_exec_profile_for(recipe.profile_code, &profile));
    first = nt_exec_generate(&recipe, seed, sizeof seed, &profile, &first_size, &budget);
    second = nt_exec_generate(&recipe, seed, sizeof seed, &profile, &second_size, &budget);
    CHECK(first != NULL && second != NULL);
    CHECK_EQ_U64(first_size, second_size);
    CHECK_EQ_U64(memcmp(first, second, first_size), 0u);
    CHECK(nt_exec_process(seed, sizeof seed, &a_first));
    seed[5] = NT_EXEC_PROFILE_MMC1;
    CHECK(nt_exec_process(seed, sizeof seed, &b_middle));
    seed[5] = NT_EXEC_PROFILE_NROM;
    CHECK(nt_exec_process(seed, sizeof seed, &a_last));
    CHECK(observation_equal(&a_first, &a_last));
    nt_exec_free(&budget, first, first_size);
    nt_exec_free(&budget, second, second_size);
    CHECK_EQ_U64(budget.current, 0u);
}

int main(void)
{
    uint8_t seed[NT_EXEC_RECIPE_SIZE];
    const uint8_t profiles[] = {NT_EXEC_PROFILE_NROM, NT_EXEC_PROFILE_MMC1,
                                NT_EXEC_PROFILE_UXROM, NT_EXEC_PROFILE_CNROM,
                                NT_EXEC_PROFILE_MMC3, NT_EXEC_PROFILE_AXROM};
    CHECK_EQ_U64(nt_exec_self_test(), 0u);
    test_verdict_controls();
    test_recipe_order_and_repeat();
    for (size_t i = 0u; i < sizeof profiles / sizeof profiles[0]; ++i) {
        struct nt_exec_observation observed;
        nt_exec_seed(seed);
        seed[5] = profiles[i];
        CHECK(nt_exec_process(seed, sizeof seed, &observed));
        CHECK(observed.states_equal);
        CHECK(observed.origin_state_hash != 0u);
        CHECK(observed.metrics.frame_jam == 0u);
        CHECK(observed.metrics.frame_ok >= 4u);
        CHECK(observed.metrics.operations > 0u);
        test_lifecycle_profile(profiles[i]);
    }
    puts("fuzz.execution.controls: real verdict controls and six-family lifecycle passed");
    CHECK_DONE();
}
