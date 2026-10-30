/* SND-01: a CPU-programmed pulse reaches the public caller-owned PCM buffer. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/internal.h"
#include "nesturbator.h"
#include "../check.h"

#define FRAME_SAMPLES 798u
static uint16_t pixels[256u * 240u];
static int16_t samples[FRAME_SAMPLES];

static void make_image(uint8_t *image, size_t size)
{
    memset(image, 0, size);
    memcpy(image, "NES\032", 4u);
    image[4] = 1u;
    image[5] = 1u;
    /* LDA #1; STA $4015; LDA #$BF; STA $4000; timer=$008; enable pulse. */
    static const uint8_t program[] = {0xa9u, 0x01u, 0x8du, 0x15u, 0x40u, 0xa9u, 0xbfu, 0x8du,
                                      0x00u, 0x40u, 0xa9u, 0x08u, 0x8du, 0x02u, 0x40u, 0xa9u,
                                      0x00u, 0x8du, 0x03u, 0x40u, 0x4cu, 0x14u, 0x80u};
    memcpy(image + 16u, program, sizeof program);
    image[16u + 0x3ffcu] = 0u;
    image[16u + 0x3ffdu] = 0x80u;
}

static nesturbator *make(void)
{
    nesturbator_config cfg;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    nesturbator *inst = NULL;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    return inst;
}

static void test_public_pulse_pcm(void)
{
    const size_t image_size = 16u + 16384u + 8192u;
    uint8_t *image = calloc(1u, image_size);
    nesturbator *inst = make();
    CHECK(image != NULL);
    if (image == NULL || inst == NULL) {
        free(image);
        nesturbator_destroy(inst);
        return;
    }
    make_image(image, image_size);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, image_size), NESTURBATOR_OK);
    nesturbator_frame io;
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = pixels;
    io.video_pitch = 256u;
    io.audio = samples;
    io.audio_capacity = FRAME_SAMPLES;
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
    CHECK_EQ_U64(io.audio_count, FRAME_SAMPLES);
    unsigned nonzero = 0u;
    for (uint32_t i = 0u; i < io.audio_count; i++)
        nonzero += samples[i] != 0;
    static const int16_t expected[] = {0, 0, 0, 3, -10, 25, -52, 106};
    for (uint32_t i = 0u; i < sizeof expected / sizeof expected[0]; i++)
        CHECK_EQ_U64((uint16_t)samples[i], (uint16_t)expected[i]);
    CHECK(nonzero > 0u);
    nesturbator_destroy(inst);
    free(image);
}

