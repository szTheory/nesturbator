/* Bounded generated-program execution and save-state continuation fuzz target.
 * Recipes store descriptors only; accepted board images are generated in memory. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__APPLE__) || defined(__linux__)
#include <sys/resource.h>
#endif
#include "nesturbator.h"
#include "../ines.h"

#define NT_EXEC_REPLAY_NAME "fuzz.execution.regress"
#define NT_EXEC_MAGIC_SIZE 4u
#define NT_EXEC_RECIPE_SIZE 16u
#ifndef NT_EXEC_MAX_PRG_BYTES
#define NT_EXEC_MAX_PRG_BYTES (128u * 1024u)
#endif
#ifndef NT_EXEC_MAX_CHR_BYTES
#define NT_EXEC_MAX_CHR_BYTES (32u * 1024u)
#endif
#ifndef NT_EXEC_MAX_INPUT_BYTES
#define NT_EXEC_MAX_INPUT_BYTES                                                                    \
    (NT_EXEC_RECIPE_SIZE + NT_EXEC_MAX_PRG_BYTES + NT_EXEC_MAX_CHR_BYTES)
#endif
#ifndef NT_EXEC_MAX_GENERATED_BYTES
#define NT_EXEC_MAX_GENERATED_BYTES (16u + NT_EXEC_MAX_PRG_BYTES + NT_EXEC_MAX_CHR_BYTES)
#endif
#ifndef NT_EXEC_MAX_STATE_BYTES
#define NT_EXEC_MAX_STATE_BYTES (256u * 1024u)
#endif
#ifndef NT_EXEC_MAX_ALLOCATION_BYTES
#define NT_EXEC_MAX_ALLOCATION_BYTES (2u * 1024u * 1024u)
#endif
#ifndef NT_EXEC_MAX_FRAMES_PER_PHASE
#define NT_EXEC_MAX_FRAMES_PER_PHASE 2u
#endif
#ifndef NT_EXEC_MAX_FRAME_CALLS
#define NT_EXEC_MAX_FRAME_CALLS 6u
#endif
#ifndef NT_EXEC_MAX_OPERATIONS
#define NT_EXEC_MAX_OPERATIONS 128u
#endif
#define NT_EXEC_VIDEO_BYTES (256u * 240u * sizeof(uint16_t))
#define NT_EXEC_AUDIO_SAMPLES 1024u
#define NT_EXEC_AUDIO_BYTES (NT_EXEC_AUDIO_SAMPLES * sizeof(int16_t))
#define NT_EXEC_PROGRAM_BYTES 768u

struct nt_exec_budget {
    size_t current;
    size_t peak;
};

struct nt_exec_recipe {
    uint8_t profile_code;
    uint8_t jam;
    uint16_t before_frames;
    uint16_t after_frames;
    uint8_t before_buttons;
    uint8_t after_buttons;
    uint32_t seed;
};

struct nt_exec_metrics {
    uint32_t accepted;
    uint32_t frames;
    uint32_t frame_ok;
    uint32_t frame_jam;
    uint32_t operations;
    uint32_t generated_bytes;
    uint32_t state_bytes;
    uint32_t allocation_peak;
    uint64_t guest_ticks;
    uint32_t stop_status;
    uint16_t mapper;
    uint8_t submapper;
    uint8_t qualified_bank_effects;
    uint32_t operation_indices[NT_EXEC_MAX_OPERATIONS];
};

struct nt_exec_branch {
    uint32_t frames;
    uint32_t stop_status;
    uint64_t output_hash;
    uint64_t guest_ticks;
    uint16_t prg_marker;
    uint16_t chr_marker;
    uint16_t guest_counter;
    uint16_t nametable_marker;
    uint8_t controller_a;
    uint32_t visible_pixels;
};

struct nt_exec_observation {
    struct nt_exec_metrics metrics;
    struct nt_exec_branch continuation_a;
    struct nt_exec_branch continuation_b;
    uint64_t state_hash_a;
    uint64_t state_hash_b;
    uint64_t origin_state_hash;
    int states_equal;
    uint16_t mapper;
    uint32_t accepted_status;
    uint32_t completed_frames;
    uint32_t operation_count;
    uint8_t bank_effects;
};

enum nt_exec_profile_code {
    NT_EXEC_PROFILE_NROM = 1u,
    NT_EXEC_PROFILE_MMC1 = 2u,
    NT_EXEC_PROFILE_UXROM = 3u,
    NT_EXEC_PROFILE_CNROM = 4u,
    NT_EXEC_PROFILE_MMC3 = 5u,
    NT_EXEC_PROFILE_AXROM = 6u,
    NT_EXEC_PROFILE_JAM = 7u,
    NT_EXEC_PROFILE_LOOP = 8u
};

enum nt_exec_bank_effect {
    NT_EXEC_EFFECT_PRG = 1u,
    NT_EXEC_EFFECT_CHR = 2u,
    NT_EXEC_EFFECT_NAMETABLE = 4u
};

struct nt_exec_profile {
    uint16_t mapper;
    uint8_t submapper;
    uint8_t prg_16k;
    uint8_t chr_8k;
    uint8_t chr_ram;
    uint8_t nes2;
    uint8_t battery;
    uint8_t prg_nvram_shift;
    uint16_t code_address;
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
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static int nt_exec_decode(const uint8_t *data, size_t size, struct nt_exec_recipe *recipe)
{
    uint32_t max_operations;
    if (size > NT_EXEC_MAX_INPUT_BYTES || size < NT_EXEC_RECIPE_SIZE || data == NULL ||
        recipe == NULL)
        return 0;
    if (data[0] != 'N' || data[1] != 'E' || data[2] != 'X' || data[3] != 'E' || data[4] != 1u ||
        data[5] < NT_EXEC_PROFILE_NROM || data[5] > NT_EXEC_PROFILE_LOOP)
        return 0;
    recipe->profile_code = data[5];
    recipe->jam = (uint8_t)(data[5] == NT_EXEC_PROFILE_JAM);
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
    max_operations = 96u + 2u * recipe->before_frames + 4u * recipe->after_frames;
    return max_operations <= NT_EXEC_MAX_OPERATIONS &&
           1u + recipe->before_frames + 2u * recipe->after_frames <= NT_EXEC_MAX_FRAME_CALLS;
}

static uint32_t nt_exec_random(uint32_t *state)
{
    *state = *state * 1664525u + 1013904223u;
    return *state;
}

static void nt_exec_emit(uint8_t *program, size_t *length, uint8_t value)
{
    if (*length >= NT_EXEC_PROGRAM_BYTES)
        nt_exec_fail("generated program exceeded its fixed buffer");
    program[(*length)++] = value;
}

static void nt_exec_emit_word(uint8_t *program, size_t *length, uint16_t value)
{
    nt_exec_emit(program, length, (uint8_t)value);
    nt_exec_emit(program, length, (uint8_t)(value >> 8));
}

static int nt_exec_profile_for(uint8_t code, struct nt_exec_profile *profile)
{
    memset(profile, 0, sizeof *profile);
    switch (code) {
    case NT_EXEC_PROFILE_NROM:
        profile->prg_16k = 1u;
        profile->chr_8k = 1u;
        profile->code_address = 0x8000u;
        return 1;
    case NT_EXEC_PROFILE_MMC1:
        profile->mapper = 1u;
        profile->prg_16k = 2u;
        profile->chr_8k = 4u;
        profile->nes2 = 1u;
        profile->battery = 1u;
        profile->prg_nvram_shift = 7u;
        profile->code_address = 0xc000u;
        return 1;
    case NT_EXEC_PROFILE_UXROM:
        profile->mapper = 2u;
        profile->submapper = 1u;
        profile->prg_16k = 4u;
        profile->chr_ram = 1u;
        profile->nes2 = 1u;
        profile->code_address = 0xc000u;
        return 1;
    case NT_EXEC_PROFILE_CNROM:
        profile->mapper = 3u;
        profile->submapper = 1u;
        profile->prg_16k = 2u;
        profile->chr_8k = 4u;
        profile->nes2 = 1u;
        profile->code_address = 0x8000u;
        return 1;
    case NT_EXEC_PROFILE_MMC3:
        profile->mapper = 4u;
        profile->prg_16k = 4u;
        profile->chr_8k = 4u;
        profile->nes2 = 1u;
        profile->battery = 1u;
        profile->prg_nvram_shift = 7u;
        profile->code_address = 0xe000u;
        return 1;
    case NT_EXEC_PROFILE_AXROM:
        profile->mapper = 7u;
        profile->submapper = 1u;
        profile->prg_16k = 4u;
        profile->chr_ram = 1u;
        profile->nes2 = 1u;
        profile->code_address = 0x8000u;
        return 1;
    case NT_EXEC_PROFILE_JAM:
    case NT_EXEC_PROFILE_LOOP:
        profile->prg_16k = 1u;
        profile->chr_8k = 1u;
        profile->code_address = 0x8000u;
        return 1;
    default:
        return 0;
    }
}

static size_t nt_exec_image_size(const struct nt_exec_profile *profile)
{
    return 16u + (size_t)profile->prg_16k * INES_PRG_BANK + (size_t)profile->chr_8k * INES_CHR_BANK;
}

static void nt_exec_emit_imm(uint8_t *code, size_t *n, uint8_t value)
{
    nt_exec_emit(code, n, 0xa9u);
    nt_exec_emit(code, n, value);
}

static void nt_exec_emit_sta(uint8_t *code, size_t *n, uint16_t address)
{
    nt_exec_emit(code, n, 0x8du);
    nt_exec_emit_word(code, n, address);
}

static void nt_exec_emit_lda(uint8_t *code, size_t *n, uint16_t address)
{
    nt_exec_emit(code, n, 0xadu);
    nt_exec_emit_word(code, n, address);
}

static void nt_exec_emit_ppu_read(uint8_t *code, size_t *n, uint16_t address, uint8_t target)
{
    nt_exec_emit_imm(code, n, (uint8_t)(address >> 8));
    nt_exec_emit_sta(code, n, 0x2006u);
    nt_exec_emit_imm(code, n, (uint8_t)address);
    nt_exec_emit_sta(code, n, 0x2006u);
    nt_exec_emit_lda(code, n, 0x2007u); /* First read fills the PPU read buffer. */
    nt_exec_emit_lda(code, n, 0x2007u);
    nt_exec_emit(code, n, 0x85u);
    nt_exec_emit(code, n, target);
}

