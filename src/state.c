/* Save states (STATE-01, STATE-02): nesturbator_state_size, nesturbator_save_state and
   nesturbator_load_state over a fixed-size little-endian format.

   Layout. A 64-byte header, then the body, then the cartridge RAM:
     header   magic, format version, header size, total size, behaviour revision, mapper,
              submapper, region, ROM CRC-32, state CRC-32, frame_number, ticks, reserved zeros
     body     CORE, CPU, BUS, APU, SYNTH, PPU and MAPPER fields, in struct order, no tags
     RAM      the whole PRG-RAM allocation, then the 8 KiB CHR-RAM when the board has it
   One field list per struct drives the SIZE, WRITE, VALIDATE and APPLY passes (state.h), so the
   four cannot drift apart. A struct is never copied as a whole: padding, pointers and host-owned
   buffers never reach the bytes (PROH-12-01-01).

   Not in a state: the allocator, save_generation, the transition sink, bus.input_pending (the host
   sets it before every frame), the frame-scoped APU and PPU pointers and counters, and the PCM
   ring. At a frame boundary those are NULL, 0 or empty (tests/core/test_boundary.c), and a load
   leaves the ring empty. The pages in nesturbator__map are derived from the registers after a load.

   The CRC-32 covers every byte except its own field (offsets 28 to 31). */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "internal.h"
#include "state.h"

#define STATE_HEADER_SIZE 64u
#define STATE_CHR_RAM_SIZE 8192u
/* Ceiling on the header's ticks: thousands of years of emulated time, far from 2^64 wrapping. */
#define STATE_TICKS_MAX (UINT64_C(1) << 62)

/* Header offsets. */
#define H_MAGIC 0u
#define H_VERSION 8u
#define H_HEADER_SIZE 10u
#define H_TOTAL_SIZE 12u
#define H_BEHAVIOUR 16u
#define H_MAPPER 20u
#define H_SUBMAPPER 22u
#define H_REGION 23u
#define H_ROM_CRC 24u
#define H_STATE_CRC 28u
#define H_FRAME 32u
#define H_TICKS 40u
#define H_RESERVED 48u

/* PNG-style magic: a high-bit byte, "NTS", CR LF, DOS end of file, LF. A text-mode transfer that
   strips the high bit or rewrites line ends breaks it. */
static const uint8_t state_magic[8] = {0x89u, 0x4eu, 0x54u, 0x53u, 0x0du, 0x0au, 0x1au, 0x0au};

/* Tripwires: a field added to one of these structs fails the build until the field list below
   visits it and the number is updated (Pitfall P14). The structs hold only fixed-width integers
   and arrays of them, so the sizes are the same on every platform. Structs that also hold
   pointers are checked by the offset of their last saved field instead. */
_Static_assert(sizeof(struct nesturbator__cpu) == 16u, "cpu fields changed: update cpu_io");
_Static_assert(sizeof(struct nesturbator__bus) == 2061u, "bus fields changed: update bus_io");
_Static_assert(sizeof(struct nesturbator__pulse) == 16u, "pulse fields changed: update pulse_io");
_Static_assert(sizeof(struct nesturbator__triangle) == 14u,
               "triangle fields changed: update triangle_io");
_Static_assert(sizeof(struct nesturbator__noise) == 12u, "noise fields changed: update noise_io");
_Static_assert(sizeof(struct nesturbator__dmc) == 22u, "dmc fields changed: update dmc_io");
_Static_assert(offsetof(struct nesturbator__apu, sample_phase) == 92u,
               "apu fields changed: update apu_io");
_Static_assert(offsetof(struct nesturbator__apu, sample_output) == 96u,
               "apu fields changed: update apu_io");
_Static_assert(offsetof(struct nesturbator__synth, pcm) ==
                   offsetof(struct nesturbator__synth, lp_output_q30) + 8u,
               "synth fields changed: update synth_io");
_Static_assert(offsetof(struct nesturbator__ppu, video_output) ==
                   ((offsetof(struct nesturbator__ppu, at_latch_hi) + 1u + 7u) & ~(size_t)7u),
               "ppu fields changed: update ppu_io");