static void test_documented_channel_sequences(void)
{
    nesturbator *inst = make();
    struct nesturbator *nes = (struct nesturbator *)inst;
    CHECK_EQ_U64(nes->apu.noise.lfsr, 0u);
    CHECK_EQ_U64(nesturbator__apu_channel_level(nes, 0u), 0u);
    CHECK_EQ_U64(nesturbator__apu_mixed_level(nes), 0u);
    nes->apu.enabled = 4u;
    nes->apu.triangle.length = 1u;
    nes->apu.triangle.linear = 1u;
    nes->apu.triangle.timer = 2u;
    CHECK_EQ_U64(nesturbator__apu_channel_level(nes, 2u), 15u);
    CHECK_EQ_U64(nesturbator__apu_mixed_level(nes), 8074u);
    nes->apu.dmc.output = 127u;
    CHECK_EQ_U64(nesturbator__apu_mixed_level(nes), 22325u);
    memset(&nes->apu, 0, sizeof nes->apu);
    nesturbator__synth_reset(nes);

    /* Independent pulse units: their own duty phase, timer and enable bit. */
    nesturbator__apu_write(nes, 0x4015u, 0x03u);
    nesturbator__apu_write(nes, 0x4000u, 0x9fu);
    nesturbator__apu_write(nes, 0x4002u, 8u);
    nesturbator__apu_write(nes, 0x4003u, 0x08u);
    nesturbator__apu_write(nes, 0x4004u, 0x5bu);
    nesturbator__apu_write(nes, 0x4006u, 8u);
    nesturbator__apu_write(nes, 0x4007u, 0x08u);
    nes->apu.pulse[0].phase = 4u;
    nes->apu.pulse[1].phase = 7u;
    CHECK_EQ_U64(nesturbator__apu_channel_level(nes, 0u), 15u);
    CHECK_EQ_U64(nesturbator__apu_channel_level(nes, 1u), 11u);
    /* The APU timer edge clocks at M2 fall before the register decode in the
       same bus cycle; the high-register write then restarts phase at zero. */
    nes->apu.pulse[0].phase = 2u;
    nes->apu.pulse[0].counter = 0u;
    nesturbator__bus_write(nes, 0x4003u, 0x08u);
    CHECK_EQ_U64(nes->apu.pulse[0].phase, 0u);
    nes->apu.pulse[0].timer = 7u;
    CHECK_EQ_U64(nesturbator__apu_channel_level(nes, 0u), 0u);
    CHECK_EQ_U64(nesturbator__apu_channel_level(nes, 1u), 11u);

    /* Envelope restart, sweep target and half-frame length countdown. */
    nesturbator__apu_write(nes, 0x4015u, 0x01u);
    nesturbator__apu_write(nes, 0x4000u, 0x0fu);
    nesturbator__apu_write(nes, 0x4002u, 8u);
    nesturbator__apu_write(nes, 0x4003u, 0x08u);
    nesturbator__apu_write(nes, 0x4001u, 0x89u); /* enable, negate, shift by one */
    nes->apu.frame_cycle = 14912u;
    nesturbator__apu_clock(nes);
    CHECK_EQ_U64(nes->apu.pulse[0].env_decay, 15u);
    CHECK_EQ_U64(nes->apu.pulse[0].timer, 3u); /* channel one negate subtracts one extra */
    CHECK_EQ_U64(nes->apu.pulse[0].length, 253u);

    /* Triangle's 32-step DAC sequence and timer gate. */
    nesturbator__apu_write(nes, 0x4015u, 0x07u);
    nesturbator__apu_write(nes, 0x4008u, 0x83u);
    nesturbator__apu_write(nes, 0x400au, 2u);
    nesturbator__apu_write(nes, 0x400bu, 0x08u);
    CHECK_EQ_U64(nesturbator__apu_channel_level(nes, 2u), 15u);
    nes->apu.triangle.counter = 0u;
    nesturbator__apu_clock(nes);
    CHECK_EQ_U64(nesturbator__apu_channel_level(nes, 2u), 14u);
    nes->apu.triangle.timer = 1u;
    CHECK_EQ_U64(nesturbator__apu_channel_level(nes, 2u), 0u);
    nes->apu.triangle.timer = 2u;
    nesturbator__apu_write(nes, 0x4008u, 3u);
    nesturbator__apu_write(nes, 0x400bu, 0x08u);
    nes->apu.frame_cycle = 7456u;
    nesturbator__apu_clock(nes);
    CHECK_EQ_U64(nes->apu.triangle.linear, 3u);
    nes->apu.frame_cycle = 7456u;
    nesturbator__apu_clock(nes);
    CHECK_EQ_U64(nes->apu.triangle.linear, 2u);

    /* RP2A03G's measured LFSR starts at zero; its first clock shifts in 1. */
    nesturbator__apu_write(nes, 0x4015u, 0x0fu);
    nesturbator__apu_write(nes, 0x400cu, 0x1fu);
    nesturbator__apu_write(nes, 0x400fu, 0x08u);
    nes->apu.noise.lfsr = 0u;
    nes->apu.noise.counter = 0u;
    CHECK_EQ_U64(nes->apu.noise.lfsr, 0u);
    CHECK_EQ_U64(nesturbator__apu_channel_level(nes, 3u), 15u);
    nesturbator__apu_clock(nes);
    CHECK_EQ_U64(nes->apu.noise.lfsr, 1u);
    CHECK_EQ_U64(nesturbator__apu_channel_level(nes, 3u), 0u);

    /* DMC DAC writes and its one-bit output transition are deterministic. */
    nesturbator__apu_write(nes, 0x4011u, 64u);
    CHECK_EQ_U64(nesturbator__apu_channel_level(nes, 4u), 64u);
    nes->apu.dmc.counter = 0u;
    nes->apu.dmc.bits = 1u;
    nes->apu.dmc.shift = 1u;
    nes->apu.pulse_clock_phase = 1u;
    nesturbator__apu_clock(nes);
    CHECK_EQ_U64(nesturbator__apu_channel_level(nes, 4u), 66u);
    nesturbator__apu_write(nes, 0x4015u, 0u);
    CHECK_EQ_U64(nesturbator__apu_channel_level(nes, 0u), 0u);
    nesturbator_destroy(inst);
}

