/* Run-ahead model over the library API (Phase 13, D-11 to D-13). Header-only, every function
   static inline; it uses the public header and check.h only, so both the libretro host and
   core.state_runahead include it.

   The oracle is an uninterrupted run: one instance, frame i given in[i], with the video hash
   (of the XRGB frame, as the adapter delivers it), the audio hash (the mono samples as the
   adapter duplicates them to both channels) and the state bytes after every frame. Run-ahead
   hosts are judged against it. The two-instance model is RetroArch's second-instance mode as
   its run-ahead guide describes it (docs.libretro.com/guides/runahead): instance A runs the real
   timeline; each tick instance B loads A's state, runs N frames and the last one is shown. */
#ifndef NESTURBATOR_TESTS_RUNAHEAD_MODEL_H
#define NESTURBATOR_TESTS_RUNAHEAD_MODEL_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "check.h"
#include "nesturbator.h"

#define RA_FNV_BASIS 14695981039346656037ull
#define RA_FNV_PRIME 1099511628211ull
#define RA_MAX_DEPTH 3u
#define RA_NMI_COUNT_ADDRESS 0x30u
#define RA_NMI_EVENTS_ADDRESS 0x40u
#define RA_SEED_1 0x0badc0deu
#define RA_SEED_2 0x5eed1234u
#define RA_PIXELS (256u * 240u)

/* FNV-1a over bytes. */
static inline uint64_t ra_fnv_bytes(uint64_t h, const void *data, size_t len)
{
    const unsigned char *p = (const unsigned char *)data;
    for (size_t i = 0u; i < len; i++)
        h = (h ^ p[i]) * RA_FNV_PRIME;
    return h;
}

/* FNV-1a over 32-bit words: one multiply per pixel. */
static inline uint64_t ra_hash_words(const uint32_t *words, size_t count)
{
    uint64_t h = RA_FNV_BASIS;
    for (size_t i = 0u; i < count; i++)
        h = (h ^ words[i]) * RA_FNV_PRIME;
    return h;
}

/* Audio hash of `frames` stereo frames, each int16 as two little-endian bytes. */
static inline uint64_t ra_audio_hash_stereo(const int16_t *lr, size_t frames)
{
    uint64_t h = RA_FNV_BASIS;
    for (size_t i = 0u; i < frames * 2u; i++) {
        unsigned char b[2];
        b[0] = (unsigned char)((uint16_t)lr[i] & 0xffu);
        b[1] = (unsigned char)((uint16_t)lr[i] >> 8);
        h = ra_fnv_bytes(h, b, 2u);
    }
    return h;
}

/* The same hash of mono samples, each written to both channels as the adapter does. */
static inline uint64_t ra_audio_hash_mono(const int16_t *mono, size_t count)
{
    uint64_t h = RA_FNV_BASIS;
    for (size_t i = 0u; i < count; i++) {
        unsigned char b[2];
        b[0] = (unsigned char)((uint16_t)mono[i] & 0xffu);
        b[1] = (unsigned char)((uint16_t)mono[i] >> 8);
        h = ra_fnv_bytes(ra_fnv_bytes(h, b, 2u), b, 2u);
    }
    return h;
}

/* The NMI host fixture must produce actual PCM, rather than a hash of silence. Frame sample
   counts are 798 or 799 at the pinned NTSC rate, so reject both canonical all-zero streams. */
static inline int ra_audio_hash_is_nonzero_pcm(uint64_t hash)
{
    static const int16_t silence[799] = {0};
    return hash != ra_audio_hash_mono(silence, 798u) && hash != ra_audio_hash_mono(silence, 799u);
}

static inline uint32_t ra_lcg(uint32_t x)
{
    return x * 1664525u + 1013904223u;
}

/* The buttons of frame t: port 0 in the low byte, port 1 in the high byte, in
   NESTURBATOR_BUTTON_* bit order. Script 1 draws a new value every frame; script 2 holds each
   drawn value for 1 to 5 frames. Both come from one fixed-seed integer LCG, so a test and the
   oracle agree on every platform. */