_Static_assert(offsetof(struct nesturbator__ppu, reset_flag) -
                       offsetof(struct nesturbator__ppu, ppu_ticks) ==
                   13u,
               "ppu fields changed: update ppu_io");
_Static_assert(sizeof(struct nesturbator__profile) == 2u, "profile fields changed: update core_io");
_Static_assert(offsetof(struct nesturbator, audio_rem) ==
                   offsetof(struct nesturbator, profile) + 4u,
               "instance fields changed: update core_io");

/* ---- Field lists ----
   Each takes the walk's mode from the visitor. A bound marked S keeps a later index, shift count
   or signed product defined; L keeps the next frame finite; the rest are invariants the core
   always keeps (RESEARCH section 3). */

static void core_io(struct nesturbator__sio *io, struct nesturbator *nes)
{
    nesturbator__sio_u64(io, &nes->cpu_cycle);
    /* frame.c divides by 315000 samples-worth of ticks (NESTURBATOR_AUDIO_TICKS_PER_PERIOD). */
    nesturbator__sio_u32(io, &nes->audio_rem, NESTURBATOR_AUDIO_TICKS_PER_PERIOD - 1u);
    nesturbator__sio_u8(io, &nes->profile.ane_magic, 255u);
    nesturbator__sio_u8(io, &nes->profile.lxa_magic, 255u);
}

static void cpu_io(struct nesturbator__sio *io, struct nesturbator__cpu *c)
{
    uint8_t nmi_pending;
    uint8_t nmi_polled;
    nesturbator__sio_u16(io, &c->pc, 0xffffu);
    nesturbator__sio_u8(io, &c->a, 255u);
    nesturbator__sio_u8(io, &c->x, 255u);
    nesturbator__sio_u8(io, &c->y, 255u);
    nesturbator__sio_u8(io, &c->s, 255u);
    nesturbator__sio_u8(io, &c->p, 255u);
    /* The detector line, captured edge and last instruction poll are separate
       history. Live PPU status/control may lower the line without clearing the
       captured request or poll result. */
    nesturbator__sio_u8(io, &c->nmi_prev, 1u);
    nmi_pending = nesturbator__sio_u8(io, &c->nmi_pending, 1u);
    nesturbator__sio_cross(io);
    nmi_polled = nesturbator__sio_u8(io, &c->nmi_polled, 1u);
    nesturbator__sio_check(io, nmi_polled == 0u || nmi_pending != 0u);
    nesturbator__sio_u8(io, &c->irq_line, 1u);
    nesturbator__sio_u8(io, &c->irq_captured, 1u);
    nesturbator__sio_u8(io, &c->poll_latch, 1u);
    /* Never read or written anywhere, but visited so every field of the struct is accounted for.
       The bound is 0 because it is always 0 today; widen it when stalled reads are modelled. */
    nesturbator__sio_u8(io, &c->halted_in_read, 0u);
    nesturbator__sio_u8(io, &c->jammed, 1u);
}

static void bus_io(struct nesturbator__sio *io, struct nesturbator__bus *b)
{
    nesturbator__sio_bytes(io, b->ram, sizeof b->ram);
    nesturbator__sio_u8(io, &b->open_bus, 255u);
    nesturbator__sio_u8(io, &b->apu_get_put_phase, 1u);
    nesturbator__sio_u8(io, &b->oam_dma_pending, 1u);
    nesturbator__sio_u8(io, &b->oam_dma_page, 255u);
    /* input_pending is the host's, set before every frame: not saved. */
    nesturbator__sio_bytes(io, b->input_buttons, sizeof b->input_buttons);
    nesturbator__sio_bytes(io, b->controller_latch, sizeof b->controller_latch);
    nesturbator__sio_bytes(io, b->controller_shift, sizeof b->controller_shift);
    nesturbator__sio_u8(io, &b->controller_strobe, 1u);
}

