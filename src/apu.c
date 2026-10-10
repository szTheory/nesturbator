/* NTSC RP2A03 APU channel clocks. Hardware facts are from
   NES-HARDWARE-CPU-APU.md section 4 (HWC.08, HWC.10, HWC.11). */
#include "internal.h"

#include <string.h>

static const uint8_t duty[4] = {0x01u, 0x03u, 0x0fu, 0xfcu};
static const uint8_t length_table[32] = {10u, 254u, 20u,  2u,  40u, 4u,  80u, 6u,  160u, 8u,  60u,
                                         10u, 14u,  12u,  26u, 14u, 12u, 16u, 24u, 18u,  48u, 20u,
                                         96u, 22u,  192u, 24u, 72u, 26u, 16u, 28u, 32u,  30u};
static const uint16_t noise_period[16] = {4u,   8u,   16u,  32u,  64u,  96u,   128u,  160u,
                                          202u, 254u, 380u, 508u, 762u, 1016u, 2034u, 4068u};
static const uint16_t dmc_period[16] = {428u, 380u, 340u, 320u, 286u, 254u, 226u, 214u,
                                        190u, 160u, 142u, 128u, 106u, 84u,  72u,  54u};
static const uint8_t triangle_sequence[32] = {15u, 14u, 13u, 12u, 11u, 10u, 9u,  8u,  7u,  6u, 5u,
                                              4u,  3u,  2u,  1u,  0u,  0u,  1u,  2u,  3u,  4u, 5u,
                                              6u,  7u,  8u,  9u,  10u, 11u, 12u, 13u, 14u, 15u};

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

static uint8_t pulse_output(const struct nesturbator__pulse *pulse, uint8_t enabled,
                            unsigned channel)
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
        return pulse_output(&nes->apu.pulse[channel], (uint8_t)(nes->apu.enabled & (1u << channel)),
                            channel);
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
    uint32_t p =
        (uint32_t)nesturbator__apu_channel_level(nes, 0u) + nesturbator__apu_channel_level(nes, 1u);
    uint32_t t = nesturbator__apu_channel_level(nes, 2u);
    uint32_t n = nesturbator__apu_channel_level(nes, 3u);
    uint32_t d = nesturbator__apu_channel_level(nes, 4u);
    uint32_t output = nesturbator__pulse_mix[p] + nesturbator__tnd_mix[t][n][d];
    return output > 32767u ? 32767u : (uint16_t)output;
}

/* Keep the observer ahead of synthesis so it sees every level transition at
   the shared CPU-cycle cursor, including multiple events with the same cycle. */
static void update_mixed_level(struct nesturbator *nes)
{
    int32_t level = (int32_t)nesturbator__apu_mixed_level(nes);
    if (level == nes->synth.mixed_level)
        return;
    if (nes->transition_sink != NULL)
        nes->transition_sink(nes->transition_sink_context, nes->ticks / 24u, level);
    nesturbator__synth_transition(nes, level);
}

void nesturbator__apu_set_transition_sink(struct nesturbator *nes,
                                          nesturbator__transition_sink sink, void *context)
{
    if (nes == NULL)
        return;
    nes->transition_sink = sink;
    nes->transition_sink_context = context;
}

/* The one writer of the CPU IRQ line. Every source calls it when it changes,
   so the line stays event-driven (NESdev "IRQ"). */
void nesturbator__irq_update(struct nesturbator *nes)
{
    nes->cpu.irq_line = (uint8_t)((nes->apu.frame_irq != 0u && nes->apu.frame_irq_inhibit == 0u) ||
                                  nes->apu.dmc.irq != 0u || nes->mapper.irq != 0u);
}

static void envelope_clock(uint8_t control, uint8_t *divider, uint8_t *decay, uint8_t *start);

static void frame_quarter_clock(struct nesturbator__apu *apu)
{
    for (unsigned i = 0u; i < 2u; i++) {
        struct nesturbator__pulse *pulse = &apu->pulse[i];
        envelope_clock(pulse->reg[0], &pulse->env_divider, &pulse->env_decay, &pulse->env_start);
    }
    envelope_clock(apu->noise.reg[0], &apu->noise.env_divider, &apu->noise.env_decay,
                   &apu->noise.env_start);
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
    update_mixed_level(nes);
}

void nesturbator__apu_end_frame(struct nesturbator *nes)
{
    if (nes->apu.sample_output != NULL && nes->apu.sample_count != 0u) {
        uint32_t drained =
            nesturbator__synth_drain(nes, nes->apu.sample_output, nes->apu.sample_count);
        if (drained < nes->apu.sample_count) {
            memset(nes->apu.sample_output + drained, 0,
                   (size_t)(nes->apu.sample_count - drained) * sizeof nes->apu.sample_output[0]);
        }
    }
    nes->apu.sample_output = NULL;
    nes->apu.sample_limit = 0u;
    nes->apu.sample_count = 0u;
}

