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
    static const uint8_t program[] = {
        0xa9u, 0x01u, 0x8du, 0x15u, 0x40u, 0xa9u, 0xbfu, 0x8du, 0x00u, 0x40u,
        0xa9u, 0x08u, 0x8du, 0x02u, 0x40u, 0xa9u, 0x00u, 0x8du, 0x03u, 0x40u,
        0x4cu, 0x14u, 0x80u};
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
    static const int16_t expected[] = {0, 0, 4895, 4895, 0, 0, 4895, 4895};
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
    CHECK_EQ_U64(nes->apu.dmc.sample_buffer, 0x5au);
    CHECK_EQ_U64(nes->apu.dmc.address, 0xc041u);
    CHECK_EQ_U64(nes->apu.dmc.remaining, 0u);
    nesturbator_destroy(inst);
    free(image);
}

int main(void)
{
    test_public_pulse_pcm();
    test_documented_channel_sequences();
    test_dmc_fetch_from_mapper0();
    CHECK_DONE();
}
