/* Declarations shared by the core's source files. Not installed.
   The core holds no mutable static or global variable: all state is here. */
#ifndef NESTURBATOR_INTERNAL_H
#define NESTURBATOR_INTERNAL_H

#include "nesturbator.h"
#include "mapper.h"

/* Time unit: one tick is half a master-clock period; NTSC has 24 ticks per
   CPU cycle and 8 per PPU dot (ARCHITECTURE section 2). An average NTSC frame
   is 89341 and a half dots, 714732 ticks, which is exactly the reported frame rate
   39375000/655171 (01-RESEARCH Open Question 1). */
#define NESTURBATOR_TICKS_PER_FRAME 714732u

/* Audio: 352 samples at 48000 Hz per 13125 CPU cycles, that is per 315000
   ticks (D-09, 01-RESEARCH Pattern 2). */
#define NESTURBATOR_AUDIO_SAMPLES_PER_PERIOD 352u
#define NESTURBATOR_AUDIO_TICKS_PER_PERIOD 315000u
#define NESTURBATOR_SYNTH_TAPS 16u
#define NESTURBATOR_SYNTH_PHASES 32u
#define NESTURBATOR_SYNTH_RING_CAPACITY 1024u

/* Video output size in pixels. */
#define NESTURBATOR_WIDTH 256u
#define NESTURBATOR_HEIGHT 240u

/* Size of each boundary struct as first released. These equal this build's
   sizeof today; they stay fixed when fields are appended later. */
#define NESTURBATOR_VERSION_SIZE_V1 ((uint32_t)sizeof(nesturbator_version))
#define NESTURBATOR_CONFIG_SIZE_V1 ((uint32_t)sizeof(nesturbator_config))
#define NESTURBATOR_INFO_SIZE_V1 ((uint32_t)sizeof(nesturbator_info))
#define NESTURBATOR_FRAME_SIZE_V1 ((uint32_t)sizeof(nesturbator_frame))
#define NESTURBATOR_INPUT_SIZE_V1 ((uint32_t)sizeof(nesturbator_input))

/* 6502 core state (D-10). P is kept exactly as loaded or pulled: flag
   writes touch only their own bits, so bits 4 and 5 change only on a pull.
   The interrupt fields and halted_in_read are unused until the bus samples
   the lines and stalls reads.
   The power-on state comes from cartridge load, which sets S, P and PC from
   the reset vector. The CPU runs during every frame. nesturbator__cpu_reset
   runs the soft-reset sequence (I set, S lowered by 3, PC from the vector)
   and clears jammed. */
struct nesturbator__cpu {
    uint16_t pc;
    uint8_t a, x, y, s, p;
    uint8_t nmi_prev, nmi_pending, irq_line, poll_latch;
    uint8_t halted_in_read; /* nonzero while the current read is stalled */
    uint8_t jammed;         /* set by a JAM opcode; cleared by cpu_reset */
};

/* Bus-side state: 2048 bytes of internal RAM and the open-bus latch, the
   last value driven on the data bus (D-12). */
struct nesturbator__bus {
    uint8_t ram[2048];
    uint8_t open_bus;
    uint8_t apu_get_put_phase; /* 1: GET, 0: PUT at the current CPU bus cycle. */
    uint8_t oam_dma_pending, oam_dma_page;
    uint8_t input_pending[2], input_buttons[2];
    uint8_t controller_latch[2], controller_shift[2], controller_strobe;
};

struct nesturbator__pulse {
    uint16_t timer;
    uint16_t counter;
    uint8_t reg[4];
    uint8_t phase, length, env_divider, env_decay, env_start, sweep_divider, sweep_reload;
};

struct nesturbator__triangle {
    uint16_t timer, counter;
    uint8_t reg[4];
    uint8_t phase, length, linear, linear_reload, linear_reload_flag;
};

struct nesturbator__noise {
    uint16_t lfsr, counter;
    uint8_t reg[4];
    uint8_t length, env_divider, env_decay, env_start;
};

struct nesturbator__dmc {
    uint16_t timer, counter, address, remaining;
    uint8_t reg[4], output, shift, bits, sample_buffer, buffer_empty;
    uint8_t irq, dma_pending, dma_halt_phase, dma_load_waiting, enable_delay;
};

struct nesturbator__apu {
    struct nesturbator__pulse pulse[2];
    struct nesturbator__triangle triangle;
    struct nesturbator__noise noise;
    struct nesturbator__dmc dmc;
    uint8_t enabled;
    uint8_t pulse_clock_phase;
    uint8_t frame_mode, frame_pending_mode, frame_irq_inhibit, frame_irq;
    uint8_t frame_irq_clear_pending;
    uint8_t frame_reset_delay;
    uint32_t frame_cycle;
    uint32_t sample_phase;
    int16_t *sample_output;
    uint32_t sample_limit;
    uint32_t sample_count;
};