static void pulse_io(struct nesturbator__sio *io, struct nesturbator__pulse *p)
{
    nesturbator__sio_u16(io, &p->timer, 0x7ffu);
    nesturbator__sio_u16(io, &p->counter, 0x7ffu);
    nesturbator__sio_bytes(io, p->reg, sizeof p->reg);
    /* S: duty[...] >> (7 - phase) in pulse_output. */
    nesturbator__sio_u8(io, &p->phase, 7u);
    nesturbator__sio_u8(io, &p->length, 255u);
    nesturbator__sio_u8(io, &p->env_divider, 15u);
    /* S: the channel level indexes the mixer tables. */
    nesturbator__sio_u8(io, &p->env_decay, 15u);
    nesturbator__sio_u8(io, &p->env_start, 1u);
    nesturbator__sio_u8(io, &p->sweep_divider, 7u);
    nesturbator__sio_u8(io, &p->sweep_reload, 1u);
}

static void triangle_io(struct nesturbator__sio *io, struct nesturbator__triangle *t)
{
    nesturbator__sio_u16(io, &t->timer, 0x7ffu);
    nesturbator__sio_u16(io, &t->counter, 0x7ffu);
    nesturbator__sio_bytes(io, t->reg, sizeof t->reg);
    /* S: indexes the 32-entry triangle sequence. */
    nesturbator__sio_u8(io, &t->phase, 31u);
    nesturbator__sio_u8(io, &t->length, 255u);
    nesturbator__sio_u8(io, &t->linear, 0x7fu);
    nesturbator__sio_u8(io, &t->linear_reload, 0x7fu);
    nesturbator__sio_u8(io, &t->linear_reload_flag, 1u);
}

static void noise_io(struct nesturbator__sio *io, struct nesturbator__noise *n)
{
    nesturbator__sio_u16(io, &n->lfsr, 0x7fffu);
    /* The largest entry of the noise period table is 4068. */
    nesturbator__sio_u16(io, &n->counter, 0xfffu);
    nesturbator__sio_bytes(io, n->reg, sizeof n->reg);
    nesturbator__sio_u8(io, &n->length, 255u);
    nesturbator__sio_u8(io, &n->env_divider, 15u);
    /* S: the channel level indexes the mixer tables. */
    nesturbator__sio_u8(io, &n->env_decay, 15u);
    nesturbator__sio_u8(io, &n->env_start, 1u);
}

static void dmc_io(struct nesturbator__sio *io, struct nesturbator__dmc *d)
{
    /* The timer is half a rate-table entry; the largest entry is 428. */
    nesturbator__sio_u16(io, &d->timer, 214u);
    nesturbator__sio_u16(io, &d->counter, 214u);
    nesturbator__sio_u16(io, &d->address, 0xffffu);
    /* (255 << 4) | 1, the longest sample. */
    nesturbator__sio_u16(io, &d->remaining, 4081u);
    nesturbator__sio_bytes(io, d->reg, sizeof d->reg);
    /* S: indexes the 128-entry last dimension of the TND mixer table. */
    nesturbator__sio_u8(io, &d->output, 127u);
    nesturbator__sio_u8(io, &d->shift, 255u);
    nesturbator__sio_u8(io, &d->bits, 8u);
    nesturbator__sio_u8(io, &d->sample_buffer, 255u);
    nesturbator__sio_u8(io, &d->buffer_empty, 1u);
    nesturbator__sio_u8(io, &d->irq, 1u);
    nesturbator__sio_u8(io, &d->dma_pending, 1u);
    nesturbator__sio_u8(io, &d->dma_halt_phase, 1u);
    nesturbator__sio_u8(io, &d->dma_load_waiting, 1u);
    nesturbator__sio_u8(io, &d->enable_delay, 4u);
}

