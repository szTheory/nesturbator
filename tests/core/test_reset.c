/* TUNE-06 (D-12 to D-14, D-18): soft reset through nesturbator_reset on a synthetic NROM image
   with a trainer, so PRG RAM exists. One function per D-18 item. */
#include <stdint.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"

#define IMAGE_CAP 40000u
#define RESET_VECTOR 0x8123u
#define CHR_RAM_SIZE 8192u
#define PRG_RAM_SIZE 8192u

/* PRG code: a jump to itself at $8000 and at $8123. */
static size_t build_image(uint8_t *out, int chr_ram)
{
    static const uint8_t trainer_fill = 0x6du;
    uint8_t trainer[INES_TRAINER_SIZE];
    uint8_t code[RESET_VECTOR - 0x8000u + 3u];
    struct ines_spec spec;
    memset(trainer, trainer_fill, sizeof trainer);
    memset(code, 0xeau, sizeof code);
    code[0] = 0x4cu;
    code[1] = 0x00u;
    code[2] = 0x80u;
    code[RESET_VECTOR - 0x8000u] = 0x4cu;
    code[RESET_VECTOR - 0x8000u + 1u] = 0x23u;
    code[RESET_VECTOR - 0x8000u + 2u] = 0x81u;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = chr_ram ? 0u : 1u;
    spec.trainer = trainer;
    spec.prg_code = code;
    spec.prg_code_len = sizeof code;
    spec.nmi_vector = 0x8000u;
    spec.reset_vector = RESET_VECTOR;
    spec.irq_vector = 0x8000u;
    return ines_build(out, IMAGE_CAP, &spec);
}

static struct nesturbator *make_instance(int chr_ram)
{
    static uint8_t image[IMAGE_CAP];
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    size_t size = build_image(image, chr_ram);
    CHECK(size != 0u);
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    return (struct nesturbator *)inst;
}

static void run_frames(struct nesturbator *nes, unsigned n)
{
    static uint16_t video[256u * 240u];
    static int16_t audio[2048];
    for (unsigned i = 0; i < n; ++i) {
        nesturbator_frame io;
        memset(&io, 0, sizeof io);
        io.size = (uint32_t)sizeof io;
        io.video = video;
        io.video_pitch = 256u;
        io.audio = audio;
        io.audio_capacity = 2048u;
        CHECK_EQ_U64(nesturbator_run_frame(nes, &io), NESTURBATOR_OK);
    }
}

static void check_null_and_empty(void)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    struct nesturbator *nes;
    uint8_t ram[16];
    CHECK_EQ_U64(nesturbator_reset(NULL), NESTURBATOR_ERR_ARGUMENT);
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    nes = (struct nesturbator *)inst;
    run_frames(nes, 1u);
    for (unsigned i = 0; i < 16u; ++i) {
        CHECK_EQ_U64(nesturbator_peek_cpu_ram(inst, (uint16_t)i, &ram[i]), NESTURBATOR_OK);
    }
    uint8_t s = nes->cpu.s, p = nes->cpu.p;
    uint16_t pc = nes->cpu.pc;
    uint64_t ticks = nes->ticks;
    CHECK_EQ_U64(nesturbator_reset(inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nes->cpu.s, s);
    CHECK_EQ_U64(nes->cpu.p, p);
    CHECK_EQ_U64(nes->cpu.pc, pc);
    CHECK_EQ_U64(nes->ticks, ticks);
    for (unsigned i = 0; i < 16u; ++i) {
        uint8_t v = 0xffu;
        CHECK_EQ_U64(nesturbator_peek_cpu_ram(inst, (uint16_t)i, &v), NESTURBATOR_OK);
        CHECK_EQ_U64(v, ram[i]);
    }
    nesturbator_destroy(inst);
}

