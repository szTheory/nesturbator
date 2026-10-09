/* NTSC RP2A03 APU channel clocks. Hardware facts are from
   NES-HARDWARE-CPU-APU.md section 4 (HWC.08, HWC.10, HWC.11). */
#include "internal.h"
#include "apu_mix_table.h"

static const uint8_t duty[4] = {0x01u, 0x03u, 0x0fu, 0xfcu};
static const uint8_t length_table[32] = {
    10u, 254u, 20u, 2u, 40u, 4u, 80u, 6u, 160u, 8u, 60u, 10u, 14u, 12u, 26u, 14u,
    12u, 16u, 24u, 18u, 48u, 20u, 96u, 22u, 192u, 24u, 72u, 26u, 16u, 28u, 32u, 30u
};
static const uint16_t noise_period[16] = {
    4u, 8u, 16u, 32u, 64u, 96u, 128u, 160u, 202u, 254u, 380u, 508u, 762u, 1016u, 2034u, 4068u};
static const uint16_t dmc_period[16] = {
    428u, 380u, 340u, 320u, 286u, 254u, 226u, 214u, 190u, 160u, 142u, 128u, 106u, 85u, 72u, 54u};
static const uint8_t triangle_sequence[32] = {
    15u, 14u, 13u, 12u, 11u, 10u, 9u, 8u, 7u, 6u, 5u, 4u, 3u, 2u, 1u, 0u,
    0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u, 13u, 14u, 15u};

static uint16_t sweep_target(const struct nesturbator__pulse *pulse, unsigned channel)
{
    unsigned shift = pulse->reg[1] & 7u;
    uint16_t change = (uint16_t)(pulse->timer >> shift);
    if ((pulse->reg[1] & 8u) != 0u)
        return (uint16_t)(pulse->timer - change - (channel == 0u ? 1u : 0u));
    return (uint16_t)(pulse->timer + change);
}

static uint8_t pulse_volume(const struct nesturbator__pulse *pulse)
{
    return (pulse->reg[0] & 0x10u) != 0u ? (uint8_t)(pulse->reg[0] & 0x0fu) : pulse->env_decay;
}

static uint8_t pulse_output(const struct nesturbator__pulse *pulse, uint8_t enabled, unsigned channel)
{
    uint16_t target = sweep_target(pulse, channel);
    if ((enabled == 0u) || pulse->length == 0u || pulse->timer < 8u || pulse->timer > 0x7ffu ||
        ((pulse->reg[1] & 7u) != 0u && target > 0x7ffu))
        return 0u;
    uint8_t sequence_bit = (uint8_t)((duty[pulse->reg[0] >> 6] >> (7u - pulse->phase)) & 1u);
    return sequence_bit != 0u ? pulse_volume(pulse) : 0u;
}

static uint8_t triangle_output(const struct nesturbator__triangle *triangle, uint8_t enabled)
{
    if ((enabled == 0u) || triangle->length == 0u || triangle->linear == 0u || triangle->timer < 2u)
        return 0u;
    return triangle_sequence[triangle->phase];
}

static uint8_t noise_output(const struct nesturbator__noise *noise, uint8_t enabled)
{
    if ((enabled == 0u) || noise->length == 0u || (noise->lfsr & 1u) != 0u)
        return 0u;
    return (noise->reg[0] & 0x10u) != 0u ? (uint8_t)(noise->reg[0] & 0x0fu) : noise->env_decay;
}

uint8_t nesturbator__apu_channel_level(const struct nesturbator *nes, unsigned channel)
{
    if (channel < 2u)
        return pulse_output(&nes->apu.pulse[channel], (uint8_t)(nes->apu.enabled & (1u << channel)), channel);
    if (channel == 2u)
        return triangle_output(&nes->apu.triangle, (uint8_t)(nes->apu.enabled & 4u));
    if (channel == 3u)
        return noise_output(&nes->apu.noise, (uint8_t)(nes->apu.enabled & 8u));
    if (channel == 4u)
        return nes->apu.dmc.output;
    return 0u;
}

/* Checked-in integer cells reproduce the nonlinear pulse and TND mixer
   contract in HWC.11 without runtime floating point. */
uint16_t nesturbator__apu_mixed_level(const struct nesturbator *nes)
{
    uint32_t p = (uint32_t)nesturbator__apu_channel_level(nes, 0u) +
                 nesturbator__apu_channel_level(nes, 1u);
    uint32_t t = nesturbator__apu_channel_level(nes, 2u);
    uint32_t n = nesturbator__apu_channel_level(nes, 3u);
    uint32_t d = nesturbator__apu_channel_level(nes, 4u);
    uint32_t output = pulse_mix[p] + tnd_mix[t][n][d];
    return output > 32767u ? 32767u : (uint16_t)output;
}

