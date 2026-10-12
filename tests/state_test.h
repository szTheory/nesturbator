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
                                   uint32_t *audio_crc, uint64_t *ticks, uint64_t *frame_number)
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

#endif /* NESTURBATOR_TESTS_STATE_TEST_H */
