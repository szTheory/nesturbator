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
    inst->profile.ane_magic = NESTURBATOR_RP2A03G_ANE_MAGIC; /* D-14 */
    inst->profile.lxa_magic = NESTURBATOR_RP2A03G_LXA_MAGIC;
    *out = inst;
    return NESTURBATOR_OK;
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

void nesturbator_unload_cartridge(nesturbator *inst)
{
    if (inst == NULL || inst->cart.bytes == NULL) {
        return;
    }
    inst->allocator.free(inst->allocator.user, inst->cart.bytes, inst->cart.size);
    memset(&inst->cart, 0, sizeof inst->cart);
    memset(&inst->ppu, 0, sizeof inst->ppu);
    memset(&inst->cpu, 0, sizeof inst->cpu);
    inst->ticks = 0;
    inst->frame_number = 0;
    inst->audio_rem = 0;
}

nesturbator_status nesturbator_load_cartridge(nesturbator *inst, const void *data, size_t size)
{
    const uint8_t *image = (const uint8_t *)data;
    size_t offset;
    uint8_t *copy;
    if (inst == NULL || data == NULL) {
        return NESTURBATOR_ERR_ARGUMENT;
    }
    if (size < 16u || image[0] != 'N' || image[1] != 'E' || image[2] != 'S' || image[3] != 0x1au ||
        image[4] != 1u || image[5] != 1u || (image[6] & 0xfcu) != 0u || (image[7] & 0xf0u) != 0u ||
        (image[7] & 0x0cu) == 0x08u) {
        return NESTURBATOR_ERR_CARTRIDGE;
    }
    offset = 16u + ((image[6] & 0x04u) != 0u ? 512u : 0u);
    if (size != offset + 16384u + 8192u) {
        return NESTURBATOR_ERR_CARTRIDGE;
    }
    copy = (uint8_t *)inst->allocator.alloc(inst->allocator.user, size);
    if (copy == NULL) {
        return NESTURBATOR_ERR_NO_MEMORY;
    }
    memcpy(copy, image, size);
    if (inst->cart.bytes != NULL) {
        inst->allocator.free(inst->allocator.user, inst->cart.bytes, inst->cart.size);
    }
    memset(&inst->cart, 0, sizeof inst->cart);
    inst->cart.bytes = copy;
    inst->cart.size = size;
    inst->cart.prg = copy + offset;
    inst->cart.chr = inst->cart.prg + 16384u;
    memset(&inst->bus, 0, sizeof inst->bus);
    memset(&inst->ppu, 0, sizeof inst->ppu);
    memset(&inst->cpu, 0, sizeof inst->cpu);
    inst->cpu.s = 0xfdu;
    inst->cpu.p = 0x24u;
    inst->cpu.pc = (uint16_t)(inst->cart.prg[0x3ffcu] | ((uint16_t)inst->cart.prg[0x3ffdu] << 8));
    inst->ticks = 0;
    inst->frame_number = 0;
    inst->audio_rem = 0;
    return NESTURBATOR_OK;
}