static void apu_io(struct nesturbator__sio *io, struct nesturbator__apu *a)
{
    pulse_io(io, &a->pulse[0]);
    pulse_io(io, &a->pulse[1]);
    triangle_io(io, &a->triangle);
    noise_io(io, &a->noise);
    dmc_io(io, &a->dmc);
    nesturbator__sio_u8(io, &a->enabled, 0x1fu);
    nesturbator__sio_u8(io, &a->pulse_clock_phase, 1u);
    nesturbator__sio_u8(io, &a->frame_mode, 1u);
    nesturbator__sio_u8(io, &a->frame_pending_mode, 1u);
    nesturbator__sio_u8(io, &a->frame_irq_inhibit, 1u);
    nesturbator__sio_u8(io, &a->frame_irq, 1u);
    nesturbator__sio_u8(io, &a->frame_irq_clear_pending, 1u);
    nesturbator__sio_u8(io, &a->frame_reset_delay, 4u);
    /* The frame sequencer wraps at 29830 or 37282 cycles. */
    nesturbator__sio_u32(io, &a->frame_cycle, 37281u);
    /* S: sample_phase / 315000 picks a kernel phase of the 32 (synth.c). */
    nesturbator__sio_u32(io, &a->sample_phase, NESTURBATOR_AUDIO_TICKS_PER_PERIOD - 1u);
    /* Frame staging and derived mixed_level_valid are not saved. */
}

/* History bounds. A mixer transition adds at most 2^15 times a 2^15 kernel coefficient to each of
   the 16 impulse slots, and the largest value seen in practice is about 2^29, so 2^40 leaves wide
   margin while keeping integrator * 32768 inside 64 bits. The filter memories are Q30 products of
   a 2^15 signal, theoretically below 2^45; 2^52 keeps multiply_q30 inside 64 bits (all S). */
#define SYNTH_HISTORY_BOUND (INT64_C(1) << 40)
#define SYNTH_FILTER_BOUND (INT64_C(1) << 52)

static void synth_io(struct nesturbator__sio *io, struct nesturbator__synth *s)
{
    int64_t sum = 0;
    int32_t level;
    int64_t integrator;
    /* S: the mixer clamps at 32767; level - mixed_level must not overflow int32_t. */
    nesturbator__sio_cross(io);
    level = nesturbator__sio_i32(io, &s->mixed_level, 0, 32767);
    for (size_t i = 0u; i < NESTURBATOR_SYNTH_TAPS; ++i) {
        int64_t v;
        nesturbator__sio_cross(io);
        v = nesturbator__sio_i64(io, &s->impulse[i], SYNTH_HISTORY_BOUND);
        /* Only VALIDATE sums: its values are in range, so the sum cannot overflow. */
        if (io->mode == NESTURBATOR__SIO_VALIDATE)
            sum += v;
    }
    /* S: indexes impulse[]. */
    nesturbator__sio_u8(io, &s->impulse_head, (uint8_t)(NESTURBATOR_SYNTH_TAPS - 1u));
    nesturbator__sio_cross(io);
    integrator = nesturbator__sio_i64(io, &s->integrator_q15, SYNTH_HISTORY_BOUND);
    /* The kernel coefficients sum to 32768, so the integrator and the impulses always add up to
       the mixed level in Q15. Every term is bounded above, so the sum cannot overflow. */
    if (io->mode == NESTURBATOR__SIO_VALIDATE)
        nesturbator__sio_check(io, integrator + sum == (int64_t)level * 32768);
    for (size_t i = 0u; i < 2u; ++i)
        nesturbator__sio_i64(io, &s->hp_input_q30[i], SYNTH_FILTER_BOUND);
    for (size_t i = 0u; i < 2u; ++i)
        nesturbator__sio_i64(io, &s->hp_output_q30[i], SYNTH_FILTER_BOUND);
    nesturbator__sio_i64(io, &s->lp_output_q30, SYNTH_FILTER_BOUND);
    /* pcm and its ring counters are empty at a frame boundary: not saved. */
}

static void flags_io(struct nesturbator__sio *io, uint8_t *p, size_t n, uint8_t max)
{
    for (size_t i = 0u; i < n; ++i)
        nesturbator__sio_u8(io, &p[i], max);
}

