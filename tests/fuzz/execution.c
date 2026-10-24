/* Bounded generated-program execution and save-state continuation fuzz target.
 * The recipe stores descriptors only; its NROM image is generated in memory. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nesturbator.h"

#define NT_EXEC_REPLAY_NAME "fuzz.execution.regress"
#define NT_EXEC_MAGIC_SIZE 4u
#define NT_EXEC_RECIPE_SIZE 16u
#define NT_EXEC_MAX_INPUT_BYTES 16u
#define NT_EXEC_MAX_PRG_BYTES 16384u
#define NT_EXEC_MAX_CHR_BYTES 8192u
#define NT_EXEC_MAX_GENERATED_BYTES (16u + NT_EXEC_MAX_PRG_BYTES + NT_EXEC_MAX_CHR_BYTES)
#define NT_EXEC_MAX_STATE_BYTES (256u * 1024u)
#define NT_EXEC_MAX_ALLOCATION_BYTES (2u * 1024u * 1024u)
#define NT_EXEC_MAX_FRAMES_PER_PHASE 2u
#define NT_EXEC_MAX_FRAME_CALLS 6u
#define NT_EXEC_MAX_OPERATIONS 34u
#define NT_EXEC_VIDEO_BYTES (256u * 240u * sizeof(uint16_t))
#define NT_EXEC_AUDIO_SAMPLES 1024u
#define NT_EXEC_AUDIO_BYTES (NT_EXEC_AUDIO_SAMPLES * sizeof(int16_t))

struct nt_exec_budget {
    size_t current;
    size_t peak;
};

struct nt_exec_recipe {
    uint8_t program;
    uint16_t before_frames;
    uint16_t after_frames;
    uint8_t before_buttons;
    uint8_t after_buttons;
    uint32_t seed;
};

struct nt_exec_metrics {
    uint32_t accepted;
    uint32_t frames;
    uint32_t operations;
    uint32_t generated_bytes;
    uint32_t state_bytes;
    uint32_t allocation_peak;
    uint64_t guest_ticks;
    uint32_t stop_status;
};

struct nt_exec_branch {
    uint32_t frames;
    uint32_t stop_status;
    uint64_t output_hash;
    uint64_t guest_ticks;
    uint16_t marker;
    uint8_t controller_a;
};

struct nt_exec_observation {
    struct nt_exec_metrics metrics;
    struct nt_exec_branch continuation_a;
    struct nt_exec_branch continuation_b;
    uint64_t state_hash_a;
    uint64_t state_hash_b;
    int states_equal;
};

enum nt_exec_read_status {
    NT_EXEC_READ_OK,
    NT_EXEC_READ_TOO_LARGE,
    NT_EXEC_READ_INVALID,
    NT_EXEC_READ_MEMORY,
    NT_EXEC_READ_FAILED
};

static void nt_exec_fail(const char *message)
{
    fprintf(stderr, NT_EXEC_REPLAY_NAME ": oracle failed: %s\n", message);
    abort();
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

static void *nt_exec_alloc(void *user, size_t size)
{
    struct nt_exec_budget *budget = (struct nt_exec_budget *)user;
    void *memory;
    if (size == 0u || budget->current > NT_EXEC_MAX_ALLOCATION_BYTES ||
        size > NT_EXEC_MAX_ALLOCATION_BYTES - budget->current)
        return NULL;
    memory = malloc(size);
    if (memory == NULL)
        return NULL;
    budget->current += size;
    if (budget->current > budget->peak)
        budget->peak = budget->current;
    return memory;
}

static void nt_exec_free(void *user, void *memory, size_t size)
{
    struct nt_exec_budget *budget = (struct nt_exec_budget *)user;
    if (memory == NULL)
        return;
    if (size > budget->current)
        nt_exec_fail("allocator released more bytes than it owns");
    budget->current -= size;
    free(memory);
}

static uint16_t nt_exec_u16le(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t nt_exec_u32le(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static int nt_exec_decode(const uint8_t *data, size_t size, struct nt_exec_recipe *recipe)
{
    uint32_t max_operations;
    if (size > NT_EXEC_MAX_INPUT_BYTES || size != NT_EXEC_RECIPE_SIZE || data == NULL ||
        recipe == NULL)
        return 0;
    if (data[0] != 'N' || data[1] != 'E' || data[2] != 'X' || data[3] != 'E' || data[4] != 1u ||
        data[5] < 1u || data[5] > 2u)
        return 0;
    recipe->program = (uint8_t)(data[5] - 1u);
    {
        uint16_t before_frames = nt_exec_u16le(data + 6u);
        uint16_t after_frames = nt_exec_u16le(data + 8u);
        if (before_frames < 257u || before_frames > 258u || after_frames < 257u ||
            after_frames > 258u)
            return 0;
        recipe->before_frames = (uint16_t)(before_frames - 256u);
        recipe->after_frames = (uint16_t)(after_frames - 256u);
    }
    recipe->before_buttons = data[10];
    recipe->after_buttons = data[11];
    recipe->seed = nt_exec_u32le(data + 12u);
    if (recipe->before_frames == 0u || recipe->after_frames == 0u ||
        recipe->before_frames > NT_EXEC_MAX_FRAMES_PER_PHASE ||
        recipe->after_frames > NT_EXEC_MAX_FRAMES_PER_PHASE)
        return 0;
    max_operations = 22u + 2u * recipe->before_frames + 4u * recipe->after_frames;
    return max_operations <= NT_EXEC_MAX_OPERATIONS &&
           recipe->before_frames + 2u * recipe->after_frames <= NT_EXEC_MAX_FRAME_CALLS;
}

static uint32_t nt_exec_random(uint32_t *state)
{
    *state = *state * 1664525u + 1013904223u;
    return *state;
}

static void nt_exec_emit(uint8_t *program, size_t *length, uint8_t value)
{
    if (*length >= 128u)
        nt_exec_fail("generated program exceeded its fixed buffer");
    program[(*length)++] = value;
}

static void nt_exec_emit_word(uint8_t *program, size_t *length, uint16_t value)
{
    nt_exec_emit(program, length, (uint8_t)value);
    nt_exec_emit(program, length, (uint8_t)(value >> 8));
}

/* NROM's 16 KiB PRG is mirrored at $C000; the vectors are placed at its last six bytes.
 * The loop enables rendering, clocks pulse 1, increments a 16-bit RAM marker and reads A from
 * the controller port. These map to the loader, frame, and controller contracts in
 * ARCHITECTURE.md sections 3, 4, 6 and 7 and include/nesturbator.h's load/input API. */
