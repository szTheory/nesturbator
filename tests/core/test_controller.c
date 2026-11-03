/* Controller serial-port behavior through the CPU bus and size-tagged API. */
#include <string.h>

#include "internal.h"
#include "../check.h"
#include "../ines.h"

#define FRAME_VIDEO_SIZE (NESTURBATOR_WIDTH * NESTURBATOR_HEIGHT)
#define FRAME_AUDIO_CAPACITY 1024u

static uint16_t frame_video[FRAME_VIDEO_SIZE];
static int16_t frame_audio[FRAME_AUDIO_CAPACITY];

static nesturbator *make(void)
{
    nesturbator_config cfg;
    nesturbator *nes = NULL;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &nes), NESTURBATOR_OK);
    return nes;
}

static void latch(nesturbator *nes)
{
    nesturbator__bus_write(nes, 0x4016u, 1u);
    nesturbator__bus_write(nes, 0x4016u, 0u);
}

static void test_two_ports_shift_independently(void)
{
    nesturbator *nes = make();
    nesturbator_input input;
    static const uint8_t expected0[] = {1u, 0u, 1u, 0u, 0u, 1u, 0u, 1u};
    static const uint8_t expected1[] = {0u, 1u, 1u, 0u, 1u, 0u, 1u, 0u};
    memset(&input, 0, sizeof input);
    input.size = (uint32_t)sizeof input;
    input.buttons[0] = (uint8_t)(NESTURBATOR_BUTTON_A | NESTURBATOR_BUTTON_SELECT |
                                 NESTURBATOR_BUTTON_DOWN | NESTURBATOR_BUTTON_RIGHT);
    input.buttons[1] = (uint8_t)(NESTURBATOR_BUTTON_B | NESTURBATOR_BUTTON_SELECT |
                                 NESTURBATOR_BUTTON_UP | NESTURBATOR_BUTTON_LEFT);
    CHECK_EQ_U64(nesturbator_set_input(nes, &input), NESTURBATOR_OK);
    nesturbator__controller_begin_frame(nes);
    latch(nes);
    for (uint32_t i = 0; i < 8u; i++) {
        CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4016u) & 1u, expected0[i]);
        CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4017u) & 1u, expected1[i]);
    }
    CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4016u) & 1u, 1u);
    CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4017u) & 1u, 1u);
    CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4016u) & 1u, 1u);
    input.buttons[0] = NESTURBATOR_BUTTON_START;
    input.buttons[1] = NESTURBATOR_BUTTON_RIGHT;
    CHECK_EQ_U64(nesturbator_set_input(nes, &input), NESTURBATOR_OK);
    nesturbator__controller_begin_frame(nes);
    nesturbator__bus_write(nes, 0x4016u, 1u);
    CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4016u) & 1u, 0u);
    nesturbator__bus_write(nes, 0x4016u, 0u);
    for (uint32_t i = 0; i < 7u; i++) {
        CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4016u) & 1u, i == 3u ? 1u : 0u);
        CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4017u) & 1u, 0u);
    }
    CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4016u) & 1u, 0u);
    CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4017u) & 1u, 1u);
    CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4016u) & 1u, 1u);
    nesturbator_destroy(nes);
}

static void test_high_strobe_tracks_a_and_invalid_input_is_unchanged(void)
{
    nesturbator *nes = make();
    nesturbator_input input, invalid;
    memset(&input, 0, sizeof input);
    input.size = (uint32_t)sizeof input;
    input.buttons[0] = NESTURBATOR_BUTTON_A;
    input.buttons[1] = NESTURBATOR_BUTTON_B;
    CHECK_EQ_U64(nesturbator_set_input(nes, &input), NESTURBATOR_OK);
    nesturbator__controller_begin_frame(nes);
    nesturbator__bus_write(nes, 0x4016u, 1u);
    CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4016u) & 1u, 1u);
    input.buttons[0] = 0u;
    CHECK_EQ_U64(nesturbator_set_input(nes, &input), NESTURBATOR_OK);
    nesturbator__controller_begin_frame(nes);
    CHECK_EQ_U64(nesturbator__bus_read(nes, 0x4016u) & 1u, 0u);
    memset(&invalid, 0, sizeof invalid);
    invalid.size = 0u;
    CHECK_EQ_U64(nesturbator_set_input(nes, &invalid), NESTURBATOR_ERR_STRUCT_SIZE);
    CHECK_EQ_U64(nes->bus.input_pending[0], 0u);
    CHECK_EQ_U64(nes->bus.input_pending[1], NESTURBATOR_BUTTON_B);
    CHECK_EQ_U64(nesturbator_set_input(nes, NULL), NESTURBATOR_ERR_ARGUMENT);
    CHECK_EQ_U64(nesturbator_set_input(NULL, &input), NESTURBATOR_ERR_ARGUMENT);
    nesturbator_destroy(nes);
}

