/* Fuzz and replay the public save-state load path (Phase 13, D-30 to D-35).
 *
 * Input: byte 0 bits 0-3 pick one of twelve synthetic carts (six boards, each with a small and a
 * maximum PRG and CHR), bit 4 selects raw mode, bits 5-7 the length mode (exact, one byte short,
 * one byte long, header only; the mode is the 3-bit value modulo 4). Bytes 1 onwards are patches,
 * three bytes each: a little-endian offset (taken modulo the candidate length) and an XOR value,
 * applied to a state the harness saved from the cart.
 *
 * Outside raw mode the harness then restores the fields a load checks before its checksum (magic,
 * version, header size, total size, mapper, submapper, region and ROM CRC-32) and recomputes the
 * state CRC-32 with its own copy of the algorithm, so the mutated bytes reach the range checks
 * behind the checksum. The library carries no fuzz-only code for this.
 *
 * A cart is an iNES header plus PRG and CHR bytes from a fixed-seed integer LCG. Normal corpus
 * replay uses no ROM files, paths or environment variables. Dormant minimization and JAM oracle
 * controls use explicit test-only selectors. Every unexpected oracle failure calls abort(), so
 * libFuzzer and the replay build both report it. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__APPLE__) || defined(__linux__)
#include <sys/resource.h>
#endif
#include "nesturbator.h"
#include "failure_control.h"

#define NT_REPLAY_NAME "fuzz.state.regress"
#define NESTURBATOR_FUZZ_ACCEPTANCE 1
#define NESTURBATOR_REPLAY_NAME NT_REPLAY_NAME
#define NESTURBATOR_REPLAY_EXTRA_MAIN 1

#define NT_CART_COUNT 12u
#define NT_HEADER 64u
#define NT_FRAMES 2u
#define NT_PAD 0xa5u
#define NT_RAW 0x10u
/* 64-byte state header + 14 serialized core bytes + CPU fields through jammed. */
#define NT_CPU_JAMMED_OFFSET 90u

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);
int nt_replay_last_accepted(void);

struct nt_cart {
    const char *name;
    uint8_t mapper;
    uint8_t prg_16k;
    uint8_t chr_8k; /* 0 is CHR RAM */
};

/* Small is the least PRG and CHR the board accepts; maximum is the most the loader accepts for
 * the board, in the iNES 1 header this harness writes. */
static const struct nt_cart nt_carts[12] = {
    {"m0-small", 0u, 1u, 1u}, {"m0-max", 0u, 2u, 1u},   {"m1-small", 1u, 2u, 1u},
    {"m1-max", 1u, 32u, 0u},  {"m2-small", 2u, 1u, 0u}, {"m2-max", 2u, 255u, 1u},
    {"m3-small", 3u, 1u, 1u}, {"m3-max", 3u, 2u, 4u},   {"m4-small", 4u, 1u, 1u},
    {"m4-max", 4u, 32u, 32u}, {"m7-small", 7u, 2u, 0u}, {"m7-max", 7u, 8u, 0u},
};

static uint8_t *nt_image;
static size_t nt_image_size;
static unsigned nt_image_cart = 0xffffffffu;
static int nt_last_accepted;
static unsigned nt_jam_statuses;
static int nt_jam_oracle_divergence_control;
static int nt_jam_oracle_divergence_injected;

#ifdef NESTURBATOR_FUZZ_METRICS
struct nt_fuzz_metrics {
    uint64_t inputs;
    uint64_t accepted;
    uint64_t refused;
    uint64_t api_operations;
    uint64_t frame_calls;
    uint64_t frame_ok;
    uint64_t frame_jam;
    uint64_t guest_ticks;
};
static struct nt_fuzz_metrics nt_metrics;
#define NT_METRIC_OPERATION(name) (nt_metrics.api_operations++, nt_fuzz_trace_operation(name))

static uint64_t nt_metric_peak_rss_bytes(void)
{
#if defined(__APPLE__) || defined(__linux__)
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) != 0 || usage.ru_maxrss < 0)
        return 0u;
#if defined(__APPLE__)
    return (uint64_t)usage.ru_maxrss;
#else
    return (uint64_t)usage.ru_maxrss * 1024u;