static void test_dmc_fetch_from_mapper0(void)
{
    const size_t image_size = 16u + 16384u + 8192u;
    uint8_t *image = calloc(1u, image_size);
    nesturbator *inst = make();
    CHECK(image != NULL);
    if (image == NULL || inst == NULL) {
        free(image);
        nesturbator_destroy(inst);
        return;
    }
    make_image(image, image_size);
    image[16u + 0x40u] = 0x5au;
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, image_size), NESTURBATOR_OK);
    struct nesturbator *nes = (struct nesturbator *)inst;
    nesturbator__apu_write(nes, 0x4010u, 0u);
    nesturbator__apu_write(nes, 0x4012u, 1u); /* $C040 in the DMC address map */
    nesturbator__apu_write(nes, 0x4013u, 0u); /* one byte */
    nesturbator__apu_write(nes, 0x4015u, 0x10u);
    nesturbator__apu_clock(nes);
    nesturbator__apu_clock(nes);
    nesturbator__apu_clock(nes);
    (void)nesturbator__bus_read(nes, 0x8000u);
    CHECK_EQ_U64(nes->apu.dmc.sample_buffer, 0x5au);
    CHECK_EQ_U64(nes->apu.dmc.address, 0xc041u);
    CHECK_EQ_U64(nes->apu.dmc.remaining, 0u);
    nesturbator_destroy(inst);
    free(image);
}

static void test_dmc_enable_waits_full_write_delay(void)
{
    const size_t image_size = 16u + 16384u + 8192u;
    uint8_t *image = calloc(1u, image_size);
    nesturbator *inst = make();
    CHECK(image != NULL);
    if (image == NULL || inst == NULL) {
        free(image);
        nesturbator_destroy(inst);
        return;
    }
    make_image(image, image_size);
    image[16u + 0x40u] = 0x5au;
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, image_size), NESTURBATOR_OK);
    struct nesturbator *nes = (struct nesturbator *)inst;
    nes->bus.apu_get_put_phase = 1u;
    nesturbator__apu_write(nes, 0x4012u, 1u);
    nesturbator__apu_write(nes, 0x4013u, 0u);
    nesturbator__apu_write(nes, 0x4015u, 0x10u);
    CHECK_EQ_U64(nes->apu.dmc.enable_delay, 4u);
    CHECK_EQ_U64(nes->apu.dmc.dma_pending, 1u);
    /* Keep the request pending while its start delay expires, without
       introducing a second request from the output unit. */
    nes->apu.dmc.buffer_empty = 0u;
    nes->apu.dmc.bits = 8u;
    nes->apu.dmc.counter = 100u;
    for (unsigned i = 0u; i < 3u; i++) {
        (void)nesturbator__bus_read(nes, 0x8000u);
        CHECK_EQ_U64(nes->apu.dmc.dma_pending, 1u);
        CHECK_EQ_U64(nes->apu.dmc.remaining, 1u);
        CHECK_EQ_U64(nes->apu.dmc.address, 0xc040u);
    }
    (void)nesturbator__bus_read(nes, 0x8000u);
    CHECK_EQ_U64(nes->apu.dmc.dma_pending, 0u);
    CHECK_EQ_U64(nes->apu.dmc.remaining, 0u);
    CHECK_EQ_U64(nes->apu.dmc.sample_buffer, 0x5au);
    nesturbator_destroy(inst);
    free(image);
}