static void nt_exec_emit_ppu_write(uint8_t *code, size_t *n, uint16_t address, uint8_t value)
{
    nt_exec_emit_imm(code, n, (uint8_t)(address >> 8));
    nt_exec_emit_sta(code, n, 0x2006u);
    nt_exec_emit_imm(code, n, (uint8_t)address);
    nt_exec_emit_sta(code, n, 0x2006u);
    nt_exec_emit_imm(code, n, value);
    nt_exec_emit_sta(code, n, 0x2007u);
}

static void nt_exec_emit_mmc1_register(uint8_t *code, size_t *n, uint16_t address, uint8_t value)
{
    for (unsigned bit = 0u; bit < 5u; ++bit) {
        nt_exec_emit_imm(code, n, (uint8_t)((value >> bit) & 1u));
        nt_exec_emit_sta(code, n, address);
        nt_exec_emit(code, n, 0xeau); /* Keep serial writes off the immediately next cycle. */
    }
}

static size_t nt_exec_make_program(uint8_t code[NT_EXEC_PROGRAM_BYTES],
                                   const struct nt_exec_recipe *recipe,
                                   const struct nt_exec_profile *profile)
{
    size_t n = 0u;
    size_t loop;
    if (recipe->jam != 0u) {
        nt_exec_emit(code, &n, 0xeeu); /* INC $0000 makes re-entry observable. */
        nt_exec_emit_word(code, &n, 0x0000u);
        nt_exec_emit(code, &n, 0x02u);
        return n;
    }
    if (recipe->profile_code == NT_EXEC_PROFILE_LOOP) {
        nt_exec_emit(code, &n, 0x4cu); /* JMP $8000: a valid guest loop with a stable PC. */
        nt_exec_emit_word(code, &n, profile->code_address);
        return n;
    }
    nt_exec_emit(code, &n, 0x78u); /* SEI */
    nt_exec_emit(code, &n, 0xd8u); /* CLD */
    nt_exec_emit(code, &n, 0xa2u);
    nt_exec_emit(code, &n, 0xffu);
    nt_exec_emit(code, &n, 0x9au); /* TXS */
    nt_exec_emit_imm(code, &n, 0u);
    for (uint8_t i = 0u; i < 9u; ++i) {
        nt_exec_emit(code, &n, 0x85u); /* Clear observation bytes and guest counter. */
        nt_exec_emit(code, &n, i);
    }
    if (profile->battery != 0u) {
        nt_exec_emit_imm(code, &n, 0xa5u);
        nt_exec_emit_sta(code, &n, 0x6000u);
    }
    if (profile->chr_ram != 0u) {
        nt_exec_emit_ppu_write(code, &n, 0x0100u, 0x6du);
        nt_exec_emit_imm(code, &n, 0x00u);
        nt_exec_emit_sta(code, &n, 0x2006u);
        nt_exec_emit_imm(code, &n, 0x00u);
        nt_exec_emit_sta(code, &n, 0x2006u);
        for (unsigned i = 0u; i < 8u; ++i) {
            nt_exec_emit_imm(code, &n, 0xffu);
            nt_exec_emit_sta(code, &n, 0x2007u);
        }
        for (unsigned i = 0u; i < 8u; ++i) {
            nt_exec_emit_imm(code, &n, 0x00u);
            nt_exec_emit_sta(code, &n, 0x2007u);
        }
    }
    nt_exec_emit_lda(code, &n, 0x8400u);
    nt_exec_emit(code, &n, 0x85u);
    nt_exec_emit(code, &n, 0x00u); /* Initial PRG bank marker. */
    if (profile->mapper == 1u || profile->mapper == 3u || profile->mapper == 4u)
        nt_exec_emit_ppu_read(code, &n, profile->mapper == 4u ? 0x1100u : 0x0100u, 0x02u);
    else
        nt_exec_emit_ppu_read(code, &n, 0x0100u, 0x02u);
    switch (profile->mapper) {
    case 1u:
        nt_exec_emit_mmc1_register(code, &n, 0xe000u, 1u);
        nt_exec_emit_lda(code, &n, 0x8400u);
        nt_exec_emit(code, &n, 0x85u);
        nt_exec_emit(code, &n, 0x01u);
        nt_exec_emit_mmc1_register(code, &n, 0xa000u, 2u);
        nt_exec_emit_ppu_read(code, &n, 0x0100u, 0x03u);
        break;
    case 2u:
        nt_exec_emit_imm(code, &n, 1u);
        nt_exec_emit_sta(code, &n, 0x8000u);
        nt_exec_emit_lda(code, &n, 0x8400u);
        nt_exec_emit(code, &n, 0x85u);
        nt_exec_emit(code, &n, 0x01u);
        break;
    case 3u:
        nt_exec_emit_imm(code, &n, 2u);
        nt_exec_emit_sta(code, &n, 0x8000u);
        nt_exec_emit_ppu_read(code, &n, 0x0100u, 0x03u);
        break;
    case 4u:
        nt_exec_emit_imm(code, &n, 6u);
        nt_exec_emit_sta(code, &n, 0x8000u);
        nt_exec_emit_imm(code, &n, 1u);
        nt_exec_emit_sta(code, &n, 0x8001u);
        nt_exec_emit_lda(code, &n, 0x8400u);
        nt_exec_emit(code, &n, 0x85u);
        nt_exec_emit(code, &n, 0x01u);
        nt_exec_emit_imm(code, &n, 2u);
        nt_exec_emit_sta(code, &n, 0x8000u);
        nt_exec_emit_imm(code, &n, 3u);
        nt_exec_emit_sta(code, &n, 0x8001u);
        nt_exec_emit_ppu_read(code, &n, 0x1100u, 0x03u);
        break;
    case 7u:
        nt_exec_emit_ppu_write(code, &n, 0x2001u, 0x5au);
        nt_exec_emit_imm(code, &n, 0x11u); /* PRG bank 1 and single-screen page 1. */
        nt_exec_emit_sta(code, &n, 0x8000u);
        nt_exec_emit_lda(code, &n, 0x8400u);
        nt_exec_emit(code, &n, 0x85u);
        nt_exec_emit(code, &n, 0x01u);
        nt_exec_emit_ppu_read(code, &n, 0x2001u, 0x04u); /* Page 1 starts independently. */
        nt_exec_emit_ppu_write(code, &n, 0x2001u, 0xa5u);
        nt_exec_emit_imm(code, &n, 0u); /* Return to bank 0 and page 0. */
        nt_exec_emit_sta(code, &n, 0x8000u);
        nt_exec_emit_ppu_read(code, &n, 0x2001u, 0x05u);
        break;
    default:
        nt_exec_emit(code, &n, 0xa5u); /* NROM has no bank register. */
        nt_exec_emit(code, &n, 0x00u);
        nt_exec_emit(code, &n, 0x85u);
        nt_exec_emit(code, &n, 0x01u);
        nt_exec_emit_ppu_read(code, &n, 0x0100u, 0x03u);
        break;
    }
    nt_exec_emit_ppu_write(code, &n, 0x3f01u, 0x21u);
    nt_exec_emit_imm(code, &n, 0x20u);
    nt_exec_emit_sta(code, &n, 0x2006u);
    nt_exec_emit_imm(code, &n, 0x00u);
    nt_exec_emit_sta(code, &n, 0x2006u);
    nt_exec_emit_imm(code, &n, 0x18u);
    nt_exec_emit_sta(code, &n, 0x2001u); /* Enable background and sprite fetches. */
    nt_exec_emit_imm(code, &n, 0x01u);
    nt_exec_emit_sta(code, &n, 0x4015u);
    nt_exec_emit_imm(code, &n, 0x3fu);
    nt_exec_emit_sta(code, &n, 0x4000u);
    nt_exec_emit_imm(code, &n, 0x08u);
    nt_exec_emit_sta(code, &n, 0x4002u);
    nt_exec_emit_sta(code, &n, 0x4003u);
    loop = n;
    nt_exec_emit(code, &n, 0xe6u);
    nt_exec_emit(code, &n, 0x06u);
    nt_exec_emit(code, &n, 0xd0u);
    nt_exec_emit(code, &n, 0x02u);
    nt_exec_emit(code, &n, 0xe6u);
    nt_exec_emit(code, &n, 0x07u);
    nt_exec_emit_imm(code, &n, 1u);
    nt_exec_emit_sta(code, &n, 0x4016u);
    nt_exec_emit_imm(code, &n, 0u);
    nt_exec_emit_sta(code, &n, 0x4016u);
    nt_exec_emit_lda(code, &n, 0x4016u);
    nt_exec_emit(code, &n, 0x29u);
    nt_exec_emit(code, &n, 1u);
    nt_exec_emit(code, &n, 0x85u);
    nt_exec_emit(code, &n, 0x08u);
    nt_exec_emit(code, &n, 0x4cu);
    nt_exec_emit_word(code, &n, (uint16_t)(profile->code_address + (uint16_t)loop));
    return n;
}

