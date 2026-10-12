/* STATE-04 (D-03, D-04, D-16): a hostile or damaged state is refused with the right status and
   changes nothing. For each ROM a state is made after 20 frames, then:
     1. cross-ROM matrix and no-cartridge: ERR_STATE_CARTRIDGE
     2. format_version 0 and 2: FORMAT; header_size 63 and 65: CHECKSUM, then INVALID with the CRC
        recomputed
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
    for (unsigned v = 0u; v <= 2u; v += 2u) {
        put_raw(b + H_VERSION, v, 2u);
        refuse(&g, b, r->state_size, NESTURBATOR_ERR_STATE_FORMAT, "format_version", r->path);
    }
    put_raw(b + H_VERSION, 1u, 2u);
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
    put_raw(b + H_VERSION, 2u, 2u);
    b[H_ROM_CRC] ^= 0xffu;
    refuse(&g, b, r->state_size, NESTURBATOR_ERR_STATE_FORMAT, "version + ROM CRC", r->path);
    put_raw(b + H_VERSION, 1u, 2u);
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
#define PULSE(i)                                                                                   \
    ROW(apu.pulse[i].timer, 0x7ff), ROW(apu.pulse[i].counter, 0x7ff), ROW(apu.pulse[i].phase, 7),  \
        ROW(apu.pulse[i].env_divider, 15), ROW(apu.pulse[i].env_decay, 15),                        \
        ROW(apu.pulse[i].env_start, 1), ROW(apu.pulse[i].sweep_divider, 7),                        \
        ROW(apu.pulse[i].sweep_reload, 1)

static const struct row ROWS[] = {
    ROW(audio_rem, 314999),
    ROW(apu.sample_phase, 314999),
    ROW(cpu.nmi_prev, 1),
    ROW(cpu.nmi_pending, 1),
    ROW(cpu.irq_line, 1),
    ROW(cpu.poll_latch, 1),
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
    ROW(apu.dmc.buffer_empty, 1),
    ROW(apu.dmc.irq, 1),
    ROW(apu.dmc.dma_pending, 1),
    ROW(apu.dmc.dma_halt_phase, 1),
    ROW(apu.dmc.dma_load_waiting, 1),
    ROW(apu.dmc.enable_delay, 4),
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

/* ---- Driver ---- */

int main(int argc, char **argv)
{
    struct rom *roms;
    size_t n, first = 1u;
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