static void test_dmc_load_halts_on_get_cycle(void)
{
    const size_t image_size = 16u + 16384u + 8192u;
    uint8_t *image = calloc(1u, image_size);
    nesturbator *inst = make();
    CHECK(image != NULL);
    if (image == NULL || inst == NULL) {
        free(image);
        nesturbator_destroy(inst);
        return;
    }
    make_image(image, image_size);
    image[16u + 0x40u] = 0x5au;
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, image_size), NESTURBATOR_OK);
    struct nesturbator *nes = (struct nesturbator *)inst;
    nes->bus.apu_get_put_phase = 1u;
    nesturbator__apu_write(nes, 0x4012u, 1u);
    nesturbator__apu_write(nes, 0x4013u, 0u);
    nesturbator__apu_write(nes, 0x4015u, 0x10u);
    nes->apu.dmc.enable_delay = 0u;
    nes->apu.dmc.buffer_empty = 0u;
    nes->apu.dmc.bits = 8u;
    nes->apu.dmc.counter = 100u;

    /* A load request waits through a PUT read and halts on the next GET. */
    (void)nesturbator__bus_read(nes, 0x8000u);
    CHECK_EQ_U64(nes->bus.apu_get_put_phase, 0u);
    CHECK_EQ_U64(nes->apu.dmc.dma_pending, 1u);
    CHECK_EQ_U64(nes->apu.dmc.address, 0xc040u);
    uint64_t load_start = nes->ticks;
    (void)nesturbator__bus_read(nes, 0x8000u);
    CHECK_EQ_U64(nes->ticks - load_start, 4u * 24u);
    CHECK_EQ_U64(nes->bus.apu_get_put_phase, 0u);
    CHECK_EQ_U64(nes->apu.dmc.dma_pending, 0u);
    CHECK_EQ_U64(nes->apu.dmc.sample_buffer, 0x5au);

    /* With no write delaying RDY, a reload first halts on PUT. This read
       reaches GET and misses; the following PUT read qualifies. */
    nes->bus.apu_get_put_phase = 0u;
    nes->apu.pulse_clock_phase = 1u;
    nes->apu.dmc.address = 0xc040u;
    nes->apu.dmc.remaining = 1u;
    nes->apu.dmc.dma_pending = 1u;
    nes->apu.dmc.dma_halt_phase = 0u;
    nes->apu.dmc.enable_delay = 0u;
    nes->apu.dmc.buffer_empty = 1u;
    nes->apu.dmc.bits = 1u;
    nes->apu.dmc.counter = 100u;
    uint64_t reload_start = nes->ticks;
    uint8_t missed_read = nesturbator__bus_read(nes, 0x8000u);
    CHECK_EQ_HEX(missed_read, 0xa9u);
    CHECK_EQ_U64(nes->ticks - reload_start, 24u);
    CHECK_EQ_U64(nes->bus.apu_get_put_phase, 1u);
    CHECK_EQ_U64(nes->apu.dmc.dma_pending, 1u);
    CHECK_EQ_U64(nes->apu.dmc.remaining, 1u);

    /* The eligible PUT start includes alignment and the resumed CPU read.
       [HWC.01, HWC.25] */
    nes->bus.apu_get_put_phase = 1u;
    uint64_t retry_start = nes->ticks;
    (void)nesturbator__bus_read(nes, 0x8000u);
    CHECK_EQ_U64(nes->ticks - retry_start, 5u * 24u);
    CHECK_EQ_U64(nes->bus.apu_get_put_phase, 0u);
    CHECK_EQ_U64(nes->apu.dmc.dma_pending, 0u);
    CHECK_EQ_U64(nes->apu.dmc.sample_buffer, 0x5au);
    nesturbator_destroy(inst);
    free(image);
}