static inline uint16_t ra_script_buttons(unsigned script, uint32_t seed, uint32_t t)
{
    uint32_t x = seed;
    if (script == 1u) {
        for (uint32_t i = 0u; i <= t; i++)
            x = ra_lcg(x);
        return (uint16_t)(x >> 16);
    }
    for (uint32_t pos = 0u;;) {
        uint16_t value;
        uint32_t len;
        x = ra_lcg(x);
        value = (uint16_t)(x >> 16);
        x = ra_lcg(x);
        len = 1u + ((x >> 16) % 5u);
        if (t < pos + len)
            return value;
        pos += len;
    }
}

static inline uint16_t *ra_script_make(unsigned script, uint32_t seed, uint32_t count)
{
    uint16_t *in = (uint16_t *)malloc((size_t)count * sizeof *in);
    if (in != NULL)
        for (uint32_t t = 0u; t < count; t++)
            in[t] = ra_script_buttons(script, seed, t);
    return in;
}

/* Which alignments of an input change against the displayed-tick cycle of N+1 frames a script
   places in ticks 0..T-1. Bit r: a change on the real frame of a tick t with t mod (N+1) == r
   (in[t] != in[t-1]; the real frame right after a load). Bit 4+r: a change on the first
   speculative frame of such a tick (in[t+1] != in[t]). in holds at least T+1 entries. */
static inline uint32_t ra_alignment_mask(const uint16_t *in, uint32_t T, unsigned N)
{
    uint32_t mask = 0u;
    for (uint32_t t = 0u; t < T; t++) {
        if (t > 0u && in[t] != in[t - 1u])
            mask |= 1u << (t % (N + 1u));
        if (in[t + 1u] != in[t])
            mask |= 1u << (4u + t % (N + 1u));
    }
    return mask;
}

/* 1 when the two scripts together cover every alignment for depth N. */
static inline int ra_alignment_covered(const uint16_t *in1, const uint16_t *in2, uint32_t T,
                                       unsigned N)
{
    uint32_t want = ((1u << (N + 1u)) - 1u) * 0x11u;
    return ((ra_alignment_mask(in1, T, N) | ra_alignment_mask(in2, T, N)) & want) == want;
}

/* ---- library runs ---- */

static inline nesturbator *ra_instance(const uint8_t *rom, size_t size)
{
    nesturbator_config cfg;
    nesturbator *nes = NULL;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &nes), NESTURBATOR_OK);
    if (nes != NULL)
        CHECK_EQ_U64(nesturbator_load_cartridge(nes, rom, size), NESTURBATOR_OK);
    return nes;
}

/* One frame with the given buttons. The video hash is over the XRGB frame the adapter hands a
   host (palette[native & 0x1ff]); the audio hash is ra_audio_hash_mono. Any output pointer may be
   NULL; *frame_number receives io.frame_number of an OK frame. Returns RA_FRAME_OK when the frame
   ran, RA_FRAME_JAM on NESTURBATOR_STOP_JAM (nothing is written, as nesturbator.h says) and
   RA_FRAME_FAIL otherwise. */
#define RA_FRAME_FAIL 0
#define RA_FRAME_OK 1
#define RA_FRAME_JAM 2

static inline int ra_lib_frame_status(nesturbator *nes, uint16_t buttons, uint64_t *video_hash,
                                      uint64_t *audio_hash, uint64_t *frame_number)
{
    static uint16_t video[RA_PIXELS];
    static int16_t audio[1024];
    static uint32_t palette[512];
    static int palette_ready;
    nesturbator_input in;
    nesturbator_frame io;
    nesturbator_status status;
    if (!palette_ready) {
        nesturbator_get_palette(NULL, palette, 512u);
        palette_ready = 1;
    }
    memset(&in, 0, sizeof in);
    in.size = (uint32_t)sizeof in;
    in.buttons[0] = (uint8_t)(buttons & 0xffu);
    in.buttons[1] = (uint8_t)(buttons >> 8);
    CHECK_EQ_U64(nesturbator_set_input(nes, &in), NESTURBATOR_OK);
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = video;
    io.video_pitch = 256u;
    io.audio = audio;
    io.audio_capacity = 1024u;
    status = nesturbator_run_frame(nes, &io);
    if (status == NESTURBATOR_STOP_JAM)
        return RA_FRAME_JAM;
    if (status != NESTURBATOR_OK) {
        CHECK(0);
        return RA_FRAME_FAIL;
    }
    if (video_hash != NULL) {
        uint64_t h = RA_FNV_BASIS;
        for (size_t i = 0u; i < RA_PIXELS; i++)
            h = (h ^ palette[video[i] & 0x1ffu]) * RA_FNV_PRIME;
        *video_hash = h;
    }
    if (audio_hash != NULL)
        *audio_hash = ra_audio_hash_mono(audio, io.audio_count);
    if (frame_number != NULL)
        *frame_number = io.frame_number;
    return RA_FRAME_OK;
}

