/* NTSC RP2A03 pulse channels and caller-buffer sampling. Hardware facts are
   from NES-HARDWARE-CPU-APU.md sections 4.1 and 5 (HWC.08, HWC.11). */
#include "internal.h"

static const uint8_t duty[4] = {0x01u, 0x03u, 0x0fu, 0xfcu};
static const uint8_t length_table[32] = {
    10u, 254u, 20u, 2u, 40u, 4u, 80u, 6u, 160u, 8u, 60u, 10u, 14u, 12u, 26u, 14u,
    12u, 16u, 24u, 18u, 48u, 20u, 96u, 22u, 192u, 24u, 72u, 26u, 16u, 28u, 32u, 30u
};

static uint8_t pulse_output(const struct nesturbator__pulse *pulse, uint8_t enabled)
{
    if ((enabled == 0u) || pulse->length == 0u || pulse->timer < 8u || pulse->timer > 0x7ffu)
        return 0u;
    uint8_t sequence_bit = (uint8_t)((duty[pulse->reg[0] >> 6] >> (7u - pulse->phase)) & 1u);
    return sequence_bit != 0u ? (uint8_t)(pulse->reg[0] & 0x0fu) : 0u;
}

static uint8_t mixed_level(const struct nesturbator__apu *apu)
{
    uint16_t level = pulse_output(&apu->pulse[0], (uint8_t)(apu->enabled & 1u)) +
                     pulse_output(&apu->pulse[1], (uint8_t)(apu->enabled & 2u));
    return level > 30u ? 30u : (uint8_t)level;
}

void nesturbator__apu_begin_frame(struct nesturbator *nes, int16_t *samples, uint32_t count)
{
    nes->apu.sample_output = samples;
    nes->apu.sample_limit = count;
    nes->apu.sample_count = 0u;
    nes->apu.sample_phase = nes->audio_rem;
}

void nesturbator__apu_end_frame(struct nesturbator *nes)
{
    nes->apu.sample_output = NULL;
    nes->apu.sample_limit = 0u;
    nes->apu.sample_count = 0u;
}

void nesturbator__apu_clock(struct nesturbator *nes)
{
    for (unsigned i = 0u; i < 2u; i++) {
        struct nesturbator__pulse *pulse = &nes->apu.pulse[i];
        if (pulse->counter == 0u) {
            pulse->counter = pulse->timer;
            pulse->phase = (uint8_t)((pulse->phase + 1u) & 7u);
        } else {
            pulse->counter--;
        }
    }
    nes->apu.sample_phase += 24u * NESTURBATOR_AUDIO_SAMPLES_PER_PERIOD;
    while (nes->apu.sample_phase >= NESTURBATOR_AUDIO_TICKS_PER_PERIOD) {
        nes->apu.sample_phase -= NESTURBATOR_AUDIO_TICKS_PER_PERIOD;
        if (nes->apu.sample_output != NULL && nes->apu.sample_count < nes->apu.sample_limit) {
            /* The first tracer uses the deterministic mixed DAC level. The
               band-limited synthesis path is a later phase deliverable. */
            nes->apu.sample_output[nes->apu.sample_count++] =
                (int16_t)((int16_t)mixed_level(&nes->apu) * 512);
        }
    }
}

void nesturbator__apu_write(struct nesturbator *nes, uint16_t addr, uint8_t value)
{
    if (addr == 0x4015u) {
        nes->apu.enabled = (uint8_t)(value & 0x03u);
        for (unsigned i = 0u; i < 2u; i++) {
            if ((nes->apu.enabled & (1u << i)) == 0u)
                nes->apu.pulse[i].length = 0u;
        }
        return;
    }
    if (addr < 0x4000u || addr > 0x4007u)
        return;
    unsigned index = (unsigned)((addr - 0x4000u) / 4u);
    unsigned reg = (unsigned)((addr - 0x4000u) & 3u);
    struct nesturbator__pulse *pulse = &nes->apu.pulse[index];
    pulse->reg[reg] = value;
    if (reg == 2u) {
        pulse->timer = (uint16_t)((pulse->timer & 0x0700u) | value);
    } else if (reg == 3u) {
        pulse->timer = (uint16_t)((pulse->timer & 0x00ffu) |
                                  ((uint32_t)(value & 7u) << 8));
        pulse->counter = pulse->timer;
        pulse->phase = 0u;
        if ((nes->apu.enabled & (1u << index)) != 0u)
            pulse->length = length_table[value >> 3];
    }
}
