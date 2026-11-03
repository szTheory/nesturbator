/* STATE-04 (D-03, D-04, D-16): a hostile or damaged state is refused with the right status and
   changes nothing. For each ROM a state is made after 20 frames, then:
     1. cross-ROM matrix and no-cartridge: ERR_STATE_CARTRIDGE
     2. all older format versions and the next version: FORMAT; header_size 63 and 65:
   CHECKSUM, then INVALID with the
   CRC recomputed
     3. truncation to every length below S, S + 1 bytes, NULL arguments, S itself loads
     4. flip every header byte and body byte: the D-04 class per offset (RAM bytes with a stride
        under --short)
     5. two faults at once report the first failing D-04 step
     6. field sweep with the CRC recomputed: every bounded field just past each bound and at the
        type's limits gives INVALID; reserved bytes 48-63 and status bits 0-4 as well
     7. at-bound table: every bounded field at its bound loads (or is refused for the named
        cross-field reason) and the next frame returns the named status
     8. the ppu_ticks catch-up bound on both sides, and the 2^62 ceiling on ticks
   Every refusal is checked against a snapshot of the whole instance and save_generation.
   The fields come from nesturbator__state_bounds, which walks the same field list the load uses,
   so this file keeps no second copy of the layout; only the names of the rows are written here.
   Usage: core.state_reject [--short] <rom>...  (dabg.nes, M1_P512K_CR8K_S32K.nes and
   M4_P128K_CR8K.nes among them supply the at-bound rows; two NROM ROMs give the same-board pair).
 */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../state_test.h"

#define H_VERSION 8u
#define H_HEADER_SIZE 10u
#define H_ROM_CRC 24u
#define H_FRAME 32u
#define H_TICKS 40u
#define H_RESERVED 48u
#define HEADER 64u
#define MAX_BOUNDS 512u

static int g_short;
static size_t g_flip_ram_stride = 1u;

struct rom {
    const char *path;
    uint8_t *data;
    size_t size;
    nesturbator *inst;
    uint8_t *state;
    size_t state_size;
    size_t ram_bytes; /* PRG-RAM plus CHR-RAM at the end of the state */
};

static void put_raw(uint8_t *p, uint64_t v, unsigned width)
{
    for (unsigned i = 0u; i < width; ++i)
        p[i] = (uint8_t)(v >> (8u * i));
}

static int64_t rd_i64(const uint8_t *p)
{
    uint64_t u = le64(p);
    int64_t v;
    memcpy(&v, &u, sizeof v);
    return v;
}

static void wr_i64(uint8_t *p, int64_t v)
{
    uint64_t u;
    memcpy(&u, &v, sizeof u);
    put_raw(p, u, 8u);
}

static int32_t rd_i32(const uint8_t *p)
{
    uint32_t u = le32(p);
    int32_t v;
    memcpy(&v, &u, sizeof v);
    return v;
}

static const char *base_name(const char *path)
{
    const char *s = strrchr(path, '/');
    const char *b = strrchr(path, '\\');
    if (b != NULL && (s == NULL || b > s))
        s = b;
    return s != NULL ? s + 1 : path;
}

static void rom_open(struct rom *r, const char *path)
{
    memset(r, 0, sizeof *r);
    r->path = path;
    if (!st_read_file(path, &r->data, &r->size)) {
        fprintf(stderr, "cannot read %s\n", path);
        exit(2);
    }
    r->inst = st_make_instance();
    st_load(r->inst, r->data, r->size);
    for (uint32_t n = 1u; n <= 20u; ++n)
        CHECK_EQ_U64(st_frame(r->inst, st_buttons(n), NULL, NULL, NULL, NULL), NESTURBATOR_OK);
    r->state_size = nesturbator_state_size(r->inst);
    CHECK(r->state_size > HEADER);
    r->state = (uint8_t *)malloc(r->state_size + 1u);
    CHECK_EQ_U64(nesturbator_save_state(r->inst, r->state, r->state_size), NESTURBATOR_OK);
    r->ram_bytes = r->inst->cart.prg_ram_size + (r->inst->cart.chr_is_ram != 0u ? 8192u : 0u);
}

static void rom_close(struct rom *r)
{
    nesturbator_destroy(r->inst);
    free(r->state);
    free(r->data);
}

/* A scratch copy of the state, for a case to damage. */
static uint8_t *copy_state(const struct rom *r)
{
    uint8_t *b = (uint8_t *)malloc(r->state_size + 1u);
    memcpy(b, r->state, r->state_size);
    return b;
}

/* Wraps the snapshot taken once per case group. */
struct guard {
    nesturbator *inst;
    struct st_snap snap;
    uint64_t gen;
};

static void guard_open(struct guard *g, nesturbator *inst)
{
    g->inst = inst;
    st_snapshot(inst, &g->snap);
    g->gen = nesturbator_save_generation(inst);
}

static void guard_close(struct guard *g)
{
    st_snap_free(&g->snap);
}

static int refuse(struct guard *g, const uint8_t *buf, size_t size, nesturbator_status want,
                  const char *what, const char *rom)
{
    char label[200];
    snprintf(label, sizeof label, "%s [%s]", what, base_name(rom));
    return st_refused_with(g->inst, &g->snap, g->gen, buf, size, want, label);
}

/* ---- Cases 1 to 5 ---- */

static void case_cross_rom(struct rom *roms, size_t n)
{
    for (size_t i = 0u; i < n; ++i) {
        nesturbator *empty = st_make_instance();
        struct guard g;
        guard_open(&g, empty);
        refuse(&g, roms[i].state, roms[i].state_size, NESTURBATOR_ERR_STATE_CARTRIDGE,
               "no cartridge", roms[i].path);
        guard_close(&g);
        nesturbator_destroy(empty);
        for (size_t j = 0u; j < n; ++j) {
            char what[100];
            if (i == j)
                continue;
            guard_open(&g, roms[j].inst);
            snprintf(what, sizeof what, "state of %s into %s", base_name(roms[i].path),
                     base_name(roms[j].path));
            refuse(&g, roms[i].state, roms[i].state_size, NESTURBATOR_ERR_STATE_CARTRIDGE, what,
                   roms[i].path);
            guard_close(&g);
        }
    }
}

static void case_header(struct rom *r)
{
    struct guard g;
    uint8_t *b = copy_state(r);
    guard_open(&g, r->inst);
    for (unsigned v = 0u; v < NESTURBATOR_STATE_FORMAT_VERSION; ++v) {
        put_raw(b + H_VERSION, v, 2u);
        refuse(&g, b, r->state_size, NESTURBATOR_ERR_STATE_FORMAT, "format_version", r->path);
    }
    put_raw(b + H_VERSION, NESTURBATOR_STATE_FORMAT_VERSION + 1u, 2u);
    refuse(&g, b, r->state_size, NESTURBATOR_ERR_STATE_FORMAT, "format_version", r->path);
    put_raw(b + H_VERSION, NESTURBATOR_STATE_FORMAT_VERSION, 2u);
    for (unsigned hs = 63u; hs <= 65u; hs += 2u) {
        put_raw(b + H_HEADER_SIZE, hs, 2u);
        refuse(&g, b, r->state_size, NESTURBATOR_ERR_STATE_CHECKSUM, "header_size, old CRC",
               r->path);
        st_recrc(b, r->state_size);
        refuse(&g, b, r->state_size, NESTURBATOR_ERR_STATE_INVALID, "header_size, new CRC",
               r->path);
    }
    put_raw(b + H_HEADER_SIZE, 64u, 2u);
    st_recrc(b, r->state_size);
    CHECK(memcmp(b, r->state, r->state_size) == 0);
    for (size_t i = H_RESERVED; i < HEADER; ++i) {
        b[i] = 1u;
        st_recrc(b, r->state_size);
        refuse(&g, b, r->state_size, NESTURBATOR_ERR_STATE_INVALID, "reserved byte", r->path);
        b[i] = 0u;
    }
    guard_close(&g);
    free(b);
}

static void case_truncation(struct rom *r)
{
    struct guard g;
    uint8_t *b = copy_state(r);
    b[r->state_size] = 0u;
    guard_open(&g, r->inst);
    for (size_t len = 0u; len < r->state_size; ++len)
        refuse(&g, b, len, NESTURBATOR_ERR_STATE_SIZE, "short", r->path);
    refuse(&g, b, r->state_size + 1u, NESTURBATOR_ERR_STATE_SIZE, "long", r->path);
    CHECK_EQ_U64(nesturbator_load_state(r->inst, NULL, 0u), NESTURBATOR_ERR_ARGUMENT);
    CHECK_EQ_U64(nesturbator_load_state(r->inst, NULL, r->state_size), NESTURBATOR_ERR_ARGUMENT);
    CHECK_EQ_U64(nesturbator_load_state(NULL, b, r->state_size), NESTURBATOR_ERR_ARGUMENT);
    CHECK(st_same(r->inst, &g.snap));
    guard_close(&g);
    /* S itself loads. */
    CHECK_EQ_U64(nesturbator_load_state(r->inst, b, r->state_size), NESTURBATOR_OK);
    free(b);
}

static nesturbator_status flip_class(size_t off)
{
    if (off < 10u)
        return NESTURBATOR_ERR_STATE_FORMAT;
    if (off < 12u)
        return NESTURBATOR_ERR_STATE_CHECKSUM;
    if (off < 16u)
        return NESTURBATOR_ERR_STATE_SIZE;
    if (off < 20u)
        return NESTURBATOR_ERR_STATE_CHECKSUM;
    if (off < 28u)
        return NESTURBATOR_ERR_STATE_CARTRIDGE;
    return NESTURBATOR_ERR_STATE_CHECKSUM;
}

static void case_flip(struct rom *r)
{
    struct guard g;
    uint8_t *b = copy_state(r);
    size_t ram_start = r->state_size - r->ram_bytes;
    guard_open(&g, r->inst);
    for (size_t off = 0u; off < r->state_size; ++off) {
        char what[64];
        if (off >= ram_start && (off - ram_start) % g_flip_ram_stride != 0u)
            continue;
        b[off] ^= 0xffu;
        snprintf(what, sizeof what, "flip offset %zu", off);
        refuse(&g, b, r->state_size, flip_class(off), what, r->path);
        b[off] ^= 0xffu;
    }
    guard_close(&g);
    free(b);
}

static void case_two_faults(struct rom *r)
{
    struct guard g;
    uint8_t *b = copy_state(r);
    guard_open(&g, r->inst);
    for (unsigned version = 1u; version <= 2u; ++version) {
        put_raw(b + H_VERSION, version, 2u);
        put_raw(b + H_HEADER_SIZE, 63u, 2u);
        b[H_ROM_CRC] ^= 0xffu;
        st_recrc(b, r->state_size);
        refuse(&g, b, r->state_size, NESTURBATOR_ERR_STATE_FORMAT,
               "old version + bad header + ROM CRC", r->path);
        b[H_ROM_CRC] ^= 0xffu;
        put_raw(b + H_HEADER_SIZE, 64u, 2u);
    }
    put_raw(b + H_VERSION, NESTURBATOR_STATE_FORMAT_VERSION + 1u, 2u);
    st_recrc(b, r->state_size);
    refuse(&g, b, r->state_size, NESTURBATOR_ERR_STATE_FORMAT, "future version", r->path);
    put_raw(b + H_VERSION, NESTURBATOR_STATE_FORMAT_VERSION, 2u);
    b[H_ROM_CRC] ^= 0xffu;
    refuse(&g, b, r->state_size - 1u, NESTURBATOR_ERR_STATE_CARTRIDGE, "ROM CRC + short", r->path);
    b[H_ROM_CRC] ^= 0xffu;
    b[HEADER + 3u] ^= 0xffu;
    refuse(&g, b, r->state_size - 1u, NESTURBATOR_ERR_STATE_SIZE, "short + bad CRC", r->path);
    guard_close(&g);
    free(b);
}