void nesturbator__apu_clock(struct nesturbator *nes)
{
    if (nes->apu.frame_irq_clear_pending != 0u && nes->bus.apu_get_put_phase != 0u) {
        nes->apu.frame_irq = 0u;
        nes->apu.frame_irq_clear_pending = 0u;
    }
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
    if (dmc->enable_delay != 0u)
        dmc->enable_delay--;
    /* DMC rate entries are CPU-cycle periods, but the timer receives only
       every other CPU cycle. Its half-rate edge is offset from the pulse
       timer edge; the AccuracyCoin page-14 sync probes pin that phase.
       [HWC.08, HWC.10, HWC.19]. The output clock occurs on the edge that
       counts down to zero; reloading only after a later zero edge adds one
       APU cycle. */
    if (nes->apu.pulse_clock_phase == 0u) {
        uint8_t output_clock = 0u;
        if (dmc->counter == 0u) {
            output_clock = 1u;
        } else {
            dmc->counter--;
            if (dmc->counter == 0u)
                output_clock = 1u;
        }
        if (output_clock != 0u) {
            uint16_t period = dmc->timer;
            if (period == 0u)
                period = (uint16_t)(dmc_period[dmc->reg[0] & 0x0fu] / 2u);
            dmc->counter = period;
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
            if (dmc->buffer_empty != 0u && dmc->remaining != 0u && dmc->dma_pending == 0u) {
                dmc->dma_pending = 1u;
                /* A re-enable while the sample buffer is full still owns the
                   first read once that buffer drains. AccuracyCoin's L/M/N
                   edge probes distinguish this delayed load (GET) from an
                   ordinary output-unit reload (PUT). [HWC.19] */
                dmc->dma_halt_phase = dmc->dma_load_waiting != 0u ? 1u : 0u;
                dmc->dma_load_waiting = 0u;
            }
        }
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
            if (nes->apu.frame_cycle == 14913u || nes->apu.frame_cycle == 29829u ||
                (nes->apu.frame_mode != 0u && nes->apu.frame_cycle == 37281u))
                length_clock(&nes->apu);
        }
        if (nes->apu.frame_mode == 0u &&
            (nes->apu.frame_cycle == 29828u || nes->apu.frame_cycle == 29829u))
            nes->apu.frame_irq = 1u;
        if (nes->apu.frame_mode == 0u && nes->apu.frame_cycle == 29830u) {
            /* The status flag is reasserted through this edge unless the
               inhibit latch suppresses the IRQ line at the final cycle. */
            nes->apu.frame_irq = (uint8_t)(nes->apu.frame_irq_inhibit == 0u);
        }
        if ((nes->apu.frame_mode == 0u && nes->apu.frame_cycle >= 29830u) ||
            (nes->apu.frame_mode != 0u && nes->apu.frame_cycle >= 37282u))
            nes->apu.frame_cycle = 0u;
    }
    nesturbator__irq_update(nes);
    update_mixed_level(nes);
    nes->apu.sample_phase += 24u * NESTURBATOR_AUDIO_SAMPLES_PER_PERIOD;
    if (nes->apu.sample_phase >= NESTURBATOR_AUDIO_TICKS_PER_PERIOD) {
        nes->apu.sample_phase -= NESTURBATOR_AUDIO_TICKS_PER_PERIOD;
        if (nes->apu.sample_output != NULL && nes->apu.sample_count < nes->apu.sample_limit) {
            nesturbator__synth_sample(nes);
            nes->apu.sample_count++;
        }
    }
}

/* Soft reset. NESdev Wiki "CPU power up state", after reset: $4015 = 0, triangle phase 0,
   [$4011] &= 1, $4017 unchanged. "APU": "Power-up and reset have the effect of writing $00" to
   $4015. "APU Frame Counter": a $4017 write takes effect after 3 or 4 CPU cycles. "PPU power up
   state": the frame counter behaves "as if the APU's $4017 were written 10 clocks before the first
   code starts executing"; the seven reset cycles plus that delay give the 2-or-1 cycle delay set
   here. The frame-IRQ clear is on no NESdev page; it rests on the blargg apu_reset readme
   (irq_flag_cleared) only. The synth and filter history are kept, so the output has no step. */
