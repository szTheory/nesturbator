/* Shared helpers for the save-state tests (Phase 12): instance creation, ROM file read, a frame
   with video and audio digests, a whole-instance snapshot, little-endian readers, a CRC fixer and
   a counting allocator. Header-only, every function static; each test program includes it once. */
#ifndef NESTURBATOR_TESTS_STATE_TEST_H
#define NESTURBATOR_TESTS_STATE_TEST_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "check.h"
#include "ines.h"

/* ---- Helpers ---- */

static inline nesturbator *st_make_instance(void)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    return inst;
}

/* Reads a whole file; returns 1 on success. The caller frees *data. */
static inline int st_read_file(const char *path, uint8_t **data, size_t *size)
{
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    long len;
    if (f == NULL)
        return 0;
    if (fseek(f, 0, SEEK_END) != 0 || (len = ftell(f)) <= 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return 0;
    }
    buf = (uint8_t *)malloc((size_t)len);
    if (buf == NULL || fread(buf, 1u, (size_t)len, f) != (size_t)len) {
        free(buf);
        fclose(f);
        return 0;
    }
    fclose(f);
    *data = buf;
    *size = (size_t)len;
    return 1;
}

static inline void st_load(nesturbator *inst, const uint8_t *rom, size_t size)
{
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, rom, size), NESTURBATOR_OK);
}

/* One frame with a 256-pitch video buffer and a 1024-sample audio buffer. The digests are the
   CRC-32 of the video pixels and of the audio samples, each serialised little-endian. */
static inline nesturbator_status st_frame(nesturbator *inst, uint16_t buttons, uint32_t *video_crc,
                                          uint32_t *audio_crc, uint64_t *ticks,
                                          uint64_t *frame_number)
{
    static uint16_t video[256u * 240u];
    static int16_t audio[1024];
    nesturbator_input in;
    nesturbator_frame io;
    nesturbator_status st;
    uint8_t bytes[2];
    uint32_t crc;
    memset(&in, 0, sizeof in);
    in.size = (uint32_t)sizeof in;
    in.buttons[0] = (uint8_t)(buttons & 0xffu);
    in.buttons[1] = (uint8_t)(buttons >> 8);
    CHECK_EQ_U64(nesturbator_set_input(inst, &in), NESTURBATOR_OK);
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = video;
    io.video_pitch = 256u;
    io.audio = audio;
    io.audio_capacity = 1024u;
    st = nesturbator_run_frame(inst, &io);
    if (st != NESTURBATOR_OK)
        return st;
    crc = 0u;
    for (size_t i = 0u; i < 256u * 240u; ++i) {
        bytes[0] = (uint8_t)(video[i] & 0xffu);
        bytes[1] = (uint8_t)(video[i] >> 8);
        crc = nesturbator__crc32(crc, bytes, 2u);
    }
    if (video_crc != NULL)
        *video_crc = crc;
    crc = 0u;
    for (uint32_t i = 0u; i < io.audio_count; ++i) {
        bytes[0] = (uint8_t)((uint16_t)audio[i] & 0xffu);
        bytes[1] = (uint8_t)((uint16_t)audio[i] >> 8);
        crc = nesturbator__crc32(crc, bytes, 2u);
    }
    if (audio_crc != NULL)
        *audio_crc = crc;
    if (ticks != NULL)
        *ticks = io.ticks;
    if (frame_number != NULL)
        *frame_number = io.frame_number;
    return st;
}

/* A copy of the whole instance struct plus the cartridge RAM it points to, for "unchanged". */
struct st_snap {
    struct nesturbator nes;
    uint8_t *prg_ram;
    size_t prg_ram_size;
    uint8_t *chr_ram;
    size_t chr_ram_size;
};