/* ---- Locating fields ---- */

/* The state offset of the width bytes at member_off in the instance: save, change the field,
   save again, and find the first byte that differs. Returns 0 when the field is not in the
   state. */
static size_t locate(struct rom *r, size_t member_off, size_t width)
{
    uint8_t *a = (uint8_t *)malloc(r->state_size);
    uint8_t *c = (uint8_t *)malloc(r->state_size);
    uint8_t *field = (uint8_t *)r->inst + member_off;
    uint8_t save[8];
    size_t found = 0u;
    /* The baseline is the instance as it is now: earlier cases may have run a frame on it. */
    CHECK_EQ_U64(nesturbator_save_state(r->inst, a, r->state_size), NESTURBATOR_OK);
    memcpy(save, field, width);
    for (size_t i = 0u; i < width; ++i)
        field[i] ^= 0xffu;
    CHECK_EQ_U64(nesturbator_save_state(r->inst, c, r->state_size), NESTURBATOR_OK);
    memcpy(field, save, width);
    for (size_t i = HEADER; i < r->state_size; ++i) {
        if (a[i] != c[i]) {
            found = i;
            break;
        }
    }
    free(a);
    free(c);
    return found;
}

struct row {
    const char *name;
    size_t member, width, count;
    int64_t hi;
    int board; /* 0: any, else the mapper id */
    int sgn;
    nesturbator_status next;
};

/* clang-format 18.1.3 and 18.1.8 lay out a braced initializer in a macro differently. */
/* clang-format off */
#define ROW(m, hi_)                                                                                \
    {#m,                                                                                           \
     offsetof(struct nesturbator, m),                                                              \
     sizeof(((struct nesturbator *)0)->m),                                                         \
     1u,                                                                                           \
     (hi_),                                                                                        \
     0,                                                                                            \
     0,                                                                                            \
     NESTURBATOR_OK}
#define ARR(m, n_, hi_)                                                                            \
    {#m,                                                                                           \
     offsetof(struct nesturbator, m[0]),                                                           \
     sizeof(((struct nesturbator *)0)->m[0]),                                                      \
     (n_),                                                                                         \
     (hi_),                                                                                        \
     0,                                                                                            \
     0,                                                                                            \
     NESTURBATOR_OK}
#define SGN(m, n_, hi_)                                                                            \
    {#m,                                                                                           \
     offsetof(struct nesturbator, m[0]),                                                           \
     sizeof(((struct nesturbator *)0)->m[0]),                                                      \
     (n_),                                                                                         \
     (hi_),                                                                                        \
     0,                                                                                            \
     1,                                                                                            \
     NESTURBATOR_OK}
#define BROW(b_, m, hi_)                                                                           \
    {#m,                                                                                           \
     offsetof(struct nesturbator, m),                                                              \
     sizeof(((struct nesturbator *)0)->m),                                                         \
     1u,                                                                                           \
     (hi_),                                                                                        \
     (b_),                                                                                         \
     0,                                                                                            \
     NESTURBATOR_OK}
/* clang-format on */
#define PULSE(i)                                                                                   \
    ROW(apu.pulse[i].timer, 0x7ff), ROW(apu.pulse[i].counter, 0x7ff), ROW(apu.pulse[i].phase, 7),  \
        ROW(apu.pulse[i].env_divider, 15), ROW(apu.pulse[i].env_decay, 15),                        \
        ROW(apu.pulse[i].env_start, 1), ROW(apu.pulse[i].sweep_divider, 7),                        \
        ROW(apu.pulse[i].sweep_reload, 1)

static const struct row ROWS[] = {
    ROW(audio_rem, 314999),
    ROW(apu.sample_phase, 314999),
    ROW(cpu.nmi_prev, 1),
    ROW(cpu.nmi_polled, 1),
    ROW(cpu.irq_line, 1),
    ROW(cpu.irq_captured, 1),
    ROW(cpu.poll_latch, 1),
    ROW(cpu.irq_accepted, 1),
    ROW(cpu.halted_in_read, 0),
    {"cpu.jammed", offsetof(struct nesturbator, cpu.jammed), 1u, 1u, 1, 0, 0, NESTURBATOR_STOP_JAM},
    ROW(bus.apu_get_put_phase, 1),
    ROW(bus.oam_dma_pending, 1),
    ROW(bus.controller_strobe, 1),
    PULSE(0),
    PULSE(1),
    ROW(apu.triangle.timer, 0x7ff),
    ROW(apu.triangle.counter, 0x7ff),
    ROW(apu.triangle.phase, 31),
    ROW(apu.triangle.linear, 0x7f),
    ROW(apu.triangle.linear_reload, 0x7f),
    ROW(apu.triangle.linear_reload_flag, 1),
    ROW(apu.noise.lfsr, 0x7fff),
    ROW(apu.noise.counter, 0xfff),
    ROW(apu.noise.env_divider, 15),
    ROW(apu.noise.env_decay, 15),
    ROW(apu.noise.env_start, 1),
    ROW(apu.dmc.timer, 214),
    ROW(apu.dmc.counter, 214),
    ROW(apu.dmc.remaining, 4081),
    ROW(apu.dmc.output, 127),
    ROW(apu.dmc.bits, 8),
    ROW(apu.dmc.silence, 1),
    ROW(apu.dmc.buffer_empty, 1),
    ROW(apu.dmc.irq, 1),
    ROW(apu.dmc.dma_pending, 1),
    ROW(apu.dmc.dma_halt_phase, 1),
    ROW(apu.dmc.dma_load_waiting, 1),
    ROW(apu.dmc.enable_delay, 4),
    ROW(apu.dmc.abort_delay, 3),
    ROW(apu.enabled, 0x1f),
    ROW(apu.pulse_clock_phase, 1),
    ROW(apu.frame_mode, 1),
    ROW(apu.frame_pending_mode, 1),
    ROW(apu.frame_irq_inhibit, 1),
    ROW(apu.frame_irq, 1),
    ROW(apu.frame_irq_clear_pending, 1),
    ROW(apu.frame_reset_delay, 4),
    ROW(apu.frame_cycle, 37281),
    ROW(synth.impulse_head, 15),
    SGN(synth.hp_input_q30, 2u, INT64_C(1) << 52),
    SGN(synth.hp_output_q30, 2u, INT64_C(1) << 52),
    {"synth.lp_output_q30", offsetof(struct nesturbator, synth.lp_output_q30), 8u, 1u,
     INT64_C(1) << 52, 0, 1, NESTURBATOR_OK},
    ROW(ppu.address_latch, 1),
    ROW(ppu.fine_x, 7),
    ROW(ppu.io_bus_age, 160000),
    ROW(ppu.vblank_suppress, 1),
    ROW(ppu.v, 0x7fff),
    ROW(ppu.t, 0x7fff),
    ARR(ppu.palette, 32u, 0x3f),
    ROW(ppu.sprite_count, 8),
    ROW(ppu.eval_count, 8),
    ROW(ppu.eval_n, 64),
    ROW(ppu.eval_m, 3),
    ROW(ppu.eval_remaining, 3),
    ARR(ppu.eval_sprite_zero, 8u, 1),
    ARR(ppu.sprite_zero, 8u, 1),
    ROW(ppu.bus_addr, 0x3fff),
    ROW(ppu.bg_at, 3),
    ROW(ppu.at_latch_lo, 1),
    ROW(ppu.at_latch_hi, 1),
    ROW(ppu.scanline, 261),
    ROW(ppu.dot, 340),
    ROW(ppu.odd_frame, 1),
    ROW(ppu.reset_flag, 1),
    ROW(mapper.irq, 1),
    BROW(1, mapper.reg.mmc1.shift, 0x1f),
    BROW(1, mapper.reg.mmc1.count, 4),
    BROW(1, mapper.reg.mmc1.control_x, 0x1f),
    BROW(1, mapper.reg.mmc1.chr0, 0x1f),
    BROW(1, mapper.reg.mmc1.chr1, 0x1f),
    BROW(1, mapper.reg.mmc1.prg, 0x1f),
    BROW(4, mapper.reg.mmc3.reload, 1),
    BROW(4, mapper.reg.mmc3.irq_enable, 1),
    BROW(4, mapper.reg.mmc3.a12, 1),
};
#define NROWS (sizeof ROWS / sizeof ROWS[0])

/* The fields with a dedicated case below, located by the same method. */
struct special {
    const char *name;
    size_t member, width;
};
static const struct special SPECIALS[] = {
    /* The maximum2 needs recognition/entry history; case_entry_history tests
       its reachable siblings and CRC-correct invalid combinations. */
    {"cpu.nmi_pending", offsetof(struct nesturbator, cpu.nmi_pending), 1u},
    {"cpu.entry_origin", offsetof(struct nesturbator, cpu.entry_origin), 1u},
    {"cpu.entry_next_action", offsetof(struct nesturbator, cpu.entry_next_action), 1u},
    {"cpu.entry_vector", offsetof(struct nesturbator, cpu.entry_vector), 1u},
    {"cpu.entry_vector_low", offsetof(struct nesturbator, cpu.entry_vector_low), 1u},
    {"synth.mixed_level", offsetof(struct nesturbator, synth.mixed_level), 4u},
    {"synth.integrator_q15", offsetof(struct nesturbator, synth.integrator_q15), 8u},
    {"ppu.status", offsetof(struct nesturbator, ppu.status), 1u},
    {"ppu.eval_copy_remaining", offsetof(struct nesturbator, ppu.eval_copy_remaining), 1u},
    {"ppu.ppu_ticks", offsetof(struct nesturbator, ppu.ppu_ticks), 8u},
};

static const struct special *special(const char *name)
{
    for (size_t i = 0u; i < sizeof SPECIALS / sizeof SPECIALS[0]; ++i)
        if (strcmp(SPECIALS[i].name, name) == 0)
            return &SPECIALS[i];
    return NULL;
}

static size_t special_off(struct rom *r, const char *name)
{
    const struct special *s = special(name);
    size_t off = locate(r, s->member, s->width);
    CHECK(off != 0u);
    return off;
}

/* The poll result is reachable independently of the current PPU line: a late
   capture may await the next instruction poll, or a poll may await service,
   after status/control have lowered the live input. */