static void nt_exec_write_vector(uint8_t *prg, size_t offset, uint16_t vector)
{
    prg[offset - 6u] = (uint8_t)vector;
    prg[offset - 5u] = (uint8_t)(vector >> 8);
    prg[offset - 4u] = (uint8_t)vector;
    prg[offset - 3u] = (uint8_t)(vector >> 8);
    prg[offset - 2u] = (uint8_t)vector;
    prg[offset - 1u] = (uint8_t)(vector >> 8);
}

static uint8_t *nt_exec_generate(const struct nt_exec_recipe *recipe, const uint8_t *input,
                                 size_t input_size, const struct nt_exec_profile *profile,
                                 size_t *generated_size, struct nt_exec_budget *budget)
{
    const size_t prg_size = (size_t)profile->prg_16k * INES_PRG_BANK;
    const size_t chr_size = (size_t)profile->chr_8k * INES_CHR_BANK;
    const size_t size = nt_exec_image_size(profile);
    uint8_t *image;
    uint8_t *prg;
    uint8_t *chr;
    uint8_t code[NT_EXEC_PROGRAM_BYTES];
    size_t code_size;
    uint32_t state;
    if (prg_size > NT_EXEC_MAX_PRG_BYTES || chr_size > NT_EXEC_MAX_CHR_BYTES ||
        size > NT_EXEC_MAX_GENERATED_BYTES ||
        input_size - NT_EXEC_RECIPE_SIZE > prg_size + chr_size)
        return NULL;
    image = (uint8_t *)nt_exec_alloc(budget, size);
    if (image == NULL)
        nt_exec_fail("generated-image allocation exceeded its cap");
    memset(image, 0, size);
    image[0] = 'N';
    image[1] = 'E';
    image[2] = 'S';
    image[3] = 0x1au;
    image[4] = profile->prg_16k;
    image[5] = profile->chr_8k;
    image[6] = (uint8_t)(((profile->mapper & 0x0fu) << 4) | (profile->battery != 0u ? 0x02u : 0u));
    image[7] = (uint8_t)((profile->mapper & 0xf0u) | (profile->nes2 != 0u ? 0x08u : 0u));
    if (profile->nes2 != 0u) {
        image[8] =
            (uint8_t)(((unsigned)profile->submapper << 4) | ((profile->mapper >> 8) & 0x0fu));
        image[10] = (uint8_t)(profile->prg_nvram_shift << 4);
        image[11] = (uint8_t)(profile->chr_ram != 0u ? 0x07u : 0u);
    }
    prg = image + 16u;
    chr = prg + prg_size;
    state = recipe->seed;
    for (size_t i = 0u; i < prg_size + chr_size; ++i) {
        image[16u + i] = (uint8_t)(nt_exec_random(&state) >> 24);
    }
    if (profile->chr_ram == 0u) {
        for (size_t offset = 0u; offset < chr_size; offset += 1024u) {
            for (size_t i = 0u; i < 8u; ++i) {
                chr[offset + i] = 0xffu;
                chr[offset + 8u + i] = 0u;
            }
        }
        if (profile->mapper == 1u) {
            chr[0x100u] = 0x51u;
            chr[INES_CHR_BANK + 0x100u] = 0x52u;
        } else if (profile->mapper == 3u) {
            chr[0x100u] = 0x51u;
            chr[2u * INES_CHR_BANK + 0x100u] = 0x53u;
        } else if (profile->mapper == 4u) {
            chr[0x100u] = 0x51u;
            chr[3u * 1024u + 0x100u] = 0x54u;
        } else {
            chr[0x100u] = 0x51u;
        }
    }
    if (recipe->jam == 0u) {
        if (profile->mapper == 4u) {
            prg[0x400u] = 0x40u;
            prg[8192u + 0x400u] = 0x41u;
        } else if (profile->mapper == 7u) {
            prg[0x400u] = 0x40u;
            prg[INES_PRG_BANK * 2u + 0x400u] = 0x41u;
        } else {
            prg[0x400u] = 0x40u;
            if (profile->prg_16k > 1u)
                prg[INES_PRG_BANK + 0x400u] = 0x41u;
        }
    }
    code_size = nt_exec_make_program(code, recipe, profile);
    if (profile->mapper == 7u && recipe->jam == 0u) {
        memcpy(prg, code, code_size);
        memcpy(prg + INES_PRG_BANK * 2u, code, code_size);
        nt_exec_write_vector(prg, INES_PRG_BANK * 2u, profile->code_address);
        nt_exec_write_vector(prg, INES_PRG_BANK * 4u, profile->code_address);
    } else {
        size_t code_offset;
        if (profile->mapper == 4u)
            code_offset = prg_size - 8192u;
        else if (profile->code_address == 0xc000u)
            code_offset = prg_size - INES_PRG_BANK;
        else
            code_offset = 0u;
        memcpy(prg + code_offset, code, code_size);
        nt_exec_write_vector(prg, prg_size, profile->code_address);
    }
    if (input_size > NT_EXEC_RECIPE_SIZE)
        memcpy(prg, input + NT_EXEC_RECIPE_SIZE, input_size - NT_EXEC_RECIPE_SIZE);
    *generated_size = size;
    return image;
}

