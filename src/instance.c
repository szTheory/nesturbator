/* Instance lifetime, version and format queries, and the size-tag helper. */
#include <stdlib.h>
#include <string.h>

#include "internal.h"

nesturbator_status nesturbator__check_size_in(const void *s, uint32_t first, uint32_t ours)
{
    const uint8_t *bytes = (const uint8_t *)s;
    uint32_t size;
    memcpy(&size, s, sizeof size); /* the tag is untrusted: read it alone first */
    if (size == 0u || size < first) {
        return NESTURBATOR_ERR_STRUCT_SIZE;
    }
    /* A newer caller's struct is accepted only if the fields this build does
       not know are all zero. */
    for (uint32_t i = ours; i < size; i++) {
        if (bytes[i] != 0u) {
            return NESTURBATOR_ERR_STRUCT_SIZE;
        }
    }
    return NESTURBATOR_OK;
}

/* Writes min(caller size, ours) bytes of a fully built output struct whose
   own size field already holds the caller's value. */
static void write_out(void *out, const void *full, uint32_t ours)
{
    uint32_t size;
    memcpy(&size, out, sizeof size);
    memcpy(out, full, size < ours ? size : ours);
}

void nesturbator_get_version(nesturbator_version *out)
{
    if (out == NULL || out->size == 0u) {
        return;
    }
    nesturbator_version v;
    v.size = out->size;
    v.major = NESTURBATOR_VERSION_MAJOR;
    v.minor = NESTURBATOR_VERSION_MINOR;
    v.patch = NESTURBATOR_VERSION_PATCH;
    v.abi = NESTURBATOR_ABI_VERSION;
    v.behaviour_revision = NESTURBATOR_BEHAVIOUR_REVISION;
    write_out(out, &v, (uint32_t)sizeof v);
}

void nesturbator_get_info(const nesturbator *inst, nesturbator_info *out)
{
    if (inst == NULL || out == NULL || out->size == 0u) {
        return;
    }
    nesturbator_info info;
    info.size = out->size;
    info.width = NESTURBATOR_WIDTH;
    info.height = NESTURBATOR_HEIGHT;
    info.fps_num = 39375000u; /* NTSC, 714732 ticks per frame (internal.h) */
    info.fps_den = 655171u;
    info.sample_rate_num = 48000u;
    info.sample_rate_den = 1u;
    write_out(out, &info, (uint32_t)sizeof info);
}

/* The default allocator, used when the config's allocator is all NULL. */
static void *default_alloc(void *user, size_t size)
{
    (void)user;
    return malloc(size);
}

static void default_free(void *user, void *ptr, size_t size)
{
    (void)user;
    (void)size;
    free(ptr);
}

nesturbator_status nesturbator_create(const nesturbator_config *cfg, nesturbator **out)
{
    if (cfg == NULL || out == NULL) {
        return NESTURBATOR_ERR_ARGUMENT;
    }
    nesturbator_status st =
        nesturbator__check_size_in(cfg, NESTURBATOR_CONFIG_SIZE_V1, (uint32_t)sizeof *cfg);
    if (st != NESTURBATOR_OK) {
        return st;
    }
    if (cfg->abi != NESTURBATOR_ABI_VERSION) {
        return NESTURBATOR_ERR_ABI;
    }
    nesturbator_allocator a = cfg->allocator;
    if (a.alloc == NULL && a.free == NULL && a.user == NULL) {
        a.alloc = default_alloc;
        a.free = default_free;
    } else if (a.alloc == NULL || a.free == NULL) {
        return NESTURBATOR_ERR_ARGUMENT;
    }
    nesturbator *inst = (nesturbator *)a.alloc(a.user, sizeof *inst);
    if (inst == NULL) {
        return NESTURBATOR_ERR_NO_MEMORY;
    }
    memset(inst, 0, sizeof *inst);
    inst->allocator = a;
    /* Start before visible line zero so the pre-render PPU work can populate
       its first background and sprite state. [HWP.03][HWP.06] */
    inst->ppu.scanline = 261u;
    inst->apu.dmc.buffer_empty = 1u;
    inst->profile.ane_magic = NESTURBATOR_RP2A03G_ANE_MAGIC; /* D-14 */
    inst->profile.lxa_magic = NESTURBATOR_RP2A03G_LXA_MAGIC;
    *out = inst;
    return NESTURBATOR_OK;
}

nesturbator_status nesturbator_set_input(nesturbator *inst, const nesturbator_input *input)
{
    if (inst == NULL || input == NULL) {
        return NESTURBATOR_ERR_ARGUMENT;
    }
    nesturbator_status st =
        nesturbator__check_size_in(input, NESTURBATOR_INPUT_SIZE_V1, (uint32_t)sizeof *input);
    if (st != NESTURBATOR_OK) {
        return st;
    }
    inst->bus.input_pending[0] = input->buttons[0];
    inst->bus.input_pending[1] = input->buttons[1];
    return NESTURBATOR_OK;
}

void nesturbator__controller_begin_frame(struct nesturbator *nes)
{
    nes->bus.input_buttons[0] = nes->bus.input_pending[0];
    nes->bus.input_buttons[1] = nes->bus.input_pending[1];
}

void nesturbator_destroy(nesturbator *inst)
{
    if (inst == NULL) {
        return;
    }
    nesturbator_allocator a = inst->allocator;
    if (inst->cart.bytes != NULL) {
        (a.free)(a.user, inst->cart.bytes, inst->cart.size);
    }
    (a.free)(a.user, inst, sizeof *inst);
}

nesturbator_status nesturbator_peek_cpu_ram(const nesturbator *inst, uint16_t address,
                                            uint8_t *value)
{
    if (inst == NULL || value == NULL || address >= 0x2000u) {
        return NESTURBATOR_ERR_ARGUMENT;
    }
    *value = inst->bus.ram[address & 0x07ffu];
    return NESTURBATOR_OK;
}

nesturbator_status nesturbator_reset(nesturbator *inst)
{
    if (inst == NULL) {
        return NESTURBATOR_ERR_ARGUMENT;
    }
    if (inst->cart.bytes == NULL) {
        return NESTURBATOR_OK;
    }
    nesturbator__ppu_run_until(inst, inst->ticks);
    /* Host input state (input_pending, input_buttons, controller_latch) stays. */
    inst->bus.controller_strobe = 0u;
    inst->bus.controller_shift[0] = 0u;
    inst->bus.controller_shift[1] = 0u;
    inst->bus.oam_dma_pending = 0u;
    nesturbator__ppu_reset(inst);
    nesturbator__apu_reset(inst);
    nesturbator__cpu_reset(inst);
    return NESTURBATOR_OK;
}