static inline void st_snapshot(const nesturbator *inst, struct st_snap *s)
{
    memcpy(&s->nes, inst, sizeof s->nes);
    s->prg_ram_size = inst->cart.prg_ram_size;
    s->prg_ram = NULL;
    if (s->prg_ram_size != 0u) {
        s->prg_ram = (uint8_t *)malloc(s->prg_ram_size);
        memcpy(s->prg_ram, inst->cart.prg_ram, s->prg_ram_size);
    }
    s->chr_ram_size = inst->cart.chr_is_ram != 0u ? inst->cart.chr_size : 0u;
    s->chr_ram = NULL;
    if (s->chr_ram_size != 0u) {
        s->chr_ram = (uint8_t *)malloc(s->chr_ram_size);
        memcpy(s->chr_ram, inst->cart.chr, s->chr_ram_size);
    }
}

/* 1 when the instance is byte for byte what the snapshot recorded. */
static inline int st_same(const nesturbator *inst, const struct st_snap *s)
{
    if (memcmp(&s->nes, inst, sizeof s->nes) != 0)
        return 0;
    if (s->prg_ram_size != 0u && memcmp(s->prg_ram, inst->cart.prg_ram, s->prg_ram_size) != 0)
        return 0;
    if (s->chr_ram_size != 0u && memcmp(s->chr_ram, inst->cart.chr, s->chr_ram_size) != 0)
        return 0;
    return 1;
}

static inline void st_snap_free(struct st_snap *s)
{
    free(s->prg_ram);
    free(s->chr_ram);
    s->prg_ram = NULL;
    s->chr_ram = NULL;
}

/* A refused load against a snapshot taken earlier (the instance must not have changed since):
   the status must be want, the instance byte-identical and nesturbator_save_generation unchanged
   (STATE-04, all-or-nothing). what names the case in the failure output. Returns 1 on success. */
static inline int st_refused_with(nesturbator *inst, const struct st_snap *snap, uint64_t gen,
                                  const uint8_t *buf, size_t size, nesturbator_status want,
                                  const char *what)
{
    nesturbator_status got = nesturbator_load_state(inst, buf, size);
    int ok = 1;
    if (got != want) {
        fprintf(stderr, "refused %s: status %d, wanted %d\n", what, (int)got, (int)want);
        ok = 0;
    }
    if (!st_same(inst, snap)) {
        fprintf(stderr, "refused %s: the instance changed\n", what);
        ok = 0;
    }
    if (nesturbator_save_generation(inst) != gen) {
        fprintf(stderr, "refused %s: save_generation changed\n", what);
        ok = 0;
    }
    if (!ok)
        check_failures++;
    return ok;
}

/* Snapshots the instance and its save generation, loads, and checks the refusal. */
static inline int st_refused(nesturbator *inst, const uint8_t *buf, size_t size,
                             nesturbator_status want)
{
    struct st_snap snap;
    int ok;
    st_snapshot(inst, &snap);
    ok = st_refused_with(inst, &snap, nesturbator_save_generation(inst), buf, size, want,
                         "st_refused");
    st_snap_free(&snap);
    return ok;
}

/* The button pattern both runs use for frame n. */
static inline uint16_t st_buttons(uint32_t n)
{
    return (uint16_t)((n * 37u + (n >> 3)) & 0xffu);
}

/* Little-endian readers for the documented header, independent of the library's own code. */
static inline uint32_t le16(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8);
}

static inline uint32_t le32(const uint8_t *p)
{
    return le16(p) | (le16(p + 2) << 16);
}

static inline uint64_t le64(const uint8_t *p)
{
    return (uint64_t)le32(p) | ((uint64_t)le32(p + 4) << 32);
}

static inline void put32(uint8_t *p, uint32_t v)
{
    for (unsigned i = 0u; i < 4u; ++i)
        p[i] = (uint8_t)(v >> (8u * i));
}

/* Recomputes the state CRC-32 (every byte but offsets 28 to 31) and stores it at 28, so a test
   can change a field and still pass the checksum step. */