static void test_dmc_halt_retries_after_cpu_writes(void)
{
    const size_t image_size = 16u + 16384u + 8192u;
    uint8_t *image = calloc(1u, image_size);
    CHECK(image != NULL);
    if (image == NULL)
        return;
    make_image(image, image_size);
    image[16u + 0x40u] = 0x5au;
    for (uint8_t initial_halt = 0u; initial_halt < 2u; initial_halt++) {
        for (uint8_t writes = 1u; writes <= 3u; writes++) {
            nesturbator *inst = make();
            CHECK(inst != NULL);
            if (inst == NULL)
                continue;
            CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, image_size), NESTURBATOR_OK);
            struct nesturbator *nes = (struct nesturbator *)inst;
            nes->bus.apu_get_put_phase = (uint8_t)(initial_halt ^ 1u);
            nes->apu.pulse_clock_phase = nes->bus.apu_get_put_phase;
            nes->apu.dmc.address = 0xc040u;
            nes->apu.dmc.remaining = 1u;
            nes->apu.dmc.dma_pending = 1u;
            nes->apu.dmc.dma_halt_phase = initial_halt;
            nes->apu.dmc.buffer_empty = 1u;
            nes->apu.dmc.bits = 8u;
            nes->apu.dmc.counter = 100u;
            /* RDY cannot halt a write. Once its first attempt is delayed,
               it retries each CPU cycle, including the opposite GET/PUT
               phase. NESdev DMA, Cadence and DMC DMA. */
            for (uint8_t n = 0u; n < writes; n++) {
                nesturbator__bus_write(nes, 0x0010u, n);
                CHECK_EQ_U64(nes->apu.dmc.dma_pending, 1u);
                CHECK_EQ_U64(nes->apu.dmc.remaining, 1u);
            }
            uint8_t read_phase = (uint8_t)(nes->bus.apu_get_put_phase ^ 1u);
            uint64_t before = nes->ticks;
            CHECK_EQ_HEX(nesturbator__bus_read(nes, 0x8000u), 0xa9u);
            CHECK_EQ_U64(nes->ticks - before, (read_phase != 0u ? 4u : 5u) * 24u);
            CHECK_EQ_HEX(nes->apu.dmc.sample_buffer, 0x5au);
            CHECK_EQ_U64(nes->apu.dmc.remaining, 0u);
            CHECK_EQ_U64(nes->apu.dmc.dma_pending, 0u);
            CHECK_EQ_U64(nes->bus.apu_get_put_phase, 0u);
            nesturbator_destroy(inst);
        }
    }
    free(image);
}

static void test_dmc_restart_preserves_full_sample_buffer(void)
{
    nesturbator *inst = make();
    CHECK(inst != NULL);
    if (inst == NULL)
        return;

    struct nesturbator *nes = (struct nesturbator *)inst;
    nes->apu.dmc.reg[0] = 0x4fu;
    nes->apu.dmc.reg[2] = 1u;
    nes->apu.dmc.reg[3] = 0u;
    nes->apu.dmc.address = 0xc041u;
    nes->apu.dmc.remaining = 0u;
    nes->apu.dmc.sample_buffer = 0xa5u;
    nes->apu.dmc.buffer_empty = 0u;
    nes->apu.dmc.shift = 0x5au;
    nes->apu.dmc.bits = 8u;

    nesturbator__apu_write(nes, 0x4015u, 0x10u);

    CHECK_EQ_U64(nes->apu.dmc.remaining, 1u);
    CHECK_EQ_U64(nes->apu.dmc.address, 0xc040u);
    CHECK_EQ_U64(nes->apu.dmc.sample_buffer, 0xa5u);
    CHECK_EQ_U64(nes->apu.dmc.buffer_empty, 0u);
    CHECK_EQ_U64(nes->apu.dmc.dma_pending, 0u);
    CHECK_EQ_U64(nes->apu.dmc.dma_load_waiting, 1u);
    CHECK_EQ_U64(nesturbator__apu_status_read(nes) & 0x10u, 0x10u);

    nesturbator_destroy(inst);
}

