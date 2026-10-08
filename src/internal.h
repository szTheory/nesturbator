/* Declarations shared by the core's source files. Not installed.
   The core holds no mutable static or global variable: all state is here. */
#ifndef NESTURBATOR_INTERNAL_H
#define NESTURBATOR_INTERNAL_H

#include "nesturbator.h"

/* Time unit: one tick is half a master-clock period; NTSC has 24 ticks per
   CPU cycle and 8 per PPU dot (ARCHITECTURE section 2). An average NTSC frame
   is 89341 and a half dots, 714732 ticks, which is exactly the reported frame rate
   39375000/655171 (01-RESEARCH Open Question 1). */
#define NESTURBATOR_TICKS_PER_FRAME 714732u

/* Audio: 352 samples at 48000 Hz per 13125 CPU cycles, that is per 315000
   ticks (D-09, 01-RESEARCH Pattern 2). */
#define NESTURBATOR_AUDIO_SAMPLES_PER_PERIOD 352u
#define NESTURBATOR_AUDIO_TICKS_PER_PERIOD 315000u

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
   The power-up state is not set yet: nesturbator_create zeroes the instance,
   so a new CPU has P, S and PC equal to 0 and no reset vector fetch. Nothing
   runs the CPU during a frame yet; the phase that does adds the reset
   sequence, which sets S, P and PC and clears jammed. */
struct nesturbator__cpu {
    uint16_t pc;
    uint8_t a, x, y, s, p;
    uint8_t nmi_prev, nmi_pending, irq_line, poll_latch;
    uint8_t halted_in_read; /* nonzero while the current read is stalled */
    uint8_t jammed;         /* set by a JAM opcode; nothing clears it yet */
};

/* Bus-side state: 2048 bytes of internal RAM and the open-bus latch, the
   last value driven on the data bus (D-12). */
struct nesturbator__bus {
    uint8_t ram[2048];
    uint8_t open_bus;
    uint8_t oam_dma_pending, oam_dma_page;
    uint8_t input_pending[2], input_buttons[2];
    uint8_t controller_latch[2], controller_shift[2], controller_strobe;
};

struct nesturbator__ppu {
    uint8_t control, mask, status, oam_addr;
    uint8_t address_latch, fine_x, read_buffer;
    uint8_t vblank_suppress;
    uint16_t v, t;
    uint8_t nametable[2048];
    uint8_t palette[32];
    uint8_t oam[256];
    uint8_t secondary_oam[32];
    uint8_t sprite_count, eval_n, eval_latch, eval_target;
    uint8_t sprite_zero[8], sprite_x[8], sprite_attr[8];
    uint8_t sprite_lo[8], sprite_hi[8];
    uint16_t *video_output;
    uint32_t video_pitch;
    uint64_t ppu_ticks;
    uint16_t scanline;
    uint16_t dot;
    uint8_t odd_frame;
};

struct nesturbator__cartridge {
    uint8_t *bytes;
    size_t size;
    uint8_t *prg;
    uint8_t *chr;
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
    struct nesturbator__ppu ppu;
    struct nesturbator__cartridge cart;
    struct nesturbator__profile profile; /* chip-dependent constants */
    uint32_t audio_rem;                  /* sample fraction carried over, in units
                                            of 1/315000 sample per tick */
};

/* One CPU read cycle at addr: advances time by one CPU cycle and returns the
   value on the data bus (src/bus.c in the library). */
uint8_t nesturbator__bus_read(struct nesturbator *nes, uint16_t addr);

/* One CPU write cycle of value to addr (src/bus.c in the library). */
void nesturbator__bus_write(struct nesturbator *nes, uint16_t addr, uint8_t value);
void nesturbator__ppu_run_until(struct nesturbator *nes, uint64_t ticks);
uint8_t nesturbator__ppu_read(struct nesturbator *nes, uint16_t addr);
void nesturbator__ppu_write(struct nesturbator *nes, uint16_t addr, uint8_t value);
uint8_t nesturbator__ppu_register_read(struct nesturbator *nes, uint16_t reg);
void nesturbator__ppu_register_write(struct nesturbator *nes, uint16_t reg, uint8_t value);
uint8_t nesturbator__cart_read(struct nesturbator *nes, uint16_t addr);

/* Runs one whole instruction, opcode fetch to last cycle (D-11). */
void nesturbator__cpu_step(struct nesturbator *nes);

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