static void ppu_io(struct nesturbator__sio *io, struct nesturbator__ppu *p)
{
    uint8_t status, eval_count, eval_copy_remaining;
    uint64_t ppu_ticks;
    nesturbator__sio_u8(io, &p->control, 255u);
    nesturbator__sio_u8(io, &p->mask, 255u);
    /* Only bits 7 to 5 are ever set. */
    nesturbator__sio_cross(io);
    status = nesturbator__sio_u8(io, &p->status, 255u);
    nesturbator__sio_check(io, (status & 0x1fu) == 0u);
    nesturbator__sio_u8(io, &p->oam_addr, 255u);
    nesturbator__sio_u8(io, &p->address_latch, 1u);
    /* S: 7 - fine_x and 15 - fine_x are shift counts. */
    nesturbator__sio_u8(io, &p->fine_x, 7u);
    nesturbator__sio_u8(io, &p->read_buffer, 255u);
    nesturbator__sio_u8(io, &p->io_bus, 255u);
    nesturbator__sio_u32(io, &p->io_bus_age, 160000u);
    nesturbator__sio_u8(io, &p->vblank_suppress, 1u);
    /* 15 bits; the core masks v with 0x7fff on every increment. */
    nesturbator__sio_u16(io, &p->v, 0x7fffu);
    nesturbator__sio_u16(io, &p->t, 0x7fffu);
    nesturbator__sio_bytes(io, p->nametable, sizeof p->nametable);
    /* A palette entry is six bits: pixel bits 9 to 15 stay zero. */
    flags_io(io, p->palette, sizeof p->palette, 0x3fu);
    nesturbator__sio_bytes(io, p->oam, sizeof p->oam);
    nesturbator__sio_bytes(io, p->secondary_oam, sizeof p->secondary_oam);
    /* S: sprite_count bounds the loop over the eight sprite slots. */
    nesturbator__sio_u8(io, &p->sprite_count, 8u);
    /* S: eval_count indexes secondary_oam and eval_sprite_zero. */
    nesturbator__sio_cross(io);
    eval_count = nesturbator__sio_u8(io, &p->eval_count, 8u);
    /* S: eval_n indexes oam (64 sprites, 64 is the finished scan). */
    nesturbator__sio_u8(io, &p->eval_n, 64u);
    /* S: eval_m is the byte within a sprite. */
    nesturbator__sio_u8(io, &p->eval_m, 3u);
    nesturbator__sio_u8(io, &p->eval_remaining, 3u);
    /* S: eval_copy_remaining pairs with eval_count - 1 as a slot index, so it needs a slot. */
    nesturbator__sio_cross(io);
    eval_copy_remaining = nesturbator__sio_u8(io, &p->eval_copy_remaining, 3u);
    nesturbator__sio_check(io, eval_copy_remaining == 0u || eval_count >= 1u);
    nesturbator__sio_u8(io, &p->eval_latch, 255u);
    nesturbator__sio_u8(io, &p->eval_target, 255u);
    flags_io(io, p->eval_sprite_zero, sizeof p->eval_sprite_zero, 1u);
    flags_io(io, p->sprite_zero, sizeof p->sprite_zero, 1u);
    nesturbator__sio_bytes(io, p->sprite_x, sizeof p->sprite_x);
    nesturbator__sio_bytes(io, p->sprite_attr, sizeof p->sprite_attr);
    nesturbator__sio_bytes(io, p->sprite_lo, sizeof p->sprite_lo);
    nesturbator__sio_bytes(io, p->sprite_hi, sizeof p->sprite_hi);
    nesturbator__sio_u16(io, &p->bus_addr, 0x3fffu);
    nesturbator__sio_u8(io, &p->ale_latch, 255u);
    nesturbator__sio_u8(io, &p->bg_nt, 255u);
    nesturbator__sio_u8(io, &p->bg_at, 3u);
    nesturbator__sio_u8(io, &p->bg_lo, 255u);
    nesturbator__sio_u8(io, &p->bg_hi, 255u);
    nesturbator__sio_u16(io, &p->bg_shift_lo, 0xffffu);
    nesturbator__sio_u16(io, &p->bg_shift_hi, 0xffffu);
    nesturbator__sio_u8(io, &p->at_shift_lo, 255u);
    nesturbator__sio_u8(io, &p->at_shift_hi, 255u);
    nesturbator__sio_u8(io, &p->at_latch_lo, 1u);
    nesturbator__sio_u8(io, &p->at_latch_hi, 1u);
    /* video_output and video_pitch belong to the frame: not saved. */
    nesturbator__sio_cross(io);
    ppu_ticks = nesturbator__sio_u64(io, &p->ppu_ticks);
    /* L: nesturbator__ppu_run_until loops (ticks - ppu_ticks) / 8 times at the start of the next
       frame (the `while (nes->ppu.ppu_ticks + 8u <= ticks)` loop in src/ppu.c), so a hostile gap
       would not finish. A running core leaves the PPU within one dot of the CPU; a state saved
       right after nesturbator_reset is up to 7 CPU cycles behind. A gap below one frame loads and
       costs at most one frame of catch-up (Pitfall P5). The header ticks is also capped at
       STATE_TICKS_MAX: `ppu_ticks + 8u <= ticks` and `ticks + NESTURBATOR_TICKS_PER_FRAME` wrap
       near 2^64 and would then never finish. ppu_ticks <= ticks, so it is capped too. The other
       64-bit stamps (cpu_cycle, the MMC1 and MMC3 write stamps) are only compared by equality or
       by wrapping difference, never used as a loop bound. */
    nesturbator__sio_check(io, ppu_ticks <= io->ticks && io->ticks <= STATE_TICKS_MAX &&
                                   io->ticks - ppu_ticks < NESTURBATOR_TICKS_PER_FRAME);
    nesturbator__sio_u16(io, &p->scanline, 261u);
    nesturbator__sio_u16(io, &p->dot, 340u);
    nesturbator__sio_u8(io, &p->odd_frame, 1u);
    nesturbator__sio_u8(io, &p->reset_flag, 1u);
    /* sprite_zero_slots is derived from the saved tags, not serialized. */
}

