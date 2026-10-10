/* Cartridge ownership, validation and board choice. */
#include <string.h>
#include <stdint.h>

#include "internal.h"

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

/* The accepted shapes of each board (BOARD-01): submapper range, PRG size and
   CHR kind. Every other id has no row and is refused. */
static int board_profile_ok(uint16_t mapper, uint8_t submapper, size_t prg_size, size_t chr_size,
                            int chr_is_ram)
{
    const int chr_8k_ok = chr_is_ram || chr_size == 8192u;
    switch (mapper) {
    case 0u:
        return chr_8k_ok && submapper == 0u && (prg_size == 16384u || prg_size == 32768u);
    case 2u:
        /* UxROM: up to 4 MiB of PRG in 16 KiB banks. */
        return chr_8k_ok && submapper <= 2u && prg_size != 0u && prg_size % 16384u == 0u &&
               prg_size <= 4194304u;
    case 3u:
        /* CNROM: 16 or 32 KiB PRG and 8, 16 or 32 KiB of CHR ROM, never CHR RAM. */
        return !chr_is_ram && submapper <= 2u && (prg_size == 16384u || prg_size == 32768u) &&
               (chr_size == 8192u || chr_size == 16384u || chr_size == 32768u);
    case 7u:
        /* AxROM: 32 to 256 KiB of PRG in 32 KiB banks and 8 KiB of CHR RAM. */
        return chr_is_ram && submapper <= 2u && prg_size >= 32768u && prg_size <= 262144u &&
               prg_size % 32768u == 0u;
    default:
        return 0;
    }
}

static int validate_image(const uint8_t *image, size_t size, struct cartridge_layout *layout)
{
    size_t total;
    int nes2;
    uint16_t mapper;
    uint8_t submapper = 0u;
    struct nesturbator__mapper_ops probe;
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
    mapper = (uint16_t)((image[6] >> 4) | (image[7] & 0xf0u));
    if (nes2) {
        size_t prg_ram, prg_nvram, chr_ram, chr_nvram;
        mapper = (uint16_t)(mapper | ((image[8] & 0x0fu) << 8));
        submapper = (uint8_t)(image[8] >> 4);
        if (image[7] & 3u || image[13] != 0u || image[14] != 0u || image[15] != 0u ||
            image[12] != 0u)
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
        else if (chr_ram != 0u)
            return 0;
        if (image[6] & 2u)
            return 0;
    } else {
        if ((image[7] & 0xf3u) != 0u)
            return 0;
        for (size_t i = 8u; i < 16u; ++i)
            if (image[i] != 0u)
                return 0;
        if (!checked_mul(image[4], 16384u, &layout->prg_size) ||
            !checked_mul(image[5], 8192u, &layout->chr_size))
            return 0;
        if (image[5] == 0u)
            layout->chr_is_ram = 1;
    }
    if (!board_profile_ok(mapper, submapper, layout->prg_size, layout->chr_size,
                          layout->chr_is_ram))
        return 0;
    if (!checked_add(16u, layout->trainer_size, &total) ||
        !checked_add(total, layout->prg_size, &total) ||
        !checked_add(total, layout->chr_size, &total) || total != size)
        return 0;
    /* Probe the board switch before anything is allocated, so an image whose
       mapper has no board leaves the instance as it was (D-10). */
    if (!nesturbator__mapper_ops_for(mapper, &probe))
        return 0;
    return 1;
}

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
    memset(&inst->mapper, 0, sizeof inst->mapper);
    memset(&inst->map, 0, sizeof inst->map);
    inst->cpu_cycle = 0;
    inst->ticks = 0;
    inst->frame_number = 0;
    inst->audio_rem = 0;
}

/* The only place a board is chosen; each board adds a case. validate_image
   probes it before allocating (D-10). */