static void nt_exec_operation(struct nt_exec_metrics *metrics)
{
    if (metrics->operations >= NT_EXEC_MAX_OPERATIONS)
        nt_exec_fail("recipe exceeded its public-API operation cap");
    metrics->operation_indices[metrics->operations] = metrics->operations;
    metrics->operations++;
}

static nesturbator *nt_exec_instance(struct nt_exec_budget *budget, const uint8_t *image,
                                     size_t image_size, struct nt_exec_metrics *metrics)
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
    if (nesturbator_load_cartridge(inst, image, image_size) != NESTURBATOR_OK)
        nt_exec_fail("generated accepted-profile image was refused");
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
    memset(video, 0xa5, NT_EXEC_VIDEO_BYTES);
    memset(audio, 0x5a, NT_EXEC_AUDIO_BYTES);
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
    if (status == NESTURBATOR_STOP_JAM) {
        metrics->frame_jam++;
        return status; /* JAM leaves frame outputs partial; inspect only canonical state later. */
    }
    if (status != NESTURBATOR_OK)
        nt_exec_fail("accepted execution returned an unexpected frame status");
    metrics->frame_ok++;
    if (frame.audio_count > NT_EXEC_AUDIO_SAMPLES || frame.ticks <= *previous_ticks ||
        frame.frame_number == 0u)
        nt_exec_fail("successful frame did not advance its output contract");
    branch->output_hash = nt_exec_hash_u32le(branch->output_hash, frame.audio_count);
    branch->output_hash = nt_exec_hash_u64le(branch->output_hash, frame.frame_number);
    branch->output_hash = nt_exec_hash_u64le(branch->output_hash, frame.ticks);
    for (size_t i = 0u; i < 256u * 240u; ++i) {
        uint16_t pixel = video[i];
        if (pixel != 0u)
            branch->visible_pixels++;
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

static uint8_t nt_exec_peek(const nesturbator *inst, uint16_t address,
                            struct nt_exec_metrics *metrics)
{
    uint8_t value = 0u;
    nt_exec_operation(metrics);
    if (nesturbator_peek_cpu_ram(inst, address, &value) != NESTURBATOR_OK)
        nt_exec_fail("guest RAM observation was refused");
    return value;
}

static void nt_exec_observe_guest(const nesturbator *inst, struct nt_exec_branch *branch,
                                  struct nt_exec_metrics *metrics)
{
    const uint8_t prg_initial = nt_exec_peek(inst, 0u, metrics);
    const uint8_t prg_selected = nt_exec_peek(inst, 1u, metrics);
    const uint8_t chr_initial = nt_exec_peek(inst, 2u, metrics);
    const uint8_t chr_selected = nt_exec_peek(inst, 3u, metrics);
    branch->prg_marker = (uint16_t)((uint16_t)prg_initial | ((uint16_t)prg_selected << 8));
    branch->chr_marker = (uint16_t)((uint16_t)chr_initial | ((uint16_t)chr_selected << 8));
    branch->nametable_marker = (uint16_t)((uint16_t)nt_exec_peek(inst, 4u, metrics) |
                                          ((uint16_t)nt_exec_peek(inst, 5u, metrics) << 8));
    branch->guest_counter = (uint16_t)((uint16_t)nt_exec_peek(inst, 6u, metrics) |
                                       ((uint16_t)nt_exec_peek(inst, 7u, metrics) << 8));
    branch->controller_a = nt_exec_peek(inst, 8u, metrics);
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

/* OK paths compare only written frame metadata, pixels and audio samples. JAM
 * paths compare stop status and canonical state, never partial output buffers. */
static int nt_exec_verdict(const struct nt_exec_branch *a, const struct nt_exec_branch *b,
                           const uint8_t *state_a, const uint8_t *state_b, size_t state_size)
{
    if (a->frames != b->frames || a->stop_status != b->stop_status ||
        memcmp(state_a, state_b, state_size) != 0)
        return 0;
    if (a->stop_status == (uint32_t)NESTURBATOR_STOP_JAM)
        return 1;
    return a->frames > 0u && a->guest_ticks > 0u && a->guest_ticks == b->guest_ticks &&
           a->output_hash == b->output_hash && a->prg_marker == b->prg_marker &&
           a->chr_marker == b->chr_marker && a->guest_counter == b->guest_counter &&
           a->nametable_marker == b->nametable_marker && a->controller_a == b->controller_a &&
           a->visible_pixels == b->visible_pixels;
}

static uint8_t nt_exec_expected_effects(const struct nt_exec_profile *profile)
{
    uint8_t effects = 0u;
    if (profile->mapper == 1u || profile->mapper == 2u || profile->mapper == 4u ||
        profile->mapper == 7u)
        effects |= NT_EXEC_EFFECT_PRG;
    if (profile->mapper == 1u || profile->mapper == 3u || profile->mapper == 4u)
        effects |= NT_EXEC_EFFECT_CHR;
    if (profile->mapper == 7u)
        effects |= NT_EXEC_EFFECT_NAMETABLE;
    return effects;
}

static uint8_t nt_exec_observed_effects(const struct nt_exec_profile *profile,
                                        const struct nt_exec_branch *branch)
{
    uint8_t effects = 0u;
    const uint8_t prg0 = (uint8_t)branch->prg_marker;
    const uint8_t prg1 = (uint8_t)(branch->prg_marker >> 8);
    const uint8_t chr0 = (uint8_t)branch->chr_marker;
    const uint8_t chr1 = (uint8_t)(branch->chr_marker >> 8);
    if (nt_exec_expected_effects(profile) & NT_EXEC_EFFECT_PRG) {
        if (prg0 == 0x40u && prg1 == 0x41u)
            effects |= NT_EXEC_EFFECT_PRG;
    }
    if (nt_exec_expected_effects(profile) & NT_EXEC_EFFECT_CHR) {
        const uint8_t expected_chr = profile->mapper == 1u   ? 0x52u
                                     : profile->mapper == 3u ? 0x53u
                                                             : 0x54u;
        if (chr0 == 0x51u && chr1 == expected_chr)
            effects |= NT_EXEC_EFFECT_CHR;
    }
    if (profile->mapper == 7u && branch->nametable_marker == 0x5a00u)
        effects |= NT_EXEC_EFFECT_NAMETABLE;
    return effects;
}

static int nt_exec_process(const uint8_t *data, size_t size, struct nt_exec_observation *out)
{
    struct nt_exec_recipe recipe;
    struct nt_exec_profile profile;
    struct nt_exec_budget budget = {0u, 0u};
    struct nt_exec_observation observed;
    struct nt_exec_branch before;
    struct nt_exec_branch interloper_branch;
    nesturbator *a = NULL;
    nesturbator *b = NULL;
    nesturbator *interloper = NULL;
    uint8_t *image = NULL;
    uint8_t *interloper_image = NULL;
    uint8_t *saved = NULL;
    uint8_t *state_a = NULL;
    uint8_t *state_b = NULL;
    uint16_t *video = NULL;
    int16_t *audio = NULL;
    size_t image_size = 0u;
    size_t interloper_image_size = 0u;
    size_t state_size = 0u;
    uint64_t ticks_a = 0u;
    uint64_t ticks_b = 0u;
    int before_finished;
    int after_a_finished;
    int after_b_finished;
    memset(&observed, 0, sizeof observed);
    memset(&before, 0, sizeof before);
    memset(&interloper_branch, 0, sizeof interloper_branch);
    before.stop_status = (uint32_t)NESTURBATOR_OK;
    if (!nt_exec_decode(data, size, &recipe) ||
        !nt_exec_profile_for(recipe.profile_code, &profile)) {
        if (out != NULL)
            *out = observed;
        return 0;
    }
    image = nt_exec_generate(&recipe, data, size, &profile, &image_size, &budget);
    if (image == NULL) {
        if (out != NULL)
            *out = observed;
        return 0;
    }
    observed.metrics.generated_bytes = (uint32_t)image_size;
    observed.metrics.mapper = profile.mapper;
    observed.metrics.submapper = profile.submapper;
    observed.mapper = profile.mapper;
    observed.accepted_status = (uint32_t)NESTURBATOR_OK;
    video = (uint16_t *)nt_exec_alloc(&budget, NT_EXEC_VIDEO_BYTES);
    audio = (int16_t *)nt_exec_alloc(&budget, NT_EXEC_AUDIO_BYTES);
    if (video == NULL || audio == NULL)
        nt_exec_fail("frame buffers exceeded the allocation cap");
    a = nt_exec_instance(&budget, image, image_size, &observed.metrics);
    before_finished = nt_exec_run_phase(a, recipe.before_buttons, recipe.before_frames, video,
                                        audio, &ticks_a, &before, &observed.metrics);
    if (before_finished)
        nt_exec_observe_guest(a, &before, &observed.metrics);
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
    observed.origin_state_hash = nt_exec_state_hash(saved, state_size);

    /* A different board runs between the source frame calls and its continuation.
       This is deterministic interleaving, not a simultaneous-call/thread-safety claim. */
    {
        uint8_t interloper_recipe[NT_EXEC_RECIPE_SIZE];
        struct nt_exec_recipe decoded_interloper;
        struct nt_exec_profile interloper_profile;
        memcpy(interloper_recipe, data, sizeof interloper_recipe);
        interloper_recipe[5] = profile.mapper == 1u ? NT_EXEC_PROFILE_NROM : NT_EXEC_PROFILE_MMC1;
        if (!nt_exec_decode(interloper_recipe, sizeof interloper_recipe, &decoded_interloper) ||
            !nt_exec_profile_for(decoded_interloper.profile_code, &interloper_profile))
            nt_exec_fail("different-board isolation recipe could not be decoded");
        interloper_image =
            nt_exec_generate(&decoded_interloper, interloper_recipe, sizeof interloper_recipe,
                             &interloper_profile, &interloper_image_size, &budget);
        if (interloper_image == NULL)
            nt_exec_fail("different-board isolation image could not be generated");
        interloper =
            nt_exec_instance(&budget, interloper_image, interloper_image_size, &observed.metrics);
        if (!nt_exec_run_phase(interloper, (uint8_t)(recipe.before_buttons ^ 0x80u), 1u, video,
                               audio, &ticks_b, &interloper_branch, &observed.metrics))
            nt_exec_fail("different-board interloper unexpectedly JAMmed");
    }
    b = nt_exec_instance(&budget, image, image_size, &observed.metrics);
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
                                         &ticks_a, &observed.continuation_a, &observed.metrics);
    after_b_finished = nt_exec_run_phase(b, recipe.after_buttons, recipe.after_frames, video, audio,
                                         &ticks_b, &observed.continuation_b, &observed.metrics);
    if (after_a_finished)
        nt_exec_observe_guest(a, &observed.continuation_a, &observed.metrics);
    if (after_b_finished)
        nt_exec_observe_guest(b, &observed.continuation_b, &observed.metrics);
    if (before_finished && after_a_finished && after_b_finished && size == NT_EXEC_RECIPE_SIZE &&
        recipe.profile_code != NT_EXEC_PROFILE_LOOP) {
        const uint8_t expected = nt_exec_expected_effects(&profile);
        observed.bank_effects = nt_exec_observed_effects(&profile, &before);
        if (observed.bank_effects != expected || before.guest_counter == 0u ||
            before.visible_pixels == 0u ||
            observed.continuation_a.guest_counter == before.guest_counter ||
            observed.continuation_a.controller_a != (uint8_t)(recipe.after_buttons & 1u)) {
            fprintf(stderr, "mapper=%u effects=%02x/%02x counter=%u/%u pixels=%u button=%u/%u\n",
                    (unsigned)profile.mapper, (unsigned)observed.bank_effects, (unsigned)expected,
                    before.guest_counter, observed.continuation_a.guest_counter,
                    before.visible_pixels, observed.continuation_a.controller_a,
                    (unsigned)(recipe.after_buttons & 1u));
            nt_exec_fail("deep recipe did not qualify its guest, bank or scheduled-input effects");
        }
    }
    observed.completed_frames = observed.metrics.frame_ok;
    observed.metrics.qualified_bank_effects = observed.bank_effects;
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
    nesturbator_unload_cartridge(interloper);
    nt_exec_operation(&observed.metrics);
    nesturbator_destroy(a);
    nt_exec_operation(&observed.metrics);
    nesturbator_destroy(b);
    nt_exec_operation(&observed.metrics);
    nesturbator_destroy(interloper);
    nt_exec_free(&budget, state_a, state_size);
    nt_exec_free(&budget, state_b, state_size);
    nt_exec_free(&budget, video, NT_EXEC_VIDEO_BYTES);
    nt_exec_free(&budget, audio, NT_EXEC_AUDIO_BYTES);
    nt_exec_free(&budget, image, image_size);
    nt_exec_free(&budget, interloper_image, interloper_image_size);
    if (budget.current != 0u)
        nt_exec_fail("per-input allocations were not fully released");
    observed.metrics.allocation_peak = (uint32_t)budget.peak;
    observed.operation_count = observed.metrics.operations;
    if (out != NULL)
        *out = observed;
    return 1;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    (void)nt_exec_process(data, size, NULL);
    return 0;
}

#ifdef NESTURBATOR_FUZZ_REPLAY
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
        const char *reason = status == NT_EXEC_READ_TOO_LARGE
                                 ? "input exceeds descriptor and board-image cap"
                             : status == NT_EXEC_READ_MEMORY ? "out of memory reading input"
                                                             : "invalid or unreadable input";
        fprintf(stderr, NT_EXEC_REPLAY_NAME ": %s: %s\n", reason, path);
        return 0;
    }
    return 1;
}