#endif
#else
    return 0u;
#endif
}

void nt_replay_report_metrics(void);
void nt_replay_report_metrics(void)
{
    printf("METRICS target=state-loader inputs=%llu accepted=%llu refused=%llu api_operations=%llu "
           "frame_calls=%llu frame_ok=%llu frame_jam=%llu guest_ticks=%llu peak_rss_bytes=%llu\n",
           (unsigned long long)nt_metrics.inputs, (unsigned long long)nt_metrics.accepted,
           (unsigned long long)nt_metrics.refused, (unsigned long long)nt_metrics.api_operations,
           (unsigned long long)nt_metrics.frame_calls, (unsigned long long)nt_metrics.frame_ok,
           (unsigned long long)nt_metrics.frame_jam, (unsigned long long)nt_metrics.guest_ticks,
           (unsigned long long)nt_metric_peak_rss_bytes());
}
#else
#define NT_METRIC_OPERATION(name) nt_fuzz_trace_operation(name)
#endif

int nt_replay_last_accepted(void)
{
    return nt_last_accepted;
}

static void nt_fail(const char *what)
{
    fprintf(stderr, NT_REPLAY_NAME ": oracle failed: %s\n", what);
    abort();
}

static void nt_put32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static uint64_t nt_get64(const uint8_t *p)
{
    uint64_t value = 0u;
    for (unsigned i = 0u; i < 8u; ++i)
        value |= (uint64_t)p[i] << (8u * i);
    return value;
}