static void nt_exec_program(uint8_t *prg, const struct nt_exec_recipe *recipe)
{
    uint8_t code[128];
    size_t n = 0u;
    size_t loop;
    if (recipe->program == 1u) {
        code[n++] = 0x02u; /* KIL/JAM is a deliberate status-validity control. */
    } else {
        nt_exec_emit(code, &n, 0x78u);             /* SEI */
        nt_exec_emit(code, &n, 0xd8u);             /* CLD */
        nt_exec_emit(code, &n, 0xa2u);             /* LDX #$ff; TXS */
        nt_exec_emit(code, &n, 0xffu);
        nt_exec_emit(code, &n, 0x9au);
        nt_exec_emit(code, &n, 0xa9u);             /* Clear RAM $00-$02. */
        nt_exec_emit(code, &n, 0x00u);
        nt_exec_emit(code, &n, 0x85u);
        nt_exec_emit(code, &n, 0x00u);
        nt_exec_emit(code, &n, 0x85u);
        nt_exec_emit(code, &n, 0x01u);
        nt_exec_emit(code, &n, 0x85u);
        nt_exec_emit(code, &n, 0x02u);
        nt_exec_emit(code, &n, 0xa9u);             /* Enable background and sprites. */
        nt_exec_emit(code, &n, 0x18u);
        nt_exec_emit(code, &n, 0x8du);
        nt_exec_emit_word(code, &n, 0x2001u);
        nt_exec_emit(code, &n, 0xa9u);             /* Enable pulse 1 with constant volume. */
        nt_exec_emit(code, &n, 0x01u);
        nt_exec_emit(code, &n, 0x8du);
        nt_exec_emit_word(code, &n, 0x4015u);
        nt_exec_emit(code, &n, 0xa9u);
        nt_exec_emit(code, &n, 0x3fu);
        nt_exec_emit(code, &n, 0x8du);
        nt_exec_emit_word(code, &n, 0x4000u);
        nt_exec_emit(code, &n, 0xa9u);
        nt_exec_emit(code, &n, 0x08u);
        nt_exec_emit(code, &n, 0x8du);
        nt_exec_emit_word(code, &n, 0x4002u);
        nt_exec_emit(code, &n, 0x8du);
        nt_exec_emit_word(code, &n, 0x4003u);
        loop = n;
        nt_exec_emit(code, &n, 0xe6u);             /* INC $00; carry into $01. */
        nt_exec_emit(code, &n, 0x00u);
        nt_exec_emit(code, &n, 0xd0u);             /* BNE past INC $01. */
        nt_exec_emit(code, &n, 0x02u);
        nt_exec_emit(code, &n, 0xe6u);
        nt_exec_emit(code, &n, 0x01u);
        nt_exec_emit(code, &n, 0xa9u);             /* Latch controller, then read A. */
        nt_exec_emit(code, &n, 0x01u);
        nt_exec_emit(code, &n, 0x8du);
        nt_exec_emit_word(code, &n, 0x4016u);
        nt_exec_emit(code, &n, 0xa9u);
        nt_exec_emit(code, &n, 0x00u);
        nt_exec_emit(code, &n, 0x8du);
        nt_exec_emit_word(code, &n, 0x4016u);
        nt_exec_emit(code, &n, 0xadu);
        nt_exec_emit_word(code, &n, 0x4016u);
        nt_exec_emit(code, &n, 0x29u);             /* AND #1; STA $02. */
        nt_exec_emit(code, &n, 0x01u);
        nt_exec_emit(code, &n, 0x85u);
        nt_exec_emit(code, &n, 0x02u);
        nt_exec_emit(code, &n, 0x4cu);             /* JMP loop. */
        nt_exec_emit_word(code, &n, (uint16_t)(0x8000u + (uint16_t)loop));
    }
    memcpy(prg, code, n);
    prg[0x3ffau] = 0x00u;
    prg[0x3ffbu] = 0x80u;
    prg[0x3ffcu] = 0x00u;
    prg[0x3ffdu] = 0x80u;
    prg[0x3ffeu] = 0x00u;
    prg[0x3fffu] = 0x80u;
}