static void update_irq_line(struct nesturbator *nes)
{
    nes->cpu.irq_line = (uint8_t)(nes->apu.frame_irq != 0u || nes->apu.dmc.irq != 0u);
}

static void envelope_clock(uint8_t control, uint8_t *divider, uint8_t *decay, uint8_t *start);

static void frame_quarter_clock(struct nesturbator__apu *apu)
{
    for (unsigned i = 0u; i < 2u; i++) {
        struct nesturbator__pulse *pulse = &apu->pulse[i];
        envelope_clock(pulse->reg[0], &pulse->env_divider, &pulse->env_decay, &pulse->env_start);
    }
    envelope_clock(apu->noise.reg[0], &apu->noise.env_divider,
                   &apu->noise.env_decay, &apu->noise.env_start);
    if (apu->triangle.linear_reload_flag != 0u)
        apu->triangle.linear = apu->triangle.linear_reload;
    else if (apu->triangle.linear != 0u)
        apu->triangle.linear--;
    if ((apu->triangle.reg[0] & 0x80u) == 0u)
        apu->triangle.linear_reload_flag = 0u;
}

static void envelope_clock(uint8_t control, uint8_t *divider, uint8_t *decay, uint8_t *start)
{
    uint8_t period = (uint8_t)(control & 0x0fu);
    if (*start != 0u) {
        *start = 0u;
        *decay = 15u;
        *divider = period;
    } else if (*divider == 0u) {
        *divider = period;
        if (*decay != 0u)
            (*decay)--;
        else if ((control & 0x20u) != 0u)
            *decay = 15u;
    } else {
        (*divider)--;
    }
}