void nesturbator__apu_reset(struct nesturbator *nes)
{
    nes->apu.triangle.phase = 0u;
    nes->apu.dmc.output = (uint8_t)(nes->apu.dmc.output & 1u);
    nesturbator__apu_write(nes, 0x4015u, 0u);
    nes->apu.frame_irq = 0u;
    nes->apu.frame_irq_clear_pending = 0u;
    nes->apu.dmc.dma_pending = 0u;
    nes->apu.dmc.dma_halt_phase = 0u;
    nes->apu.frame_reset_delay = (uint8_t)(nes->bus.apu_get_put_phase != 0u ? 2u : 1u);
    nesturbator__irq_update(nes);
}

uint8_t nesturbator__apu_status_read(struct nesturbator *nes)
{
    uint8_t status =
        (uint8_t)((nes->apu.pulse[0].length != 0u ? 1u : 0u) |
                  (nes->apu.pulse[1].length != 0u ? 2u : 0u) |
                  (nes->apu.triangle.length != 0u ? 4u : 0u) |
                  (nes->apu.noise.length != 0u ? 8u : 0u) |
                  (nes->apu.dmc.remaining != 0u ? 0x10u : 0u) |
                  (nes->apu.frame_irq != 0u ? 0x40u : 0u) | (nes->apu.dmc.irq != 0u ? 0x80u : 0u));
    nes->apu.frame_irq_clear_pending = 1u;
    nesturbator__irq_update(nes);
    return status;
}

static void apu_write_register(struct nesturbator *nes, uint16_t addr, uint8_t value)
{
    if (addr == 0x4017u) {
        nes->apu.frame_irq_inhibit = (uint8_t)((value >> 6) & 1u);
        if (nes->apu.frame_irq_inhibit != 0u)
            nes->apu.frame_irq = 0u;
        nes->apu.frame_pending_mode = (uint8_t)((value >> 7) & 1u);
        /* The bus has clocked the write cycle before register decode. GET
           writes take four subsequent cycles and PUT writes take three;
           AccuracyCoin page 14 exercises both edges. */
        nes->apu.frame_reset_delay = (uint8_t)(nes->bus.apu_get_put_phase != 0u ? 4u : 3u);
        nesturbator__irq_update(nes);
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
        if ((nes->apu.enabled & 0x10u) == 0u) {
            /* A buffer-empty reload can already be waiting for the next
               enabled CPU read. Disabling the reader stops sample progress,
               but does not cancel that scheduled DMA. [HWC.10] */
            nes->apu.dmc.remaining = 0u;
            nes->apu.dmc.enable_delay = 0u;
            nes->apu.dmc.dma_load_waiting = 0u;
        } else if (nes->apu.dmc.remaining == 0u) {
            nes->apu.dmc.address = (uint16_t)(0xc000u | ((uint32_t)nes->apu.dmc.reg[2] << 6));
            nes->apu.dmc.remaining = (uint16_t)(((uint32_t)nes->apu.dmc.reg[3] << 4) | 1u);
            /* Restarting the memory reader does not discard a byte already
               held in the sample buffer. A fresh load is requested only if
               that buffer is empty; an output-unit refill retains reload
               phase. [HWC.10] */
            if (nes->apu.dmc.buffer_empty != 0u && nes->apu.dmc.dma_pending == 0u) {
                nes->apu.dmc.dma_pending = 1u;
                nes->apu.dmc.dma_halt_phase = 1u; /* load DMA halts on GET */
                nes->apu.dmc.dma_load_waiting = 0u;
            } else if (nes->apu.dmc.buffer_empty == 0u && nes->apu.dmc.dma_pending == 0u) {
                nes->apu.dmc.dma_load_waiting = 1u;
            }
            nes->apu.dmc.enable_delay = (uint8_t)(nes->bus.apu_get_put_phase != 0u ? 4u : 3u);
        }
        nes->apu.dmc.irq = 0u;
        nesturbator__irq_update(nes);
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
            triangle->timer =
                (uint16_t)((triangle->timer & 0x00ffu) | ((uint32_t)(value & 7u) << 8));
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
            dmc->timer = (uint16_t)(dmc_period[value & 0x0fu] / 2u);
            if ((value & 0x80u) == 0u)
                dmc->irq = 0u;
            nesturbator__irq_update(nes);
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
        pulse->timer = (uint16_t)((pulse->timer & 0x00ffu) | ((uint32_t)(value & 7u) << 8));
        pulse->counter = pulse->timer;
        pulse->phase = 0u;
        pulse->env_start = 1u;
        if ((nes->apu.enabled & (1u << index)) != 0u)
            pulse->length = length_table[value >> 3];
    }
}

void nesturbator__apu_write(struct nesturbator *nes, uint16_t addr, uint8_t value)
{
    apu_write_register(nes, addr, value);
    update_mixed_level(nes);
}