/* Every asserted field starts non-default, so a skipped clear or keep changes the result. */
static void check_cpu_and_bus(void)
{
    struct nesturbator *nes = make_instance(0);
    nesturbator *inst = (nesturbator *)nes;
    uint8_t v = 0u;
    run_frames(nes, 2u);
    nes->bus.ram[0x0010u] = 0x5au;
    nes->cart.prg_ram[0] = 0xa5u;
    nes->cpu.a = 0x11u;
    nes->cpu.x = 0x22u;
    nes->cpu.y = 0x33u;
    nes->cpu.s = 0xf0u;
    nes->cpu.p = 0xebu;
    nes->cpu.jammed = 1u;
    nes->cpu.poll_latch = 1u;
    nes->bus.controller_strobe = 1u;
    nes->bus.controller_shift[0] = 0xa5u;
    nes->bus.controller_shift[1] = 0x5au;
    nes->bus.oam_dma_pending = 1u;
    nes->bus.input_pending[0] = 1u;
    nes->bus.input_pending[1] = 1u;
    nes->bus.input_buttons[0] = 0x81u;
    nes->bus.input_buttons[1] = 0x42u;
    nes->bus.controller_latch[0] = 0xc3u;
    nes->bus.controller_latch[1] = 0x3cu;
    uint64_t ticks = nes->ticks;
    uint64_t frames = nes->frame_number;

    CHECK_EQ_U64(nesturbator_reset(inst), NESTURBATOR_OK);
    CHECK_EQ_HEX(nesturbator_peek_cpu_ram(inst, 0x0010u, &v), NESTURBATOR_OK);
    CHECK_EQ_HEX(v, 0x5au);
    CHECK_EQ_HEX(nes->cart.prg_ram[0], 0xa5u);
    CHECK_EQ_HEX(nes->cpu.a, 0x11u);
    CHECK_EQ_HEX(nes->cpu.x, 0x22u);
    CHECK_EQ_HEX(nes->cpu.y, 0x33u);
    CHECK_EQ_HEX(nes->cpu.s, 0xedu);
    CHECK_EQ_HEX(nes->cpu.p, 0xefu);
    CHECK_EQ_U64(nes->cpu.jammed, 0u);
    CHECK_EQ_U64(nes->cpu.poll_latch, 0u);
    CHECK_EQ_U64(nes->bus.controller_strobe, 0u);
    CHECK_EQ_U64(nes->bus.controller_shift[0], 0u);
    CHECK_EQ_U64(nes->bus.controller_shift[1], 0u);
    CHECK_EQ_U64(nes->bus.oam_dma_pending, 0u);
    CHECK_EQ_U64(nes->bus.input_pending[0], 1u);
    CHECK_EQ_U64(nes->bus.input_pending[1], 1u);
    CHECK_EQ_HEX(nes->bus.input_buttons[0], 0x81u);
    CHECK_EQ_HEX(nes->bus.input_buttons[1], 0x42u);
    CHECK_EQ_HEX(nes->bus.controller_latch[0], 0xc3u);
    CHECK_EQ_HEX(nes->bus.controller_latch[1], 0x3cu);
    CHECK_EQ_HEX(nes->cpu.pc, RESET_VECTOR);
    CHECK_EQ_U64(nes->ticks, ticks + 168u);
    CHECK_EQ_U64(nes->frame_number, frames);

    /* Idempotency: a second reset lowers S by 3 again and keeps memory. */
    CHECK_EQ_U64(nesturbator_reset(inst), NESTURBATOR_OK);
    CHECK_EQ_HEX(nes->cpu.s, 0xeau);
    CHECK_EQ_U64(nes->ticks, ticks + 336u);
    CHECK_EQ_HEX(nes->bus.ram[0x0010u], 0x5au);
    CHECK_EQ_HEX(nes->cart.prg_ram[0], 0xa5u);
    nesturbator_destroy(inst);
}

/* TUNE-06: battery RAM and mapper registers are kept. NROM has no mapper registers today; the
   struct compare covers any mapper-state field later phases add to struct nesturbator__cartridge
   (a mapper that keeps its registers elsewhere extends this check). */