static inline void st_recrc(uint8_t *state, size_t size)
{
    uint32_t crc = nesturbator__crc32(0u, state, 28u);
    crc = nesturbator__crc32(crc, state + 32, size - 32u);
    put32(state + 28, crc);
}

/* An allocator that counts calls and live bytes, for "never allocates" checks. */
struct st_counting_allocator {
    unsigned allocs;
    unsigned frees;
};

static inline void *st_counting_alloc(void *user, size_t size)
{
    struct st_counting_allocator *c = (struct st_counting_allocator *)user;
    c->allocs++;
    return malloc(size);
}

static inline void st_counting_free(void *user, void *ptr, size_t size)
{
    struct st_counting_allocator *c = (struct st_counting_allocator *)user;
    (void)size;
    c->frees++;
    free(ptr);
}

/* An instance whose allocator is the counting one; the counts include create. */
static inline nesturbator *st_make_instance_counting(struct st_counting_allocator *c)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    cfg.allocator.alloc = st_counting_alloc;
    cfg.allocator.free = st_counting_free;
    cfg.allocator.user = c;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    return inst;
}

/* Fills every field the state visitor saves with 0xA5, so a field the visitor misses keeps the
   pattern after a load and shows up as a divergence. Kept: the identity fields mapper.id and
   mapper.submapper, the cartridge pointers and sizes, the map page tables, ops and watch mask,
   the allocator, and the host or frame-scoped fields (input_pending, the sample and video output
   pointers). map.a12 is derived and poisoned so the APPLY rebuild is exercised. */
static inline void st_poison(nesturbator *inst)
{
    uint8_t pending[2];
    memcpy(pending, inst->bus.input_pending, sizeof pending);
    memset(&inst->cpu, 0xA5, sizeof inst->cpu);
    memset(&inst->bus, 0xA5, sizeof inst->bus);
    memcpy(inst->bus.input_pending, pending, sizeof pending);
    memset(&inst->apu, 0xA5, sizeof inst->apu);
    inst->apu.sample_output = NULL;
    inst->apu.sample_limit = 0u;
    inst->apu.sample_count = 0u;
    memset(&inst->synth, 0xA5, sizeof inst->synth);
    inst->synth.pcm_read = 0u;
    inst->synth.pcm_write = 0u;
    inst->synth.pcm_count = 0u;
    inst->synth.overflow = 0u;
    memset(&inst->ppu, 0xA5, sizeof inst->ppu);
    inst->ppu.video_output = NULL;
    inst->ppu.video_pitch = 0u;
    memset(&inst->mapper.irq, 0xA5, sizeof inst->mapper.irq);
    memset(&inst->mapper.reg, 0xA5, sizeof inst->mapper.reg);
    inst->map.a12 = 0xA5u;
    if (inst->cart.prg_ram_size != 0u)
        memset(inst->cart.prg_ram, 0xA5, inst->cart.prg_ram_size);
    if (inst->cart.chr_is_ram != 0u)
        memset(inst->cart.chr, 0xA5, inst->cart.chr_size);
}