/* The strict form: any status other than OK, a JAM included, is an error. Returns 1 when the
   frame ran. */
static inline int ra_lib_frame(nesturbator *nes, uint16_t buttons, uint64_t *video_hash,
                               uint64_t *audio_hash)
{
    int r = ra_lib_frame_status(nes, buttons, video_hash, audio_hash, NULL);
    if (r != RA_FRAME_OK) {
        CHECK(0);
        return 0;
    }
    return 1;
}

/* The uninterrupted run: per frame the video and audio hashes, and the state after the frame
   at state + i * state_size. */
/* One frame of the uninterrupted run. A frame that returned STOP_JAM is not valid: its hash is
   that of the last valid frame (the picture a host that repeats its last converted frame shows),
   its frame_number is held at the latch. */
struct ra_frame {
    uint64_t hash;
    uint64_t frame_number;
    int valid;
};

#define RA_NO_JAM 0xffffffffu

struct ra_oracle {
    uint32_t frames;
    size_t state_size;
    uint64_t *video;
    uint64_t *audio;
    uint8_t *state;
    uint8_t *guest_marker;
    struct ra_frame *rec; /* per frame, filled by the JAM-aware run only */
    uint32_t jam_frame;   /* the first STOP_JAM, or RA_NO_JAM */
};

static inline void ra_oracle_free(struct ra_oracle *o)
{
    free(o->video);
    free(o->audio);
    free(o->state);
    free(o->guest_marker);
    free(o->rec);
    memset(o, 0, sizeof *o);
}

static inline int ra_oracle_alloc(struct ra_oracle *o, uint32_t frames, size_t state_size)
{
    memset(o, 0, sizeof *o);
    o->frames = frames;
    o->state_size = state_size;
    o->video = (uint64_t *)calloc(frames, sizeof *o->video);
    o->audio = (uint64_t *)calloc(frames, sizeof *o->audio);
    o->state = (uint8_t *)calloc((size_t)frames * state_size, 1u);
    o->guest_marker = (uint8_t *)calloc(frames, sizeof *o->guest_marker);
    o->rec = (struct ra_frame *)calloc(frames, sizeof *o->rec);
    o->jam_frame = RA_NO_JAM;
    if (o->video == NULL || o->audio == NULL || o->state == NULL || o->guest_marker == NULL ||
        o->rec == NULL) {
        ra_oracle_free(o);
        return 0;
    }
    return 1;
}

/* Records one frame of a JAM-aware run: an OK frame is valid and moves frame_number; a STOP_JAM
   frame is not valid, holds the previous hash and frame_number, and has no audio (the hash of
   zero samples). Returns 0 when the frame neither ran nor JAMmed, or when a JAM comes before any
   valid frame. */
static inline int ra_record_frame(struct ra_oracle *o, uint32_t i, nesturbator *nes,
                                  uint16_t buttons, int jam_aware)
{
    uint64_t number = 0u;
    int r = ra_lib_frame_status(nes, buttons, &o->video[i], &o->audio[i], &number);
    if (r == RA_FRAME_JAM && jam_aware && i > 0u) {
        o->video[i] = o->rec[i - 1u].hash;
        o->audio[i] = ra_audio_hash_mono(NULL, 0u);
        o->rec[i].hash = o->video[i];
        o->rec[i].frame_number = o->rec[i - 1u].frame_number;
        o->rec[i].valid = 0;
        if (o->jam_frame == RA_NO_JAM)
            o->jam_frame = i;
        return 1;
    }
    if (r != RA_FRAME_OK) {
        CHECK(0);
        return 0;
    }
    CHECK_EQ_U64(nesturbator_peek_cpu_ram(nes, RA_NMI_COUNT_ADDRESS, &o->guest_marker[i]),
                 NESTURBATOR_OK);
    o->rec[i].hash = o->video[i];
    o->rec[i].frame_number = number;
    o->rec[i].valid = 1;
    return 1;
}