static void mapper_io(struct nesturbator__sio *io, struct nesturbator *nes)
{
    /* id and submapper are the cartridge's identity, in the header: not repeated here. */
    nesturbator__sio_u8(io, &nes->mapper.irq, 1u);
    /* The board registers, in struct field order; the page tables are rebuilt after APPLY. */
    nes->map.ops.state(&nes->mapper, io);
}

/* The body, in the order the format fixes. */
static void body_io(struct nesturbator__sio *io, struct nesturbator *nes)
{
    core_io(io, nes);
    cpu_io(io, &nes->cpu);
    bus_io(io, &nes->bus);
    apu_io(io, &nes->apu);
    synth_io(io, &nes->synth);
    ppu_io(io, &nes->ppu);
    mapper_io(io, nes);
}

/* ---- Header bytes ---- */

static void put_le(uint8_t *p, uint64_t v, unsigned width)
{
    for (unsigned i = 0u; i < width; ++i)
        p[i] = (uint8_t)(v >> (8u * i));
}

static uint64_t get_le(const uint8_t *p, unsigned width)
{
    uint64_t v = 0u;
    for (unsigned i = 0u; i < width; ++i)
        v |= (uint64_t)p[i] << (8u * i);
    return v;
}

/* CRC-32 over every byte except the CRC field itself. */
static uint32_t state_crc(const uint8_t *b, size_t size)
{
    uint32_t crc = nesturbator__crc32(0u, b, H_STATE_CRC);
    return nesturbator__crc32(crc, b + H_STATE_CRC + 4u, size - (H_STATE_CRC + 4u));
}

size_t nesturbator_state_size(const nesturbator *inst)
{
    struct nesturbator__sio io;
    if (inst == NULL || inst->cart.bytes == NULL)
        return 0u;
    memset(&io, 0, sizeof io);
    io.mode = NESTURBATOR__SIO_SIZE;
    /* SIZE and WRITE only read through the field pointers, and the instance is not a const
       object, so this cast is defined (Pitfall P15). */
    body_io(&io, (struct nesturbator *)inst);
    return STATE_HEADER_SIZE + io.n + inst->cart.prg_ram_size +
           (inst->cart.chr_is_ram != 0u ? STATE_CHR_RAM_SIZE : 0u);
}