/* ---- Stress image (STATE-03, Pitfall P16) ----
   A synthetic program that keeps OAM DMA, DMC DMA, the APU frame IRQ, the NMI edge and (mapper 4)
   the MMC3 scanline IRQ in flight across frame boundaries. Mapper 0 puts the code at $8000 in a
   32 KiB NROM; mapper 4 puts it in the last 8 KiB, which MMC3 fixes at $E000. The DMC sample is
   read from $C000. Every byte is authored here. Returns the image size, or 0 if cap is too small.
   Register facts: NESdev Wiki "APU DMC" ($4010-$4013: loop, rate, address, length), "APU"
   ($4015 enables channels and reading it acknowledges the frame IRQ; writing it with the DMC bit
   clear then set restarts the sample and requests a DMA), "APU Frame Counter" ($4017 = 0: 4-step
   mode, frame IRQ enabled), "PPU registers" ($2000 bit 7 enables NMI, $2001 enables rendering),
   "PPU OAM" ($4014 copies a CPU page to OAM and stalls the CPU), "MMC3" ($C000 latch, $C001
   reload, $E000 disable and acknowledge, $E001 enable the scanline IRQ).

   A frame ends at the first instruction end at or after a fixed tick count, so the end drifts
   against the PPU frame by the overshoot of each frame. The NMI handler waits a counted delay
   (about 2400 CPU cycles, where that drift starts) and then does its DMC restart and its $4014
   write at two instants a few cycles apart; the DMC sample ends with its IRQ enabled and is
   restarted by that same handler, so the IRQ line is high for most of each frame. As the frame end
   drifts forward it passes each instant in turn, so on one frame it follows the restart and on
   another the $4014 write: those are the frames where DMC DMA or OAM DMA is still pending at the
   boundary. The delay counts below were chosen by measuring both boards (test_state_fork reports
   what it saw). */
#ifndef ST_STRESS_Y
#define ST_STRESS_Y 1
#endif
#ifndef ST_STRESS_X
#define ST_STRESS_X 220
#endif
#ifndef ST_STRESS_N
#define ST_STRESS_N 0
#endif
#ifndef ST_STRESS_G
#define ST_STRESS_G 5
#endif
#define ST_E(buf, idx, ...)                                                                        \
    do {                                                                                           \
        static const uint8_t b_[] = {__VA_ARGS__};                                                 \
        memcpy((buf) + (idx), b_, sizeof b_);                                                      \
        (idx) += sizeof b_;                                                                        \
    } while (0)