static void case_nmi_history(struct rom *r)
{
    uint8_t *b = (uint8_t *)malloc(r->state_size);
    uint8_t *early = (uint8_t *)malloc(r->state_size);
    uint8_t *roundtrip = (uint8_t *)malloc(r->state_size);
    struct nesturbator *nes = (struct nesturbator *)r->inst;
    CHECK(b != NULL && early != NULL && roundtrip != NULL);
    if (b == NULL || early == NULL || roundtrip == NULL) {
        free(b);
        free(early);
        free(roundtrip);
        return;
    }

    /* Produce a captured edge and its later poll through the production bus
       sampler, then lower the live PPU input without clearing either latch. */
    CHECK_EQ_U64(nesturbator_reset(r->inst), NESTURBATOR_OK);
    st_assert_nmi_pin(nes); /* test pin stimulus; the CPU latch is not seeded */
    (void)nesturbator__bus_read(nes, nes->cpu.pc);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 1u);
    CHECK_EQ_U64(nes->cpu.nmi_polled, 0u);
    CHECK_EQ_U64(nesturbator_save_state(r->inst, early, r->state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_state(r->inst, early, r->state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(r->inst, roundtrip, r->state_size), NESTURBATOR_OK);
    CHECK(memcmp(roundtrip, early, r->state_size) == 0);

    nes->ppu.control &= 0x7fu; /* the live input falls; the captured edge remains */
    (void)nesturbator__bus_read(nes, nes->cpu.pc);
    CHECK_EQ_U64(nes->cpu.nmi_prev, 0u);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 1u);
    CHECK_EQ_U64(nes->cpu.nmi_polled, 1u);
    CHECK_EQ_U64(nesturbator__ppu_nmi_line(nes), 0u);

    CHECK_EQ_U64(nesturbator_save_state(r->inst, b, r->state_size), NESTURBATOR_OK);
    memcpy(r->state, b, r->state_size);
    CHECK_EQ_U64(nesturbator_load_state(r->inst, b, r->state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(r->inst, roundtrip, r->state_size), NESTURBATOR_OK);
    CHECK(memcmp(roundtrip, b, r->state_size) == 0);
    printf("row nmi-history production-edge live-line=low pending/polled=(1,0),(1,1) "
           "roundtrip=equal\n");

    /* A polled bit without its captured request is still an invalid candidate. */
    {
        uint8_t *bad = copy_state(r);
        size_t pending = locate(r, offsetof(struct nesturbator, cpu.nmi_pending), 1u);
        size_t polled = locate(r, offsetof(struct nesturbator, cpu.nmi_polled), 1u);
        struct guard g;
        CHECK(pending != 0u && polled != 0u);
        bad[pending] = 0u;
        st_recrc(bad, r->state_size);
        guard_open(&g, r->inst);
        refuse(&g, bad, r->state_size, NESTURBATOR_ERR_STATE_INVALID,
               "nmi_polled requires captured request", r->path);
        guard_close(&g);
        CHECK_EQ_U64(b[polled], 1u);
        free(bad);
    }
    free(roundtrip);
    free(early);
    free(b);
}

static void entry_state(struct rom *r, uint8_t *baseline, uint8_t *state,
                        enum nesturbator_entry_origin origin, uint8_t next_action,
                        uint8_t select_nmi)
{
    struct nesturbator *nes = (struct nesturbator *)r->inst;
    uint8_t first = origin == NESTURBATOR_ENTRY_NONE  ? 0u
                    : origin == NESTURBATOR_ENTRY_BRK ? 2u
                                                      : 1u;
    CHECK_EQ_U64(nesturbator_load_state(r->inst, baseline, r->state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_reset(r->inst), NESTURBATOR_OK);
    nesturbator__cpu_entry_begin(nes, origin);
    while (nes->cpu.entry_origin != NESTURBATOR_ENTRY_NONE &&
           nes->cpu.entry_next_action < next_action) {
        if (select_nmi != 0u && nes->cpu.entry_next_action == 4u)
            st_assert_nmi_pin(nes);
        CHECK_EQ_U64(nesturbator__cpu_entry_action(nes), 1u);
    }
    CHECK_EQ_U64(nes->cpu.entry_origin, origin);
    CHECK_EQ_U64(nes->cpu.entry_next_action, next_action);
    if (origin == NESTURBATOR_ENTRY_NMI)
        CHECK_EQ_U64(nes->cpu.entry_vector, NESTURBATOR_ENTRY_VECTOR_NMI);
    else if (next_action < 5u)
        CHECK_EQ_U64(nes->cpu.entry_vector, NESTURBATOR_ENTRY_VECTOR_NONE);
    else
        CHECK_EQ_U64(nes->cpu.entry_vector, select_nmi != 0u ? NESTURBATOR_ENTRY_VECTOR_NMI
                                                             : NESTURBATOR_ENTRY_VECTOR_IRQ);
    CHECK(first <= next_action);
    CHECK_EQ_U64(nesturbator_save_state(r->inst, state, r->state_size), NESTURBATOR_OK);
}

static void reject_entry_control(struct rom *r, const uint8_t *valid, size_t field, uint8_t value,
                                 const char *what)
{
    uint8_t *bad = (uint8_t *)malloc(r->state_size);
    uint8_t *actual = (uint8_t *)malloc(r->state_size);
    uint8_t *expected = (uint8_t *)malloc(r->state_size);
    nesturbator *control = st_make_instance();
    struct guard g;
    CHECK(bad != NULL && actual != NULL && expected != NULL && control != NULL);
    if (bad == NULL || actual == NULL || expected == NULL || control == NULL)
        goto done;

    memcpy(bad, valid, r->state_size);
    bad[field] = value;
    st_recrc(bad, r->state_size);
    CHECK_EQ_U64(nesturbator_load_state(r->inst, valid, r->state_size), NESTURBATOR_OK);
    st_load(control, r->data, r->size);
    CHECK_EQ_U64(nesturbator_load_state(control, valid, r->state_size), NESTURBATOR_OK);
    guard_open(&g, r->inst);
    refuse(&g, bad, r->state_size, NESTURBATOR_ERR_STATE_INVALID, what, r->path);
    guard_close(&g);

    /* A refused candidate must leave the next real entry action identical. */
    CHECK_EQ_U64(nesturbator__cpu_entry_action((struct nesturbator *)r->inst),
                 nesturbator__cpu_entry_action((struct nesturbator *)control));
    CHECK_EQ_U64(nesturbator_save_state(r->inst, actual, r->state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(control, expected, r->state_size), NESTURBATOR_OK);
    CHECK(memcmp(actual, expected, r->state_size) == 0);
    CHECK_EQ_U64(nesturbator_load_state(r->inst, valid, r->state_size), NESTURBATOR_OK);

done:
    if (control != NULL)
        nesturbator_destroy(control);
    free(expected);
    free(actual);
    free(bad);
}

static void check_entry_roundtrip(struct rom *r, const uint8_t *state, const char *label)
{
    uint8_t *roundtrip = (uint8_t *)malloc(r->state_size);
    struct nesturbator *nes = (struct nesturbator *)r->inst;
    CHECK(roundtrip != NULL);
    if (roundtrip == NULL)
        return;
    CHECK_EQ_U64(nesturbator_load_state(r->inst, state, r->state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(r->inst, roundtrip, r->state_size), NESTURBATOR_OK);
    CHECK(memcmp(roundtrip, state, r->state_size) == 0);
    printf("row entry %-28s origin=%u next=%u vector=%u low=%02x load=0 roundtrip=equal\n", label,
           (unsigned)nes->cpu.entry_origin, (unsigned)nes->cpu.entry_next_action,
           (unsigned)nes->cpu.entry_vector, (unsigned)nes->cpu.entry_vector_low);
    free(roundtrip);
}

static void case_entry_history(struct rom *r)
{
    uint8_t *baseline = (uint8_t *)malloc(r->state_size);
    uint8_t *valid = (uint8_t *)malloc(r->state_size);
    struct nesturbator *nes = (struct nesturbator *)r->inst;
    size_t origin = locate(r, offsetof(struct nesturbator, cpu.entry_origin), 1u);
    size_t action = locate(r, offsetof(struct nesturbator, cpu.entry_next_action), 1u);
    size_t vector = locate(r, offsetof(struct nesturbator, cpu.entry_vector), 1u);
    size_t low = locate(r, offsetof(struct nesturbator, cpu.entry_vector_low), 1u);
    size_t jammed = locate(r, offsetof(struct nesturbator, cpu.jammed), 1u);
    size_t nmi_polled = locate(r, offsetof(struct nesturbator, cpu.nmi_polled), 1u);
    size_t nmi_pending = locate(r, offsetof(struct nesturbator, cpu.nmi_pending), 1u);
    size_t nmi_prev = locate(r, offsetof(struct nesturbator, cpu.nmi_prev), 1u);
    size_t poll_latch = locate(r, offsetof(struct nesturbator, cpu.poll_latch), 1u);
    size_t irq_accepted = locate(r, offsetof(struct nesturbator, cpu.irq_accepted), 1u);
    CHECK(baseline != NULL && valid != NULL);
    CHECK(origin != 0u && action != 0u && vector != 0u && low != 0u && jammed != 0u &&
          nmi_polled != 0u && nmi_pending != 0u && nmi_prev != 0u && poll_latch != 0u &&
          irq_accepted != 0u);
    if (baseline == NULL || valid == NULL || origin == 0u || action == 0u || vector == 0u ||
        low == 0u || jammed == 0u || nmi_polled == 0u || poll_latch == 0u || nmi_pending == 0u ||
        nmi_prev == 0u || irq_accepted == 0u) {
        free(valid);
        free(baseline);
        return;
    }

    CHECK_EQ_U64(nesturbator_reset(r->inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(r->inst, baseline, r->state_size), NESTURBATOR_OK);

    /* Every source-reachable completed action cutpoint has a public-loader
       sibling. This is independent of the existing interrupt-entry suite's
       instruction/stack assertions. */
    for (enum nesturbator_entry_origin o = NESTURBATOR_ENTRY_BRK; o <= NESTURBATOR_ENTRY_NMI; ++o) {
        uint8_t first = o == NESTURBATOR_ENTRY_BRK ? 2u : 1u;
        for (uint8_t select_nmi = 0u; select_nmi <= 1u; ++select_nmi) {
            if (o == NESTURBATOR_ENTRY_NMI && select_nmi != 0u)
                continue;
            for (uint8_t next = first; next <= 7u; ++next) {
                if (select_nmi != 0u && next < 5u)
                    continue;
                entry_state(r, baseline, valid, o, next, select_nmi);
                check_entry_roundtrip(r, valid,
                                      o == NESTURBATOR_ENTRY_BRK   ? "BRK"
                                      : o == NESTURBATOR_ENTRY_IRQ ? "IRQ"
                                                                   : "NMI");
            }
        }
    }

    /* A distinct NMI edge after IRQ's C4 vector choice stays pending while
       the older selected IRQ pair remains live. The bus action samples the
       event; the CPU latch itself is never injected. */
    entry_state(r, baseline, valid, NESTURBATOR_ENTRY_IRQ, 5u, 0u);
    CHECK_EQ_U64(nes->cpu.entry_vector, NESTURBATOR_ENTRY_VECTOR_IRQ);
    st_assert_nmi_pin(nes);
    CHECK_EQ_U64(nesturbator__cpu_entry_action(nes), 1u); /* C5 */
    CHECK_EQ_U64(nes->cpu.entry_next_action, 6u);
    CHECK_EQ_U64(nes->cpu.entry_vector, NESTURBATOR_ENTRY_VECTOR_IRQ);
    CHECK_EQ_U64(nes->cpu.nmi_pending, NESTURBATOR_NMI_INHIBITED);
    CHECK_EQ_U64(nesturbator_save_state(r->inst, valid, r->state_size), NESTURBATOR_OK);
    check_entry_roundtrip(r, valid, "IRQ C6 + new edge");
    reject_entry_control(r, valid, nmi_prev, 0u, "inhibited input was sampled asserted");
    reject_entry_control(r, valid, nmi_polled, 1u, "inhibited input is not recognized");
    reject_entry_control(r, valid, nmi_pending, NESTURBATOR_NMI_RECOGNIZED,
                         "IRQ post-choice input cannot already be recognized");
    reject_entry_control(r, valid, action, 5u, "inhibited input starts only after C5");
    reject_entry_control(r, valid, jammed, 1u, "inhibited input contradicts JAM");
    /* A released live input can still differ from the last sampled high at a
       saved cutpoint. Its next entry sample cancels the unrecognized candidate. */
    nes->ppu.control &= 0x7fu;
    CHECK_EQ_U64(nesturbator_save_state(r->inst, valid, r->state_size), NESTURBATOR_OK);
    check_entry_roundtrip(r, valid, "IRQ C6 inhibited, live released");

    entry_state(r, baseline, valid, NESTURBATOR_ENTRY_IRQ, 5u, 0u);
    reject_entry_control(r, valid, nmi_pending, NESTURBATOR_NMI_INHIBITED,
                         "pre-C5 cannot contain inhibited candidate");

    /* C7 can leave a real inhibited candidate while continuation is idle.
       No ordinary poll or JAM executes between C7 and handler T1. Retain a
       released live line to cover raw/history divergence at this handoff. */
    entry_state(r, baseline, valid, NESTURBATOR_ENTRY_IRQ, 7u, 0u);
    st_assert_nmi_pin(nes);
    CHECK_EQ_U64(nesturbator__cpu_entry_action(nes), 1u);
    CHECK_EQ_U64(nes->cpu.entry_origin, NESTURBATOR_ENTRY_NONE);
    CHECK_EQ_U64(nes->cpu.nmi_pending, NESTURBATOR_NMI_INHIBITED);
    nes->ppu.control &= 0x7fu;
    CHECK_EQ_U64(nesturbator_save_state(r->inst, valid, r->state_size), NESTURBATOR_OK);
    check_entry_roundtrip(r, valid, "idle after C7 inhibited, released");
    reject_entry_control(r, valid, irq_accepted, 1u, "idle candidate precedes IRQ arbitration");
    reject_entry_control(r, valid, jammed, 1u, "idle candidate precedes handler JAM");
    reject_entry_control(r, valid, poll_latch, 1u, "idle candidate has no ordinary IRQ poll");
    reject_entry_control(r, valid, nmi_polled, 1u, "idle candidate has no recognized NMI poll");
    reject_entry_control(r, valid, nmi_prev, 0u, "idle candidate retains sampled high");

    /* Each failed control is a one-byte, CRC-correct mutation of a real legal
       sibling. reject_entry_control also runs and compares its next action. */
    entry_state(r, baseline, valid, NESTURBATOR_ENTRY_NONE, 0u, 0u);
    reject_entry_control(r, valid, action, 1u, "idle has empty continuation");
    reject_entry_control(r, valid, vector, 3u, "idle vector enum range");
    reject_entry_control(r, valid, low, 1u, "idle low byte canonical zero");

    entry_state(r, baseline, valid, NESTURBATOR_ENTRY_IRQ, 1u, 0u);
    reject_entry_control(r, valid, origin, 4u, "entry origin enum range");
    reject_entry_control(r, valid, action, 0u, "active action lower bound");
    reject_entry_control(r, valid, action, 8u, "active action upper bound");
    reject_entry_control(r, valid, vector, 3u, "active vector enum range");

    entry_state(r, baseline, valid, NESTURBATOR_ENTRY_BRK, 2u, 0u);
    reject_entry_control(r, valid, action, 1u, "BRK starts after opcode fetch");
    reject_entry_control(r, valid, vector, NESTURBATOR_ENTRY_VECTOR_IRQ,
                         "BRK vector is unselected through C4");
    reject_entry_control(r, valid, jammed, 1u, "active entry contradicts JAM");
    reject_entry_control(r, valid, nmi_polled, 1u, "active entry cannot retain NMI poll");
    reject_entry_control(r, valid, poll_latch, 1u, "active entry cannot retain IRQ poll");
    reject_entry_control(r, valid, irq_accepted, 1u, "active entry cannot retain accepted IRQ");

    entry_state(r, baseline, valid, NESTURBATOR_ENTRY_IRQ, 5u, 0u);
    reject_entry_control(r, valid, vector, NESTURBATOR_ENTRY_VECTOR_NONE,
                         "IRQ vector is selected after C4");
    reject_entry_control(r, valid, low, 1u, "IRQ low byte absent before C6");

    entry_state(r, baseline, valid, NESTURBATOR_ENTRY_NMI, 1u, 0u);
    reject_entry_control(r, valid, vector, NESTURBATOR_ENTRY_VECTOR_IRQ,
                         "NMI origin keeps NMI vector");
    reject_entry_control(r, valid, vector, NESTURBATOR_ENTRY_VECTOR_NONE,
                         "NMI vector cannot be unselected");

    entry_state(r, baseline, valid, NESTURBATOR_ENTRY_NMI, 6u, 0u);
    reject_entry_control(r, valid, low, 1u, "NMI low byte absent before C6");
    printf("CASE entry-reject CRC-correct controls no-mutation and future-action equal\n");
    free(valid);
    free(baseline);
}

/* ---- Case 6: the CRC-recomputed sweep over the recorded bounds ---- */

static int hex_name(char *dst, size_t n, size_t off, const char *side, int64_t v)
{
    return snprintf(dst, n, "offset %zu %s value %lld", off, side, (long long)v);
}

static void sweep_value(struct rom *r, struct guard *g, uint8_t *b,
                        const struct nesturbator__state_bound *f, const char *side, int64_t v)
{
    char what[120];
    uint8_t old[8];
    memcpy(old, b + f->offset, f->width);
    put_raw(b + f->offset, (uint64_t)v, f->width);
    st_recrc(b, r->state_size);
    hex_name(what, sizeof what, f->offset, side, v);
    refuse(g, b, r->state_size, NESTURBATOR_ERR_STATE_INVALID, what, r->path);
    memcpy(b + f->offset, old, f->width);
}

static void case_field_sweep(struct rom *r, const struct nesturbator__state_bound *bd, size_t nb)
{
    struct guard g;
    uint8_t *b = copy_state(r);
    guard_open(&g, r->inst);
    for (size_t i = 0u; i < nb; ++i) {
        const struct nesturbator__state_bound *f = &bd[i];
        int64_t tmax, tmin;
        if (f->width == 8u && !f->is_signed)
            continue; /* every 64-bit pattern is legal */
        if (f->is_signed) {
            tmax = f->width == 4u ? INT32_MAX : INT64_MAX;
            tmin = f->width == 4u ? INT32_MIN : INT64_MIN;
        } else {
            tmax = f->width == 1u ? 255 : f->width == 2u ? 65535 : (int64_t)UINT32_MAX;
            tmin = 0;
        }
        if (f->hi < tmax) {
            sweep_value(r, &g, b, f, "above", f->hi + 1);
            sweep_value(r, &g, b, f, "type max", tmax);
        }
        if (f->lo > tmin) {
            sweep_value(r, &g, b, f, "below", f->lo - 1);
            sweep_value(r, &g, b, f, "type min", tmin);
        }
    }
    {
        size_t status = special_off(r, "ppu.status");
        for (unsigned bit = 0u; bit < 5u; ++bit) {
            char what[40];
            uint8_t old = b[status];
            b[status] = (uint8_t)(old | (1u << bit));
            st_recrc(b, r->state_size);
            snprintf(what, sizeof what, "status bit %u", bit);
            refuse(&g, b, r->state_size, NESTURBATOR_ERR_STATE_INVALID, what, r->path);
            b[status] = old;
        }
    }
    guard_close(&g);
    free(b);
}

/* Every recorded field is either a named row or a special: no field escapes the at-bound table. */
static void case_coverage(struct rom *r, const struct nesturbator__state_bound *bd, size_t nb)
{
    size_t known[MAX_BOUNDS], nk = 0u;
    for (size_t i = 0u; i < NROWS; ++i) {
        for (size_t k = 0u; k < ROWS[i].count; ++k) {
            size_t off = locate(r, ROWS[i].member + k * ROWS[i].width, ROWS[i].width);
            if (off != 0u && nk < MAX_BOUNDS)
                known[nk++] = off;
            else if (off == 0u &&
                     (ROWS[i].board == 0 || ROWS[i].board == (int)r->inst->mapper.id)) {
                fprintf(stderr, "row %s is not in the state of %s\n", ROWS[i].name, r->path);
                check_failures++;
            }
        }
    }
    for (size_t i = 0u; i < sizeof SPECIALS / sizeof SPECIALS[0]; ++i)
        known[nk++] = special_off(r, SPECIALS[i].name);
    for (size_t k = 0u; k < NESTURBATOR_SYNTH_TAPS; ++k)
        known[nk++] = locate(r, offsetof(struct nesturbator, synth.impulse[0]) + 8u * k, 8u);
    for (size_t i = 0u; i < nb; ++i) {
        int hit = 0;
        for (size_t k = 0u; k < nk; ++k)
            if (known[k] == bd[i].offset)
                hit = 1;
        if (!hit) {
            fprintf(stderr, "recorded field at offset %zu of %s has no row\n", bd[i].offset,
                    r->path);
            check_failures++;
        }
    }
}

/* ---- Case 7: the at-bound table ---- */

static void load_expect(struct rom *r, uint8_t *b, nesturbator_status want_load,
                        nesturbator_status want_next, const char *what)
{
    nesturbator_status st;
    st_recrc(b, r->state_size);
    if (want_load != NESTURBATOR_OK) {
        struct guard g;
        guard_open(&g, r->inst);
        if (refuse(&g, b, r->state_size, want_load, what, r->path))
            printf("row %-44s load=%d\n", what, (int)want_load);
        guard_close(&g);
        return;
    }
    st = nesturbator_load_state(r->inst, b, r->state_size);
    if (st != NESTURBATOR_OK) {
        fprintf(stderr, "at-bound %s [%s]: load status %d\n", what, base_name(r->path), (int)st);
        check_failures++;
        return;
    }
    st = st_frame(r->inst, st_buttons(21u), NULL, NULL, NULL, NULL);
    if (st == want_next)
        printf("row %-44s load=0 next=%d\n", what, (int)st);
    if (st != want_next) {
        fprintf(stderr, "at-bound %s [%s]: next frame status %d, wanted %d\n", what,
                base_name(r->path), (int)st, (int)want_next);
        check_failures++;
    }
}

static void case_at_bound(struct rom *r, int which, const struct nesturbator__state_bound *bd,
                          size_t nb)
{
    uint8_t *b = copy_state(r);
    for (size_t i = 0u; i < NROWS; ++i) {
        const struct row *w = &ROWS[i];
        if (w->board != which)
            continue;
        for (size_t k = 0u; k < w->count; ++k) {
            size_t off = locate(r, w->member + k * w->width, w->width);
            const struct nesturbator__state_bound *f = NULL;
            int64_t vals[2];
            size_t nv = 1u;
            char what[96];
            for (size_t j = 0u; j < nb; ++j)
                if (bd[j].offset == off)
                    f = &bd[j];
            if (off == 0u || f == NULL) {
                fprintf(stderr, "row %s[%zu] has no recorded bound\n", w->name, k);
                check_failures++;
                continue;
            }
            if (f->hi != w->hi || f->width != w->width) {
                fprintf(stderr, "row %s: recorded hi %lld width %u, table %lld width %zu\n",
                        w->name, (long long)f->hi, (unsigned)f->width, (long long)w->hi, w->width);
                check_failures++;
            }
            vals[0] = w->hi;
            if (w->sgn) {
                vals[1] = -w->hi;
                nv = 2u;
            }
            for (size_t v = 0u; v < nv; ++v) {
                uint8_t old[8];
                memcpy(old, b + off, w->width);
                put_raw(b + off, (uint64_t)vals[v], (unsigned)w->width);
                snprintf(what, sizeof what, "%s[%zu]=%lld", w->name, k, (long long)vals[v]);
                load_expect(r, b, NESTURBATOR_OK, w->next, what);
                memcpy(b + off, old, w->width);
            }
        }
    }
    free(b);
}

/* Synthesis rows: the sum invariant in the at-bound table. */
static void case_synth(struct rom *r)
{
    uint8_t *b = copy_state(r);
    size_t lvl = special_off(r, "synth.mixed_level");
    size_t integ = special_off(r, "synth.integrator_q15");
    size_t imp[NESTURBATOR_SYNTH_TAPS];
    int32_t old_lvl = rd_i32(b + lvl);
    int64_t old_int = rd_i64(b + integ);
    struct guard g;
    const int64_t B = INT64_C(1) << 40;
    for (size_t k = 0u; k < NESTURBATOR_SYNTH_TAPS; ++k)
        imp[k] = locate(r, offsetof(struct nesturbator, synth.impulse[0]) + 8u * k, 8u);
    guard_open(&g, r->inst);

    /* mixed_level at its bound, alone: the sum no longer holds. */
    if (old_lvl != 32767) {
        put_raw(b + lvl, 32767u, 4u);
        st_recrc(b, r->state_size);
        refuse(&g, b, r->state_size, NESTURBATOR_ERR_STATE_INVALID, "mixed_level alone", r->path);
        wr_i64(b + integ, old_int + (int64_t)(32767 - old_lvl) * 32768);
        load_expect(r, b, NESTURBATOR_OK, NESTURBATOR_OK, "mixed_level=32767 + integrator");
        put_raw(b + lvl, (uint64_t)(uint32_t)old_lvl, 4u);
        wr_i64(b + integ, old_int);
    }
    guard_close(&g);
    for (size_t k = 0u; k < NESTURBATOR_SYNTH_TAPS; ++k) {
        int64_t old_imp = rd_i64(b + imp[k]);
        int64_t x = old_int + old_imp;
        int64_t nv = x >= 0 ? B : -B;
        char what[64];
        guard_open(&g, r->inst);
        wr_i64(b + imp[k], nv);
        st_recrc(b, r->state_size);
        snprintf(what, sizeof what, "impulse[%zu] alone", k);
        refuse(&g, b, r->state_size, NESTURBATOR_ERR_STATE_INVALID, what, r->path);
        guard_close(&g);
        wr_i64(b + integ, x - nv);
        snprintf(what, sizeof what, "impulse[%zu]=+-2^40 + integrator", k);
        load_expect(r, b, NESTURBATOR_OK, NESTURBATOR_OK, what);
        wr_i64(b + imp[k], old_imp);
        wr_i64(b + integ, old_int);
    }
    {
        int64_t old_imp = rd_i64(b + imp[0]);
        int64_t x = old_int + old_imp;
        int64_t nv = x >= 0 ? B : -B;
        guard_open(&g, r->inst);
        wr_i64(b + integ, nv);
        st_recrc(b, r->state_size);
        refuse(&g, b, r->state_size, NESTURBATOR_ERR_STATE_INVALID, "integrator alone", r->path);
        guard_close(&g);
        wr_i64(b + imp[0], x - nv);
        load_expect(r, b, NESTURBATOR_OK, NESTURBATOR_OK, "integrator=+-2^40 + impulse[0]");
    }
    free(b);
}

/* eval_copy_remaining and its pairing rule. */
static void case_eval(struct rom *r)
{
    uint8_t *b = copy_state(r);
    size_t ecr = special_off(r, "ppu.eval_copy_remaining");
    size_t ec = locate(r, offsetof(struct nesturbator, ppu.eval_count), 1u);
    struct guard g;
    b[ecr] = 3u;
    b[ec] = 0u;
    st_recrc(b, r->state_size);
    guard_open(&g, r->inst);
    refuse(&g, b, r->state_size, NESTURBATOR_ERR_STATE_INVALID,
           "eval_copy_remaining 3 needs eval_count >= 1", r->path);
    guard_close(&g);
    b[ec] = 1u;
    load_expect(r, b, NESTURBATOR_OK, NESTURBATOR_OK, "eval_copy_remaining=3 eval_count=1");
    {
        size_t st = special_off(r, "ppu.status");
        b[ecr] = 0u;
        b[ec] = 0u;
        b[st] = 0xe0u;
        load_expect(r, b, NESTURBATOR_OK, NESTURBATOR_OK, "ppu.status=0xe0");
    }
    free(b);
}

/* ---- Case 8: the ppu_ticks gap ---- */

static void case_ppu_gap(struct rom *r)
{
    uint8_t *b = copy_state(r);
    size_t pt_off = special_off(r, "ppu.ppu_ticks");
    uint64_t pt = le64(b + pt_off);
    uint64_t tpf = NESTURBATOR_TICKS_PER_FRAME;
    put_raw(b + H_TICKS, pt + tpf - 1u, 8u);
    load_expect(r, b, NESTURBATOR_OK, NESTURBATOR_OK, "ticks - ppu_ticks = TPF - 1");
    put_raw(b + H_TICKS, pt + tpf, 8u);
    load_expect(r, b, NESTURBATOR_ERR_STATE_INVALID, NESTURBATOR_OK, "ticks - ppu_ticks = TPF");
    put_raw(b + H_TICKS, pt + tpf + 1u, 8u);
    load_expect(r, b, NESTURBATOR_ERR_STATE_INVALID, NESTURBATOR_OK, "ticks - ppu_ticks = TPF + 1");
    put_raw(b + H_TICKS, pt, 8u);
    put_raw(b + pt_off, pt + 1u, 8u);
    load_expect(r, b, NESTURBATOR_ERR_STATE_INVALID, NESTURBATOR_OK, "ppu_ticks = ticks + 1");
    put_raw(b + pt_off, 0u, 8u);
    put_raw(b + H_TICKS, UINT64_C(1) << 50, 8u);
    load_expect(r, b, NESTURBATOR_ERR_STATE_INVALID, NESTURBATOR_OK, "ppu_ticks = 0, ticks = 2^50");
    /* CR-01: ticks near 2^64 wrapped the frame loop and never returned. */
    put_raw(b + pt_off, UINT64_MAX, 8u);
    put_raw(b + H_TICKS, UINT64_MAX, 8u);
    load_expect(r, b, NESTURBATOR_ERR_STATE_INVALID, NESTURBATOR_OK, "ticks = ppu_ticks = 2^64-1");
    put_raw(b + pt_off, (UINT64_C(1) << 62) + 1u, 8u);
    put_raw(b + H_TICKS, (UINT64_C(1) << 62) + 1u, 8u);
    load_expect(r, b, NESTURBATOR_ERR_STATE_INVALID, NESTURBATOR_OK,
                "ticks = ppu_ticks = 2^62 + 1");
    put_raw(b + pt_off, UINT64_C(1) << 62, 8u);
    put_raw(b + H_TICKS, UINT64_C(1) << 62, 8u);
    load_expect(r, b, NESTURBATOR_OK, NESTURBATOR_OK, "ticks = ppu_ticks = 2^62");
    free(b);
}

struct regional_capture {
    uint16_t pixels[256u * 240u];
    int16_t pcm[1024];
    nesturbator_frame frame;
};

static void regional_frame(nesturbator *inst, uint16_t buttons, struct regional_capture *out)
{
    nesturbator_input input;
    memset(&input, 0, sizeof input);
    input.size = (uint32_t)sizeof input;
    input.buttons[0] = (uint8_t)buttons;
    input.buttons[1] = (uint8_t)(buttons >> 8);
    memset(out->pixels, 0, sizeof out->pixels);
    memset(out->pcm, 0, sizeof out->pcm);
    CHECK_EQ_U64(nesturbator_set_input(inst, &input), NESTURBATOR_OK);
    memset(&out->frame, 0, sizeof out->frame);
    out->frame.size = (uint32_t)sizeof out->frame;
    out->frame.video = out->pixels;
    out->frame.video_pitch = 256u;
    out->frame.audio = out->pcm;
    out->frame.audio_capacity = 1024u;
    CHECK_EQ_U64(nesturbator_run_frame(inst, &out->frame), NESTURBATOR_OK);
}

static int regional_capture_equal(const struct regional_capture *a,
                                  const struct regional_capture *b)
{
    return memcmp(a->pixels, b->pixels, sizeof a->pixels) == 0 &&
           a->frame.audio_count == b->frame.audio_count &&
           memcmp(a->pcm, b->pcm, (size_t)a->frame.audio_count * sizeof a->pcm[0]) == 0 &&
           a->frame.ticks == b->frame.ticks && a->frame.frame_number == b->frame.frame_number;
}

static enum nesturbator_region_preference regional_preference(enum nesturbator_region region)
{
    return region == NESTURBATOR_REGION_PAL ? NESTURBATOR_REGION_PREFERENCE_PAL
                                            : NESTURBATOR_REGION_PREFERENCE_NTSC;
}

static void regional_mismatch(const uint8_t *image, size_t image_size,
                              enum nesturbator_region source_region,
                              enum nesturbator_region dest_region)
{
    nesturbator *source = st_make_instance();
    nesturbator *dest = st_make_instance();
    nesturbator *twin = st_make_instance();
    uint8_t *state = NULL, *bad = NULL, *dest_state = NULL, *twin_state = NULL;
    struct regional_capture expected, actual;
    size_t size = 0u;
    CHECK(source != NULL && dest != NULL && twin != NULL);
    if (source == NULL || dest == NULL || twin == NULL)
        goto done;
    CHECK_EQ_U64(nesturbator_load_cartridge_region(source, image, image_size,
                                                   regional_preference(source_region)),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge_region(dest, image, image_size,
                                                   regional_preference(dest_region)),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge_region(twin, image, image_size,
                                                   regional_preference(dest_region)),
                 NESTURBATOR_OK);
    CHECK_EQ_HEX(source->cart.rom_crc32, dest->cart.rom_crc32);
    size = nesturbator_state_size(source);
    CHECK_EQ_U64(nesturbator_state_size(dest), size);
    state = (uint8_t *)malloc(size + 1u);
    bad = (uint8_t *)malloc(size + 1u);
    dest_state = (uint8_t *)malloc(size);
    twin_state = (uint8_t *)malloc(size);
    CHECK(size > HEADER && state != NULL && bad != NULL && dest_state != NULL &&
          twin_state != NULL);
    if (size <= HEADER || state == NULL || bad == NULL || dest_state == NULL || twin_state == NULL)
        goto done;

    for (uint32_t frame = 1u; frame <= 2u; ++frame) {
        CHECK_EQ_U64(st_frame(source, st_buttons(frame), NULL, NULL, NULL, NULL), NESTURBATOR_OK);
        CHECK_EQ_U64(st_frame(dest, st_buttons(frame), NULL, NULL, NULL, NULL), NESTURBATOR_OK);
        CHECK_EQ_U64(st_frame(twin, st_buttons(frame), NULL, NULL, NULL, NULL), NESTURBATOR_OK);
    }
    CHECK_EQ_U64(nesturbator_save_state(source, state, size), NESTURBATOR_OK);
    CHECK_EQ_U64(state[23], source_region == NESTURBATOR_REGION_PAL ? 1u : 0u);
    CHECK_EQ_U64(nesturbator_save_state(dest, dest_state, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(twin, twin_state, size), NESTURBATOR_OK);
    CHECK(memcmp(dest_state, twin_state, size) == 0);

    CHECK(st_refused(dest, state, size, NESTURBATOR_ERR_STATE_CARTRIDGE));
    CHECK(st_refused(dest, state, size - 1u, NESTURBATOR_ERR_STATE_CARTRIDGE));
    /* The regional identity check wins over size, checksum and body failures. */
    memcpy(bad, state, size);
    bad[12] ^= 1u;
    bad[64] ^= 1u;
    bad[28] ^= 1u;
    CHECK(st_refused(dest, bad, size, NESTURBATOR_ERR_STATE_CARTRIDGE));
    {
        static const uint8_t unknown_regions[] = {2u, 255u};
        for (size_t index = 0u; index < sizeof unknown_regions / sizeof unknown_regions[0];
             ++index) {
            uint8_t unknown = unknown_regions[index];
            memcpy(bad, state, size);
            bad[23] = unknown;
            st_recrc(bad, size);
            CHECK(st_refused(dest, bad, size, NESTURBATOR_ERR_STATE_CARTRIDGE));
            bad[12] ^= 1u;
            bad[64] ^= 1u;
            bad[28] ^= 1u;
            CHECK(st_refused(dest, bad, size, NESTURBATOR_ERR_STATE_CARTRIDGE));
        }
    }

    for (uint32_t frame = 3u; frame <= 5u; ++frame) {
        regional_frame(dest, st_buttons(frame), &expected);
        regional_frame(twin, st_buttons(frame), &actual);
        if (!regional_capture_equal(&expected, &actual)) {
            size_t pixel = 0u, sample = 0u;
            while (pixel < 256u * 240u && expected.pixels[pixel] == actual.pixels[pixel])
                pixel++;
            while (sample < expected.frame.audio_count && sample < actual.frame.audio_count &&
                   expected.pcm[sample] == actual.pcm[sample])
                sample++;
            fprintf(
                stderr,
                "regional future mismatch %s -> %s frame=%u pixels=%lu/%04x/%04x "
                "audio-count=%u/%u sample=%lu/%d/%d ticks=%llu/%llu frame-number=%llu/%llu\n",
                source_region == NESTURBATOR_REGION_PAL ? "PAL" : "NTSC",
                dest_region == NESTURBATOR_REGION_PAL ? "PAL" : "NTSC", frame, (unsigned long)pixel,
                (unsigned)(pixel < 256u * 240u ? expected.pixels[pixel] : 0u),
                (unsigned)(pixel < 256u * 240u ? actual.pixels[pixel] : 0u),
                expected.frame.audio_count, actual.frame.audio_count, (unsigned long)sample,
                (int)(sample < 1024u ? expected.pcm[sample] : 0),
                (int)(sample < 1024u ? actual.pcm[sample] : 0),
                (unsigned long long)expected.frame.ticks, (unsigned long long)actual.frame.ticks,
                (unsigned long long)expected.frame.frame_number,
                (unsigned long long)actual.frame.frame_number);
            check_failures++;
        }
    }
    CHECK_EQ_U64(nesturbator_save_state(dest, dest_state, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(twin, twin_state, size), NESTURBATOR_OK);
    CHECK(memcmp(dest_state, twin_state, size) == 0);
    printf("regional state refusal: %s -> %s, identity precedes length/checksum/body; future frame "
           "and state unchanged\n",
           source_region == NESTURBATOR_REGION_PAL ? "PAL" : "NTSC",
           dest_region == NESTURBATOR_REGION_PAL ? "PAL" : "NTSC");

done:
    free(state);
    free(bad);
    free(dest_state);
    free(twin_state);
    nesturbator_destroy(source);
    nesturbator_destroy(dest);
    nesturbator_destroy(twin);
}

static const struct nesturbator__state_bound *
regional_bound(const struct nesturbator__state_bound *bounds, size_t count, size_t offset)
{
    for (size_t i = 0u; i < count; ++i)
        if (bounds[i].offset == offset)
            return &bounds[i];
    return NULL;
}

static void regional_continue_loaded(struct rom *r, const char *label)
{
    uint64_t before_ticks = r->inst->ticks, before_frame = r->inst->frame_number;
    uint64_t frame_ticks = nesturbator__frame_ticks(r->inst);
    uint64_t target_ticks = nesturbator__is_pal(r->inst) != 0 ? (before_frame + 1u) * frame_ticks
                                                              : before_ticks + frame_ticks;
    uint64_t after_ticks = 0u, after_frame = 0u;
    nesturbator_status status =
        st_frame(r->inst, st_buttons(3u), NULL, NULL, &after_ticks, &after_frame);
    if (status != NESTURBATOR_OK) {
        fprintf(stderr, "regional bound %s: next frame status %d\n", label, (int)status);
        check_failures++;
        return;
    }
    CHECK_EQ_U64(after_frame, before_frame + 1u);
    CHECK(after_ticks >= before_ticks);
    CHECK(after_ticks >= target_ticks);
    printf("regional bound continuation: %s frame=%llu ticks=%llu (before=%llu target=%llu)\n",
           label, (unsigned long long)after_frame, (unsigned long long)after_ticks,
           (unsigned long long)before_ticks, (unsigned long long)target_ticks);
}

static void regional_bound_value(struct rom *r, const uint8_t *baseline, uint8_t *mutated,
                                 size_t offset, size_t width, uint64_t value,
                                 nesturbator_status expected, const char *label)
{
    memcpy(mutated, baseline, r->state_size);
    put_raw(mutated + offset, value, (unsigned)width);
    st_recrc(mutated, r->state_size);
    if (expected == NESTURBATOR_OK) {
        nesturbator_status status = nesturbator_load_state(r->inst, mutated, r->state_size);
        if (status != NESTURBATOR_OK) {
            fprintf(stderr, "regional bound %s: load status %d\n", label, (int)status);
            check_failures++;
        } else {
            regional_continue_loaded(r, label);
        }
    } else {
        CHECK(st_refused(r->inst, mutated, r->state_size, expected));
    }
}

static void regional_apu_value(struct rom *r, const uint8_t *baseline, uint8_t *mutated,
                               size_t frame_mode_off, size_t frame_cycle_off, uint8_t frame_mode,
                               uint32_t frame_cycle, nesturbator_status expected, const char *label)
{
    memcpy(mutated, baseline, r->state_size);
    put_raw(mutated + frame_mode_off, frame_mode, 1u);
    put_raw(mutated + frame_cycle_off, frame_cycle, 4u);
    st_recrc(mutated, r->state_size);
    if (expected == NESTURBATOR_OK) {
        nesturbator_status status = nesturbator_load_state(r->inst, mutated, r->state_size);
        if (status != NESTURBATOR_OK) {
            fprintf(stderr, "regional APU %s: load status %d\n", label, (int)status);
            check_failures++;
        } else {
            regional_continue_loaded(r, label);
        }
    } else {
        CHECK(st_refused(r->inst, mutated, r->state_size, expected));
    }
}

static void regional_frame_value(struct rom *r, const uint8_t *baseline, uint8_t *mutated,
                                 uint64_t frame_number, nesturbator_status expected,
                                 const char *label)
{
    memcpy(mutated, baseline, r->state_size);
    put_raw(mutated + H_FRAME, frame_number, 8u);
    st_recrc(mutated, r->state_size);
    if (expected == NESTURBATOR_OK) {
        nesturbator_status status = nesturbator_load_state(r->inst, mutated, r->state_size);
        if (status != NESTURBATOR_OK) {
            fprintf(stderr, "regional frame %s: load status %d\n", label, (int)status);
            check_failures++;
        } else {
            regional_continue_loaded(r, label);
        }
    } else {
        CHECK(st_refused(r->inst, mutated, r->state_size, expected));
    }
}

static void regional_frame_counter_cases(struct rom *r, const uint8_t *baseline, uint8_t *mutated,
                                         uint64_t frame_ticks, int pal)
{
    const uint64_t ticks = le64(baseline + H_TICKS);
    const uint64_t quotient = ticks / frame_ticks;
    const uint64_t frame_number = le64(baseline + H_FRAME);

    CHECK_EQ_U64(quotient, 2u);
    CHECK_EQ_U64(frame_number, quotient);
    regional_frame_value(r, baseline, mutated, UINT64_MAX, NESTURBATOR_ERR_STATE_INVALID,
                         "frame counter UINT64_MAX");
    regional_frame_value(r, baseline, mutated, quotient + 1u, NESTURBATOR_ERR_STATE_INVALID,
                         "frame counter ahead of ticks");
    regional_frame_value(r, baseline, mutated, quotient, NESTURBATOR_OK,
                         "frame counter equals tick quotient");
    if (pal != 0) {
        regional_frame_value(r, baseline, mutated, quotient - 1u, NESTURBATOR_OK,
                             "PAL frame counter lags one frame");
        regional_frame_value(r, baseline, mutated, quotient - 2u, NESTURBATOR_ERR_STATE_INVALID,
                             "PAL frame counter lags two frames");
    } else {
        regional_frame_value(r, baseline, mutated, quotient - 2u, NESTURBATOR_OK,
                             "NTSC frame counter lags over one frame");
    }
}

static void regional_pal_five_step_endpoint(struct rom *r, const uint8_t *baseline,
                                            uint8_t *mutated, size_t frame_mode_off,
                                            size_t frame_cycle_off)
{
    struct nesturbator__state_bound bounds[MAX_BOUNDS];
    uint8_t *roundtrip = (uint8_t *)malloc(r->state_size);
    size_t count;
    const struct nesturbator__state_bound *frame_cycle_bound;
    nesturbator_status status;

    CHECK(roundtrip != NULL);
    if (roundtrip == NULL)
        return;

    CHECK_EQ_U64(r->inst->apu.frame_mode, 0u);
    count = nesturbator__state_bounds(r->inst, bounds, MAX_BOUNDS);
    frame_cycle_bound = regional_bound(bounds, count, frame_cycle_off);
    CHECK(frame_cycle_bound != NULL);
    if (frame_cycle_bound != NULL)
        CHECK_EQ_U64(frame_cycle_bound->hi, 37281u);

    memcpy(mutated, baseline, r->state_size);
    put_raw(mutated + frame_mode_off, 1u, 1u);
    put_raw(mutated + frame_cycle_off, 41565u, 4u);
    st_recrc(mutated, r->state_size);
    status = nesturbator_load_state(r->inst, mutated, r->state_size);
    CHECK_EQ_U64(status, NESTURBATOR_OK);
    if (status == NESTURBATOR_OK) {
        CHECK_EQ_U64(r->inst->apu.frame_mode, 1u);
        CHECK_EQ_U64(r->inst->apu.frame_cycle, 41565u);
        CHECK_EQ_U64(nesturbator_save_state(r->inst, roundtrip, r->state_size), NESTURBATOR_OK);
        CHECK(memcmp(roundtrip, mutated, r->state_size) == 0);
        count = nesturbator__state_bounds(r->inst, bounds, MAX_BOUNDS);
        frame_cycle_bound = regional_bound(bounds, count, frame_cycle_off);
        CHECK(frame_cycle_bound != NULL);
        if (frame_cycle_bound != NULL)
            CHECK_EQ_U64(frame_cycle_bound->hi, 41565u);
        regional_continue_loaded(r, "PAL five-step endpoint");
    }

    memcpy(mutated, baseline, r->state_size);
    put_raw(mutated + frame_mode_off, 1u, 1u);
    put_raw(mutated + frame_cycle_off, 41566u, 4u);
    st_recrc(mutated, r->state_size);
    CHECK(st_refused(r->inst, mutated, r->state_size, NESTURBATOR_ERR_STATE_INVALID));
    regional_apu_value(r, baseline, mutated, frame_mode_off, frame_cycle_off, 1u, 37281u,
                       NESTURBATOR_OK, "PAL five-step cursor 37281");
    regional_apu_value(r, baseline, mutated, frame_mode_off, frame_cycle_off, 1u, 37282u,
                       NESTURBATOR_OK, "PAL five-step cursor 37282");
    free(roundtrip);
}

static size_t regional_pal_loop_image(uint8_t *image, size_t image_size)
{
    static const uint8_t code[] = {0x4cu, 0x00u, 0x80u}; /* JMP $8000 */
    struct ines_spec spec;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = 1u;
    spec.prg_code = code;
    spec.prg_code_len = sizeof code;
    spec.nmi_vector = 0x8000u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    return ines_build(image, image_size, &spec);
}

static size_t regional_pal_gated_jam_image(uint8_t *image, size_t image_size)
{
    static const uint8_t code[] = {
        0xa9u, 0x00u,                     /* LDA #$00: two cycles */
        0xeau,                            /* NOP: two cycles */
        0x4cu, 0x06u, 0x80u,              /* JMP $8006: three cycles */
        0xa5u, 0x10u, 0xf0u, 0xfcu, 0x02u /* LDA $10; BEQ $8006; JAM */
    };
    struct ines_spec spec;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = 1u;
    spec.prg_code = code;
    spec.prg_code_len = sizeof code;
    spec.nmi_vector = 0x8000u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    return ines_build(image, image_size, &spec);
}

static void regional_pal_instruction_overrun(void)
{
    static uint8_t image[16u + INES_PRG_BANK + INES_CHR_BANK];
    const uint64_t frame_ticks = UINT64_C(1063920);
    const size_t image_size = regional_pal_loop_image(image, sizeof image);
    nesturbator *source = st_make_instance();
    nesturbator *loaded = st_make_instance();
    uint8_t *state = NULL, *roundtrip = NULL, *source_final = NULL, *loaded_final = NULL;
    size_t state_size;
    uint64_t first_ticks = 0u, first_frame = 0u;
    uint32_t source_video = 0u, source_audio = 0u, loaded_video = 0u, loaded_audio = 0u;
    uint64_t source_ticks = 0u, source_frame = 0u, loaded_ticks = 0u, loaded_frame = 0u;

    CHECK(image_size != 0u);
    CHECK_EQ_U64(nesturbator_load_cartridge_region(source, image, image_size,
                                                   NESTURBATOR_REGION_PREFERENCE_PAL),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge_region(loaded, image, image_size,
                                                   NESTURBATOR_REGION_PREFERENCE_PAL),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(st_frame(source, 0u, NULL, NULL, &first_ticks, &first_frame), NESTURBATOR_OK);
    CHECK_EQ_U64(first_frame, 1u);
    CHECK(first_ticks > frame_ticks);
    CHECK(first_ticks - frame_ticks < 96u);
    CHECK_EQ_U64(first_ticks / frame_ticks, 1u);

    state_size = nesturbator_state_size(source);
    state = (uint8_t *)malloc(state_size);
    roundtrip = (uint8_t *)malloc(state_size);
    source_final = (uint8_t *)malloc(state_size);
    loaded_final = (uint8_t *)malloc(state_size);
    CHECK(state != NULL && roundtrip != NULL && source_final != NULL && loaded_final != NULL);
    if (state == NULL || roundtrip == NULL || source_final == NULL || loaded_final == NULL)
        goto done;
    CHECK_EQ_U64(nesturbator_save_state(source, state, state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_state(loaded, state, state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(loaded, roundtrip, state_size), NESTURBATOR_OK);
    CHECK(memcmp(state, roundtrip, state_size) == 0);

    CHECK_EQ_U64(st_frame(source, 1u, &source_video, &source_audio, &source_ticks, &source_frame),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(st_frame(loaded, 1u, &loaded_video, &loaded_audio, &loaded_ticks, &loaded_frame),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(source_frame, 2u);
    CHECK(source_ticks >= UINT64_C(2) * frame_ticks);
    CHECK(source_ticks - UINT64_C(2) * frame_ticks < 96u);
    CHECK_EQ_U64(loaded_frame, source_frame);
    CHECK_EQ_U64(loaded_ticks, source_ticks);
    CHECK_EQ_HEX(loaded_video, source_video);
    CHECK_EQ_HEX(loaded_audio, source_audio);
    CHECK_EQ_U64(nesturbator_save_state(source, source_final, state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(loaded, loaded_final, state_size), NESTURBATOR_OK);
    CHECK(memcmp(source_final, loaded_final, state_size) == 0);
    printf("regional PAL instruction overrun: first=%llu second=%llu target=%llu\n",
           (unsigned long long)first_ticks, (unsigned long long)source_ticks,
           (unsigned long long)(UINT64_C(2) * frame_ticks));

done:
    free(state);
    free(roundtrip);
    free(source_final);
    free(loaded_final);
    nesturbator_destroy(source);
    nesturbator_destroy(loaded);
}

static void regional_pal_jam_lag(void)
{
    static uint8_t image[16u + INES_PRG_BANK + INES_CHR_BANK];
    const uint64_t frame_ticks = UINT64_C(1063920);
    const size_t image_size = regional_pal_gated_jam_image(image, sizeof image);
    nesturbator *jammed = st_make_instance();
    nesturbator *loaded = st_make_instance();
    struct nesturbator *nes = (struct nesturbator *)jammed;
    uint8_t *jam_state = NULL, *again = NULL, *reset_state = NULL, *peer_state = NULL;
    uint8_t *jam_final = NULL, *loaded_final = NULL;
    size_t state_size;
    uint64_t target = frame_ticks;
    size_t steps = 0u;

    CHECK(image_size != 0u);
    CHECK_EQ_U64(nesturbator_load_cartridge_region(jammed, image, image_size,
                                                   NESTURBATOR_REGION_PREFERENCE_PAL),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge_region(loaded, image, image_size,
                                                   NESTURBATOR_REGION_PREFERENCE_PAL),
                 NESTURBATOR_OK);
    while (steps < 100000u) {
        if (nes->cpu.pc == 0x8008u && nes->ticks < target && target - nes->ticks <= 575u)
            break;
        if (nes->ticks >= target) {
            CHECK(0 && "gated PAL guest did not reach its JAM window before the frame target");
            goto done;
        }
        nesturbator__cpu_step(nes);
        steps++;
    }
    CHECK(steps < 100000u);
    CHECK_EQ_U64(nes->cpu.pc, 0x8008u);
    CHECK(nes->ticks < target && target - nes->ticks <= 575u);
    nes->bus.ram[0x10] = 1u;
    CHECK_EQ_U64(st_frame(jammed, 0u, NULL, NULL, NULL, NULL), NESTURBATOR_STOP_JAM);
    CHECK_EQ_U64(nes->cpu.jammed, 1u);
    CHECK_EQ_U64(nes->frame_number, 0u);
    CHECK(nes->ticks >= target);
    CHECK_EQ_U64(nes->ticks / frame_ticks - nes->frame_number, 1u);

    state_size = nesturbator_state_size(jammed);
    jam_state = (uint8_t *)malloc(state_size);
    again = (uint8_t *)malloc(state_size);
    reset_state = (uint8_t *)malloc(state_size);
    peer_state = (uint8_t *)malloc(state_size);
    jam_final = (uint8_t *)malloc(state_size);
    loaded_final = (uint8_t *)malloc(state_size);
    CHECK(jam_state != NULL && again != NULL && reset_state != NULL && peer_state != NULL &&
          jam_final != NULL && loaded_final != NULL);
    if (jam_state == NULL || again == NULL || reset_state == NULL || peer_state == NULL ||
        jam_final == NULL || loaded_final == NULL)
        goto done;
    CHECK_EQ_U64(nesturbator_save_state(jammed, jam_state, state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(le64(jam_state + H_FRAME), 0u);
    CHECK_EQ_U64(le64(jam_state + H_TICKS) / frame_ticks, 1u);
    CHECK_EQ_U64(nesturbator_load_state(loaded, jam_state, state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(st_frame(loaded, 0u, NULL, NULL, NULL, NULL), NESTURBATOR_STOP_JAM);
    CHECK_EQ_U64(nesturbator_save_state(loaded, again, state_size), NESTURBATOR_OK);
    CHECK(memcmp(jam_state, again, state_size) == 0);

    jammed->bus.ram[0x10] = 0u;
    loaded->bus.ram[0x10] = 0u;
    CHECK_EQ_U64(nesturbator_reset(jammed), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_reset(loaded), NESTURBATOR_OK);
    CHECK_EQ_U64(jammed->frame_number, 0u);
    CHECK_EQ_U64(loaded->frame_number, 0u);
    CHECK_EQ_U64(jammed->ticks / frame_ticks - jammed->frame_number, 1u);
    CHECK_EQ_U64(loaded->ticks / frame_ticks - loaded->frame_number, 1u);
    CHECK_EQ_U64(nesturbator_save_state(jammed, reset_state, state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(loaded, peer_state, state_size), NESTURBATOR_OK);
    CHECK(memcmp(reset_state, peer_state, state_size) == 0);
    CHECK_EQ_U64(nesturbator_load_state(loaded, reset_state, state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(loaded, again, state_size), NESTURBATOR_OK);
    CHECK(memcmp(reset_state, again, state_size) == 0);

    for (uint32_t frame = 1u; frame <= 2u; ++frame) {
        uint32_t jam_video = 0u, jam_audio = 0u, loaded_video = 0u, loaded_audio = 0u;
        uint64_t jam_ticks = 0u, jam_frame = 0u, loaded_ticks = 0u, loaded_frame = 0u;
        CHECK_EQ_U64(
            st_frame(jammed, st_buttons(frame), &jam_video, &jam_audio, &jam_ticks, &jam_frame),
            NESTURBATOR_OK);
        CHECK_EQ_U64(st_frame(loaded, st_buttons(frame), &loaded_video, &loaded_audio,
                              &loaded_ticks, &loaded_frame),
                     NESTURBATOR_OK);
        CHECK_EQ_U64(jam_frame, frame);
        CHECK_EQ_U64(loaded_frame, jam_frame);
        CHECK_EQ_U64(loaded_ticks, jam_ticks);
        CHECK_EQ_HEX(loaded_video, jam_video);
        CHECK_EQ_HEX(loaded_audio, jam_audio);
        if (frame == 2u)
            CHECK(jam_ticks >= UINT64_C(2) * frame_ticks);
    }
    CHECK_EQ_U64(nesturbator_save_state(jammed, jam_final, state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(loaded, loaded_final, state_size), NESTURBATOR_OK);
    CHECK(memcmp(jam_final, loaded_final, state_size) == 0);
    printf("regional PAL JAM/reset lag: ticks=%llu frame=%llu\n", (unsigned long long)jammed->ticks,
           (unsigned long long)jammed->frame_number);

done:
    free(jam_state);
    free(again);
    free(reset_state);
    free(peer_state);
    free(jam_final);
    free(loaded_final);
    nesturbator_destroy(jammed);
    nesturbator_destroy(loaded);
}

static void regional_bounds(const uint8_t *image, size_t image_size, enum nesturbator_region region)
{
    const int pal = region == NESTURBATOR_REGION_PAL;
    const uint32_t denominator = pal ? 709379u : 315000u;
    const uint16_t max_scanline = pal ? 311u : 261u;
    const uint64_t frame_ticks = pal ? UINT64_C(1063920) : UINT64_C(714732);
    nesturbator *inst = st_make_instance();
    struct rom r;
    struct nesturbator__state_bound bounds[MAX_BOUNDS];
    uint8_t *baseline = NULL, *mutated = NULL;
    size_t count, audio_off, sample_off, scanline_off, ppu_ticks_off;
    size_t frame_mode_off, frame_cycle_off;
    const struct nesturbator__state_bound *audio_bound, *sample_bound, *scanline_bound;
    const struct nesturbator__state_bound *ppu_ticks_bound;
    uint64_t ticks, ppu_ticks;

    memset(&r, 0, sizeof r);
    if (inst == NULL)
        goto done;
    CHECK_EQ_U64(
        nesturbator_load_cartridge_region(inst, image, image_size, regional_preference(region)),
        NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_get_region(inst), region);
    for (uint32_t frame = 1u; frame <= 2u; ++frame)
        CHECK_EQ_U64(st_frame(inst, st_buttons(frame), NULL, NULL, NULL, NULL), NESTURBATOR_OK);
    r.inst = inst;
    r.state_size = nesturbator_state_size(inst);
    baseline = (uint8_t *)malloc(r.state_size);
    mutated = (uint8_t *)malloc(r.state_size);
    CHECK(r.state_size > HEADER && baseline != NULL && mutated != NULL);
    if (baseline == NULL || mutated == NULL || r.state_size <= HEADER)
        goto done;
    CHECK_EQ_U64(nesturbator_save_state(inst, baseline, r.state_size), NESTURBATOR_OK);

    audio_off = locate(&r, offsetof(struct nesturbator, audio_rem), 4u);
    sample_off = locate(&r, offsetof(struct nesturbator, apu.sample_phase), 4u);
    frame_mode_off = locate(&r, offsetof(struct nesturbator, apu.frame_mode), 1u);
    frame_cycle_off = locate(&r, offsetof(struct nesturbator, apu.frame_cycle), 4u);
    scanline_off = locate(&r, offsetof(struct nesturbator, ppu.scanline), 2u);
    ppu_ticks_off = special_off(&r, "ppu.ppu_ticks");
    count = nesturbator__state_bounds(inst, bounds, MAX_BOUNDS);
    audio_bound = regional_bound(bounds, count, audio_off);
    sample_bound = regional_bound(bounds, count, sample_off);
    scanline_bound = regional_bound(bounds, count, scanline_off);
    ppu_ticks_bound = regional_bound(bounds, count, ppu_ticks_off);
    CHECK(audio_bound != NULL && sample_bound != NULL && scanline_bound != NULL &&
          ppu_ticks_bound != NULL && frame_mode_off != 0u && frame_cycle_off != 0u);
    if (audio_bound == NULL || sample_bound == NULL || scanline_bound == NULL ||
        ppu_ticks_bound == NULL || frame_mode_off == 0u || frame_cycle_off == 0u)
        goto done;
    CHECK_EQ_U64(audio_bound->hi, denominator - 1u);
    CHECK_EQ_U64(sample_bound->hi, denominator - 1u);
    CHECK_EQ_U64(scanline_bound->hi, max_scanline);
    CHECK_EQ_U64(audio_bound->width, 4u);
    CHECK_EQ_U64(sample_bound->width, 4u);
    CHECK_EQ_U64(scanline_bound->width, 2u);
    CHECK_EQ_U64(ppu_ticks_bound->width, 8u);
    CHECK_EQ_U64(ppu_ticks_bound->cross, 1u);
    regional_bound_value(&r, baseline, mutated, audio_off, 4u, denominator - 1u, NESTURBATOR_OK,
                         "audio remainder max");
    regional_bound_value(&r, baseline, mutated, audio_off, 4u, denominator,
                         NESTURBATOR_ERR_STATE_INVALID, "audio remainder max+1");
    regional_bound_value(&r, baseline, mutated, sample_off, 4u, denominator - 1u, NESTURBATOR_OK,
                         "sample phase max");
    regional_bound_value(&r, baseline, mutated, sample_off, 4u, denominator,
                         NESTURBATOR_ERR_STATE_INVALID, "sample phase max+1");
    regional_bound_value(&r, baseline, mutated, scanline_off, 2u, max_scanline, NESTURBATOR_OK,
                         "scanline max");
    regional_bound_value(&r, baseline, mutated, scanline_off, 2u, max_scanline + 1u,
                         NESTURBATOR_ERR_STATE_INVALID, "scanline max+1");
    if (pal != 0)
        regional_pal_five_step_endpoint(&r, baseline, mutated, frame_mode_off, frame_cycle_off);
    if (pal != 0) {
        CHECK_EQ_U64(r.inst->apu.frame_mode, 1u);
        regional_apu_value(&r, baseline, mutated, frame_mode_off, frame_cycle_off, 0u, 37282u,
                           NESTURBATOR_ERR_STATE_INVALID,
                           "PAL four-step cursor with five-step destination");
    } else {
        CHECK_EQ_U64(r.inst->apu.frame_mode, 0u);
        regional_apu_value(&r, baseline, mutated, frame_mode_off, frame_cycle_off, 1u, 37282u,
                           NESTURBATOR_ERR_STATE_INVALID,
                           "NTSC five-step cursor with four-step destination");
    }
    regional_frame_counter_cases(&r, baseline, mutated, frame_ticks, pal);

    ticks = le64(baseline + H_TICKS);
    ppu_ticks = le64(baseline + ppu_ticks_off);
    CHECK(ppu_ticks <= ticks && ticks > frame_ticks);
    CHECK(regional_bound(bounds, count, ppu_ticks_off) != NULL);
    if (ticks > frame_ticks) {
        memcpy(mutated, baseline, r.state_size);
        put_raw(mutated + H_TICKS, ppu_ticks + frame_ticks - 1u, 8u);
        st_recrc(mutated, r.state_size);
        {
            nesturbator_status status = nesturbator_load_state(inst, mutated, r.state_size);
            CHECK_EQ_U64(status, NESTURBATOR_OK);
            if (status == NESTURBATOR_OK)
                regional_continue_loaded(&r, "PPU gap frame_ticks-1");
        }
        memcpy(mutated, baseline, r.state_size);
        put_raw(mutated + H_TICKS, ppu_ticks + frame_ticks, 8u);
        st_recrc(mutated, r.state_size);
        CHECK(st_refused(inst, mutated, r.state_size, NESTURBATOR_ERR_STATE_INVALID));
        memcpy(mutated, baseline, r.state_size);
        put_raw(mutated + ppu_ticks_off, ticks + 1u, 8u);
        st_recrc(mutated, r.state_size);
        CHECK(st_refused(inst, mutated, r.state_size, NESTURBATOR_ERR_STATE_INVALID));
    }
    printf("regional state bounds: %s denominator=%u scanline=%u PPU gap<%llu\n",
           pal ? "PAL" : "NTSC", denominator, max_scanline, (unsigned long long)frame_ticks);

done:
    free(baseline);
    free(mutated);
    nesturbator_destroy(inst);
}

static void case_regional_state(void)
{
    static uint8_t image[16u + 2u * INES_PRG_BANK + INES_CHR_BANK];
    const size_t image_size = st_stress_image(image, sizeof image, 0u);
    CHECK(image_size != 0u);
    if (image_size == 0u)
        return;
    for (size_t byte = 0u; byte < 17u; ++byte)
        image[16u + 0x4000u + byte] = 0x77u;
    regional_mismatch(image, image_size, NESTURBATOR_REGION_PAL, NESTURBATOR_REGION_NTSC);
    regional_mismatch(image, image_size, NESTURBATOR_REGION_NTSC, NESTURBATOR_REGION_PAL);
    regional_bounds(image, image_size, NESTURBATOR_REGION_NTSC);
    regional_bounds(image, image_size, NESTURBATOR_REGION_PAL);
    regional_pal_instruction_overrun();
    regional_pal_jam_lag();
}

/* ---- Driver ---- */

int main(int argc, char **argv)
{
    struct rom *roms;
    size_t n, first = 1u;
    if (argc == 2 && strcmp(argv[1], "--regional") == 0) {
        case_regional_state();
        CHECK_DONE();
    }
    if (argc > 1 && strcmp(argv[1], "--short") == 0) {
        g_short = 1;
        g_flip_ram_stride = 97u;
        first = 2u;
    }
    if (argc <= (int)first) {
        fprintf(stderr, "usage: core.state_reject [--short] <rom>...\n");
        return 2;
    }
    n = (size_t)argc - first;
    roms = (struct rom *)calloc(n, sizeof *roms);
    for (size_t i = 0u; i < n; ++i)
        rom_open(&roms[i], argv[first + i]);

    case_cross_rom(roms, n);
    for (size_t i = 0u; i < n; ++i) {
        struct nesturbator__state_bound bd[MAX_BOUNDS];
        size_t nb = nesturbator__state_bounds(roms[i].inst, bd, MAX_BOUNDS);
        const char *nm = base_name(roms[i].path);
        int which = -1;
        CHECK(nb > 0u && nb <= MAX_BOUNDS);
        if (strcmp(nm, "dabg.nes") == 0)
            which = 0;
        else if (strcmp(nm, "M1_P512K_CR8K_S32K.nes") == 0)
            which = 1;
        else if (strcmp(nm, "M4_P128K_CR8K.nes") == 0)
            which = 4;
        printf("%s: %zu bytes, %zu bounded fields\n", nm, roms[i].state_size, nb);
        case_header(&roms[i]);
        case_truncation(&roms[i]);
        case_nmi_history(&roms[i]);
        case_entry_history(&roms[i]);
        case_two_faults(&roms[i]);
        case_flip(&roms[i]);
        case_field_sweep(&roms[i], bd, nb);
        case_coverage(&roms[i], bd, nb);
        if (which >= 0) {
            case_at_bound(&roms[i], which, bd, nb);
            if (which == 0) {
                case_synth(&roms[i]);
                case_eval(&roms[i]);
                case_ppu_gap(&roms[i]);
            }
        }
    }
    for (size_t i = 0u; i < n; ++i)
        rom_close(&roms[i]);
    free(roms);
    CHECK_DONE();
}