static uint8_t *nt_exec_generate(const struct nt_exec_recipe *recipe, struct nt_exec_budget *budget)
{
    uint8_t *image = (uint8_t *)nt_exec_alloc(budget, NT_EXEC_MAX_GENERATED_BYTES);
    uint8_t *prg;
    uint8_t *chr;
    uint32_t state;
    if (image == NULL)
        nt_exec_fail("generated-image allocation exceeded its cap");
    memset(image, 0, 16u);
    image[0] = 'N';
    image[1] = 'E';
    image[2] = 'S';
    image[3] = 0x1au;
    image[4] = 1u;
    image[5] = 1u;
    prg = image + 16u;
    chr = prg + NT_EXEC_MAX_PRG_BYTES;
    state = recipe->seed;
    for (size_t i = 0u; i < NT_EXEC_MAX_PRG_BYTES + NT_EXEC_MAX_CHR_BYTES; ++i)
        image[16u + i] = (uint8_t)(nt_exec_random(&state) >> 24);
    nt_exec_program(prg, recipe);
    (void)chr;
    return image;
}

static void nt_exec_operation(struct nt_exec_metrics *metrics)
{
    metrics->operations++;
    if (metrics->operations > NT_EXEC_MAX_OPERATIONS)
        nt_exec_fail("recipe exceeded its public-API operation cap");
}

static nesturbator *nt_exec_instance(struct nt_exec_budget *budget, const uint8_t *image,
                                     struct nt_exec_metrics *metrics)
{
    nesturbator_config config;
    nesturbator *inst = NULL;
    memset(&config, 0, sizeof config);
    config.size = (uint32_t)sizeof config;
    config.abi = NESTURBATOR_ABI_VERSION;
    config.allocator.alloc = nt_exec_alloc;
    config.allocator.free = nt_exec_free;
    config.allocator.user = budget;
    nt_exec_operation(metrics);
    if (nesturbator_create(&config, &inst) != NESTURBATOR_OK)
        nt_exec_fail("instance creation failed within the allocation cap");
    nt_exec_operation(metrics);
    if (nesturbator_load_cartridge(inst, image, NT_EXEC_MAX_GENERATED_BYTES) != NESTURBATOR_OK)
        nt_exec_fail("generated NROM image was refused");
    return inst;
}

static uint64_t nt_exec_hash_byte(uint64_t hash, uint8_t value)
{
    return (hash ^ value) * 1099511628211ull;
}

static uint64_t nt_exec_hash_u32le(uint64_t hash, uint32_t value)
{
    for (unsigned i = 0u; i < 4u; ++i)
        hash = nt_exec_hash_byte(hash, (uint8_t)(value >> (8u * i)));
    return hash;
}

static uint64_t nt_exec_hash_u64le(uint64_t hash, uint64_t value)
{
    for (unsigned i = 0u; i < 8u; ++i)
        hash = nt_exec_hash_byte(hash, (uint8_t)(value >> (8u * i)));
    return hash;
}