/* CRC-32, IEEE, reflected polynomial 0xEDB88320: the algorithm the header documents. */
static uint32_t nt_fuzz_crc32(uint32_t crc, const uint8_t *p, size_t n)
{
    crc = ~crc;
    for (size_t i = 0u; i < n; ++i) {
        crc ^= p[i];
        for (int k = 0; k < 8; ++k)
            crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}

/* The state CRC-32 covers every byte except offsets 28 to 31 (nesturbator.h). */
static void nt_recrc(uint8_t *state, size_t size)
{
    uint32_t crc = nt_fuzz_crc32(0u, state, 28u);
    crc = nt_fuzz_crc32(crc, state + 32, size - 32u);
    nt_put32(state + 28, crc);
}

/* Builds, or reuses, the image of one cart. */
static const uint8_t *nt_cart_image(unsigned index, size_t *size)
{
    const struct nt_cart *cart = &nt_carts[index];
    size_t total = 16u + (size_t)cart->prg_16k * 16384u + (size_t)cart->chr_8k * 8192u;
    uint32_t x = 0x13579bdfu + index;
    if (nt_image_cart != index) {
        free(nt_image);
        nt_image = (uint8_t *)malloc(total);
        if (nt_image == NULL)
            nt_fail("out of memory building a cart");
        memset(nt_image, 0, 16u);
        nt_image[0] = 'N';
        nt_image[1] = 'E';
        nt_image[2] = 'S';
        nt_image[3] = 0x1au;
        nt_image[4] = cart->prg_16k;
        nt_image[5] = cart->chr_8k;
        nt_image[6] = (uint8_t)(((unsigned)cart->mapper << 4) | 1u);
        nt_image[7] = (uint8_t)((unsigned)cart->mapper & 0xf0u);
        for (size_t i = 16u; i < total; ++i) {
            x = x * 1664525u + 1013904223u;
            nt_image[i] = (uint8_t)(x >> 24);
        }
        nt_image_size = total;
        nt_image_cart = index;
    }
    *size = nt_image_size;
    return nt_image;
}

static nesturbator *nt_new(unsigned cart)
{
    nesturbator *inst = NULL;
    nesturbator_config config = {0};
    size_t size;
    const uint8_t *image = nt_cart_image(cart, &size);
    config.size = (uint32_t)sizeof config;
    config.abi = NESTURBATOR_ABI_VERSION;
    NT_METRIC_OPERATION("nesturbator_create");
    if (nesturbator_create(&config, &inst) != NESTURBATOR_OK)
        nt_fail("create");
    NT_METRIC_OPERATION("nesturbator_load_cartridge");
    if (nesturbator_load_cartridge(inst, image, size) != NESTURBATOR_OK)
        nt_fail("synthetic cart refused");
    return inst;
}

static uint8_t *nt_save(const nesturbator *inst, size_t *size)
{
    uint8_t *buf;
    NT_METRIC_OPERATION("nesturbator_state_size");
    *size = nesturbator_state_size(inst);
    if (*size < NT_HEADER)
        nt_fail("state size below the header");
    buf = (uint8_t *)malloc(*size);
    if (buf == NULL)
        nt_fail("out of memory saving");
    NT_METRIC_OPERATION("nesturbator_save_state");
    if (nesturbator_save_state(inst, buf, *size) != NESTURBATOR_OK)
        nt_fail("save_state refused");
    return buf;
}

static uint64_t nt_hash(uint64_t h, const void *data, size_t len)
{
    const unsigned char *p = (const unsigned char *)data;
    for (size_t i = 0u; i < len; ++i)
        h = (h ^ p[i]) * 1099511628211ull;
    return h;
}

/* Runs NT_FRAMES frames with a fixed pad. JAM provides only a valid stop status and state. */
static uint64_t nt_run(nesturbator *inst, uint64_t ticks_origin, uint8_t output_poison)
{
    static uint16_t video[256u * 240u];
    static int16_t audio[1024];
    nesturbator_input input;
    nesturbator_frame frame;
    uint64_t h = 14695981039346656037ull;
    uint64_t previous_ticks = ticks_origin;
    for (size_t i = 0u; i < sizeof video / sizeof video[0]; ++i)
        video[i] = output_poison;
    for (size_t i = 0u; i < sizeof audio / sizeof audio[0]; ++i)
        audio[i] = output_poison;
    memset(&input, 0, sizeof input);
    input.size = (uint32_t)sizeof input;
    input.buttons[0] = NT_PAD;
    for (unsigned i = 0u; i < NT_FRAMES; ++i) {
        nesturbator_status status;
        NT_METRIC_OPERATION("nesturbator_set_input");
        if (nesturbator_set_input(inst, &input) != NESTURBATOR_OK)
            nt_fail("set_input");
        memset(&frame, 0, sizeof frame);
        frame.size = (uint32_t)sizeof frame;
        frame.video = video;
        frame.video_pitch = 256u;
        frame.audio = audio;
        frame.audio_capacity = 1024u;
        NT_METRIC_OPERATION("nesturbator_run_frame");
#ifdef NESTURBATOR_FUZZ_METRICS
        nt_metrics.frame_calls++;
#endif
        status = nesturbator_run_frame(inst, &frame);
#ifdef NESTURBATOR_FUZZ_METRICS
        if (status == NESTURBATOR_OK) {
            nt_metrics.frame_ok++;
        } else if (status == NESTURBATOR_STOP_JAM) {
            nt_metrics.frame_jam++;
        }
#endif
        if (status != NESTURBATOR_OK && status != NESTURBATOR_STOP_JAM)
            nt_fail("run_frame status after an accepted load");
        h = nt_hash(h, &status, sizeof status);
        if (status == NESTURBATOR_STOP_JAM) {
            nt_jam_statuses++;
            break;
        }
        if (frame.audio_count > sizeof audio / sizeof audio[0] || frame.ticks <= previous_ticks)
            nt_fail("successful frame did not advance its output contract");
#ifdef NESTURBATOR_FUZZ_METRICS
        nt_metrics.guest_ticks += frame.ticks - previous_ticks;
#endif
        previous_ticks = frame.ticks;
        h = nt_hash(h, video, sizeof video);
        h = nt_hash(h, &frame.audio_count, sizeof frame.audio_count);
        h = nt_hash(h, audio, (size_t)frame.audio_count * sizeof audio[0]);
        h = nt_hash(h, &frame.frame_number, sizeof frame.frame_number);
        h = nt_hash(h, &frame.ticks, sizeof frame.ticks);
    }
    return h;
}

static int nt_refusal_documented(nesturbator_status s)
{
    return s == NESTURBATOR_ERR_ARGUMENT || s == NESTURBATOR_ERR_STATE_FORMAT ||
           s == NESTURBATOR_ERR_STATE_CARTRIDGE || s == NESTURBATOR_ERR_STATE_SIZE ||
           s == NESTURBATOR_ERR_STATE_CHECKSUM || s == NESTURBATOR_ERR_STATE_INVALID;
}

static void nt_same(const uint8_t *a, const uint8_t *b, size_t n, const char *what)
{
    if (memcmp(a, b, n) != 0)
        nt_fail(what);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    const uint8_t selector = size > 0u ? data[0] : 0u;
    const unsigned cart = (unsigned)(selector & 15u) % 12u;
    const unsigned mode = (unsigned)((selector >> 5) & 7u) % 4u;
    nesturbator *a = nt_new(cart);
    size_t n, len, before_n, after_n;
    uint8_t *base = nt_save(a, &n);
    uint8_t *cand;
    uint8_t *before = nt_save(a, &before_n);
    nesturbator_status status;
#ifdef NESTURBATOR_FUZZ_METRICS
    nt_metrics.inputs++;
#endif
    nt_last_accepted = 0;

    len = n;
    if (mode == 1u)
        len = n - 1u;
    else if (mode == 2u)
        len = n + 1u;
    else if (mode == 3u)
        len = NT_HEADER;
    cand = (uint8_t *)calloc(len, 1u);
    if (cand == NULL)
        nt_fail("out of memory building a candidate");
    memcpy(cand, base, len < n ? len : n);
    for (size_t i = 1u; i + 2u < size; i += 3u) {
        size_t off = ((size_t)data[i] | ((size_t)data[i + 1u] << 8)) % len;
        cand[off] ^= data[i + 2u];
    }
    if ((selector & NT_RAW) == 0u) {
        memcpy(cand, base, 16u);
        memcpy(cand + 20, base + 20, 8u);
        nt_recrc(cand, len);
    }

    NT_METRIC_OPERATION("nesturbator_load_state");
    status = nesturbator_load_state(a, cand, len);
    if (status != NESTURBATOR_OK) {
#ifdef NESTURBATOR_FUZZ_METRICS
        nt_metrics.refused++;
#endif
        uint8_t *after;
        if (!nt_refusal_documented(status))
            nt_fail("load_state returned an undocumented status");
        after = nt_save(a, &after_n);
        if (after_n != before_n)
            nt_fail("a refused load changed the state size");
        nt_same(before, after, before_n, "a refused load changed the instance");
        free(after);
    } else {
#ifdef NESTURBATOR_FUZZ_METRICS
        nt_metrics.accepted++;
#endif
        nesturbator *b;
        nesturbator *c;
        size_t s0_n, s1_n, s2_n, s3_n, post_a_n, post_b_n;
        uint8_t *expect, *s0, *s1, *s2, *s3, *post_a, *post_b;
        uint64_t hash_a, hash_b;
        if (len != n)
            nt_fail("a state of the wrong length was accepted");
        nt_last_accepted = 1;
        if (nt_fuzz_control_matches("state-loader", data, size, 1u, (uint8_t)'S'))
            nt_fuzz_control_crash("state-loader");
        b = nt_new(cart);
        c = nt_new(cart);
        /* save(load(S)) is S with the library's behaviour revision and a fresh checksum. */
        expect = (uint8_t *)malloc(len);
        if (expect == NULL)
            nt_fail("out of memory");
        memcpy(expect, cand, len);
        nt_put32(expect + 16, (uint32_t)NESTURBATOR_BEHAVIOUR_REVISION);
        nt_recrc(expect, len);
        s0 = nt_save(a, &s0_n);
        if (s0_n != len)
            nt_fail("state size changed across a load");
        nt_same(expect, s0, len, "save(load(S)) differs from S");
        {
            const uint64_t ticks_origin = nt_get64(s0 + 40u);
            NT_METRIC_OPERATION("nesturbator_load_state");
            if (nesturbator_load_state(b, cand, len) != NESTURBATOR_OK)
                nt_fail("the same state loaded into a second fresh instance was refused");
            hash_a = nt_run(a, ticks_origin, 0xa5u);
            hash_b = nt_run(b, ticks_origin, nt_jam_oracle_divergence_control ? 0xa5u : 0x5au);
            if (hash_a != hash_b)
                nt_fail("two instances loading the same state diverged");
            if (nt_jam_oracle_divergence_control) {
                if (nt_jam_statuses != 2u)
                    nt_fail("JAM divergence control did not observe two stopped instances");
                NT_METRIC_OPERATION("nesturbator_reset");
                if (nesturbator_reset(b) != NESTURBATOR_OK)
                    nt_fail("JAM divergence control could not alter the canonical state");
                nt_jam_oracle_divergence_injected = 1;
            }
            post_a = nt_save(a, &post_a_n);
            post_b = nt_save(b, &post_b_n);
            if (post_a_n != post_b_n)
                nt_fail("two instances loading the same state produced different state sizes");
            nt_same(post_a, post_b, post_a_n,
                    "two instances loading the same state produced different post-run states");
            free(post_a);
            free(post_b);
        }
        s1 = nt_save(a, &s1_n);
        NT_METRIC_OPERATION("nesturbator_load_state");
        if (nesturbator_load_state(c, s1, s1_n) != NESTURBATOR_OK)
            nt_fail("a saved state did not reload into a fresh instance");
        s2 = nt_save(c, &s2_n);
        nt_same(s1, s2, s1_n, "save, load, save changed the bytes");
        NT_METRIC_OPERATION("nesturbator_load_state");
        if (nesturbator_load_state(c, s2, s2_n) != NESTURBATOR_OK)
            nt_fail("a saved state did not reload into its own instance");
        s3 = nt_save(c, &s3_n);
        nt_same(s2, s3, s2_n, "load of a saved state is not idempotent");
        free(expect);
        free(s0);
        free(s1);
        free(s2);
        free(s3);
        NT_METRIC_OPERATION("nesturbator_destroy");
        nesturbator_destroy(b);
        NT_METRIC_OPERATION("nesturbator_destroy");
        nesturbator_destroy(c);
    }
    free(base);
    free(before);
    free(cand);
    NT_METRIC_OPERATION("nesturbator_destroy");
    nesturbator_destroy(a);
    return 0;
}

#ifdef NESTURBATOR_FUZZ_REPLAY

/* ---- seeds ---------------------------------------------------------------------------------
 * The corpus is generated by this harness: --write-seeds DIR [PIN] writes it and prints one
 * manifest line per seed; --check-seeds DIR [FILES...] regenerates it in memory and compares. */

struct nt_seed {
    char name[32];
    uint8_t bytes[8];
    size_t len;
};

#define NT_SEED_MAX (NT_CART_COUNT + 5u)

static void nt_seeds(struct nt_seed *seeds, unsigned *count)
{
    unsigned n = 0u;
    for (unsigned i = 0u; i < NT_CART_COUNT; ++i) {
        struct nt_seed *s = &seeds[n++];
        memset(s, 0, sizeof *s);
        (void)snprintf(s->name, sizeof s->name, "valid-%s", nt_carts[i].name);
        s->bytes[0] = (uint8_t)i;
        s->len = 1u;
    }
    {
        struct nt_seed *s = &seeds[n++];
        memset(s, 0, sizeof *s);
        (void)snprintf(s->name, sizeof s->name, "reject-raw-magic");
        s->bytes[0] = NT_RAW; /* raw, exact, XOR 0xff into the first magic byte */
        s->bytes[3] = 0xffu;
        s->len = 4u;
    }
    {
        struct nt_seed *s = &seeds[n++];
        memset(s, 0, sizeof *s);
        (void)snprintf(s->name, sizeof s->name, "reject-raw-checksum");
        s->bytes[0] = NT_RAW; /* raw, exact, flip a bit past the header */
        s->bytes[1] = 100u;
        s->bytes[3] = 1u;
        s->len = 4u;
    }
    {
        struct nt_seed *s = &seeds[n++];
        memset(s, 0, sizeof *s);
        (void)snprintf(s->name, sizeof s->name, "reject-short");
        s->bytes[0] = 1u << 5;
        s->len = 1u;
    }
    {
        struct nt_seed *s = &seeds[n++];
        memset(s, 0, sizeof *s);
        (void)snprintf(s->name, sizeof s->name, "reject-long");
        s->bytes[0] = 2u << 5;
        s->len = 1u;
    }
    {
        struct nt_seed *s = &seeds[n++];
        memset(s, 0, sizeof *s);
        (void)snprintf(s->name, sizeof s->name, "reject-header-only");
        s->bytes[0] = 3u << 5;
        s->len = 1u;
    }
    *count = n;
}

/* ---- SHA-256 (FIPS 180-4), for the manifest lines ---- */
static uint32_t nt_rotr(uint32_t x, unsigned n)
{
    return (x >> n) | (x << (32u - n));
}

static void nt_sha256(const uint8_t *msg, size_t len, char out[65])
{
    static const uint32_t k[64] = {
        0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u,
        0xab1c5ed5u, 0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu,
        0x9bdc06a7u, 0xc19bf174u, 0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu,
        0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau, 0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
        0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u, 0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu,
        0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u, 0xa2bfe8a1u, 0xa81a664bu,
        0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u, 0x19a4c116u,
        0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
        0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u,
        0xc67178f2u};
    uint32_t h[8] = {0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
                     0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u};
    size_t padded = ((len + 9u + 63u) / 64u) * 64u;
    uint8_t *buf = (uint8_t *)calloc(padded, 1u);
    if (buf == NULL)
        nt_fail("out of memory hashing");
    if (len != 0u)
        memcpy(buf, msg, len);
    buf[len] = 0x80u;
    for (unsigned i = 0u; i < 8u; ++i)
        buf[padded - 1u - i] = (uint8_t)(((uint64_t)len * 8u) >> (8u * i));
    for (size_t off = 0u; off < padded; off += 64u) {
        uint32_t w[64], v[8];
        for (unsigned i = 0u; i < 16u; ++i)
            w[i] = ((uint32_t)buf[off + 4u * i] << 24) | ((uint32_t)buf[off + 4u * i + 1u] << 16) |
                   ((uint32_t)buf[off + 4u * i + 2u] << 8) | (uint32_t)buf[off + 4u * i + 3u];
        for (unsigned i = 16u; i < 64u; ++i) {
            uint32_t s0 = nt_rotr(w[i - 15], 7) ^ nt_rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = nt_rotr(w[i - 2], 17) ^ nt_rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }
        memcpy(v, h, sizeof v);
        for (unsigned i = 0u; i < 64u; ++i) {
            uint32_t s1 = nt_rotr(v[4], 6) ^ nt_rotr(v[4], 11) ^ nt_rotr(v[4], 25);
            uint32_t ch = (v[4] & v[5]) ^ (~v[4] & v[6]);
            uint32_t t1 = v[7] + s1 + ch + k[i] + w[i];
            uint32_t s0 = nt_rotr(v[0], 2) ^ nt_rotr(v[0], 13) ^ nt_rotr(v[0], 22);
            uint32_t mj = (v[0] & v[1]) ^ (v[0] & v[2]) ^ (v[1] & v[2]);
            uint32_t t2 = s0 + mj;
            v[7] = v[6];
            v[6] = v[5];
            v[5] = v[4];
            v[4] = v[3] + t1;
            v[3] = v[2];
            v[2] = v[1];
            v[1] = v[0];
            v[0] = t1 + t2;
        }
        for (unsigned i = 0u; i < 8u; ++i)
            h[i] += v[i];
    }
    for (unsigned i = 0u; i < 8u; ++i)
        (void)snprintf(out + 8u * i, 9u, "%08x", (unsigned)h[i]);
    free(buf);
}

static int nt_write_seeds(const char *dir, const char *pin)
{
    struct nt_seed seeds[NT_SEED_MAX];
    unsigned count;
    nt_seeds(seeds, &count);
    for (unsigned i = 0u; i < count; ++i) {
        char path[1024], sha[65];
        FILE *f;
        (void)snprintf(path, sizeof path, "%s/%s", dir, seeds[i].name);
        f = fopen(path, "wb");
        if (f == NULL || fwrite(seeds[i].bytes, 1u, seeds[i].len, f) != seeds[i].len ||
            fclose(f) != 0) {
            fprintf(stderr, NT_REPLAY_NAME ": cannot write seed: %s\n", path);
            return 1;
        }
        nt_sha256(seeds[i].bytes, seeds[i].len, sha);
        printf("tests/fuzz/corpus_state/%s\thttps://github.com/szTheory/nesturbator\t%s\tMIT\t%s\n",
               seeds[i].name, pin, sha);
    }
    return 0;
}

static const char *nt_base_name(const char *path)
{
    const char *base = path;
    for (const char *p = path; *p != '\0'; ++p)
        if (*p == '/' || *p == '\\')
            base = p + 1;
    return base;
}

static int nt_check_seeds(const char *dir, int argc, char **argv)
{
    struct nt_seed seeds[NT_SEED_MAX];
    unsigned count;
    int bad = 0;
    nt_seeds(seeds, &count);
    for (unsigned i = 0u; i < count; ++i) {
        char path[1024];
        uint8_t got[16];
        size_t got_n;
        FILE *f;
        (void)snprintf(path, sizeof path, "%s/%s", dir, seeds[i].name);
        f = fopen(path, "rb");
        if (f == NULL) {
            fprintf(stderr, NT_REPLAY_NAME ": generated seed is not committed: %s\n",
                    seeds[i].name);
            bad = 1;
            continue;
        }
        got_n = fread(got, 1u, sizeof got, f);
        (void)fclose(f);
        if (got_n != seeds[i].len || memcmp(got, seeds[i].bytes, got_n) != 0) {
            fprintf(stderr, NT_REPLAY_NAME ": committed seed differs from its generation: %s\n",
                    seeds[i].name);
            bad = 1;
            continue;
        }
        if (strncmp(seeds[i].name, "valid-", 6u) == 0) {
            (void)LLVMFuzzerTestOneInput(seeds[i].bytes, seeds[i].len);
            if (!nt_last_accepted) {
                fprintf(stderr, NT_REPLAY_NAME ": valid seed was not accepted: %s\n",
                        seeds[i].name);
                bad = 1;
            }
        }
    }
    for (int a = 0; a < argc; ++a) {
        const char *base = nt_base_name(argv[a]);
        int known = strncmp(base, "regress-", 8u) == 0;
        for (unsigned i = 0u; i < count; ++i)
            if (strcmp(base, seeds[i].name) == 0)
                known = 1;
        if (!known) {
            fprintf(stderr, NT_REPLAY_NAME ": committed file is not a generated seed: %s\n", base);
            bad = 1;
        }
    }
    return bad;
}

static int nt_replay_extra_main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--jam-oracle-regression") == 0) {
        static const uint8_t jammed_state_mutation[] = {0u, NT_CPU_JAMMED_OFFSET, 0u, 1u};
        (void)LLVMFuzzerTestOneInput(jammed_state_mutation, sizeof jammed_state_mutation);
        if (!nt_last_accepted || nt_jam_statuses != 2u) {
            fprintf(stderr,
                    NT_REPLAY_NAME ": JAM oracle regression did not exercise both stop calls\n");
            return 1;
        }
        puts("JAM oracle regression passed: differing stale frame buffers were ignored; status and "
             "canonical state matched");
        return 0;
    }
    if (argc == 2 && strcmp(argv[1], "--jam-oracle-divergence") == 0) {
        static const uint8_t jammed_state_mutation[] = {0u, NT_CPU_JAMMED_OFFSET, 0u, 1u};
        nt_jam_oracle_divergence_control = 1;
        (void)LLVMFuzzerTestOneInput(jammed_state_mutation, sizeof jammed_state_mutation);
        if (!nt_last_accepted || !nt_jam_oracle_divergence_injected) {
            fprintf(stderr,
                    NT_REPLAY_NAME ": JAM oracle did not reach the deliberate state difference\n");
            return 1;
        }
        fprintf(stderr,
                NT_REPLAY_NAME ": oracle failed to reject different canonical JAM states\n");
        return 1;
    }
    if (argc >= 3 && strcmp(argv[1], "--write-seeds") == 0)
        return nt_write_seeds(argv[2], argc >= 4 ? argv[3] : "PIN");
    if (argc >= 3 && strcmp(argv[1], "--check-seeds") == 0)
        return nt_check_seeds(argv[2], argc - 3, argv + 3);
    return -1;
}

#endif

#include "replay_main.h"