static void test_upper_three_bits_retain_cpu_open_bus(void)
{
    nesturbator *nes = make();
    CHECK(nes != NULL);
    if (nes == NULL)
        return;
    /* AccuracyCoin 6bee919, TEST_ImpliedDummyRead, requires the controller
       operands after JSR $4015 to retain $20 and then $00 from stack writes.
       All three upper bits float; the usual $40 is the address high byte,
       rather than a bit driven high by the controller. */
    for (uint8_t port = 0u; port < 2u; port++) {
        for (uint8_t upper = 0u; upper < 8u; upper++) {
            for (uint8_t bit = 0u; bit < 2u; bit++) {
                nes->bus.controller_strobe = 0u;
                nes->bus.controller_shift[port] = bit;
                nes->bus.open_bus = (uint8_t)(((uint32_t)upper << 5) | 0x1fu);
                uint8_t expected = (uint8_t)(((uint32_t)upper << 5) | bit);
                CHECK_EQ_HEX(nesturbator__bus_read(nes, (uint16_t)(0x4016u + port)), expected);
                CHECK_EQ_HEX(nes->bus.open_bus, expected);
            }
        }
    }
    nesturbator_destroy(nes);
}

static void test_public_frames_snapshot_inputs_across_instruction_overrun(void)
{
    /* The 10-cycle loop reads live A, stores it, then jumps back. The first
       29,780 cycles end at a loop boundary; LDA $4016 starts 12 master ticks
       before the frame target and completes after that target. */
    static const uint8_t loop_code[] = {
        0xadu, 0x16u, 0x40u, /* LDA $4016 (4 cycles) */
        0x85u, 0x00u,        /* STA $00 (3 cycles) */
        0x4cu, 0x00u, 0x80u  /* JMP $8000 (3 cycles) */
    };
    static const uint64_t ntsc_frame_ticks = UINT64_C(714732);
    static uint8_t image[16u + INES_PRG_BANK + INES_CHR_BANK];
    struct ines_spec spec;
    nesturbator_input input;
    nesturbator_frame io;
    nesturbator *nes = make();

    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = 1u;
    spec.prg_code = loop_code;
    spec.prg_code_len = sizeof loop_code;
    spec.nmi_vector = 0x8000u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    CHECK_EQ_U64(ines_build(image, sizeof image, &spec), sizeof image);
    CHECK_EQ_U64(nesturbator_load_cartridge(nes, image, sizeof image), NESTURBATOR_OK);

    /* The guest keeps the controller strobe high so each LDA observes the A
       bit from the snapshot installed at the start of its public frame call. */
    nes->bus.controller_strobe = 1u;
    memset(&input, 0, sizeof input);
    input.size = (uint32_t)sizeof input;
    input.buttons[0] = NESTURBATOR_BUTTON_A;
    CHECK_EQ_U64(nesturbator_set_input(nes, &input), NESTURBATOR_OK);
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = frame_video;
    io.video_pitch = NESTURBATOR_WIDTH;
    io.audio = frame_audio;
    io.audio_capacity = FRAME_AUDIO_CAPACITY;
    CHECK_EQ_U64(nesturbator_run_frame(nes, &io), NESTURBATOR_OK);

    /* 29,781 nominal cycles are required; the crossing LDA completes at
       cycle 29,784 and is the instruction active when run_frame returns. */
    CHECK_EQ_U64(io.frame_number, 1u);
    CHECK_EQ_U64(io.ticks, ntsc_frame_ticks + 84u);
    CHECK_EQ_U64(nes->cpu_cycle, 29784u);
    CHECK_EQ_HEX(nes->cpu.pc, 0x8003u);
    CHECK_EQ_HEX(nes->cpu.a & 1u, 1u);

    input.buttons[0] = 0u;
    CHECK_EQ_U64(nesturbator_set_input(nes, &input), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_run_frame(nes, &io), NESTURBATOR_OK);
    CHECK_EQ_U64(io.frame_number, 2u);
    CHECK_EQ_HEX(nes->cpu.a & 1u, 0u);
    CHECK_EQ_HEX(nes->bus.ram[0] & 1u, 0u);
    nesturbator_destroy(nes);
}

int main(void)
{
    test_two_ports_shift_independently();
    test_high_strobe_tracks_a_and_invalid_input_is_unchanged();
    test_upper_three_bits_retain_cpu_open_bus();
    test_public_frames_snapshot_inputs_across_instruction_overrun();
    CHECK_DONE();
}