static inline size_t st_stress_image(uint8_t *out, size_t cap, uint16_t mapper)
{
    uint8_t main_code[160], nmi[96], irq[64];
    size_t m = 0u, n = 0u, q = 0u;
    uint16_t base = mapper == 4u ? 0xE000u : 0x8000u;
    uint32_t off = mapper == 4u ? 0x6000u : 0x0000u;
    uint16_t nmi_addr = (uint16_t)(base + 0x100u), irq_addr = (uint16_t)(base + 0x180u);
    uint16_t loop_addr;
    struct ines_segment seg[3];
    struct ines_spec spec;
    ST_E(main_code, m, 0x78, 0xD8, 0xA2, 0xFF, 0x9A); /* SEI CLD LDX #$FF TXS */
    ST_E(main_code, m, 0xA9, 0x8F, 0x8D, 0x10, 0x40); /* $4010: IRQ at the end, fastest rate */
    ST_E(main_code, m, 0xA9, 0x00, 0x8D, 0x12, 0x40); /* $4012: sample at $C000 */
    ST_E(main_code, m, 0xA9, 0x01, 0x8D, 0x13, 0x40); /* $4013: 17 bytes */
    ST_E(main_code, m, 0xA9, 0x1F, 0x8D, 0x15, 0x40); /* $4015: all channels, DMC on */
    ST_E(main_code, m, 0xA9, 0x00, 0x8D, 0x17, 0x40); /* $4017: frame IRQ enabled */
    if (mapper == 4u) {
        ST_E(main_code, m, 0xA9, 0x07, 0x8D, 0x00, 0xC0);       /* $C000: latch 7 */
        ST_E(main_code, m, 0x8D, 0x01, 0xC0, 0x8D, 0x01, 0xE0); /* $C001 reload, $E001 enable */
    }
    ST_E(main_code, m, 0xA9, 0x1E, 0x8D, 0x01, 0x20); /* $2001: show background, sprites */
    ST_E(main_code, m, 0xA9, 0x80, 0x8D, 0x00, 0x20); /* $2000: NMI on */
    ST_E(main_code, m, 0x58);                         /* CLI */
    loop_addr = (uint16_t)(base + m);
    ST_E(main_code, m, 0xE6, 0x10, 0xA5, 0x11, 0xE6, 0x13); /* INC $10, LDA $11, INC $13 */
    main_code[m++] = 0x4C;                                  /* JMP loop */
    main_code[m++] = (uint8_t)(loop_addr & 0xffu);
    main_code[m++] = (uint8_t)(loop_addr >> 8);
    /* NMI: a delay of Y passes of 255 DEX/BNE pairs plus X more pairs, then NOPs, then the DMC
       restart; a gap of G pairs; then the OAM DMA from page 2 after changing a byte of it. */
    ST_E(nmi, n, 0x48, 0xEE, 0x00, 0x02); /* PHA, INC $0200 */
    nmi[n++] = 0xA0;                      /* LDY #Y */
    nmi[n++] = (uint8_t)ST_STRESS_Y;
    ST_E(nmi, n, 0xA2, 0xFF, 0xCA, 0xD0, 0xFD, 0x88, 0xD0, 0xF8); /* LDX #$FF; DEX BNE; DEY BNE */
    nmi[n++] = 0xA2;                                              /* LDX #X */
    nmi[n++] = (uint8_t)ST_STRESS_X;
    ST_E(nmi, n, 0xCA, 0xD0, 0xFD); /* DEX BNE */
    for (unsigned i = ST_STRESS_N; i > 0u; --i)
        ST_E(nmi, n, 0xEA);                     /* NOP */
    ST_E(nmi, n, 0xA9, 0x0F, 0x8D, 0x15, 0x40); /* $4015: DMC off */
    ST_E(nmi, n, 0xA9, 0x1F, 0x8D, 0x15, 0x40); /* $4015: DMC on, restart, DMA */
    nmi[n++] = 0xA2;                            /* LDX #G */
    nmi[n++] = (uint8_t)ST_STRESS_G;
    ST_E(nmi, n, 0xCA, 0xD0, 0xFD);             /* DEX BNE */
    ST_E(nmi, n, 0xA9, 0x02, 0x8D, 0x14, 0x40); /* $4014: OAM DMA from page 2 */
    ST_E(nmi, n, 0x68, 0x40);                   /* PLA, RTI */
    ST_E(irq, q, 0x48);                         /* PHA */
    if (mapper == 4u) {
        ST_E(irq, q, 0x8D, 0x00, 0xE0);                   /* $E000: ack the scanline IRQ */
        ST_E(irq, q, 0xA9, 0x07, 0x8D, 0x00, 0xC0);       /* $C000: latch 7 */
        ST_E(irq, q, 0x8D, 0x01, 0xC0, 0x8D, 0x01, 0xE0); /* $C001 reload, $E001 enable */
    }
    /* The frame IRQ is acknowledged only on every 64th entry, so the line stays asserted across
       many instruction ends and the IRQ poll latch is set at frame boundaries. */
    ST_E(irq, q, 0xE6, 0x12, 0xA5, 0x12, 0x29, 0x3F, 0xD0,
         0x03);                     /* INC $12, LDA, AND #$3F, BNE +3 */
    ST_E(irq, q, 0xAD, 0x15, 0x40); /* LDA $4015: ack frame IRQ */
    ST_E(irq, q, 0x68, 0x40);       /* PLA, RTI */
    seg[0].prg_offset = off;
    seg[0].bytes = main_code;
    seg[0].len = m;
    seg[1].prg_offset = off + 0x100u;
    seg[1].bytes = nmi;
    seg[1].len = n;
    seg[2].prg_offset = off + 0x180u;
    seg[2].bytes = irq;
    seg[2].len = q;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 2u;
    spec.chr_8k = 1u;
    spec.mapper = mapper;
    spec.segments = seg;
    spec.segment_count = 3u;
    spec.nmi_vector = nmi_addr;
    spec.reset_vector = base;
    spec.irq_vector = irq_addr;
    return ines_build(out, cap, &spec);
}
#undef ST_E

#endif /* NESTURBATOR_TESTS_STATE_TEST_H */