static nesturbator_status nt_exec_frame(nesturbator *inst, uint8_t buttons, uint16_t *video,
                                        int16_t *audio, uint64_t *previous_ticks,
                                        struct nt_exec_branch *branch,
                                        struct nt_exec_metrics *metrics)
{
    nesturbator_input input;
    nesturbator_frame frame;
    nesturbator_status status;
    memset(&input, 0, sizeof input);
    input.size = (uint32_t)sizeof input;
    input.buttons[0] = buttons;
    nt_exec_operation(metrics);
    if (nesturbator_set_input(inst, &input) != NESTURBATOR_OK)
        nt_exec_fail("controller input was refused");
    memset(&frame, 0, sizeof frame);
    frame.size = (uint32_t)sizeof frame;
    frame.video = video;
    frame.video_pitch = 256u;
    frame.audio = audio;
    frame.audio_capacity = NT_EXEC_AUDIO_SAMPLES;
    nt_exec_operation(metrics);
    status = nesturbator_run_frame(inst, &frame);
    metrics->frames++;
    if (metrics->frames > NT_EXEC_MAX_FRAME_CALLS)
        nt_exec_fail("recipe exceeded its frame-call cap");
    branch->frames++;
    branch->output_hash = nt_exec_hash_u32le(branch->output_hash, (uint32_t)status);
    branch->stop_status = (uint32_t)status;
    metrics->stop_status = (uint32_t)status;
    if (status == NESTURBATOR_STOP_JAM)
        return status; /* JAM leaves frame outputs partial; inspect only canonical state later. */
    if (status != NESTURBATOR_OK)
        nt_exec_fail("accepted execution returned an unexpected frame status");
    if (frame.audio_count > NT_EXEC_AUDIO_SAMPLES || frame.ticks < *previous_ticks)
        nt_exec_fail("frame metadata exceeded its output contract");
    branch->output_hash = nt_exec_hash_u32le(branch->output_hash, frame.audio_count);
    branch->output_hash = nt_exec_hash_u64le(branch->output_hash, frame.frame_number);
    branch->output_hash = nt_exec_hash_u64le(branch->output_hash, frame.ticks);
    for (size_t i = 0u; i < 256u * 240u; ++i) {
        uint16_t pixel = video[i];
        branch->output_hash = nt_exec_hash_byte(branch->output_hash, (uint8_t)pixel);
        branch->output_hash = nt_exec_hash_byte(branch->output_hash, (uint8_t)(pixel >> 8));
    }
    for (uint32_t i = 0u; i < frame.audio_count; ++i) {
        uint16_t sample = (uint16_t)audio[i];
        branch->output_hash = nt_exec_hash_byte(branch->output_hash, (uint8_t)sample);
        branch->output_hash = nt_exec_hash_byte(branch->output_hash, (uint8_t)(sample >> 8));
    }
    branch->guest_ticks += frame.ticks - *previous_ticks;
    metrics->guest_ticks += frame.ticks - *previous_ticks;
    *previous_ticks = frame.ticks;
    return status;
}

static uint16_t nt_exec_marker(const nesturbator *inst, struct nt_exec_metrics *metrics)
{
    uint8_t low = 0u;
    uint8_t high = 0u;
    nt_exec_operation(metrics);
    if (nesturbator_peek_cpu_ram(inst, 0u, &low) != NESTURBATOR_OK)
        nt_exec_fail("RAM marker low byte could not be observed");
    nt_exec_operation(metrics);
    if (nesturbator_peek_cpu_ram(inst, 1u, &high) != NESTURBATOR_OK)
        nt_exec_fail("RAM marker high byte could not be observed");
    return (uint16_t)((uint16_t)low | ((uint16_t)high << 8));
}

static uint8_t nt_exec_controller_a(const nesturbator *inst, struct nt_exec_metrics *metrics)
{
    uint8_t value = 0u;
    nt_exec_operation(metrics);
    if (nesturbator_peek_cpu_ram(inst, 2u, &value) != NESTURBATOR_OK)
        nt_exec_fail("controller result could not be observed");
    return value;
}

static size_t nt_exec_save(nesturbator *inst, uint8_t *state, size_t capacity,
                           struct nt_exec_metrics *metrics)
{
    size_t size;
    nt_exec_operation(metrics);
    size = nesturbator_state_size(inst);
    if (size == 0u || size > NT_EXEC_MAX_STATE_BYTES || size > capacity)
        nt_exec_fail("machine state exceeded its fixed serialization cap");
    nt_exec_operation(metrics);
    if (nesturbator_save_state(inst, state, capacity) != NESTURBATOR_OK)
        nt_exec_fail("machine state could not be saved");
    return size;
}

static int nt_exec_run_phase(nesturbator *inst, uint8_t buttons, uint16_t count, uint16_t *video,
                             int16_t *audio, uint64_t *previous_ticks,
                             struct nt_exec_branch *branch, struct nt_exec_metrics *metrics)
{
    branch->output_hash = 14695981039346656037ull;
    for (uint16_t i = 0u; i < count; ++i) {
        if (nt_exec_frame(inst, buttons, video, audio, previous_ticks, branch, metrics) ==
            NESTURBATOR_STOP_JAM)
            return 0;
    }
    return 1;
}

static uint64_t nt_exec_state_hash(const uint8_t *state, size_t size)
{
    uint64_t hash = 14695981039346656037ull;
    for (size_t i = 0u; i < size; ++i)
        hash = nt_exec_hash_byte(hash, state[i]);
    return hash;
}

/* Compares output only when the frame status says it is valid. A JAM path is
 * judged by its stop status and canonical saved state, never partial buffers. */