static void check_cartridge_state_kept(void)
{
    struct nesturbator *nes = make_instance(0);
    nesturbator *inst = (nesturbator *)nes;
    struct nesturbator__cartridge before;
    static uint8_t ram_before[PRG_RAM_SIZE];
    run_frames(nes, 2u);
    for (unsigned i = 0; i < PRG_RAM_SIZE; ++i) {
        nesturbator__bus_write(nes, (uint16_t)(0x6000u + i), (uint8_t)(i * 5u + 3u));
    }
    memcpy(&before, &nes->cart, sizeof before);
    memcpy(ram_before, nes->cart.prg_ram, PRG_RAM_SIZE);
    CHECK_EQ_U64(nesturbator_reset(inst), NESTURBATOR_OK);
    CHECK_EQ_U64(memcmp(&before, &nes->cart, sizeof before), 0);
    CHECK_EQ_U64(memcmp(ram_before, nes->cart.prg_ram, PRG_RAM_SIZE), 0);
    nesturbator_destroy(inst);
}

/* D-13: CHR RAM is kept. The CHR ROM of the trainer image cannot show this. */
static void check_chr_ram_kept(void)
{
    struct nesturbator *nes = make_instance(1);
    nesturbator *inst = (nesturbator *)nes;
    static uint8_t copy[CHR_RAM_SIZE];
    CHECK_EQ_U64(nes->cart.chr_is_ram, 1u);
    run_frames(nes, 2u);
    for (unsigned i = 0; i < CHR_RAM_SIZE; ++i) {
        nes->cart.chr[i] = (uint8_t)(i * 7u + 1u);
    }
    memcpy(copy, nes->cart.chr, CHR_RAM_SIZE);
    CHECK_EQ_U64(nesturbator_reset(inst), NESTURBATOR_OK);
    CHECK_EQ_U64(memcmp(copy, nes->cart.chr, CHR_RAM_SIZE), 0);
    nesturbator_destroy(inst);
}

/* ---- PPU (D-13, D-15), APU (D-16), vblank, two instances (D-18) ---- */

#define WINDOW_LAST_DROPPED 29666u /* cycle k, counted from the start of the reset */

/* Gives every PPU and NMI field core.reset asserts a non-default value. The explicit catch-up
   first means the catch-up inside nesturbator_reset runs no dots. */
static void ppu_setup(struct nesturbator *nes)
{
    nesturbator__ppu_run_until(nes, nes->ticks);
    nes->ppu.control = 0x3bu;
    nes->ppu.mask = 0xe7u;
    nes->ppu.address_latch = 1u;
    nes->ppu.t = 0x2bcdu;
    nes->ppu.fine_x = 5u;
    nes->ppu.read_buffer = 0xa7u;
    nes->ppu.odd_frame = 1u;
    nes->ppu.vblank_suppress = 1u;
    nes->cpu.nmi_pending = 1u;
    nes->cpu.nmi_prev = 1u;
    nes->ppu.scanline = 100u;
    nes->ppu.dot = 200u;
    nes->ppu.v = 0x2345u;
    nes->ppu.status = 0x60u;
    nes->ppu.oam_addr = 0x5cu;
    nes->ppu.palette[3] = 0x2au;
    nes->ppu.nametable[0x123] = 0x77u;
    nes->ppu.oam[17] = 0x99u;
}

static void check_ppu_kept(const struct nesturbator *nes)
{
    CHECK_EQ_HEX(nes->ppu.v, 0x2345u);
    CHECK_EQ_HEX(nes->ppu.status, 0x60u);
    CHECK_EQ_HEX(nes->ppu.oam_addr, 0x5cu);
    CHECK_EQ_HEX(nes->ppu.palette[3], 0x2au);
    CHECK_EQ_HEX(nes->ppu.nametable[0x123], 0x77u);
    CHECK_EQ_HEX(nes->ppu.oam[17], 0x99u);
}