typedef void (*nesturbator__transition_sink)(void *context, uint64_t cpu_cycle, int32_t level);

/* Per-instance band-limited synthesis, filters, and caller-frame staging. The
   impulse cursor advances only when the 48 kHz sample clock emits a sample. */
struct nesturbator__synth {
    int32_t mixed_level;
    int64_t impulse[NESTURBATOR_SYNTH_TAPS];
    uint8_t impulse_head;
    int64_t integrator_q15;
    int64_t hp_input_q30[2];
    int64_t hp_output_q30[2];
    int64_t lp_output_q30;
    int16_t pcm[NESTURBATOR_SYNTH_RING_CAPACITY];
    uint16_t pcm_read, pcm_write, pcm_count;
    uint8_t overflow;
};

struct nesturbator__ppu {
    uint8_t control, mask, status, oam_addr;
    uint8_t address_latch, fine_x, read_buffer, io_bus;
    uint32_t io_bus_age;
    uint8_t vblank_suppress;
    uint16_t v, t;
    uint8_t nametable[2048];
    uint8_t palette[32];
    uint8_t oam[256];
    uint8_t secondary_oam[32];
    uint8_t sprite_count, eval_count, eval_n, eval_m, eval_remaining;
    uint8_t eval_copy_remaining;
    uint8_t eval_latch, eval_target;
    uint8_t eval_sprite_zero[8];
    uint8_t sprite_zero[8], sprite_x[8], sprite_attr[8];
    uint8_t sprite_lo[8], sprite_hi[8];
    uint16_t *video_output;
    uint32_t video_pitch;
    uint64_t ppu_ticks;
    uint16_t scanline;
    uint16_t dot;
    uint8_t odd_frame;
    /* Set by a soft reset, cleared at scanline 261 dot 1: while set, writes to
       $2000, $2001, $2005 and $2006 are dropped. Load and unload zero the PPU,
       so power-on has no such window (D-17). */
    uint8_t reset_flag;
};

struct nesturbator__cartridge {
    uint8_t *bytes;
    size_t size;
    uint8_t *prg;
    size_t prg_size;
    uint8_t *chr;
    uint8_t *prg_ram;
    size_t chr_size; /* CHR size the loader validated (8192) */
    uint8_t chr_is_ram;
};

/* Machine profile: values that differ between chips of the same model
   (D-14). ANE (0x8B) computes A = (A | ane_magic) & X & imm and LXA (0xAB)
   computes A = X = (A | lxa_magic) & imm. */
struct nesturbator__profile {
    uint8_t ane_magic;
    uint8_t lxa_magic;
};

/* NTSC RP2A03G: both constants are 0xEE. The 65x02 nes6502 vectors fit 0xEE
   in all 10000 tests of each opcode, and the NESdev Wiki "CPU unofficial
   opcodes" (revision 23975) describes the constant as chip dependent. LXA's
   value is settled against a console test ROM in Phase 3 (deferred). */
#define NESTURBATOR_RP2A03G_ANE_MAGIC 0xEEu
#define NESTURBATOR_RP2A03G_LXA_MAGIC 0xEEu

struct nesturbator {
    nesturbator_allocator allocator; /* copy of the config's, defaults filled in */
    uint64_t frame_number;           /* frames run since create */
    uint64_t ticks;                  /* ticks run since create */
    struct nesturbator__cpu cpu;     /* the 6502 */
    struct nesturbator__bus bus;     /* RAM and the open-bus latch */
    struct nesturbator__apu apu;
    struct nesturbator__synth synth;
    nesturbator__transition_sink transition_sink;
    void *transition_sink_context;
    struct nesturbator__ppu ppu;
    struct nesturbator__cartridge cart;
    struct nesturbator__mapper mapper; /* board registers: kept by a soft reset */
    struct nesturbator__map map;       /* derived pages, rebuilt by mapper_load */
    /* CPU bus cycles since load: kept by nesturbator_reset, zeroed by load and
       unload. Unlike ticks / 24 it is correct on every region (D-04). */
    uint64_t cpu_cycle;
    struct nesturbator__profile profile; /* chip-dependent constants */
    uint32_t audio_rem;                  /* sample fraction carried over, in units
                                            of 1/315000 sample per tick */
};

/* A 1 KiB PRG page by bank number. A true modulo against the loader-validated
   size, because NES 2.0 sizes need not be powers of two, so a register value
   cannot point outside the allocation (D-03). */
static inline const uint8_t *nesturbator__map_prg(const struct nesturbator *nes, uint32_t bank_1k)
{
    return nes->cart.prg + (size_t)(bank_1k % (uint32_t)(nes->cart.prg_size / 1024u)) * 1024u;
}