static int nt_exec_verdict(const struct nt_exec_branch *a, const struct nt_exec_branch *b,
                           const uint8_t *state_a, const uint8_t *state_b, size_t state_size)
{
    return a->frames == b->frames && a->stop_status == b->stop_status &&
           a->guest_ticks == b->guest_ticks && a->output_hash == b->output_hash &&
           a->marker == b->marker && a->controller_a == b->controller_a &&
           memcmp(state_a, state_b, state_size) == 0;
}

static int nt_exec_process(const uint8_t *data, size_t size, struct nt_exec_observation *out)
{
    struct nt_exec_recipe recipe;
    struct nt_exec_budget budget = {0u, 0u};
    struct nt_exec_observation observed;
    struct nt_exec_branch before = {0u, (uint32_t)NESTURBATOR_OK, 14695981039346656037ull,
                                    0u, 0u, 0u};
    nesturbator *a = NULL;
    nesturbator *b = NULL;
    uint8_t *image = NULL;
    uint8_t *saved = NULL;
    uint8_t *state_a = NULL;
    uint8_t *state_b = NULL;
    uint16_t *video = NULL;
    int16_t *audio = NULL;
    size_t state_size = 0u;
    uint64_t ticks_a = 0u;
    uint64_t ticks_b = 0u;
    int before_finished;
    int after_a_finished;
    int after_b_finished;
    memset(&observed, 0, sizeof observed);
    if (!nt_exec_decode(data, size, &recipe)) {
        if (out != NULL)
            *out = observed;
        return 0;
    }
    image = nt_exec_generate(&recipe, &budget);
    observed.metrics.generated_bytes = NT_EXEC_MAX_GENERATED_BYTES;
    video = (uint16_t *)nt_exec_alloc(&budget, NT_EXEC_VIDEO_BYTES);
    audio = (int16_t *)nt_exec_alloc(&budget, NT_EXEC_AUDIO_BYTES);
    if (video == NULL || audio == NULL)
        nt_exec_fail("frame buffers exceeded the allocation cap");
    a = nt_exec_instance(&budget, image, &observed.metrics);
    before_finished = nt_exec_run_phase(a, recipe.before_buttons, recipe.before_frames, video, audio,
                                        &ticks_a, &before, &observed.metrics);
    before.marker = nt_exec_marker(a, &observed.metrics);
    if (before_finished && before.marker == 0u)
        nt_exec_fail("non-JAM recipe did not execute its RAM marker loop");
    nt_exec_operation(&observed.metrics);
    state_size = nesturbator_state_size(a);
    if (state_size == 0u || state_size > NT_EXEC_MAX_STATE_BYTES)
        nt_exec_fail("machine state size exceeded its pre-allocation cap");
    saved = (uint8_t *)nt_exec_alloc(&budget, state_size);
    if (saved == NULL)
        nt_exec_fail("save-state allocation exceeded the cap");
    observed.metrics.state_bytes = (uint32_t)state_size;
    if (nt_exec_save(a, saved, state_size, &observed.metrics) != state_size)
        nt_exec_fail("save-state length changed unexpectedly");
    b = nt_exec_instance(&budget, image, &observed.metrics);
    nt_exec_operation(&observed.metrics);
    if (nesturbator_load_state(b, saved, state_size) != NESTURBATOR_OK)
        nt_exec_fail("saved continuation was refused by a matching fresh instance");
    nt_exec_free(&budget, saved, state_size);
    saved = NULL;
    state_a = (uint8_t *)nt_exec_alloc(&budget, state_size);
    state_b = (uint8_t *)nt_exec_alloc(&budget, state_size);
    if (state_a == NULL || state_b == NULL)
        nt_exec_fail("continuation snapshots exceeded the allocation cap");
    ticks_b = ticks_a;
    after_a_finished = nt_exec_run_phase(a, recipe.after_buttons, recipe.after_frames, video, audio,
                                         &ticks_a, &observed.continuation_a,
                                         &observed.metrics);
    after_b_finished = nt_exec_run_phase(b, recipe.after_buttons, recipe.after_frames, video, audio,
                                         &ticks_b, &observed.continuation_b,
                                         &observed.metrics);
    observed.continuation_a.marker = nt_exec_marker(a, &observed.metrics);
    observed.continuation_b.marker = nt_exec_marker(b, &observed.metrics);
    observed.continuation_a.controller_a = nt_exec_controller_a(a, &observed.metrics);
    observed.continuation_b.controller_a = nt_exec_controller_a(b, &observed.metrics);
    if (before_finished && after_a_finished && after_b_finished &&
        (observed.continuation_a.marker == before.marker ||
         observed.continuation_a.controller_a != (uint8_t)(recipe.after_buttons & 1u)))
        nt_exec_fail("scheduled input did not produce continued guest execution");
    nt_exec_operation(&observed.metrics);
    if (nesturbator_state_size(a) != state_size)
        nt_exec_fail("source state size changed during continuation");
    nt_exec_operation(&observed.metrics);
    if (nesturbator_state_size(b) != state_size)
        nt_exec_fail("state size changed during continuation");
    nt_exec_operation(&observed.metrics);
    if (nesturbator_save_state(a, state_a, state_size) != NESTURBATOR_OK)
        nt_exec_fail("continued source instance could not be saved");
    nt_exec_operation(&observed.metrics);
    if (nesturbator_save_state(b, state_b, state_size) != NESTURBATOR_OK)
        nt_exec_fail("restored continuation could not be saved");
    observed.state_hash_a = nt_exec_state_hash(state_a, state_size);
    observed.state_hash_b = nt_exec_state_hash(state_b, state_size);
    observed.states_equal = nt_exec_verdict(&observed.continuation_a, &observed.continuation_b,
                                            state_a, state_b, state_size);
    if (!observed.states_equal)
        nt_exec_fail("uninterrupted and restored continuations diverged");
    observed.metrics.accepted = 1u;
    observed.metrics.allocation_peak = (uint32_t)budget.peak;
    nt_exec_operation(&observed.metrics);
    nesturbator_unload_cartridge(a);
    nt_exec_operation(&observed.metrics);
    nesturbator_unload_cartridge(b);
    nt_exec_operation(&observed.metrics);
    nesturbator_destroy(a);
    nt_exec_operation(&observed.metrics);
    nesturbator_destroy(b);
    nt_exec_free(&budget, state_a, state_size);
    nt_exec_free(&budget, state_b, state_size);
    nt_exec_free(&budget, video, NT_EXEC_VIDEO_BYTES);
    nt_exec_free(&budget, audio, NT_EXEC_AUDIO_BYTES);
    nt_exec_free(&budget, image, NT_EXEC_MAX_GENERATED_BYTES);
    if (budget.current != 0u)
        nt_exec_fail("per-input allocations were not fully released");
    observed.metrics.allocation_peak = (uint32_t)budget.peak;
    if (out != NULL)
        *out = observed;
    return 1;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    (void)nt_exec_process(data, size, NULL);
    return 0;
}