int nesturbator__mapper_ops_for(uint16_t id, struct nesturbator__mapper_ops *out)
{
    switch (id) {
    case 0u:
        nesturbator__mapper_nrom_ops(out);
        return 1;
    case 2u:
        nesturbator__mapper_uxrom_ops(out);
        return 1;
    case 3u:
        nesturbator__mapper_cnrom_ops(out);
        return 1;
    case 7u:
        nesturbator__mapper_axrom_ops(out);
        return 1;
    default:
        return 0;
    }
}

/* A state load calls this too, because the pages are derived and never
   serialised (ARCHITECTURE section 6). */
int nesturbator__mapper_load(struct nesturbator *nes)
{
    memset(&nes->map, 0, sizeof nes->map);
    if (!nesturbator__mapper_ops_for(nes->mapper.id, &nes->map.ops))
        return 0;
    nes->map.ops.init(nes);
    nes->map.ops.rebuild(nes);
    return 1;
}

nesturbator_status nesturbator_load_cartridge(nesturbator *inst, const void *data, size_t size)
{
    const uint8_t *image = (const uint8_t *)data;
    struct cartridge_layout layout;
    size_t offset, allocation_size, chr_ram_offset;
    uint8_t *copy;
    if (inst == NULL || data == NULL) {
        return NESTURBATOR_ERR_ARGUMENT;
    }
    memset(&layout, 0, sizeof layout);
    if (!validate_image(image, size, &layout))
        return NESTURBATOR_ERR_CARTRIDGE;
    offset = 16u + layout.trainer_size;
    allocation_size = size;
    chr_ram_offset = size;
    if (layout.trainer_size != 0u) {
        if (!checked_add(allocation_size, 8192u, &allocation_size))
            return NESTURBATOR_ERR_CARTRIDGE;
        chr_ram_offset += 8192u;
    }
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
    inst->cart.prg_size = layout.prg_size;
    inst->cart.chr = inst->cart.prg + layout.prg_size;
    if (layout.trainer_size != 0u) {
        /* HWP.14 places the 512-byte trainer at CPU $7000-$71FF. */
        inst->cart.prg_ram = copy + size;
        memset(inst->cart.prg_ram, 0, 8192u);
        memcpy(inst->cart.prg_ram + 0x1000u, image + 16u, 512u);
    }
    inst->cart.chr_size = layout.chr_is_ram ? 8192u : layout.chr_size;
    inst->cart.chr_is_ram = (uint8_t)layout.chr_is_ram;
    if (layout.chr_is_ram)
        inst->cart.chr = copy + chr_ram_offset;
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
    inst->ticks = 0;
    inst->frame_number = 0;
    inst->audio_rem = 0;
    inst->cpu_cycle = 0;
    /* iNES: mapper number from flags 6 and 7; NES2: bits 8-11 in byte 8 and the
       submapper in its high nibble. The per-board profile decides what the validator accepts. */
    memset(&inst->mapper, 0, sizeof inst->mapper);
    inst->mapper.id = (uint16_t)((image[6] >> 4) | (image[7] & 0xf0u));
    if ((image[7] & 0x0cu) == 0x08u) {
        inst->mapper.id = (uint16_t)(inst->mapper.id | ((image[8] & 0x0fu) << 8));
        inst->mapper.submapper = (uint8_t)(image[8] >> 4);
    }
    /* Last, so the pages see the final cartridge pointers. */
    if (!nesturbator__mapper_load(inst)) {
        /* Unreachable: validate_image probed the same switch. */
        nesturbator_unload_cartridge(inst);
        return NESTURBATOR_ERR_CARTRIDGE;
    }
    /* The reset vector is read through the board's power-on pages, not the
       file's last bytes: AxROM powers on in bank 0 (D-11). No bus cycle, no
       time. */
    inst->cpu.pc = (uint16_t)(nesturbator__map_cpu_read(inst, 0xfffcu) |
                              ((uint16_t)nesturbator__map_cpu_read(inst, 0xfffdu) << 8));
    return NESTURBATOR_OK;
}