/* A 1 KiB CHR page by bank number; the same modulo rule. */
static inline uint8_t *nesturbator__map_chr(const struct nesturbator *nes, uint32_t bank_1k)
{
    return nes->cart.chr + (size_t)(bank_1k % (uint32_t)(nes->cart.chr_size / 1024u)) * 1024u;
}

/* A CPU read of $4020-$FFFF through the page table. A NULL page is open bus (D-03). */
static inline uint8_t nesturbator__map_cpu_read(struct nesturbator *nes, uint16_t addr)
{
    const uint8_t *page = nes->map.cpu_r[(addr - 0x4000u) >> 10];
    return page != NULL ? page[addr & 0x3ffu] : nes->bus.open_bus;
}

/* One CPU read cycle at addr: advances time by one CPU cycle and returns the
   value on the data bus (src/bus.c in the library). */
uint8_t nesturbator__bus_read(struct nesturbator *nes, uint16_t addr);

/* One CPU write cycle of value to addr (src/bus.c in the library). */
void nesturbator__bus_write(struct nesturbator *nes, uint16_t addr, uint8_t value);
void nesturbator__apu_clock(struct nesturbator *nes);

/* The one writer of cpu.irq_line: the frame IRQ (unless inhibited), the DMC IRQ
   and the board's IRQ ORed. Every source calls it when it changes (NESdev "IRQ"). */
void nesturbator__irq_update(struct nesturbator *nes);
void nesturbator__apu_write(struct nesturbator *nes, uint16_t addr, uint8_t value);
void nesturbator__apu_set_transition_sink(struct nesturbator *nes,
                                          nesturbator__transition_sink sink, void *context);
uint8_t nesturbator__apu_channel_level(const struct nesturbator *nes, unsigned channel);
uint8_t nesturbator__apu_status_read(struct nesturbator *nes);
uint16_t nesturbator__apu_mixed_level(const struct nesturbator *nes);
void nesturbator__apu_begin_frame(struct nesturbator *nes, int16_t *samples, uint32_t count);
void nesturbator__apu_end_frame(struct nesturbator *nes);
void nesturbator__synth_reset(struct nesturbator *nes);
void nesturbator__synth_transition(struct nesturbator *nes, int32_t level);
void nesturbator__synth_sample(struct nesturbator *nes);
uint32_t nesturbator__synth_drain(struct nesturbator *nes, int16_t *out, uint32_t count);
extern const int16_t nesturbator__synth_kernel[NESTURBATOR_SYNTH_PHASES][NESTURBATOR_SYNTH_TAPS];
extern const uint16_t nesturbator__pulse_mix[31];
extern const uint16_t nesturbator__tnd_mix[16][16][128];
void nesturbator__ppu_run_until(struct nesturbator *nes, uint64_t ticks);
uint8_t nesturbator__ppu_read(struct nesturbator *nes, uint16_t addr);
void nesturbator__ppu_write(struct nesturbator *nes, uint16_t addr, uint8_t value);
uint8_t nesturbator__ppu_register_read(struct nesturbator *nes, uint16_t reg);
void nesturbator__ppu_register_write(struct nesturbator *nes, uint16_t reg, uint8_t value);

/* Runs one whole instruction, opcode fetch to last cycle (D-11). */
void nesturbator__cpu_step(struct nesturbator *nes);

/* Soft reset of the CPU: I set, seven bus cycles, S lowered by 3, PC from
   $FFFC/$FFFD, jammed and the poll latch cleared (src/cpu.c). */
void nesturbator__cpu_reset(struct nesturbator *nes);

/* Soft reset of the PPU (src/ppu.c): top of the picture, registers and latches
   cleared, write-ignore flag set. Run nesturbator__ppu_run_until first. */
void nesturbator__ppu_reset(struct nesturbator *nes);

/* Soft reset of the APU (src/apu.c): silenced, IRQs and DMC DMA latches cleared,
   the last $4017 write re-applied. */
void nesturbator__apu_reset(struct nesturbator *nes);

/* Validates an input struct's size tag before any other field is read.
   's' points at the struct; 'first' is its first released size; 'ours' is
   sizeof in this build. Returns NESTURBATOR_OK or
   NESTURBATOR_ERR_STRUCT_SIZE. */
nesturbator_status nesturbator__check_size_in(const void *s, uint32_t first, uint32_t ours);
void nesturbator__controller_begin_frame(struct nesturbator *nes);

/* The no-cartridge test card: native pixel at column x, row y (D-01, D-02). */
uint16_t nesturbator__test_pixel(uint32_t x, uint32_t y);

/* Native pixel to XRGB8888 0x00RRGGBB, generated by tools/palgen from NTSC
   signal facts (D-12, D-13). */
extern const uint32_t nesturbator__palette_ntsc[512];

#endif /* NESTURBATOR_INTERNAL_H */