static void nt_exec_seed(uint8_t seed[NT_EXEC_RECIPE_SIZE])
{
    static const uint8_t bytes[NT_EXEC_RECIPE_SIZE] = {
        'N', 'E', 'X', 'E', 1u, 1u, 1u, 1u, 1u, 1u, 0x01u, 0x80u, 0x78u, 0x56u, 0x34u, 0x12u};
    memcpy(seed, bytes, sizeof bytes);
}

static const char *nt_exec_basename(const char *path)
{
    const char *name = path;
    for (const char *p = path; *p != '\0'; ++p)
        if (*p == '/' || *p == '\\')
            name = p + 1;
    return name;
}

static enum nt_exec_read_status nt_exec_read_stream(FILE *file, uint8_t **bytes, size_t *size)
{
    long length;
    uint8_t *data = NULL;
    if (file == NULL || bytes == NULL || size == NULL || fseek(file, 0, SEEK_END) != 0 ||
        (length = ftell(file)) < 0 || fseek(file, 0, SEEK_SET) != 0)
        return NT_EXEC_READ_INVALID;
    if ((unsigned long)length > NT_EXEC_MAX_INPUT_BYTES)
        return NT_EXEC_READ_TOO_LARGE;
    if (length > 0) {
        data = (uint8_t *)malloc((size_t)length);
        if (data == NULL)
            return NT_EXEC_READ_MEMORY;
        if (fread(data, 1u, (size_t)length, file) != (size_t)length) {
            free(data);
            return NT_EXEC_READ_FAILED;
        }
    }
    *bytes = data;
    *size = (size_t)length;
    return NT_EXEC_READ_OK;
}

static int nt_exec_read_path(const char *path, uint8_t **bytes, size_t *size)
{
    FILE *file = fopen(path, "rb");
    enum nt_exec_read_status status;
    if (file == NULL) {
        fprintf(stderr, NT_EXEC_REPLAY_NAME ": cannot open input: %s\n", path);
        return 0;
    }
    status = nt_exec_read_stream(file, bytes, size);
    if (fclose(file) != 0 && status == NT_EXEC_READ_OK) {
        free(*bytes);
        *bytes = NULL;
        status = NT_EXEC_READ_FAILED;
    }
    if (status != NT_EXEC_READ_OK) {
        const char *reason = status == NT_EXEC_READ_TOO_LARGE ? "input exceeds 16-byte cap" :
                             status == NT_EXEC_READ_MEMORY ? "out of memory reading input" :
                             "invalid or unreadable input";
        fprintf(stderr, NT_EXEC_REPLAY_NAME ": %s: %s\n", reason, path);
        return 0;
    }
    return 1;
}

