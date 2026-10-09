/* Mapper-0 cartridge ownership and PRG access. */
#include <string.h>
#include <stdint.h>

#define NESTURBATOR_CART_MAX_SIZE (64u * 1024u * 1024u)

struct cartridge_layout {
    size_t prg_size;
    size_t chr_size;
    size_t trainer_size;
    int chr_is_ram;
};

static int checked_add(size_t a, size_t b, size_t *out)
{
    if (b > SIZE_MAX - a)
        return 0;
    *out = a + b;
    return 1;
}

static int checked_mul(size_t a, size_t b, size_t *out)
{
    if (a != 0u && b > SIZE_MAX / a)
        return 0;
    *out = a * b;
    return 1;
}

static int nes2_rom_size(uint8_t low, uint8_t high, size_t unit, size_t *out)
{
    if (high != 0x0fu) {
        size_t count = (size_t)low | ((size_t)high << 8);
        return checked_mul(count, unit, out);
    }
    unsigned exponent = low >> 2;
    unsigned multiplier = low & 3u;
    if (exponent >= sizeof(size_t) * 8u)
        return 0;
    size_t base = (size_t)1u << exponent;
    return checked_mul(base, (size_t)(multiplier * 2u + 1u), out);
}

static int nes2_ram_size(uint8_t shift, size_t *out)
{
    if (shift == 0u) {
        *out = 0u;
        return 1;
    }
    if (shift > 15u)
        return 0;
    *out = (size_t)64u << shift;
    return 1;
}

static int validate_image(const uint8_t *image, size_t size, struct cartridge_layout *layout)
{
    size_t total;
    int nes2;
    if (size < 16u || size > NESTURBATOR_CART_MAX_SIZE || image[0] != 'N' || image[1] != 'E' ||
        image[2] != 'S' || image[3] != 0x1au)
        return 0;
    /* iNES v1 and NES v2 header/layout rules are documented by NESdev. [HWP 14] */
    nes2 = (image[7] & 0x0cu) == 0x08u;
    if ((image[7] & 0x0cu) == 0x04u || (image[7] & 0x0cu) == 0x0cu)
        return 0;
    if ((image[6] & 0x0au) != 0u || (image[7] & 0xf0u) != 0u)
        return 0;
    layout->trainer_size = (image[6] & 4u) != 0u ? 512u : 0u;
    layout->chr_is_ram = 0;
    if (nes2) {
        size_t prg_ram, prg_nvram, chr_ram, chr_nvram;
        uint16_t mapper =
            (uint16_t)((image[6] >> 4) | (image[7] & 0xf0u) | ((image[8] & 0x0fu) << 8));
        if (mapper != 0u || (image[8] >> 4) != 0u || image[7] & 3u || image[13] != 0u ||
            image[14] != 0u || image[15] != 0u || image[12] != 0u)
            return 0;
        if (!nes2_rom_size(image[4], image[9] & 0x0fu, 16384u, &layout->prg_size) ||
            !nes2_rom_size(image[5], image[9] >> 4, 8192u, &layout->chr_size) ||
            !nes2_ram_size(image[10] & 0x0fu, &prg_ram) ||
            !nes2_ram_size(image[10] >> 4, &prg_nvram) ||
            !nes2_ram_size(image[11] & 0x0fu, &chr_ram) ||
            !nes2_ram_size(image[11] >> 4, &chr_nvram))
            return 0;
        if (prg_ram != 0u || prg_nvram != 0u || chr_nvram != 0u)
            return 0;
        if (layout->chr_size == 0u && chr_ram == 8192u)
            layout->chr_is_ram = 1;
        else if (layout->chr_size != 8192u || chr_ram != 0u)
            return 0;
        if (image[6] & 2u)
            return 0;
    } else {
        if ((image[6] >> 4) != 0u || (image[7] & 0xf3u) != 0u)
            return 0;
        for (size_t i = 8u; i < 16u; ++i)
            if (image[i] != 0u)
                return 0;
        if (!checked_mul(image[4], 16384u, &layout->prg_size) ||
            !checked_mul(image[5], 8192u, &layout->chr_size))
            return 0;
        if (image[4] == 0u || (image[4] != 1u && image[4] != 2u))
            return 0;
        if (image[5] == 0u)
            layout->chr_is_ram = 1;
        else if (image[5] != 1u)
            return 0;
    }
    if (layout->prg_size != 16384u && layout->prg_size != 32768u)
        return 0;
    if (!layout->chr_is_ram && layout->chr_size != 8192u)
        return 0;
    if (!checked_add(16u, layout->trainer_size, &total) ||
        !checked_add(total, layout->prg_size, &total) ||
        !checked_add(total, layout->chr_size, &total) || total != size)
        return 0;
    return 1;
}