/* The 7 reset cycles run the PPU 21 dots from scanline 0 dot 0. */
static void check_ppu_cleared_and_kept(void)
{
    struct nesturbator *nes = make_instance(0);
    run_frames(nes, 2u);
    ppu_setup(nes);
    CHECK_EQ_U64(nesturbator_reset((nesturbator *)nes), NESTURBATOR_OK);
    CHECK_EQ_U64(nes->ppu.scanline, 0u);
    CHECK_EQ_U64(nes->ppu.dot, 21u);
    CHECK_EQ_U64(nes->ppu.control, 0u);
    CHECK_EQ_U64(nes->ppu.mask, 0u);
    CHECK_EQ_U64(nes->ppu.address_latch, 0u);
    CHECK_EQ_U64(nes->ppu.t, 0u);
    CHECK_EQ_U64(nes->ppu.fine_x, 0u);
    CHECK_EQ_U64(nes->ppu.read_buffer, 0u);
    CHECK_EQ_U64(nes->ppu.odd_frame, 0u);
    CHECK_EQ_U64(nes->ppu.vblank_suppress, 0u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 0u);
    CHECK_EQ_U64(nes->cpu.nmi_prev, 0u);
    CHECK_EQ_U64(nes->ppu.reset_flag, 1u);
    check_ppu_kept(nes);
    nesturbator_destroy((nesturbator *)nes);
}

/* Advances one CPU cycle at a time with RAM reads until k cycles have passed since t0. */
static void advance_to_cycle(struct nesturbator *nes, uint64_t t0, uint64_t k)
{
    while ((nes->ticks - t0) / 24u < k) {
        (void)nesturbator__bus_read(nes, 0x0000u);
    }
    CHECK_EQ_U64((nes->ticks - t0) / 24u, k);
}

/* A write in cycle 29,666 sees scanline 261 dot 0 and is dropped; the same write in cycle 29,667
   sees scanline 261 dot 3 and lands: 29,667 CPU cycles from the start of the reset. */
static void check_window_boundary(uint16_t reg)
{
    struct nesturbator *nes = make_instance(0);
    uint64_t t0;
    run_frames(nes, 2u);
    ppu_setup(nes);
    t0 = nes->ticks;
    CHECK_EQ_U64(nesturbator_reset((nesturbator *)nes), NESTURBATOR_OK);
    advance_to_cycle(nes, t0, WINDOW_LAST_DROPPED);
    nesturbator__bus_write(nes, reg, 0x13u);
    CHECK_EQ_U64(nes->ppu.scanline, 261u);
    CHECK_EQ_U64(nes->ppu.dot, 0u);
    CHECK_EQ_U64(nes->ppu.reset_flag, 1u);
    CHECK_EQ_U64(nes->ppu.control, 0u);
    CHECK_EQ_U64(nes->ppu.mask, 0u);
    CHECK_EQ_U64(nes->ppu.address_latch, 0u);
    CHECK_EQ_U64(nes->ppu.t, 0u);
    CHECK_EQ_U64(nes->ppu.fine_x, 0u);
    CHECK_EQ_HEX(nes->ppu.v, 0x2345u);
    nesturbator__bus_write(nes, reg, 0x13u);
    CHECK_EQ_U64(nes->ppu.scanline, 261u);
    CHECK_EQ_U64(nes->ppu.dot, 3u);
    CHECK_EQ_U64(nes->ppu.reset_flag, 0u);
    if (reg == 0x2000u) {
        CHECK_EQ_HEX(nes->ppu.control, 0x13u);
        CHECK_EQ_HEX(nes->ppu.t, 0x0c00u);
    } else if (reg == 0x2001u) {
        CHECK_EQ_HEX(nes->ppu.mask, 0x13u);
    } else if (reg == 0x2005u) {
        CHECK_EQ_U64(nes->ppu.address_latch, 1u);
        CHECK_EQ_U64(nes->ppu.fine_x, 3u);
        CHECK_EQ_HEX(nes->ppu.t, 0x0002u);
    } else {
        CHECK_EQ_U64(nes->ppu.address_latch, 1u);
        CHECK_EQ_HEX(nes->ppu.t, 0x1300u);
    }
    nesturbator_destroy((nesturbator *)nes);
}