static void length_clock(struct nesturbator__apu *apu)
{
    for (unsigned i = 0u; i < 2u; i++) {
        struct nesturbator__pulse *pulse = &apu->pulse[i];
        if ((pulse->reg[0] & 0x20u) == 0u && pulse->length != 0u)
            pulse->length--;
    }
    if ((apu->triangle.reg[0] & 0x80u) == 0u && apu->triangle.length != 0u)
        apu->triangle.length--;
    if ((apu->noise.reg[0] & 0x20u) == 0u && apu->noise.length != 0u)
        apu->noise.length--;
    for (unsigned i = 0u; i < 2u; i++) {
        struct nesturbator__pulse *pulse = &apu->pulse[i];
        uint8_t period = (uint8_t)((pulse->reg[1] >> 4) & 7u);
        uint16_t target = sweep_target(pulse, i);
        if (pulse->sweep_divider == 0u && (pulse->reg[1] & 0x80u) != 0u &&
            (pulse->reg[1] & 7u) != 0u && pulse->timer >= 8u && target <= 0x7ffu)
            pulse->timer = target;
        if (pulse->sweep_divider == 0u || pulse->sweep_reload != 0u) {
            pulse->sweep_divider = period;
            pulse->sweep_reload = 0u;
        } else {
            pulse->sweep_divider--;
        }
    }
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
    nes->apu.pulse_clock_phase ^= 1u;
    if (nes->apu.pulse_clock_phase == 0u) {
        for (unsigned i = 0u; i < 2u; i++) {
            struct nesturbator__pulse *pulse = &nes->apu.pulse[i];
            if (pulse->counter == 0u) {
                pulse->counter = pulse->timer;
                pulse->phase = (uint8_t)((pulse->phase + 1u) & 7u);
            } else {
                pulse->counter--;
            }
        }
    }
    struct nesturbator__triangle *triangle = &nes->apu.triangle;
    if (triangle->counter == 0u) {
        triangle->counter = triangle->timer;
        if (triangle->length != 0u && triangle->linear != 0u && triangle->timer >= 2u)
            triangle->phase = (uint8_t)((triangle->phase + 1u) & 31u);
    } else {
        triangle->counter--;
    }
    struct nesturbator__noise *noise = &nes->apu.noise;
    if (noise->counter == 0u) {
        noise->counter = noise_period[noise->reg[2] & 0x0fu];
        /* RP2A03G's measured power-up LFSR is zero; its first clock shifts in
           one. This follows CPU power-up measurement HWC.05, while HWC.08's
           operational “loads 1” wording describes the usual running model. */
        if (noise->lfsr == 0u) {
            noise->lfsr = 1u;
        } else {
            uint16_t tap = (noise->reg[2] & 0x80u) != 0u ? 6u : 1u;
            uint16_t feedback = (uint16_t)((noise->lfsr ^ (noise->lfsr >> tap)) & 1u);
            noise->lfsr = (uint16_t)((noise->lfsr >> 1) | (feedback << 14));
        }
    } else {
        noise->counter--;
    }
    struct nesturbator__dmc *dmc = &nes->apu.dmc;
    if (dmc->counter == 0u) {
        dmc->counter = dmc_period[dmc->reg[0] & 0x0fu];
        if (dmc->bits == 0u) {
            if (dmc->buffer_empty == 0u) {
                dmc->shift = dmc->sample_buffer;
                dmc->buffer_empty = 1u;
                dmc->bits = 8u;
            }
        }
        if (dmc->bits != 0u) {
            if ((dmc->shift & 1u) != 0u) {
                if (dmc->output <= 125u)
                    dmc->output = (uint8_t)(dmc->output + 2u);
            } else if (dmc->output >= 2u) {
                dmc->output = (uint8_t)(dmc->output - 2u);
            }
            dmc->shift >>= 1;
            dmc->bits--;
        }
        if (dmc->buffer_empty != 0u && dmc->remaining != 0u)
            dmc->dma_pending = 1u;
    } else {
        dmc->counter--;
    }
    /* Frame-counter writes reset after three or four CPU cycles according to
       APU phase; the sequence edge follows two or three cycles later. The
       cycle-level model follows HWC.23 and the AccuracyCoin page-14 timing
       cases; this boundary intentionally has no M2-high subcycle state. */
    if (nes->apu.frame_reset_delay != 0u) {
        nes->apu.frame_reset_delay--;
        if (nes->apu.frame_reset_delay == 0u) {
            nes->apu.frame_mode = nes->apu.frame_pending_mode;
            nes->apu.frame_cycle = 0u;
            if (nes->apu.frame_mode != 0u) {
                frame_quarter_clock(&nes->apu);
                length_clock(&nes->apu);
            }
        }
    } else {
        nes->apu.frame_cycle++;
        if (nes->apu.frame_cycle == 7457u || nes->apu.frame_cycle == 14913u ||
            nes->apu.frame_cycle == 22371u ||
            (nes->apu.frame_mode == 0u && nes->apu.frame_cycle == 29829u) ||
            (nes->apu.frame_mode != 0u && nes->apu.frame_cycle == 37281u)) {
            frame_quarter_clock(&nes->apu);
            if (nes->apu.frame_cycle == 14913u || nes->apu.frame_cycle == 37281u)
                length_clock(&nes->apu);
        }
        if (nes->apu.frame_mode == 0u && nes->apu.frame_cycle >= 29828u &&
            nes->apu.frame_cycle <= 29830u && nes->apu.frame_irq_inhibit == 0u)
            nes->apu.frame_irq = 1u;
        if ((nes->apu.frame_mode == 0u && nes->apu.frame_cycle >= 29830u) ||
            (nes->apu.frame_mode != 0u && nes->apu.frame_cycle >= 37282u))
            nes->apu.frame_cycle = 0u;
    }
    update_irq_line(nes);
    nes->apu.sample_phase += 24u * NESTURBATOR_AUDIO_SAMPLES_PER_PERIOD;
    while (nes->apu.sample_phase >= NESTURBATOR_AUDIO_TICKS_PER_PERIOD) {
        nes->apu.sample_phase -= NESTURBATOR_AUDIO_TICKS_PER_PERIOD;
        if (nes->apu.sample_output != NULL && nes->apu.sample_count < nes->apu.sample_limit) {
            /* Integer-only table mix; filtered synthesis is a later phase
               deliverable. */
            nes->apu.sample_output[nes->apu.sample_count++] =
                (int16_t)nesturbator__apu_mixed_level(nes);
        }
    }
}

uint8_t nesturbator__apu_status_read(struct nesturbator *nes)
{
    uint8_t status = (uint8_t)((nes->apu.pulse[0].length != 0u ? 1u : 0u) |
                               (nes->apu.pulse[1].length != 0u ? 2u : 0u) |
                               (nes->apu.triangle.length != 0u ? 4u : 0u) |
                               (nes->apu.noise.length != 0u ? 8u : 0u) |
                               (nes->apu.dmc.remaining != 0u ? 0x10u : 0u) |
                               (nes->apu.frame_irq != 0u ? 0x40u : 0u) |
                               (nes->apu.dmc.irq != 0u ? 0x80u : 0u));
    nes->apu.frame_irq = 0u;
    update_irq_line(nes);
    return status;
}