#include "internal.h"

void nesturbator_unload_cartridge(nesturbator *inst)
{
    if (inst == NULL || inst->cart.bytes == NULL) {
        return;
    }
    inst->allocator.free(inst->allocator.user, inst->cart.bytes, inst->cart.size);
    memset(&inst->cart, 0, sizeof inst->cart);
    memset(&inst->apu, 0, sizeof inst->apu);
    inst->apu.dmc.buffer_empty = 1u;
    nesturbator__synth_reset(inst);
    memset(&inst->ppu, 0, sizeof inst->ppu);
    memset(&inst->cpu, 0, sizeof inst->cpu);
    inst->ticks = 0;
    inst->frame_number = 0;
    inst->audio_rem = 0;
}

nesturbator_status nesturbator_load_cartridge(nesturbator *inst, const void *data, size_t size)
{
    const uint8_t *image = (const uint8_t *)data;
    struct cartridge_layout layout;
    size_t offset, allocation_size;
    uint8_t *copy;
    if (inst == NULL || data == NULL) {
        return NESTURBATOR_ERR_ARGUMENT;
    }
    memset(&layout, 0, sizeof layout);
    if (!validate_image(image, size, &layout))
        return NESTURBATOR_ERR_CARTRIDGE;
    offset = 16u + layout.trainer_size;
    allocation_size = size;
    if (layout.chr_is_ram && !checked_add(allocation_size, 8192u, &allocation_size))
        return NESTURBATOR_ERR_CARTRIDGE;
    copy = (uint8_t *)inst->allocator.alloc(inst->allocator.user, allocation_size);
    if (copy == NULL) {
        return NESTURBATOR_ERR_NO_MEMORY;
    }
    memcpy(copy, image, size);
    if (inst->cart.bytes != NULL) {
        inst->allocator.free(inst->allocator.user, inst->cart.bytes, inst->cart.size);
    }
    memset(&inst->cart, 0, sizeof inst->cart);
    inst->cart.bytes = copy;
    inst->cart.size = allocation_size;
    inst->cart.prg = copy + offset;
    inst->cart.chr = inst->cart.prg + layout.prg_size;
    inst->cart.chr_is_ram = (uint8_t)layout.chr_is_ram;
    if (layout.chr_is_ram)
        memset(inst->cart.chr, 0, 8192u);
    memset(&inst->bus, 0, sizeof inst->bus);
    memset(&inst->apu, 0, sizeof inst->apu);
    inst->apu.dmc.buffer_empty = 1u;
    nesturbator__synth_reset(inst);
    memset(&inst->ppu, 0, sizeof inst->ppu);
    memset(&inst->cpu, 0, sizeof inst->cpu);
    inst->cpu.s = 0xfdu;
    inst->cpu.p = 0x24u;
    /* The reset vector is at the end of PRG ROM; 16 KiB NROM mirrors its
       single bank, while 32 KiB NROM stores the vectors in the upper bank.
       [HWP.14] */
    inst->cpu.pc = (uint16_t)(inst->cart.prg[layout.prg_size - 4u] |
                              ((uint16_t)inst->cart.prg[layout.prg_size - 3u] << 8));
    inst->ticks = 0;
    inst->frame_number = 0;
    inst->audio_rem = 0;
    return NESTURBATOR_OK;
}

uint8_t nesturbator__cart_read(struct nesturbator *nes, uint16_t addr)
{
    return nes->cart.prg[(addr - 0x8000u) & (nes->cart.size >= 32784u ? 0x7fffu : 0x3fffu)];
}