/* The owned NMI fixture writes its count and one-based event sequence into guest RAM.
   Keeping this read independent of frame/state equality makes a shared host/core bug visible. */
static inline void ra_check_nmi_marker(nesturbator *nes, uint8_t expected_count)
{
    uint8_t value = 0u;
    int matches = nesturbator_peek_cpu_ram(nes, RA_NMI_COUNT_ADDRESS, &value) == NESTURBATOR_OK &&
                  value == expected_count;
    for (uint16_t event = 1u; matches && event <= expected_count; event++)
        matches = nesturbator_peek_cpu_ram(nes, (uint16_t)(RA_NMI_EVENTS_ADDRESS + event),
                                           &value) == NESTURBATOR_OK &&
                  value == event;
    CHECK(matches);
}

static inline uint64_t ra_state_u64le(const uint8_t *state, size_t offset)
{
    uint64_t value = 0u;
    for (uint32_t byte = 0u; byte < 8u; byte++)
        value |= (uint64_t)state[offset + byte] << (8u * byte);
    return value;
}

/* The public state layout puts frame_number at byte 32 and ticks at byte 40. */
static inline uint64_t ra_state_frame_number(const uint8_t *state)
{
    return ra_state_u64le(state, 32u);
}

static inline uint64_t ra_state_ticks(const uint8_t *state)
{
    return ra_state_u64le(state, 40u);
}

static inline int ra_nmi_marker_matches(nesturbator *nes, uint8_t expected_count)
{
    uint8_t value = 0u;
    if (nesturbator_peek_cpu_ram(nes, RA_NMI_COUNT_ADDRESS, &value) != NESTURBATOR_OK ||
        value != expected_count)
        return 0;
    for (uint16_t event = 1u; event <= expected_count; event++) {
        if (nesturbator_peek_cpu_ram(nes, (uint16_t)(RA_NMI_EVENTS_ADDRESS + event), &value) !=
                NESTURBATOR_OK ||
            value != event)
            return 0;
    }
    return 1;
}

/* Load a host-produced state through the public API before inspecting the guest marker. */
static inline int ra_state_has_nmi_marker(const uint8_t *rom, size_t size, const uint8_t *state,
                                          size_t state_size, uint8_t expected_count)
{
    nesturbator *nes = ra_instance(rom, size);
    nesturbator_status status;
    if (nes == NULL)
        return 0;
    status = nesturbator_load_state(nes, state, state_size);
    CHECK_EQ_U64(status, NESTURBATOR_OK);
    int matches = status == NESTURBATOR_OK && ra_nmi_marker_matches(nes, expected_count);
    nesturbator_destroy(nes);
    return matches;
}

/* Runs frames 0..frames-1 with in[i]. With jam_aware set, a STOP_JAM frame is recorded as not
   valid and the run goes on (the instance stays latched); otherwise any status other than OK is
   an error. Returns 1 on success; the caller frees with ra_oracle_free. */
static inline int ra_run_uninterrupted_ex(const uint8_t *rom, size_t size, const uint16_t *in,
                                          uint32_t frames, struct ra_oracle *o, int jam_aware)
{
    nesturbator *nes = ra_instance(rom, size);
    int ok = 0;
    if (nes == NULL)
        return 0;
    if (ra_oracle_alloc(o, frames, nesturbator_state_size(nes))) {
        ok = 1;
        for (uint32_t i = 0u; i < frames && ok; i++) {
            ok = ra_record_frame(o, i, nes, in[i], jam_aware);
            if (ok) {
                nesturbator_status st = nesturbator_save_state(
                    nes, o->state + (size_t)i * o->state_size, o->state_size);
                CHECK_EQ_U64(st, NESTURBATOR_OK);
                ok = st == NESTURBATOR_OK;
            }
        }
        if (!ok)
            ra_oracle_free(o);
    }
    nesturbator_destroy(nes);
    return ok;
}