size_t nesturbator__state_bounds(const struct nesturbator *inst,
                                 struct nesturbator__state_bound *out, size_t cap)
{
    struct nesturbator__sio io;
    if (inst == NULL || inst->cart.bytes == NULL)
        return 0u;
    memset(&io, 0, sizeof io);
    io.mode = NESTURBATOR__SIO_RECORD;
    io.rec = out;
    io.rec_cap = cap;
    /* RECORD only reads through the field pointers, as SIZE does. */
    body_io(&io, (struct nesturbator *)inst);
    return io.rec_n;
}

nesturbator_status nesturbator_save_state(const nesturbator *inst, void *buf, size_t capacity)
{
    struct nesturbator__sio io;
    uint8_t *b = (uint8_t *)buf;
    size_t size;
    if (inst == NULL || buf == NULL)
        return NESTURBATOR_ERR_ARGUMENT;
    if (inst->cart.bytes == NULL)
        return NESTURBATOR_ERR_STATE_CARTRIDGE;
    size = nesturbator_state_size(inst);
    if (capacity < size)
        return NESTURBATOR_ERR_BUFFER_TOO_SMALL;
    /* Every check has passed: from here the buffer is written. */
    memset(b, 0, STATE_HEADER_SIZE);
    memcpy(b + H_MAGIC, state_magic, sizeof state_magic);
    put_le(b + H_VERSION, NESTURBATOR_STATE_FORMAT_VERSION, 2u);
    put_le(b + H_HEADER_SIZE, STATE_HEADER_SIZE, 2u);
    put_le(b + H_TOTAL_SIZE, size, 4u);
    put_le(b + H_BEHAVIOUR, NESTURBATOR_BEHAVIOUR_REVISION, 4u);
    put_le(b + H_MAPPER, inst->mapper.id, 2u);
    b[H_SUBMAPPER] = inst->mapper.submapper;
    b[H_REGION] = 0u; /* NTSC: the instance has no other region */
    put_le(b + H_ROM_CRC, inst->cart.rom_crc32, 4u);
    put_le(b + H_FRAME, inst->frame_number, 8u);
    put_le(b + H_TICKS, inst->ticks, 8u);
    memset(&io, 0, sizeof io);
    io.mode = NESTURBATOR__SIO_WRITE;
    io.w = b + STATE_HEADER_SIZE;
    body_io(&io, (struct nesturbator *)inst); /* WRITE only reads the fields, as in state_size */
    if (inst->cart.prg_ram_size != 0u) {
        memcpy(io.w, inst->cart.prg_ram, inst->cart.prg_ram_size);
        io.w += inst->cart.prg_ram_size;
    }
    if (inst->cart.chr_is_ram != 0u)
        memcpy(io.w, inst->cart.chr, STATE_CHR_RAM_SIZE);
    put_le(b + H_STATE_CRC, state_crc(b, size), 4u);
    return NESTURBATOR_OK;
}