static int nt_exec_seed_code(const char *name)
{
    static const char *const names[] = {"nrom-deep", "mmc1-deep",  "uxrom-deep", "cnrom-deep",
                                        "mmc3-deep", "axrom-deep", "jam",        "loop"};
    for (size_t i = 0u; i < sizeof names / sizeof names[0]; ++i)
        if (strcmp(name, names[i]) == 0)
            return (int)i + 1;
    return 0;
}

static int nt_exec_check_seeds(int argc, char **argv)
{
    uint8_t seen[9] = {0u};
    unsigned valid_count = 0u;
    if (argc < 3) {
        fputs(NT_EXEC_REPLAY_NAME ": --check-seeds requires the eight committed recipes\n", stderr);
        return 1;
    }
    for (int i = 2; i < argc; ++i) {
        uint8_t expected[NT_EXEC_RECIPE_SIZE];
        uint8_t *bytes = NULL;
        size_t size = 0u;
        struct nt_exec_observation observed;
        struct nt_exec_profile profile;
        const int code = nt_exec_seed_code(nt_exec_basename(argv[i]));
        if (code == 0 || !nt_exec_read_path(argv[i], &bytes, &size)) {
            fprintf(stderr, NT_EXEC_REPLAY_NAME ": unexpected recipe seed: %s\n", argv[i]);
            free(bytes);
            return 1;
        }
        nt_exec_seed(expected);
        expected[5] = (uint8_t)code;
        if (seen[code] != 0u || size != sizeof expected ||
            memcmp(bytes, expected, sizeof expected) != 0 ||
            !nt_exec_profile_for((uint8_t)code, &profile)) {
            fprintf(stderr, NT_EXEC_REPLAY_NAME ": seed descriptor or family path mismatch: %s\n",
                    argv[i]);
            free(bytes);
            return 1;
        }
        if (!nt_exec_process(bytes, size, &observed)) {
            fprintf(stderr, NT_EXEC_REPLAY_NAME ": recipe did not execute: %s\n", argv[i]);
            free(bytes);
            return 1;
        }
        {
            int qualified =
                observed.metrics.accepted != 0u && observed.states_equal &&
                observed.operation_count != 0u &&
                observed.metrics.generated_bytes == nt_exec_image_size(&profile) &&
                observed.metrics.mapper == profile.mapper &&
                observed.metrics.qualified_bank_effects == nt_exec_expected_effects(&profile);
            if (code == NT_EXEC_PROFILE_JAM) {
                qualified = qualified && observed.metrics.stop_status == NESTURBATOR_STOP_JAM &&
                            observed.metrics.frame_jam == 3u && observed.metrics.frame_ok == 1u &&
                            observed.metrics.frames == 4u;
            } else {
                qualified = qualified && observed.metrics.frame_ok == observed.metrics.frames &&
                            observed.metrics.frames >= 3u &&
                            observed.completed_frames == observed.metrics.frames;
            }
            if (!qualified) {
                fprintf(stderr,
                        NT_EXEC_REPLAY_NAME
                        ": seed did not qualify: %s accepted=%u frames=%u/%u completed=%u ops=%u "
                        "bytes=%u mapper=%u effects=0x%02x expected=0x%02x state=%d\n",
                        argv[i], observed.metrics.accepted, observed.metrics.frame_ok,
                        observed.metrics.frames, observed.completed_frames,
                        observed.operation_count, observed.metrics.generated_bytes,
                        (unsigned)observed.metrics.mapper,
                        (unsigned)observed.metrics.qualified_bank_effects,
                        (unsigned)nt_exec_expected_effects(&profile), observed.states_equal);
                free(bytes);
                return 1;
            }
        }
        seen[code] = 1u;
        valid_count++;
        printf("%s: mapper=%u frames=%u operations=%u effects=0x%02x state=%u\n",
               nt_exec_basename(argv[i]), (unsigned)observed.mapper, observed.completed_frames,
               observed.operation_count, (unsigned)observed.bank_effects, observed.states_equal);
        free(bytes);
    }
    if (valid_count != 8u) {
        fputs(NT_EXEC_REPLAY_NAME ": corpus must contain exactly eight qualified recipes\n",
              stderr);
        return 1;
    }
    for (size_t i = 1u; i <= 8u; ++i) {
        if (seen[i] == 0u) {
            fputs(NT_EXEC_REPLAY_NAME ": a required family seed is missing\n", stderr);
            return 1;
        }
    }
    puts(NT_EXEC_REPLAY_NAME ": six family, loop and JAM recipes executed and restored");
    return 0;
}