static inline int ra_run_uninterrupted(const uint8_t *rom, size_t size, const uint16_t *in,
                                       uint32_t frames, struct ra_oracle *o)
{
    return ra_run_uninterrupted_ex(rom, size, in, frames, o, 0);
}

/* The two-instance model (RetroArch's second-instance mode): A runs the real timeline with
   in[t] and is recorded in *a; each tick B loads A's state after frame t, runs depth frames
   with in[t] repeated, and the last frame's video hash is visible[t] (A's own frame when depth
   is 0). With jam_aware set a JAM frame (on A or on B) is not an error: it leaves the picture
   the host last converted. */
static inline int ra_run_two_instance_ex(const uint8_t *rom, size_t size, const uint16_t *in,
                                         uint32_t T, unsigned depth, struct ra_oracle *a,
                                         uint64_t *visible, int jam_aware)
{
    nesturbator *na = ra_instance(rom, size);
    nesturbator *nb = ra_instance(rom, size);
    int ok = 0;
    if (na != NULL && nb != NULL && ra_oracle_alloc(a, T, nesturbator_state_size(na))) {
        ok = 1;
        for (uint32_t t = 0u; t < T && ok; t++) {
            uint8_t *s = a->state + (size_t)t * a->state_size;
            ok = ra_record_frame(a, t, na, in[t], jam_aware);
            if (!ok)
                break;
            CHECK_EQ_U64(nesturbator_save_state(na, s, a->state_size), NESTURBATOR_OK);
            CHECK_EQ_U64(nesturbator_load_state(nb, s, a->state_size), NESTURBATOR_OK);
            visible[t] = a->video[t];
            for (unsigned j = 0u; j < depth && ok; j++) {
                int r = ra_lib_frame_status(nb, in[t], &visible[t], NULL, NULL);
                /* A frame that JAMs shows the last converted frame: visible[t] stays. */
                ok = r == RA_FRAME_OK || (r == RA_FRAME_JAM && jam_aware);
                if (!ok)
                    CHECK(0);
            }
        }
        if (!ok)
            ra_oracle_free(a);
    }
    nesturbator_destroy(nb);
    nesturbator_destroy(na);
    return ok;
}

static inline int ra_run_two_instance(const uint8_t *rom, size_t size, const uint16_t *in,
                                      uint32_t T, unsigned depth, struct ra_oracle *a,
                                      uint64_t *visible)
{
    return ra_run_two_instance_ex(rom, size, in, T, depth, a, visible, 0);
}

/* The one-instance model through the library: save after the real frame, run depth frames with
   in[t] repeated, keep the last frame's video hash, load the state back. */
static inline int ra_run_one_instance(const uint8_t *rom, size_t size, const uint16_t *in,
                                      uint32_t T, unsigned depth, uint64_t *visible)
{
    nesturbator *nes = ra_instance(rom, size);
    uint8_t *s = NULL;
    size_t ss;
    int ok = 0;
    if (nes == NULL)
        return 0;
    ss = nesturbator_state_size(nes);
    s = (uint8_t *)malloc(ss);
    if (s != NULL) {
        ok = 1;
        for (uint32_t t = 0u; t < T && ok; t++) {
            ok = ra_lib_frame(nes, in[t], &visible[t], NULL);
            if (!ok)
                break;
            CHECK_EQ_U64(nesturbator_save_state(nes, s, ss), NESTURBATOR_OK);
            for (unsigned j = 0u; j < depth && ok; j++)
                ok = ra_lib_frame(nes, in[t], &visible[t], NULL);
            CHECK_EQ_U64(nesturbator_load_state(nes, s, ss), NESTURBATOR_OK);
        }
    }
    free(s);
    nesturbator_destroy(nes);
    return ok;
}

#endif /* NESTURBATOR_TESTS_RUNAHEAD_MODEL_H */