/* $2002, $2003, $2004, $2007 work inside the window. */
static void check_window_other_registers(void)
{
    struct nesturbator *nes = make_instance(0);
    uint8_t value;
    run_frames(nes, 2u);
    ppu_setup(nes);
    nes->ppu.status = 0xe0u;
    CHECK_EQ_U64(nesturbator_reset((nesturbator *)nes), NESTURBATOR_OK);
    nesturbator__bus_write(nes, 0x2003u, 0x44u);
    nesturbator__bus_write(nes, 0x2004u, 0x9du);
    CHECK_EQ_HEX(nes->ppu.oam[0x44], 0x9du);
    CHECK_EQ_HEX(nes->ppu.oam_addr, 0x45u);
    nesturbator__bus_write(nes, 0x2007u, 0x31u);
    CHECK_EQ_HEX(nes->ppu.v, 0x2346u);
    value = nesturbator__bus_read(nes, 0x2002u);
    CHECK_EQ_HEX(value & 0xe0u, 0xe0u);
    CHECK_EQ_HEX(nes->ppu.status & 0x80u, 0u);
    CHECK_EQ_U64(nes->ppu.reset_flag, 1u);
    nesturbator_destroy((nesturbator *)nes);
}

/* Two consecutive resets: the flag stays set and the window restarts from the second. */
static void check_window_idempotent(void)
{
    struct nesturbator *nes = make_instance(0);
    uint64_t t0;
    run_frames(nes, 2u);
    CHECK_EQ_U64(nesturbator_reset((nesturbator *)nes), NESTURBATOR_OK);
    advance_to_cycle(nes, nes->ticks - 24u * 7u, 20000u);
    CHECK_EQ_U64(nes->ppu.reset_flag, 1u);
    t0 = nes->ticks;
    CHECK_EQ_U64(nesturbator_reset((nesturbator *)nes), NESTURBATOR_OK);
    CHECK_EQ_U64(nes->ppu.reset_flag, 1u);
    CHECK_EQ_U64(nes->ppu.scanline, 0u);
    advance_to_cycle(nes, t0, WINDOW_LAST_DROPPED);
    nesturbator__bus_write(nes, 0x2000u, 0x10u);
    CHECK_EQ_U64(nes->ppu.control, 0u);
    nesturbator__bus_write(nes, 0x2000u, 0x10u);
    CHECK_EQ_HEX(nes->ppu.control, 0x10u);
    nesturbator_destroy((nesturbator *)nes);
}

/* D-18: a reset placed in vblank keeps the VBL flag; the flag and the window end at 261/1. */
static struct nesturbator *vblank_instance(void)
{
    struct nesturbator *nes = make_instance(0);
    run_frames(nes, 2u);
    nesturbator__ppu_run_until(nes, nes->ticks);
    nes->ppu.scanline = 245u;
    nes->ppu.dot = 100u;
    nes->ppu.status = 0xe0u;
    nes->ppu.vblank_suppress = 0u;
    CHECK_EQ_U64(nesturbator_reset((nesturbator *)nes), NESTURBATOR_OK);
    CHECK_EQ_U64(nes->ppu.scanline, 0u);
    CHECK_EQ_U64(nes->ppu.reset_flag, 1u);
    CHECK_EQ_HEX(nes->ppu.status & 0x80u, 0x80u);
    return nes;
}

static void check_reset_in_vblank(void)
{
    struct nesturbator *nes = vblank_instance();
    CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x2002u) & 0x80u, 0x80u);
    nesturbator_destroy((nesturbator *)nes);

    nes = vblank_instance();
    /* Up to 261 dot 0 nothing has cleared; one more dot passes 261 dot 1. */
    nesturbator__ppu_run_until(nes, nes->ppu.ppu_ticks + 8u * (89001u - 21u));
    CHECK_EQ_U64(nes->ppu.scanline, 261u);
    CHECK_EQ_U64(nes->ppu.dot, 0u);
    CHECK_EQ_U64(nes->ppu.reset_flag, 1u);
    CHECK_EQ_HEX(nes->ppu.status & 0x80u, 0x80u);
    nesturbator__ppu_run_until(nes, nes->ppu.ppu_ticks + 8u);
    CHECK_EQ_U64(nes->ppu.dot, 1u);
    CHECK_EQ_U64(nes->ppu.reset_flag, 0u);
    CHECK_EQ_HEX(nes->ppu.status & 0x80u, 0u);
    nesturbator_destroy((nesturbator *)nes);
}