static int nt_exec_self_test(void)
{
    uint8_t seed[NT_EXEC_RECIPE_SIZE];
    uint8_t oversize[NT_EXEC_MAX_INPUT_BYTES + 1u] = {0u};
    uint8_t mutation[NT_EXEC_RECIPE_SIZE + 1u];
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
    seed[6] = 1u;
    seed[8] = 2u;
    if (!nt_exec_decode(seed, sizeof seed, &recipe) ||
        96u + 2u * recipe.before_frames + 4u * recipe.after_frames > NT_EXEC_MAX_OPERATIONS ||
        1u + recipe.before_frames + 2u * recipe.after_frames != NT_EXEC_MAX_FRAME_CALLS)
        return 1;
    seed[6] = 3u;
    if (nt_exec_decode(seed, sizeof seed, &recipe))
        return 1;
    nt_exec_seed(seed);
    memcpy(mutation, seed, sizeof seed);
    mutation[NT_EXEC_RECIPE_SIZE] = 0xffu; /* Arbitrary raw ROM-byte mutation remains accepted. */
    if (!nt_exec_decode(mutation, sizeof mutation, &recipe))
        return 1;
    nt_exec_seed(seed);
    seed[6] = 0xffu;
    seed[7] = 0xffu;
    if (nt_exec_decode(seed, sizeof seed, &recipe))
        return 1;
    nt_exec_seed(seed);
    seed[5] = 9u;
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
    seed[5] = NT_EXEC_PROFILE_JAM;
    if (!nt_exec_process(seed, sizeof seed, &jam_observation) ||
        jam_observation.metrics.accepted == 0u || !jam_observation.states_equal ||
        jam_observation.metrics.stop_status != (uint32_t)NESTURBATOR_STOP_JAM ||
        jam_observation.metrics.frames != 4u || jam_observation.metrics.frame_ok != 1u ||
        jam_observation.metrics.frame_jam != 3u)
        return 1;
    puts(NT_EXEC_REPLAY_NAME
         ": descriptor, raw-byte mutation, allocation, frame and JAM boundaries passed");
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
    mismatched.prg_marker ^= 1u;
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
    uint32_t max_state_bytes = 0u;
    uint32_t last_stop = 0u;
    uint32_t last_mapper = 0u;
    uint32_t last_submapper = 0u;
    uint32_t last_effects = 0u;
    uint32_t last_completed_frames = 0u;
    uint32_t last_operation = 0u;
    uint64_t peak_rss_bytes = 0u;
#if defined(__APPLE__) || defined(__linux__)
    struct rusage usage;
#endif
    uint64_t frame_ok = 0u;
    uint64_t frame_jam = 0u;
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
            frame_ok += observed.metrics.frame_ok;
            frame_jam += observed.metrics.frame_jam;
            operations += observed.metrics.operations;
            generated_bytes += observed.metrics.generated_bytes;
            guest_ticks += observed.metrics.guest_ticks;
            if (observed.metrics.allocation_peak > allocation_peak)
                allocation_peak = observed.metrics.allocation_peak;
            if (observed.metrics.state_bytes > max_state_bytes)
                max_state_bytes = observed.metrics.state_bytes;
            last_stop = observed.metrics.stop_status;
            last_mapper = observed.metrics.mapper;
            last_submapper = observed.metrics.submapper;
            last_effects = observed.metrics.qualified_bank_effects;
            last_completed_frames = observed.completed_frames;
            last_operation = observed.operation_count > 0u ? observed.operation_count - 1u : 0u;
            free(bytes);
        }
    }