void nesturbator__apu_write(struct nesturbator *nes, uint16_t addr, uint8_t value)
{
    if (addr == 0x4017u) {
        nes->apu.frame_irq_inhibit = (uint8_t)((value >> 6) & 1u);
        if (nes->apu.frame_irq_inhibit != 0u)
            nes->apu.frame_irq = 0u;
        nes->apu.frame_pending_mode = (uint8_t)((value >> 7) & 1u);
        nes->apu.frame_reset_delay = (uint8_t)(((nes->ticks / 24u) & 1u) != 0u ? 3u : 4u);
        update_irq_line(nes);
        return;
    }
    if (addr == 0x4015u) {
        nes->apu.enabled = (uint8_t)(value & 0x1fu);
        for (unsigned i = 0u; i < 4u; i++) {
            if ((nes->apu.enabled & (1u << i)) == 0u) {
                if (i < 2u)
                    nes->apu.pulse[i].length = 0u;
                else if (i == 2u)
                    nes->apu.triangle.length = 0u;
                else
                    nes->apu.noise.length = 0u;
            }
        }
        if ((nes->apu.enabled & 0x10u) == 0u)
            nes->apu.dmc.remaining = 0u;
        else if (nes->apu.dmc.remaining == 0u) {
            nes->apu.dmc.address = (uint16_t)(0xc000u | ((uint32_t)nes->apu.dmc.reg[2] << 6));
            nes->apu.dmc.remaining = (uint16_t)(((uint32_t)nes->apu.dmc.reg[3] << 4) | 1u);
            nes->apu.dmc.buffer_empty = 1u;
            nes->apu.dmc.dma_pending = 1u;
        }
        nes->apu.dmc.irq = 0u;
        update_irq_line(nes);
        return;
    }
    if (addr >= 0x4008u && addr <= 0x400bu) {
        struct nesturbator__triangle *triangle = &nes->apu.triangle;
        unsigned reg = (unsigned)(addr - 0x4008u);
        triangle->reg[reg] = value;
        if (reg == 0u)
            triangle->linear_reload = (uint8_t)(value & 0x7fu);
        else if (reg == 2u)
            triangle->timer = (uint16_t)((triangle->timer & 0x0700u) | value);
        else if (reg == 3u) {
            triangle->timer = (uint16_t)((triangle->timer & 0x00ffu) |
                                         ((uint32_t)(value & 7u) << 8));
            triangle->counter = triangle->timer;
            triangle->phase = 0u;
            triangle->linear = triangle->linear_reload;
            triangle->linear_reload_flag = 1u;
            if ((nes->apu.enabled & 4u) != 0u)
                triangle->length = length_table[value >> 3];
        }
        return;
    }
    if (addr >= 0x400cu && addr <= 0x400fu) {
        struct nesturbator__noise *noise = &nes->apu.noise;
        unsigned reg = (unsigned)(addr - 0x400cu);
        noise->reg[reg] = value;
        if (reg == 3u) {
            noise->env_start = 1u;
            if ((nes->apu.enabled & 8u) != 0u)
                noise->length = length_table[value >> 3];
        }
        return;
    }
    if (addr >= 0x4010u && addr <= 0x4013u) {
        struct nesturbator__dmc *dmc = &nes->apu.dmc;
        unsigned reg = (unsigned)(addr - 0x4010u);
        dmc->reg[reg] = value;
        if (reg == 0u) {
            dmc->timer = dmc_period[value & 0x0fu];
            if ((value & 0x80u) == 0u)
                dmc->irq = 0u;
            update_irq_line(nes);
        } else if (reg == 1u)
            dmc->output = (uint8_t)(value & 0x7fu);
        return;
    }
    if (addr < 0x4000u || addr > 0x4007u)
        return;
    unsigned index = (unsigned)((addr - 0x4000u) / 4u);
    unsigned reg = (unsigned)((addr - 0x4000u) & 3u);
    struct nesturbator__pulse *pulse = &nes->apu.pulse[index];
    pulse->reg[reg] = value;
    if (reg == 1u)
        pulse->sweep_reload = 1u;
    if (reg == 2u) {
        pulse->timer = (uint16_t)((pulse->timer & 0x0700u) | value);
    } else if (reg == 3u) {
        pulse->timer = (uint16_t)((pulse->timer & 0x00ffu) |
                                  ((uint32_t)(value & 7u) << 8));
        pulse->counter = pulse->timer;
        pulse->phase = 0u;
        pulse->env_start = 1u;
        if ((nes->apu.enabled & (1u << index)) != 0u)
            pulse->length = length_table[value >> 3];
    }
}