static void test_frame_counter_modes_and_irq_sources(void)
{
    nesturbator *inst = make();
    struct nesturbator *nes = (struct nesturbator *)inst;

    /* 4-step mode raises frame IRQ at the last half-frame edge. */
    nes->apu.frame_cycle = 29826u;
    nesturbator__apu_clock(nes);
    CHECK_EQ_U64(nes->apu.frame_irq, 0u);
    nesturbator__apu_clock(nes);
    CHECK_EQ_U64(nes->apu.frame_irq, 1u);
    CHECK((nesturbator__apu_status_read(nes) & 0x40u) != 0u);
    nes->bus.apu_get_put_phase = 1u;
    nesturbator__apu_clock(nes);
    CHECK_EQ_U64(nes->apu.frame_cycle, 29829u);
    CHECK_EQ_U64(nes->apu.frame_irq, 1u);
    CHECK_EQ_U64(nes->cpu.irq_line, 1u);

    /* Inhibit clears only the frame source; DMC remains asserted. */
    nes->apu.dmc.irq = 1u;
    nesturbator__apu_write(nes, 0x4017u, 0x40u);
    CHECK_EQ_U64(nes->apu.frame_irq, 0u);
    CHECK_EQ_U64(nes->apu.dmc.irq, 1u);
    CHECK_EQ_U64(nes->cpu.irq_line, 1u);

    /* Five-step mode clocks the delayed immediate quarter/half edge and
       never generates a frame IRQ. */
    nesturbator__apu_write(nes, 0x4017u, 0x80u);
    nes->apu.frame_reset_delay = 0u;
    nes->apu.frame_cycle = 14912u;
    uint8_t length = nes->apu.pulse[0].length;
    nesturbator__apu_clock(nes);
    CHECK_EQ_U64(nes->apu.frame_irq, 0u);
    CHECK_EQ_U64(nes->apu.pulse[0].length, length);
    nesturbator_destroy(inst);
}

static void test_get_put_delays(void)
{
    nesturbator *inst = make();
    struct nesturbator *nes = (struct nesturbator *)inst;

    /* The observed ROM trace labels phase 1 GET and phase 0 PUT. Both
       documented post-write delay lengths remain explicit at decode. */
    nes->bus.apu_get_put_phase = 1u;
    nesturbator__apu_write(nes, 0x4017u, 0x80u);
    CHECK_EQ_U64(nes->apu.frame_reset_delay, 4u);
    nes->apu.frame_reset_delay = 0u;
    nes->bus.apu_get_put_phase = 0u;
    nesturbator__apu_write(nes, 0x4017u, 0x80u);
    CHECK_EQ_U64(nes->apu.frame_reset_delay, 3u);

    nesturbator__apu_write(nes, 0x4010u, 0u);
    nesturbator__apu_write(nes, 0x4012u, 1u);
    nesturbator__apu_write(nes, 0x4013u, 0u);
    nes->bus.apu_get_put_phase = 1u;
    nesturbator__apu_write(nes, 0x4015u, 0x10u);
    CHECK_EQ_U64(nes->apu.dmc.enable_delay, 4u);
    nes->apu.dmc.remaining = 0u;
    nes->apu.dmc.dma_pending = 0u;
    nes->bus.apu_get_put_phase = 0u;
    nesturbator__apu_write(nes, 0x4015u, 0x10u);
    CHECK_EQ_U64(nes->apu.dmc.enable_delay, 3u);
    nesturbator_destroy(inst);
}