/* Gives every APU field core.reset asserts a non-default value. */
static void apu_setup(struct nesturbator *nes, unsigned phase)
{
    nesturbator__bus_write(nes, 0x4017u, 0xc0u);
    for (unsigned i = 0u; i < 12u; ++i) {
        (void)nesturbator__bus_read(nes, 0x0000u);
    }
    nesturbator__bus_write(nes, 0x4015u, 0x1fu);
    nesturbator__bus_write(nes, 0x4003u, 0x08u);
    nesturbator__bus_write(nes, 0x4007u, 0x08u);
    nesturbator__bus_write(nes, 0x400bu, 0x08u);
    nesturbator__bus_write(nes, 0x400fu, 0x08u);
    nes->apu.triangle.phase = 17u;
    nes->apu.dmc.output = 0x55u;
    nes->apu.frame_irq = 1u;
    nes->apu.frame_irq_clear_pending = 1u;
    nes->apu.dmc.irq = 1u;
    nes->apu.dmc.dma_pending = 1u;
    nes->apu.dmc.dma_halt_phase = 1u;
    nes->bus.apu_get_put_phase = (uint8_t)phase;
}

/* Called directly, so cycles after the call cannot clear a latch the reset forgot. */
static void check_apu_cleared(void)
{
    struct nesturbator *nes = make_instance(0);
    run_frames(nes, 2u);
    apu_setup(nes, 1u);
    CHECK_EQ_U64(nes->apu.frame_mode, 1u);
    CHECK_EQ_U64(nes->apu.frame_irq_inhibit, 1u);
    nesturbator__apu_reset(nes);
    CHECK_EQ_U64(nes->apu.frame_irq, 0u);
    CHECK_EQ_U64(nes->apu.frame_irq_clear_pending, 0u);
    CHECK_EQ_U64(nes->apu.dmc.irq, 0u);
    CHECK_EQ_U64(nes->apu.dmc.dma_pending, 0u);
    CHECK_EQ_U64(nes->apu.dmc.dma_halt_phase, 0u);
    CHECK_EQ_HEX(nes->apu.dmc.output, 0x01u);
    CHECK_EQ_U64(nes->apu.triangle.phase, 0u);
    CHECK_EQ_U64(nes->cpu.irq_line, 0u);
    CHECK_EQ_U64(nes->apu.frame_mode, 1u);
    CHECK_EQ_U64(nes->apu.frame_irq_inhibit, 1u);
    nesturbator_destroy((nesturbator *)nes);
}

/* Through the public call: $4015 reads 0 and $4017 is re-applied 10 clocks before the first
   instruction: frame_cycle is 5 (GET phase) or 6 (PUT phase) when it starts. */
static void check_apu_through_reset(unsigned phase, uint32_t frame_cycle)
{
    struct nesturbator *nes = make_instance(0);
    run_frames(nes, 2u);
    apu_setup(nes, phase);
    CHECK_EQ_U64(nesturbator_reset((nesturbator *)nes), NESTURBATOR_OK);
    CHECK_EQ_U64(nes->apu.frame_reset_delay, 0u);
    CHECK_EQ_U64(nes->apu.frame_cycle, frame_cycle);
    CHECK_EQ_U64(nes->apu.frame_mode, 1u);
    CHECK_EQ_U64(nes->apu.frame_irq_inhibit, 1u);
    CHECK_EQ_U64(nes->apu.frame_irq, 0u);
    CHECK_EQ_U64(nes->apu.dmc.irq, 0u);
    CHECK_EQ_HEX(nes->apu.dmc.output, 0x01u);
    CHECK_EQ_U64(nes->apu.triangle.phase, 0u);
    CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x4015u), 0x00u);
    nesturbator_destroy((nesturbator *)nes);
}