static int nt_exec_check_seeds(int argc, char **argv)
{
    uint8_t expected[NT_EXEC_RECIPE_SIZE];
    unsigned valid_count = 0u;
    nt_exec_seed(expected);
    if (argc < 3) {
        fputs(NT_EXEC_REPLAY_NAME ": --check-seeds requires committed seed files\n", stderr);
        return 1;
    }
    for (int i = 2; i < argc; ++i) {
        uint8_t *bytes = NULL;
        size_t size = 0u;
        struct nt_exec_observation observed;
        if (!nt_exec_read_path(argv[i], &bytes, &size))
            return 1;
        if (strcmp(nt_exec_basename(argv[i]), "nrom-deep") != 0 || size != sizeof expected ||
            memcmp(bytes, expected, sizeof expected) != 0) {
            fprintf(stderr, NT_EXEC_REPLAY_NAME ": unexpected recipe seed: %s\n", argv[i]);
            free(bytes);
            return 1;
        }
        if (!nt_exec_process(bytes, size, &observed) || observed.metrics.accepted == 0u ||
            observed.metrics.frames < 2u || observed.continuation_a.frames == 0u ||
            observed.metrics.generated_bytes != NT_EXEC_MAX_GENERATED_BYTES ||
            !observed.states_equal) {
            fprintf(stderr, NT_EXEC_REPLAY_NAME ": seed did not qualify: %s\n", argv[i]);
            free(bytes);
            return 1;
        }
        valid_count++;
        free(bytes);
    }
    if (valid_count != 1u) {
        fputs(NT_EXEC_REPLAY_NAME ": corpus must contain exactly one nrom-deep seed\n", stderr);
        return 1;
    }
    puts(NT_EXEC_REPLAY_NAME ": one deterministic non-JAM recipe executed and restored");
    return 0;
}

static int nt_exec_self_test(void)
{
    uint8_t seed[NT_EXEC_RECIPE_SIZE];
    uint8_t oversize[NT_EXEC_MAX_INPUT_BYTES + 1u] = {0u};
    struct nt_exec_recipe recipe;
    struct nt_exec_budget budget = {0u, 0u};
    struct nt_exec_observation jam_observation;
    FILE *file;
    uint8_t *read_bytes = NULL;
    size_t read_size = 0u;
    void *allocation;
    int accepted;
    nt_exec_seed(seed);
    accepted = nt_exec_decode(seed, sizeof seed, &recipe);
    if (!accepted || recipe.before_frames != 1u || recipe.after_frames != 1u)
        return 1;
    if (nt_exec_decode(seed, 0u, &recipe) || nt_exec_decode(seed, sizeof seed - 1u, &recipe))
        return 1;
    seed[4] = 2u;
    if (nt_exec_decode(seed, sizeof seed, &recipe))
        return 1;
    nt_exec_seed(seed);
    seed[6] = 2u; /* The exact frame-call cap remains valid. */
    seed[8] = 2u;
    if (!nt_exec_decode(seed, sizeof seed, &recipe) ||
        22u + 2u * recipe.before_frames + 4u * recipe.after_frames != NT_EXEC_MAX_OPERATIONS)
        return 1;
    seed[6] = 3u;
    if (nt_exec_decode(seed, sizeof seed, &recipe))
        return 1;
    nt_exec_seed(seed);
    seed[6] = 0xffu;
    seed[7] = 0xffu;
    if (nt_exec_decode(seed, sizeof seed, &recipe))
        return 1;
    nt_exec_seed(seed);
    seed[5] = 3u;
    if (nt_exec_decode(seed, sizeof seed, &recipe))
        return 1;
    nt_exec_seed(seed);
    if (nt_exec_decode(oversize, sizeof oversize, &recipe))
        return 1;
    allocation = nt_exec_alloc(&budget, NT_EXEC_MAX_ALLOCATION_BYTES);
    if (allocation == NULL || budget.current != NT_EXEC_MAX_ALLOCATION_BYTES ||
        nt_exec_alloc(&budget, 1u) != NULL) {
        nt_exec_free(&budget, allocation, NT_EXEC_MAX_ALLOCATION_BYTES);
        return 1;
    }
    nt_exec_free(&budget, allocation, NT_EXEC_MAX_ALLOCATION_BYTES);
    if (budget.current != 0u || budget.peak != NT_EXEC_MAX_ALLOCATION_BYTES)
        return 1;
    file = tmpfile();
    if (file == NULL)
        return 1;
    if (fwrite(oversize, 1u, sizeof oversize, file) != sizeof oversize ||
        nt_exec_read_stream(file, &read_bytes, &read_size) != NT_EXEC_READ_TOO_LARGE) {
        (void)fclose(file);
        free(read_bytes);
        return 1;
    }
    (void)fclose(file);
    free(read_bytes);
    nt_exec_seed(seed);
    seed[10] = (uint8_t)(NESTURBATOR_BUTTON_A | NESTURBATOR_BUTTON_RIGHT);
    if (!nt_exec_decode(seed, sizeof seed, &recipe))
        return 1;
    nt_exec_seed(seed);
    seed[5] = 2u;
    if (!nt_exec_process(seed, sizeof seed, &jam_observation) ||
        jam_observation.metrics.accepted == 0u || !jam_observation.states_equal ||
        jam_observation.metrics.stop_status != (uint32_t)NESTURBATOR_STOP_JAM ||
        jam_observation.metrics.frames != 3u || jam_observation.metrics.guest_ticks != 0u)
        return 1;
    puts(NT_EXEC_REPLAY_NAME ": empty, truncated, version, byte, allocation, frame, and operation boundaries passed");
    return 0;
}