nesturbator_status nesturbator_load_state(nesturbator *inst, const void *buf, size_t size)
{
    struct nesturbator__sio io;
    const uint8_t *b = (const uint8_t *)buf;
    const uint8_t *ram;
    int save_changed = 0;
    if (inst == NULL || buf == NULL)
        return NESTURBATOR_ERR_ARGUMENT;
    if (size < STATE_HEADER_SIZE)
        return NESTURBATOR_ERR_STATE_SIZE;
    /* The format check covers the magic and the version only; a bad header size is found after
       the checksum, with the other fields (D-03, D-04). */
    if (memcmp(b + H_MAGIC, state_magic, sizeof state_magic) != 0 ||
        get_le(b + H_VERSION, 2u) != NESTURBATOR_STATE_FORMAT_VERSION)
        return NESTURBATOR_ERR_STATE_FORMAT;
    /* Identity before length: the length depends on the cartridge, so another game's state
       reports the wrong game and not a wrong length. */
    if (inst->cart.bytes == NULL || get_le(b + H_MAPPER, 2u) != inst->mapper.id ||
        b[H_SUBMAPPER] != inst->mapper.submapper || b[H_REGION] != 0u ||
        get_le(b + H_ROM_CRC, 4u) != inst->cart.rom_crc32)
        return NESTURBATOR_ERR_STATE_CARTRIDGE;
    if (size != nesturbator_state_size(inst) || get_le(b + H_TOTAL_SIZE, 4u) != size)
        return NESTURBATOR_ERR_STATE_SIZE;
    if (get_le(b + H_STATE_CRC, 4u) != state_crc(b, size))
        return NESTURBATOR_ERR_STATE_CHECKSUM;
    if (get_le(b + H_HEADER_SIZE, 2u) != STATE_HEADER_SIZE)
        return NESTURBATOR_ERR_STATE_INVALID;
    for (size_t i = H_RESERVED; i < STATE_HEADER_SIZE; ++i) {
        if (b[i] != 0u)
            return NESTURBATOR_ERR_STATE_INVALID;
    }
    /* VALIDATE walks the same field lists over the same bytes that APPLY reads, and changes
       nothing in the instance (the primitives pass the field's value along but ignore it), so a
       refused load has written nothing. */
    memset(&io, 0, sizeof io);
    io.mode = NESTURBATOR__SIO_VALIDATE;
    io.r = b + STATE_HEADER_SIZE;
    io.ticks = get_le(b + H_TICKS, 8u);
    body_io(&io, inst);
    if (io.bad != 0)
        return NESTURBATOR_ERR_STATE_INVALID;

    /* APPLY cannot fail. */
    ram = io.r;
    /* STATE-05: the battery span inside the state's PRG RAM block is compared before the copy, so
       a load that changes the battery bytes counts once and every other load counts nothing. The
       span sits at the same offset in the instance's allocation and in the state. */
    if (inst->cart.save_size != 0u)
        save_changed = memcmp(inst->cart.save, ram + (size_t)(inst->cart.save - inst->cart.prg_ram),
                              inst->cart.save_size) != 0;
    inst->frame_number = get_le(b + H_FRAME, 8u);
    inst->ticks = io.ticks;
    io.mode = NESTURBATOR__SIO_APPLY;
    io.r = b + STATE_HEADER_SIZE;
    body_io(&io, inst);
    inst->ppu.sprite_zero_slots = 0u;
    for (unsigned slot = 0u; slot < 8u; slot++)
        if (inst->ppu.sprite_zero[slot] != 0u)
            inst->ppu.sprite_zero_slots |= (uint8_t)(1u << slot);
    /* A restored mixed level must be checked on the next CPU clock, even
       when no oscillator or frame-counter event occurs on that clock. */
    inst->apu.mixed_level_valid = 0u;
    /* The PCM ring is empty at a frame boundary, not in the state. */
    inst->synth.pcm_read = 0u;
    inst->synth.pcm_write = 0u;
    inst->synth.pcm_count = 0u;
    inst->synth.overflow = 0u;
    /* The RAM is copied into the existing allocations, so the pointers a host holds stay valid. */
    if (inst->cart.prg_ram_size != 0u) {
        memcpy(inst->cart.prg_ram, ram, inst->cart.prg_ram_size);
        ram += inst->cart.prg_ram_size;
    }
    if (inst->cart.chr_is_ram != 0u)
        memcpy(inst->cart.chr, ram, STATE_CHR_RAM_SIZE);
    /* Rebuild the pages from the restored registers. The identity check above matched the loaded
       board, so a board exists and this cannot return 0. */
    (void)nesturbator__mapper_load(inst);
    /* map.a12 is the edge filter in front of the board's A12 hook; mapper_load zeroed it. It is
       derived from the PPU address bus, not saved (Pitfall P1). */
    inst->map.a12 = (uint8_t)((inst->ppu.bus_addr >> 12) & 1u);
    if (save_changed != 0)
        inst->save_generation++;
    return NESTURBATOR_OK;
}