static void test_dmc_timer_uses_half_rate_phase(void)
{
    nesturbator *inst = make();
    struct nesturbator *nes = (struct nesturbator *)inst;
    nes->apu.dmc.reg[0] = 0x0fu;
    nes->apu.dmc.timer = 27u; /* 54 CPU cycles at the fastest NTSC rate. */
    nes->apu.dmc.counter = 1u;
    nes->apu.dmc.bits = 2u;
    nes->apu.dmc.shift = 3u;
    nes->apu.dmc.output = 64u;
    nes->apu.pulse_clock_phase = 1u;

    nesturbator__apu_clock(nes);
    CHECK_EQ_U64(nes->apu.dmc.counter, 27u);
    CHECK_EQ_U64(nes->apu.dmc.output, 66u);

    /* After 27 more APU edges (54 CPU cycles), the next bit clocks. */
    for (unsigned i = 0u; i < 53u; i++)
        nesturbator__apu_clock(nes);
    CHECK_EQ_U64(nes->apu.dmc.output, 66u);
    nesturbator__apu_clock(nes);
    CHECK_EQ_U64(nes->apu.dmc.output, 68u);
    nesturbator_destroy(inst);
}

static void test_frame_irq_cpu_entry(void)
{
    const size_t image_size = 16u + 16384u + 8192u;
    uint8_t *image = calloc(1u, image_size);
    nesturbator *inst = make();
    CHECK(image != NULL);
    if (image == NULL || inst == NULL) {
        free(image);
        nesturbator_destroy(inst);
        return;
    }
    make_image(image, image_size);
    image[16u + 0x3ffeu] = 0x00u;
    image[16u + 0x3fffu] = 0x90u;
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, image_size), NESTURBATOR_OK);
    struct nesturbator *nes = (struct nesturbator *)inst;
    nes->cpu.pc = 0x8123u;
    nes->cpu.s = 0xfdu;
    nes->cpu.p = 0x20u;
    nes->cpu.irq_line = 1u;
    nes->cpu.poll_latch = 1u;
    nesturbator__cpu_step(nes);
    CHECK_EQ_U64(nes->cpu.pc, 0x9000u);
    CHECK_EQ_U64(nes->cpu.s, 0xfau);
    CHECK((nes->cpu.p & 0x04u) != 0u);
    CHECK_EQ_U64(nes->cpu.poll_latch, 0u);
    nesturbator_destroy(inst);
    free(image);
}

/* The stateless channel-level reader is independent of the event-selection
   optimization. Check every cycle, including both frame-counter modes. */