#if defined(__APPLE__)
    if (getrusage(RUSAGE_SELF, &usage) == 0 && usage.ru_maxrss >= 0)
        peak_rss_bytes = (uint64_t)usage.ru_maxrss;
#elif defined(__linux__)
    if (getrusage(RUSAGE_SELF, &usage) == 0 && usage.ru_maxrss >= 0)
        peak_rss_bytes = (uint64_t)usage.ru_maxrss * 1024u;
#endif
    printf("{\"inputs\":%llu,\"accepted\":%llu,\"frames\":%llu,\"frame_ok\":%llu,"
           "\"frame_jam\":%llu,\"api_operations\":%llu,"
           "\"generated_bytes\":%llu,\"max_state_bytes\":%u,\"guest_ticks\":%llu,"
           "\"peak_allocation_bytes\":%u,"
           "\"last_stop_status\":%u,\"peak_rss_bytes\":%llu,\"last_mapper\":%u,"
           "\"last_submapper\":%u,\"last_bank_effects\":%u,\"last_completed_frames\":%u,"
           "\"last_operation\":%u}\n",
           (unsigned long long)inputs, (unsigned long long)accepted, (unsigned long long)frames,
           (unsigned long long)frame_ok, (unsigned long long)frame_jam,
           (unsigned long long)operations, (unsigned long long)generated_bytes, max_state_bytes,
           (unsigned long long)guest_ticks, allocation_peak, last_stop,
           (unsigned long long)peak_rss_bytes, last_mapper, last_submapper, last_effects,
           last_completed_frames, last_operation);
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