/* No cartridge: PPU and APU state are unchanged. */
static void check_no_cartridge_state(void)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    struct nesturbator *nes;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    nes = (struct nesturbator *)inst;
    ppu_setup(nes);
    nes->apu.triangle.phase = 17u;
    nes->apu.dmc.output = 0x55u;
    nes->apu.frame_irq = 1u;
    nes->apu.dmc.irq = 1u;
    nes->apu.dmc.dma_pending = 1u;
    CHECK_EQ_U64(nesturbator_reset(inst), NESTURBATOR_OK);
    CHECK_EQ_HEX(nes->ppu.control, 0x3bu);
    CHECK_EQ_HEX(nes->ppu.mask, 0xe7u);
    CHECK_EQ_U64(nes->ppu.scanline, 100u);
    CHECK_EQ_U64(nes->ppu.dot, 200u);
    CHECK_EQ_U64(nes->ppu.address_latch, 1u);
    CHECK_EQ_HEX(nes->ppu.t, 0x2bcdu);
    CHECK_EQ_U64(nes->ppu.fine_x, 5u);
    CHECK_EQ_HEX(nes->ppu.read_buffer, 0xa7u);
    CHECK_EQ_U64(nes->ppu.odd_frame, 1u);
    CHECK_EQ_U64(nes->ppu.vblank_suppress, 1u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 1u);
    CHECK_EQ_U64(nes->cpu.nmi_prev, 1u);
    CHECK_EQ_U64(nes->ppu.reset_flag, 0u);
    check_ppu_kept(nes);
    CHECK_EQ_U64(nes->apu.triangle.phase, 17u);
    CHECK_EQ_HEX(nes->apu.dmc.output, 0x55u);
    CHECK_EQ_U64(nes->apu.frame_irq, 1u);
    CHECK_EQ_U64(nes->apu.dmc.irq, 1u);
    CHECK_EQ_U64(nes->apu.dmc.dma_pending, 1u);
    nesturbator_destroy(inst);
}

static void run_into(struct nesturbator *nes, unsigned n, uint16_t *video)
{
    static int16_t audio[2048];
    for (unsigned i = 0; i < n; ++i) {
        nesturbator_frame io;
        memset(&io, 0, sizeof io);
        io.size = (uint32_t)sizeof io;
        io.video = video;
        io.video_pitch = 256u;
        io.audio = audio;
        io.audio_capacity = 2048u;
        CHECK_EQ_U64(nesturbator_run_frame(nes, &io), NESTURBATOR_OK);
    }
}

/* Same image, 2 frames, reset, 3 frames: equal frames and ticks; one reset leaves the other
   instance untouched. */
static void check_two_instances(void)
{
    static uint16_t video_a[256u * 240u], video_b[256u * 240u];
    static unsigned char snapshot[sizeof(struct nesturbator)];
    struct nesturbator *a = make_instance(0);
    struct nesturbator *b = make_instance(0);
    run_into(a, 2u, video_a);
    run_into(b, 2u, video_b);
    memcpy(snapshot, b, sizeof snapshot);
    CHECK_EQ_U64(nesturbator_reset((nesturbator *)a), NESTURBATOR_OK);
    CHECK_EQ_U64(memcmp(snapshot, b, sizeof snapshot), 0);
    CHECK_EQ_U64(nesturbator_reset((nesturbator *)b), NESTURBATOR_OK);
    run_into(a, 3u, video_a);
    run_into(b, 3u, video_b);
    CHECK_EQ_U64(memcmp(video_a, video_b, sizeof video_a), 0);
    CHECK_EQ_U64(a->ticks, b->ticks);
    nesturbator_destroy((nesturbator *)a);
    nesturbator_destroy((nesturbator *)b);
}

int main(void)
{
    check_null_and_empty();
    check_cpu_and_bus();
    check_cartridge_state_kept();
    check_chr_ram_kept();
    check_ppu_cleared_and_kept();
    check_window_boundary(0x2000u);
    check_window_boundary(0x2001u);
    check_window_boundary(0x2005u);
    check_window_boundary(0x2006u);
    check_window_other_registers();
    check_window_idempotent();
    check_reset_in_vblank();
    check_apu_cleared();
    check_apu_through_reset(1u, 5u);
    check_apu_through_reset(0u, 6u);
    check_no_cartridge_state();
    check_two_instances();
    CHECK_DONE();
}
