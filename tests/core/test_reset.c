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

int main(void)
{
    check_null_and_empty();
    check_cpu_and_bus();
    check_cartridge_state_kept();
    check_chr_ram_kept();
    CHECK_DONE();
}
