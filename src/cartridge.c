/* Mapper-0 cartridge ownership and PRG access. */
#include <string.h>

#include "internal.h"

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

uint8_t nesturbator__cart_read(struct nesturbator *nes, uint16_t addr)
{
    return nes->cart.prg[(addr - 0x8000u) & 0x3fffu];
}