static void test_mixer_tracks_all_clock_events(void)
{
    nesturbator *inst = make();
    CHECK(inst != NULL);
    if (inst == NULL)
        return;
    struct nesturbator *nes = (struct nesturbator *)inst;
    static const uint8_t initial[20] = {0x3fu, 0x89u, 8u,    8u, 0xbfu, 0x8au, 16u, 8u,  0x9fu, 0u,
                                        3u,    8u,    0x3fu, 0u, 0u,    8u,    15u, 64u, 0u,    0u};
    nesturbator__apu_write(nes, 0x4015u, 0x1fu);
    for (uint16_t reg = 0u; reg < 20u; reg++)
        nesturbator__apu_write(nes, (uint16_t)(0x4000u + reg), initial[reg]);
    unsigned nonzero[5] = {0u};
    for (uint32_t cycle = 0u; cycle < 160000u; cycle++) {
        if (cycle == 120000u || cycle == 130000u)
            nesturbator__apu_write(nes, 0x4015u, cycle == 120000u ? 0u : 0x1fu);
        if (cycle == 40000u || cycle == 100000u)
            nesturbator__apu_write(nes, 0x4017u, cycle == 40000u ? 0x80u : 0u);
        if ((cycle & 8191u) == 0u) {
            nesturbator__apu_write(nes, 0x4003u, 8u);
            nesturbator__apu_write(nes, 0x4007u, 8u);
            nesturbator__apu_write(nes, 0x400bu, 8u);
            nesturbator__apu_write(nes, 0x400fu, 8u);
            nes->apu.dmc.sample_buffer = (uint8_t)(cycle >> 8);
            nes->apu.dmc.buffer_empty = 0u;
        }
        nes->ticks += 24u;
        nes->bus.apu_get_put_phase ^= 1u;
        nesturbator__apu_clock(nes);
        CHECK_EQ_U64(nes->synth.mixed_level, nesturbator__apu_mixed_level(nes));
        for (unsigned channel = 0u; channel < 5u; channel++)
            nonzero[channel] += nesturbator__apu_channel_level(nes, channel) != 0u;
    }
    for (unsigned channel = 0u; channel < 5u; channel++)
        CHECK(nonzero[channel] != 0u);
    nesturbator_destroy(inst);
}

/* Native-valid synthesis history can differ from the channel sum. Loading
   it must force a mixer event on the first otherwise uneventful clock. */
static void test_mixer_rechecks_restored_history(void)
{
    const size_t image_size = 16u + 16384u + 8192u;
    uint8_t *image = calloc(1u, image_size);
    nesturbator *inst = make();
    CHECK(image != NULL && inst != NULL);
    if (image == NULL || inst == NULL) {
        free(image);
        nesturbator_destroy(inst);
        return;
    }
    make_image(image, image_size);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, image_size), NESTURBATOR_OK);
    struct nesturbator *nes = (struct nesturbator *)inst;
    nesturbator__apu_write(nes, 0x4015u, 0u);
    nesturbator__apu_write(nes, 0x4011u, 0u);
    nes->apu.pulse[0].counter = 100u;
    nes->apu.pulse[1].counter = 100u;
    nes->apu.triangle.counter = 100u;
    nes->apu.noise.counter = 100u;
    nes->apu.dmc.counter = 100u;
    nes->apu.frame_cycle = 1u;
    nes->synth.mixed_level = 123;
    nes->synth.integrator_q15 = 123 * INT64_C(32768);
    size_t size = nesturbator_state_size(inst);
    uint8_t *state = malloc(size);
    uint8_t *round_trip = malloc(size);
    CHECK(state != NULL && round_trip != NULL);
    if (state != NULL && round_trip != NULL) {
        CHECK_EQ_U64(nesturbator_save_state(inst, state, size), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_load_state(inst, state, size), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_save_state(inst, round_trip, size), NESTURBATOR_OK);
        CHECK_EQ_U64(memcmp(state, round_trip, size), 0u);
        nes->ticks += 24u;
        nes->bus.apu_get_put_phase ^= 1u;
        nesturbator__apu_clock(nes);
        CHECK_EQ_U64(nes->synth.mixed_level, 0u);
    }
    free(state);
    free(round_trip);
    free(image);
    nesturbator_destroy(inst);
}

int main(void)
{
    test_public_pulse_pcm();
    test_mixer_tracks_all_clock_events();
    test_mixer_rechecks_restored_history();
    test_documented_channel_sequences();
    test_dmc_fetch_from_mapper0();
    test_dmc_enable_waits_full_write_delay();
    test_dmc_load_halts_on_get_cycle();
    test_dmc_halt_retries_after_cpu_writes();
    test_dmc_restart_preserves_full_sample_buffer();
    test_frame_counter_modes_and_irq_sources();
    test_get_put_delays();
    test_frame_irq_cpu_entry();
    test_dmc_timer_uses_half_rate_phase();
    CHECK_DONE();
}