static int nt_exec_oracle_failure(void)
{
    uint8_t seed[NT_EXEC_RECIPE_SIZE];
    struct nt_exec_observation observed;
    struct nt_exec_branch mismatched;
    uint8_t state[1] = {0u};
    nt_exec_seed(seed);
    if (!nt_exec_process(seed, sizeof seed, &observed))
        return 2;
    mismatched = observed.continuation_a;
    mismatched.marker ^= 1u;
    if (nt_exec_verdict(&observed.continuation_a, &mismatched, state, state, sizeof state))
        return 2;
    fputs(NT_EXEC_REPLAY_NAME ": intentional execution oracle failure\n", stderr);
    return 1;
}

static int nt_exec_measure(int argc, char **argv)
{
    char *end = NULL;
    unsigned long repeats;
    uint64_t inputs = 0u;
    uint64_t accepted = 0u;
    uint64_t frames = 0u;
    uint64_t operations = 0u;
    uint64_t generated_bytes = 0u;
    uint64_t guest_ticks = 0u;
    uint32_t allocation_peak = 0u;
    uint32_t last_stop = 0u;
    if (argc < 4) {
        fputs(NT_EXEC_REPLAY_NAME ": --measure requires repeats and corpus files\n", stderr);
        return 1;
    }
    repeats = strtoul(argv[2], &end, 10);
    if (end == argv[2] || *end != '\0' || repeats == 0ul || repeats > 1000ul) {
        fputs(NT_EXEC_REPLAY_NAME ": repeat count must be in 1..1000\n", stderr);
        return 1;
    }
    for (unsigned long pass = 0ul; pass < repeats; ++pass) {
        for (int i = 3; i < argc; ++i) {
            uint8_t *bytes = NULL;
            size_t size = 0u;
            struct nt_exec_observation observed;
            if (!nt_exec_read_path(argv[i], &bytes, &size))
                return 1;
            (void)nt_exec_process(bytes, size, &observed);
            inputs++;
            accepted += observed.metrics.accepted;
            frames += observed.metrics.frames;
            operations += observed.metrics.operations;
            generated_bytes += observed.metrics.generated_bytes;
            guest_ticks += observed.metrics.guest_ticks;
            if (observed.metrics.allocation_peak > allocation_peak)
                allocation_peak = observed.metrics.allocation_peak;
            last_stop = observed.metrics.stop_status;
            free(bytes);
        }
    }
    printf("{\"inputs\":%llu,\"accepted\":%llu,\"frames\":%llu,\"api_operations\":%llu,"
           "\"generated_bytes\":%llu,\"guest_ticks\":%llu,\"peak_allocation_bytes\":%u,"
           "\"last_stop_status\":%u}\n",
           (unsigned long long)inputs, (unsigned long long)accepted, (unsigned long long)frames,
           (unsigned long long)operations, (unsigned long long)generated_bytes,
           (unsigned long long)guest_ticks, allocation_peak, last_stop);
    return inputs > 0u ? 0 : 1;
}

static int nt_exec_replay(int argc, char **argv)
{
    if (argc < 2) {
        fputs(NT_EXEC_REPLAY_NAME ": no recipe files supplied\n", stderr);
        return 1;
    }
    for (int i = 1; i < argc; ++i) {
        uint8_t *bytes = NULL;
        size_t size = 0u;
        if (!nt_exec_read_path(argv[i], &bytes, &size))
            return 1;
        (void)nt_exec_process(bytes, size, NULL);
        free(bytes);
    }
    return 0;
}

#ifdef NESTURBATOR_FUZZ_REPLAY
int main(int argc, char **argv)
{
    if (argc >= 2 && strcmp(argv[1], "--check-seeds") == 0)
        return nt_exec_check_seeds(argc, argv);
    if (argc == 2 && strcmp(argv[1], "--self-test") == 0)
        return nt_exec_self_test();
    if (argc == 2 && strcmp(argv[1], "--oracle-failure") == 0)
        return nt_exec_oracle_failure();
    if (argc >= 2 && strcmp(argv[1], "--measure") == 0)
        return nt_exec_measure(argc, argv);
    return nt_exec_replay(argc, argv);
}
#endif
