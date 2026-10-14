/* frame_index ROM W IMAGE...
   frame_index --dups ROM W

   Test code only (D-27 as corrected at planning). A RetroArch screenshot is RGB, while the
   runner's --hash-frame hashes the native 16-bit buffer, so the two cannot be compared by hash.
   frame_index runs frames 1..W of ROM once, converts each frame to RGB with the palette and
   nesturbator_host_convert (the conversion the runner and the adapter use), and prints, for
   each IMAGE (a .bmp, or a .ppm when the name ends in .ppm), every frame index whose RGB equals
   the image: "<image> <i> <j> ..." or "<image> none". Exit 0 when every image matched at least
   one index, 1 when one did not, 2 on a read or usage error.

   --dups exits 0 only when every frame equal to an earlier frame has an equal partner at most 3
   frames before it, and prints the number of distinct frames. nesteroids repeats frames in
   adjacent pairs, so a screenshot can identify two indices. */
#include "bmp_ppm.h"
#include "convert.h"
#include "nesturbator.h"
#include "progression.h"
#include "state_test.h"

#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define W_PX 256u
#define H_PX 240u
#define MAX_PARTNER_GAP 3u
#define MAX_W 100000u

/* Phase 16's test-only encoding: 16 little-endian bit cells between four checker guards.
   All 8x8 pixels of each cell are checked; the counter never wraps in this window. */
#define MARKER_FIRST 4u
#define MARKER_LAST 10000u
#define MARKER_ROM_SIZE (16u + 16384u + 8192u)
#define MARKER_Y 224u

static const char *marker_decode(const uint8_t *rgb, const uint32_t *palette, uint32_t *counter)
{
    uint32_t value = 0;
    for (unsigned cell = 0; cell < 20u; ++cell) {
        unsigned bit = 2u;
        for (unsigned y = 0; y < 8u; ++y) {
            for (unsigned x = 0; x < 8u; ++x) {
                size_t p = ((MARKER_Y + y) * W_PX + 32u + cell * 8u + x) * 3u;
                uint32_t color =
                    ((uint32_t)rgb[p] << 16) | ((uint32_t)rgb[p + 1u] << 8) | rgb[p + 2u];
                unsigned on;
                if (color == (palette[0x0f] & 0xffffffu))
                    on = 0u;
                else if (color == (palette[0x30] & 0xffffffu))
                    on = 1u;
                else
                    return "color";
                if (cell < 2u || cell >= 18u) {
                    if (on != ((x + y + (cell >= 18u ? 1u : 0u)) & 1u))
                        return "guard";
                } else if (bit == 2u)
                    bit = on;
                else if (bit != on)
                    return "cell";
            }
        }
        if (cell >= 2u && cell < 18u)
            value |= (uint32_t)bit << (cell - 2u);
    }
    *counter = value;
    return NULL;
}

/* Independent expected pixels, unrelated to the guest's CHR/nametable writer. */
static void marker_pattern(uint8_t *rgb, const uint32_t *palette, uint32_t value)
{
    memset(rgb, 0, NESTURBATOR_TEST_RGB_SIZE);
    for (unsigned cell = 0; cell < 20u; ++cell) {
        for (unsigned y = 0; y < 8u; ++y) {
            for (unsigned x = 0; x < 8u; ++x) {
                unsigned on = cell < 2u || cell >= 18u ? ((x + y + (cell >= 18u ? 1u : 0u)) & 1u)
                                                       : ((value >> (cell - 2u)) & 1u);
                uint32_t color = palette[on ? 0x30u : 0x0fu];
                size_t p = ((MARKER_Y + y) * W_PX + 32u + cell * 8u + x) * 3u;
                rgb[p] = (uint8_t)(color >> 16);
                rgb[p + 1u] = (uint8_t)(color >> 8);
                rgb[p + 2u] = (uint8_t)color;
            }
        }
    }
}

static int marker_pixel_control(void)
{
    uint32_t palette[512] = {0};
    static uint8_t rgb[NESTURBATOR_TEST_RGB_SIZE];
    uint32_t value = 0;
    palette[0x30] = 0xffffffu;
    marker_pattern(rgb, palette, 257u);
    const char *reason = marker_decode(rgb, palette, &value);
    if (reason != NULL || value != 257u) {
        fprintf(stderr, "ASSERT marker identity: expected 257, actual %u reason %s\n", value,
                reason != NULL ? reason : "none");
        return 1;
    }
    return 0;
}

static uint8_t *read_file(const char *path, size_t *size)
{
    FILE *f = fopen(path, "rb");
    long length;
    uint8_t *bytes;
    if (f == NULL)
        return NULL;
    if (fseek(f, 0, SEEK_END) != 0 || (length = ftell(f)) <= 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return NULL;
    }
    bytes = (uint8_t *)malloc((size_t)length);
    if (bytes == NULL || fread(bytes, 1, (size_t)length, f) != (size_t)length) {
        free(bytes);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *size = (size_t)length;
    return bytes;
}

static int has_suffix(const char *s, const char *suffix)
{
    size_t n = strlen(s);
    size_t m = strlen(suffix);
    return n >= m && strcmp(s + n - m, suffix) == 0;
}

static int marker_write(const char *path, const void *data, size_t size)
{
    FILE *f = fopen(path, "wb");
    if (f == NULL)
        return 0;
    int ok = fwrite(data, 1u, size, f) == size;
    return fclose(f) == 0 && ok;
}

static int progression_anchor_reason(const char *reason)
{
    return reason != NULL && strncmp(reason, "container-", 10u) == 0 ? 2 : 1;
}

static uint8_t *marker_read(const char *path, size_t limit, size_t *size)
{
    FILE *f = fopen(path, "rb");
    uint8_t *bytes = (uint8_t *)malloc(limit + 1u);
    if (f == NULL || bytes == NULL) {
        if (f != NULL)
            fclose(f);
        free(bytes);
        return NULL;
    }
    *size = fread(bytes, 1u, limit + 1u, f);
    int ok = !ferror(f) && *size <= limit;
    fclose(f);
    if (!ok) {
        free(bytes);
        return NULL;
    }
    return bytes;
}

/* Owned 6502 program, authored from the documented PPU register operations (preparation
   NES-HARDWARE-PPU-CARTRIDGE sections 2/4, NESdev PPU registers). Two vblanks precede VRAM
   setup; NMI increments a 16-bit counter and writes its bits while rendering is in vblank.
   Tile 0 is clear, tile 1 solid, tiles 2/3 complementary checkers. No assembler is needed. */
static void marker_rom(uint8_t *rom)
{
    size_t p = 0;
    memset(rom, 0, MARKER_ROM_SIZE);
    memcpy(rom, "NES\032", 4u);
    rom[4] = 1u;
    rom[5] = 1u;
#define EMIT(...)                                                                                  \
    do {                                                                                           \
        const uint8_t op[] = {__VA_ARGS__};                                                        \
        memcpy(rom + 16u + p, op, sizeof op);                                                      \
        p += sizeof op;                                                                            \
    } while (0)
    EMIT(0x78, 0xd8, 0xa2, 0xff, 0x9a, 0xe8, 0x8e, 0x00, 0x20, 0x8e, 0x01, 0x20);
    EMIT(0x2c, 0x02, 0x20, 0x10, 0xfb, 0x2c, 0x02, 0x20, 0x10, 0xfb);
    EMIT(0xa9, 0x00, 0x85, 0x00, 0x85, 0x01);
    EMIT(0xa9, 0x20, 0x8d, 0x06, 0x20, 0xa9, 0x00, 0x8d, 0x06, 0x20);
    EMIT(0xa0, 0x04, 0xa2, 0x00, 0x8d, 0x07, 0x20, 0xe8, 0xd0, 0xfa, 0x88, 0xd0, 0xf5);
    EMIT(0xa9, 0x3f, 0x8d, 0x06, 0x20, 0xa9, 0x00, 0x8d, 0x06, 0x20);
    EMIT(0xa2, 0x20, 0xa9, 0x0f, 0x8d, 0x07, 0x20, 0xca, 0xd0, 0xfa);
    EMIT(0xa9, 0x3f, 0x8d, 0x06, 0x20, 0xa9, 0x01, 0x8d, 0x06, 0x20);
    EMIT(0xa9, 0x30, 0x8d, 0x07, 0x20);
    EMIT(0xa9, 0x23, 0x8d, 0x06, 0x20, 0xa9, 0x84, 0x8d, 0x06, 0x20);
    EMIT(0xa9, 0x02, 0x8d, 0x07, 0x20, 0x8d, 0x07, 0x20);
    EMIT(0xa9, 0x23, 0x8d, 0x06, 0x20, 0xa9, 0x96, 0x8d, 0x06, 0x20);
    EMIT(0xa9, 0x03, 0x8d, 0x07, 0x20, 0x8d, 0x07, 0x20);
    EMIT(0xa9, 0x00, 0x8d, 0x05, 0x20, 0x8d, 0x05, 0x20);
    EMIT(0xa9, 0x80, 0x8d, 0x00, 0x20, 0xa9, 0x0a, 0x8d, 0x01, 0x20);
    uint16_t loop = (uint16_t)(0x8000u + p);
    EMIT(0x4c, (uint8_t)loop, (uint8_t)(loop >> 8));
    p = 0x200u;
    EMIT(0x48, 0x8a, 0x48, 0xe6, 0x00, 0xd0, 0x02, 0xe6, 0x01);
    EMIT(0xa5, 0x00, 0x85, 0x02, 0xa5, 0x01, 0x85, 0x03);
    EMIT(0x2c, 0x02, 0x20, 0xa9, 0x23, 0x8d, 0x06, 0x20, 0xa9, 0x86, 0x8d, 0x06, 0x20);
    EMIT(0xa2, 0x10, 0x46, 0x03, 0x66, 0x02, 0xa9, 0x00, 0x69, 0x00, 0x8d, 0x07, 0x20, 0xca, 0xd0,
         0xf2);
    EMIT(0xa9, 0x00, 0x8d, 0x05, 0x20, 0x8d, 0x05, 0x20, 0x68, 0xaa, 0x68, 0x40);
    rom[16u + 0x3ffau] = 0x00;
    rom[16u + 0x3ffbu] = 0x82;
    rom[16u + 0x3ffcu] = 0x00;
    rom[16u + 0x3ffdu] = 0x80;
    rom[16u + 0x3ffeu] = 0x00;
    rom[16u + 0x3fffu] = 0x82;
    for (unsigned y = 0; y < 8u; ++y) {
        rom[16u + 16384u + 16u + y] = 0xffu;
        rom[16u + 16384u + 32u + y] = (uint8_t)(y & 1u ? 0xaau : 0x55u);
        rom[16u + 16384u + 48u + y] = (uint8_t)(y & 1u ? 0x55u : 0xaau);
    }
#undef EMIT
}

static nesturbator *marker_instance(const uint8_t *rom)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    if (nesturbator_create(&cfg, &inst) != NESTURBATOR_OK)
        return NULL;
    if (nesturbator_load_cartridge(inst, rom, MARKER_ROM_SIZE) != NESTURBATOR_OK) {
        nesturbator_destroy(inst);
        return NULL;
    }
    return inst;
}

static int marker_frame(nesturbator *inst, uint8_t *rgb, uint64_t *absolute)
{
    static uint16_t video[W_PX * H_PX];
    static uint32_t xrgb[W_PX * H_PX];
    static int16_t audio[4096];
    uint32_t palette[512];
    nesturbator_frame io;
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = video;
    io.video_pitch = W_PX;
    io.audio = audio;
    io.audio_capacity = 4096u;
    if (nesturbator_run_frame(inst, &io) != NESTURBATOR_OK)
        return 0;
    *absolute = io.frame_number;
    nesturbator_get_palette(inst, palette, 512u);
    nesturbator_host_convert(palette, video, W_PX, xrgb, W_PX);
    for (size_t i = 0; i < W_PX * H_PX; ++i) {
        rgb[i * 3u] = (uint8_t)(xrgb[i] >> 16);
        rgb[i * 3u + 1u] = (uint8_t)(xrgb[i] >> 8);
        rgb[i * 3u + 2u] = (uint8_t)xrgb[i];
    }
    return 1;
}

/* Measure startup again when reading a calibration: the file cannot choose its own mapping. */
static int marker_offset(const uint8_t *rom, uint32_t *offset)
{
    nesturbator *inst = marker_instance(rom);
    static uint8_t rgb[NESTURBATOR_TEST_RGB_SIZE];
    uint32_t palette[512], value = 0;
    uint64_t absolute = 0;
    int ok = inst != NULL;
    if (ok)
        nesturbator_get_palette(inst, palette, 512u);
    for (unsigned n = 1; ok && n <= MARKER_FIRST + 2u; ++n) {
        ok = marker_frame(inst, rgb, &absolute) && absolute == n;
        const char *reason = marker_decode(rgb, palette, &value);
        if (n < MARKER_FIRST)
            ok = ok && (reason != NULL || value == 0u);
        else if (n == MARKER_FIRST) {
            ok = ok && reason == NULL && value > 0u && value < n;
            *offset = n - value;
        } else
            ok = ok && reason == NULL && value + *offset == n;
    }
    nesturbator_destroy(inst);
    return ok;
}

static void marker_calibration(char *text, size_t cap, uint32_t offset)
{
    snprintf(text, cap,
             "marker_version=1\nfirst=%u\nlast=%u\nabsolute_minus_guest=%u\n"
             "behavior_revision=%u\n",
             MARKER_FIRST, MARKER_LAST, offset, NESTURBATOR_BEHAVIOUR_REVISION);
}

static uint64_t marker_le(const uint8_t *p, unsigned n)
{
    uint64_t value = 0;
    for (unsigned i = 0; i < n; ++i)
        value |= (uint64_t)p[i] << (8u * i);
    return value;
}

/* Exact observed 1.22.2 uncompressed envelope. Native validation owns all machine invariants. */
static const char *marker_anchor(nesturbator *inst, const uint8_t *bytes, size_t length,
                                 uint64_t expected, uint32_t offset)
{
    size_t size = nesturbator_state_size(inst);
    size_t padded = (size + 7u) & ~(size_t)7u;
    if (length != 16u + padded + 8u)
        return "container-size";
    if (memcmp(bytes, "RASTATE\001MEM ", 12u) != 0 || marker_le(bytes + 12u, 4u) != size)
        return "container-header";
    for (size_t i = size; i < padded; ++i)
        if (bytes[16u + i] != 0u)
            return "container-padding";
    if (memcmp(bytes + 16u + padded, "END \0\0\0\0", 8u) != 0)
        return "container-end";
    const uint8_t *native = bytes + 16u;
    nesturbator_status status = nesturbator_load_state(inst, native, size);
    if (status != NESTURBATOR_OK) {
        if (status == NESTURBATOR_ERR_STATE_FORMAT)
            return "native-format";
        if (status == NESTURBATOR_ERR_STATE_CARTRIDGE)
            return "native-content";
        if (status == NESTURBATOR_ERR_STATE_CHECKSUM)
            return "native-checksum";
        return "native-invalid";
    }
    if (marker_le(native + 16u, 4u) != NESTURBATOR_BEHAVIOUR_REVISION)
        return "native-revision";
    if (expected < MARKER_FIRST || expected >= MARKER_LAST ||
        marker_le(native + 32u, 8u) != expected)
        return "native-origin";
    static uint8_t rgb[NESTURBATOR_TEST_RGB_SIZE];
    uint32_t palette[512], value = 0;
    uint64_t absolute;
    nesturbator_get_palette(inst, palette, 512u);
    if (!marker_frame(inst, rgb, &absolute) || absolute != expected + 1u)
        return "native-continuation";
    const char *reason = marker_decode(rgb, palette, &value);
    if (reason != NULL || value + offset != absolute)
        return "native-marker";
    return NULL;
}

/* The qualification transition verdict is also consumed by independently authored controls. */
static const char *marker_progress(uint32_t before, uint32_t after, uint32_t steps)
{
    if (steps > 0u && after == before)
        return "duplicate-identity";
    if (after < before || after - before != steps)
        return "progression";
    return NULL;
}

static const char *marker_pixels(const uint8_t *rgb, const uint32_t *palette, uint32_t offset,
                                 uint32_t *absolute)
{
    uint32_t value = 0;
    const char *reason = marker_decode(rgb, palette, &value);
    if (reason != NULL)
        return reason;
    if (value + offset < MARKER_FIRST || value + offset > MARKER_LAST)
        return "window";
    *absolute = value + offset;
    return NULL;
}

static const char *marker_image(const char *path, const uint32_t *palette, uint32_t offset,
                                uint32_t *absolute)
{
    static uint8_t rgb[NESTURBATOR_TEST_RGB_SIZE];
    int read_result;
    if (has_suffix(path, ".ppm"))
        read_result = nesturbator_test_read_ppm(path, rgb);
    else {
        uint32_t w = 0, h = 0;
        read_result = nesturbator_test_read_bmp(path, &w, &h, rgb, sizeof rgb);
        if (w != W_PX || h != H_PX)
            read_result = 1;
    }
    return read_result != 0 ? "image-geometry-or-format"
                            : marker_pixels(rgb, palette, offset, absolute);
}

static int marker_expect(const char *name, const char *actual, const char *expected,
                         unsigned *count)
{
    int ok = actual == NULL ? expected == NULL : expected != NULL && strcmp(actual, expected) == 0;
    printf("control %s expected=%s actual=%s %s\n", name, expected != NULL ? expected : "pass",
           actual != NULL ? actual : "pass", ok ? "PASS" : "FAIL");
    ++*count;
    return ok;
}

static int progression_make_path(char *path, size_t capacity, const char *work, const char *name)
{
    int length = snprintf(path, capacity, "%s/%s", work, name);
    return length >= 0 && (size_t)length < capacity;
}

static int progression_write_ppm(const char *path, const uint8_t *rgb)
{
    static const char header[] = "P6\n256 240\n255\n";
    FILE *file = fopen(path, "wb");
    if (file == NULL)
        return 0;
    int ok = fwrite(header, 1u, sizeof header - 1u, file) == sizeof header - 1u &&
             fwrite(rgb, 1u, NESTURBATOR_TEST_RGB_SIZE, file) == NESTURBATOR_TEST_RGB_SIZE;
    return fclose(file) == 0 && ok;
}

static int progression_write_path(FILE *file, const char *length_key, const char *path_key,
                                  const char *path)
{
    size_t length = strlen(path);
    return strchr(path, '\n') == NULL && strchr(path, '\r') == NULL &&
           fprintf(file, "%s=%zu\n%s=%s\n", length_key, length, path_key, path) >= 0;
}

static int progression_write_record(const char *path, const progression_session *session)
{
    FILE *file = fopen(path, "wb");
    if (file == NULL)
        return 0;
    int ok = 1;
#define PROGRESSION_PRINT(...)                                                                     \
    do {                                                                                           \
        if (ok && fprintf(file, __VA_ARGS__) < 0)                                                  \
            ok = 0;                                                                                \
    } while (0)
    PROGRESSION_PRINT("NESTURBATOR-PROGRESSION 1\nmode=%s\ncontent_identity=%016" PRIx64
                      "\ncalibration_identity=%016" PRIx64 "\nterminal_exit=",
                      session->mode == PROGRESSION_PREFIX ? "prefix" : "complete",
                      session->content_identity, session->calibration_identity);
    PROGRESSION_PRINT("%s\n", session->terminal_exit < 0 ? "running" : "0");
    if (session->terminal_exit > 0 && ok) {
        /* Test fixtures only use a live prefix or a clean completed session. */
        ok = 0;
    }
    if (ok)
        ok = progression_write_path(file, "original_anchor_path_length", "original_anchor_path",
                                    session->original_anchor_path);
    PROGRESSION_PRINT("original_anchor_identity=%016" PRIx64 "\noriginal_anchor_frame=%" PRIu64
                      "\n",
                      session->original_anchor_identity, session->original_anchor_frame);
    if (ok)
        ok = progression_write_path(file, "rewound_anchor_path_length", "rewound_anchor_path",
                                    session->rewound_anchor_path);
    PROGRESSION_PRINT("rewound_anchor_identity=%016" PRIx64 "\nrewound_anchor_frame=%" PRIu64
                      "\nboundary_count=%u\n",
                      session->rewound_anchor_identity, session->rewound_anchor_frame,
                      session->boundary_count);
    for (uint32_t i = 0; ok && i < session->boundary_count; ++i) {
        const progression_boundary *b = &session->boundaries[i];
        const char *operation = progression_operation_name(b->operation);
        const char *relation = progression_relation_name(b->relation);
        const char *origin = progression_origin_name(b->origin);
        if (operation == NULL || relation == NULL || origin == NULL) {
            ok = 0;
            break;
        }
        PROGRESSION_PRINT("boundary\nid=%u\noperation=%s\nrelation=%s\norigin=%s\n"
                          "command_count=%u\nsend_count=%u\nartifact_identity=%" PRIu64 "\n",
                          b->id, operation, relation, origin, b->command_count, b->send_count,
                          b->artifact_identity);
        if (ok)
            ok = progression_write_path(file, "artifact_path_length", "artifact_path",
                                        b->artifact_path);
        PROGRESSION_PRINT(
            "artifact_hash=%016" PRIx64 "\ndisplay_frame=%" PRIu64
            "\ncandidate_count=%u\ncandidate_first=%" PRIu64 "\ncandidate_last=%" PRIu64
            "\nanchor_identity=%016" PRIx64 "\nanchor_frame=%" PRIu64
            "\ndisplay_lead=%d\nexpected_delta=%u\n"
            "elapsed_ms=%" PRIu64 "\ndeadline_ms=%" PRIu64 "\nprocess_alive=%d\nboundary_end\n",
            b->artifact_hash, b->display_frame, b->candidate_count, b->candidate_first,
            b->candidate_last, b->anchor_identity, b->anchor_frame, b->display_lead,
            b->expected_delta, b->elapsed_ms, b->deadline_ms, b->process_alive);
    }
    PROGRESSION_PRINT("END\n");
#undef PROGRESSION_PRINT
    if (fclose(file) != 0)
        ok = 0;
    return ok;
}

static int progression_write_anchor(const char *path, const uint8_t *native, size_t native_size,
                                    uint64_t *identity)
{
    if (native_size > UINT32_MAX)
        return 0;
    size_t padded_size = (native_size + 7u) & ~(size_t)7u;
    size_t envelope_size = 16u + padded_size + 8u;
    uint8_t *envelope = (uint8_t *)calloc(envelope_size, 1u);
    if (envelope == NULL)
        return 0;
    memcpy(envelope, "RASTATE\001MEM ", 12u);
    put32(envelope + 12u, (uint32_t)native_size);
    memcpy(envelope + 16u, native, native_size);
    memcpy(envelope + 16u + padded_size, "END ", 4u);
    int ok = marker_write(path, envelope, envelope_size) &&
             ((*identity = progression_hash64(envelope, envelope_size)) != 0u);
    free(envelope);
    return ok;
}

static int progression_check_current_image(nesturbator *inst, const uint8_t *rgb,
                                           uint64_t expected_frame, uint32_t offset)
{
    uint32_t palette[512], observed = 0;
    nesturbator_get_palette(inst, palette, 512u);
    const char *reason = marker_pixels(rgb, palette, offset, &observed);
    if (reason != NULL || observed != expected_frame) {
        fprintf(stderr, "progression fixture image: expected=%" PRIu64 " actual=%u reason=%s\n",
                expected_frame, observed, reason != NULL ? reason : "frame");
        return 0;
    }
    return 1;
}

static int progression_save_current_image(nesturbator *inst, const uint8_t *rgb, const char *path,
                                          uint64_t expected_frame, uint32_t offset)
{
    return progression_check_current_image(inst, rgb, expected_frame, offset) &&
           progression_write_ppm(path, rgb);
}

static int marker_progression_fixtures(const char *work)
{
    static uint8_t rom[MARKER_ROM_SIZE];
    static uint8_t rgb[NESTURBATOR_TEST_RGB_SIZE];
    static uint8_t pause_rgb[NESTURBATOR_TEST_RGB_SIZE];
    static uint8_t advance_rgb[NESTURBATOR_TEST_RGB_SIZE];
    char rom_path[PROGRESSION_PATH_CAP], calibration_path[PROGRESSION_PATH_CAP];
    char anchor_path[PROGRESSION_PATH_CAP], pause_one[PROGRESSION_PATH_CAP];
    char pause_two[PROGRESSION_PATH_CAP], advance_path[PROGRESSION_PATH_CAP];
    char wrong_anchor_path[PROGRESSION_PATH_CAP];
    char record_path[PROGRESSION_PATH_CAP];
    if (!progression_make_path(rom_path, sizeof rom_path, work, "marker.nes") ||
        !progression_make_path(calibration_path, sizeof calibration_path, work,
                               "calibration.txt") ||
        !progression_make_path(anchor_path, sizeof anchor_path, work,
                               "retained original anchor.ra") ||
        !progression_make_path(wrong_anchor_path, sizeof wrong_anchor_path, work,
                               "valid wrong origin.ra") ||
        !progression_make_path(pause_one, sizeof pause_one, work, "pause capture one.ppm") ||
        !progression_make_path(pause_two, sizeof pause_two, work, "pause capture two.ppm") ||
        !progression_make_path(advance_path, sizeof advance_path, work, "advance one.ppm") ||
        !progression_make_path(record_path, sizeof record_path, work, "progression-prefix.rec"))
        return 0;

    marker_rom(rom);
    uint32_t offset = 0;
    if (!marker_offset(rom, &offset) || !marker_write(rom_path, rom, sizeof rom))
        return 0;
    char calibration[256];
    marker_calibration(calibration, sizeof calibration, offset);
    if (!marker_write(calibration_path, calibration, strlen(calibration)))
        return 0;

    nesturbator *inst = marker_instance(rom);
    if (inst == NULL)
        return 0;
    size_t native_size = nesturbator_state_size(inst);
    size_t padded_size = (native_size + 7u) & ~(size_t)7u;
    size_t envelope_size = 16u + padded_size + 8u;
    uint8_t *native = (uint8_t *)malloc(native_size);
    uint8_t *wrong_native = (uint8_t *)malloc(native_size);
    uint8_t *envelope = (uint8_t *)calloc(envelope_size, 1u);
    uint8_t *wrong_envelope = (uint8_t *)calloc(envelope_size, 1u);
    if (native == NULL || wrong_native == NULL || envelope == NULL || wrong_envelope == NULL)
        return 0;
    uint64_t absolute = 0;
    for (unsigned frame = 1; frame <= 150u; ++frame) {
        if (!marker_frame(inst, rgb, &absolute) || absolute != frame)
            return 0;
        if (frame == 149u &&
            nesturbator_save_state(inst, wrong_native, native_size) != NESTURBATOR_OK)
            return 0;
        if (frame == 150u) {
            memcpy(pause_rgb, rgb, sizeof pause_rgb);
            if (nesturbator_save_state(inst, native, native_size) != NESTURBATOR_OK)
                return 0;
        }
    }
    memcpy(envelope, "RASTATE\001MEM ", 12u);
    put32(envelope + 12u, (uint32_t)native_size);
    memcpy(envelope + 16u, native, native_size);
    memcpy(envelope + 16u + padded_size, "END ", 4u);
    memcpy(wrong_envelope, "RASTATE\001MEM ", 12u);
    put32(wrong_envelope + 12u, (uint32_t)native_size);
    memcpy(wrong_envelope + 16u, wrong_native, native_size);
    memcpy(wrong_envelope + 16u + padded_size, "END ", 4u);
    uint64_t anchor_identity = progression_hash64(envelope, envelope_size);
    if (!marker_write(anchor_path, envelope, envelope_size) ||
        !marker_write(wrong_anchor_path, wrong_envelope, envelope_size) ||
        !progression_write_ppm(pause_one, pause_rgb) ||
        !progression_write_ppm(pause_two, pause_rgb))
        return 0;

    nesturbator *anchor_validator = marker_instance(rom);
    if (anchor_validator == NULL ||
        marker_anchor(anchor_validator, envelope, envelope_size, 150u, offset) != NULL)
        return 0;
    nesturbator_destroy(anchor_validator);
    anchor_validator = marker_instance(rom);
    if (anchor_validator == NULL ||
        marker_anchor(anchor_validator, wrong_envelope, envelope_size, 149u, offset) != NULL)
        return 0;
    nesturbator_destroy(anchor_validator);
    nesturbator *continuation = marker_instance(rom);
    if (continuation == NULL ||
        nesturbator_load_state(continuation, native, native_size) != NESTURBATOR_OK ||
        !marker_frame(continuation, rgb, &absolute) || absolute != 151u)
        return 0;
    uint32_t continuation_palette[512], observed = 0;
    nesturbator_get_palette(continuation, continuation_palette, 512u);
    const char *continuation_reason = marker_pixels(rgb, continuation_palette, offset, &observed);
    if (continuation_reason != NULL || observed != 151u) {
        fprintf(stderr, "progression fixture continuation: expected 151, got %u (%s)\n", observed,
                continuation_reason != NULL ? continuation_reason : "frame");
        return 0;
    }
    memcpy(advance_rgb, rgb, sizeof advance_rgb);
    if (!progression_write_ppm(advance_path, advance_rgb) ||
        marker_image(advance_path, continuation_palette, offset, &observed) != NULL ||
        observed != 151u) {
        fprintf(stderr, "progression fixture image: expected decoded 151, got %u\n", observed);
        return 0;
    }
    nesturbator_destroy(continuation);
    nesturbator_destroy(inst);
    free(native);
    free(wrong_native);
    free(envelope);
    free(wrong_envelope);

    progression_session session;
    memset(&session, 0, sizeof session);
    session.mode = PROGRESSION_PREFIX;
    session.content_identity = progression_hash64(rom, sizeof rom);
    session.calibration_identity = progression_hash64(calibration, strlen(calibration));
    session.terminal_exit = -1;
    snprintf(session.original_anchor_path, sizeof session.original_anchor_path, "%s", anchor_path);
    session.original_anchor_identity = anchor_identity;
    session.original_anchor_frame = 150u;
    session.boundary_count = 3u;
    const char *artifact_paths[] = {pause_one, pause_two, advance_path};
    const uint64_t frames[] = {150u, 150u, 151u};
    for (uint32_t i = 0; i < session.boundary_count; ++i) {
        progression_boundary *b = &session.boundaries[i];
        b->id = i + 1u;
        b->operation = i < 2u ? PROGRESSION_CAPTURE : PROGRESSION_ADVANCE;
        b->relation = i < 2u ? PROGRESSION_PAUSE_EQUAL_ORIGINAL : PROGRESSION_ORIGINAL_PLUS;
        b->origin = PROGRESSION_ORIGIN_ORIGINAL;
        b->command_count = 1u;
        b->send_count = 1u;
        b->artifact_identity = i + 1u;
        snprintf(b->artifact_path, sizeof b->artifact_path, "%s", artifact_paths[i]);
        if (!progression_hash_file(b->artifact_path, &b->artifact_hash))
            return 0;
        b->display_frame = frames[i];
        b->candidate_count = 1u;
        b->candidate_first = frames[i];
        b->candidate_last = frames[i];
        b->anchor_identity = anchor_identity;
        b->anchor_frame = 150u;
        b->expected_delta = i == 2u ? 1u : 0u;
        b->elapsed_ms = 1u;
        b->deadline_ms = 1000u;
        b->process_alive = 1;
    }
    if (!progression_write_record(record_path, &session))
        return 0;
    printf("progression fixture files: ready (3 decoded boundaries, retained native origin 150)\n");
    return 1;
}

static int marker_progression_fixtures(const char *work);
static int marker_progression_cli(const char *rom_path, const char *calibration_path,
                                  const char *record_path, int prefix);

static const char *progression_validate_anchor(nesturbator *inst, const char *path,
                                               uint64_t identity, uint64_t frame, uint32_t offset)
{
    uint64_t actual_identity = 0;
    if (path[0] == '\0' || identity == 0u)
        return "ORIGIN";
    if (!progression_hash_file(path, &actual_identity))
        return "IO";
    if (actual_identity != identity)
        return "ORIGIN";
    size_t state_size = nesturbator_state_size(inst);
    size_t limit = 16u + ((state_size + 7u) & ~(size_t)7u) + 8u;
    size_t size = 0;
    uint8_t *bytes = marker_read(path, limit, &size);
    if (bytes == NULL)
        return "IO";
    const char *reason = marker_anchor(inst, bytes, size, frame, offset);
    free(bytes);
    return reason == NULL ? NULL : progression_anchor_reason(reason) == 2 ? "PARSE" : "ORIGIN";
}

static const char *progression_palette_for_frame(const uint8_t *rom, const char *path,
                                                 uint64_t identity, uint64_t anchor_frame,
                                                 uint64_t target_frame, uint32_t offset,
                                                 uint32_t *palette)
{
    uint64_t actual_identity = 0;
    if (target_frame < anchor_frame)
        return "PROGRESSION";
    if (target_frame > MARKER_LAST)
        return "IDENTITY";
    if (!progression_hash_file(path, &actual_identity))
        return "IO";
    if (actual_identity != identity)
        return "ORIGIN";
    nesturbator *shape = marker_instance(rom);
    if (shape == NULL)
        return "IO";
    size_t state_size = nesturbator_state_size(shape);
    size_t limit = 16u + ((state_size + 7u) & ~(size_t)7u) + 8u;
    nesturbator_destroy(shape);
    size_t size = 0;
    uint8_t *bytes = marker_read(path, limit, &size);
    if (bytes == NULL)
        return "IO";
    nesturbator *validator = marker_instance(rom);
    if (validator == NULL) {
        free(bytes);
        return "IO";
    }
    const char *reason = marker_anchor(validator, bytes, size, anchor_frame, offset);
    nesturbator_destroy(validator);
    if (reason != NULL) {
        free(bytes);
        return progression_anchor_reason(reason) == 2 ? "PARSE" : "ORIGIN";
    }
    nesturbator *simulator = marker_instance(rom);
    if (simulator == NULL) {
        free(bytes);
        return "IO";
    }
    static uint8_t rgb[NESTURBATOR_TEST_RGB_SIZE];
    uint64_t absolute = 0;
    for (uint64_t frame = 1u; frame <= anchor_frame; ++frame) {
        if (!marker_frame(simulator, rgb, &absolute) || absolute != frame) {
            nesturbator_destroy(simulator);
            free(bytes);
            return "PROCESS";
        }
    }
    if (nesturbator_load_state(simulator, bytes + 16u, state_size) != NESTURBATOR_OK) {
        nesturbator_destroy(simulator);
        free(bytes);
        return "ORIGIN";
    }
    free(bytes);
    nesturbator_get_palette(simulator, palette, 512u);
    absolute = anchor_frame;
    while (absolute < target_frame) {
        if (!marker_frame(simulator, rgb, &absolute)) {
            nesturbator_destroy(simulator);
            return "PROCESS";
        }
    }
    nesturbator_get_palette(simulator, palette, 512u);
    uint32_t observed = 0;
    reason = marker_pixels(rgb, palette, offset, &observed);
    nesturbator_destroy(simulator);
    if (reason != NULL)
        return "IDENTITY";
    return observed == target_frame ? NULL : "PROGRESSION";
}

static int progression_run_to(nesturbator *inst, uint64_t target, uint8_t *rgb)
{
    uint64_t absolute = 0;
    for (uint64_t frame = 1u; frame <= target; ++frame)
        if (!marker_frame(inst, rgb, &absolute) || absolute != frame)
            return 0;
    return 1;
}

static int progression_full_fixtures(const char *work)
{
    static uint8_t rom[MARKER_ROM_SIZE];
    static uint8_t rgb[NESTURBATOR_TEST_RGB_SIZE];
    static const uint64_t frames[PROGRESSION_REQUIRED_BOUNDARIES] = {
        150u, 150u, 151u, 152u, 153u, 154u, 155u, 151u,
        152u, 153u, 154u, 130u, 131u, 132u, 133u, 132u};
    static const char *const names[PROGRESSION_REQUIRED_BOUNDARIES] = {
        "boundary 01 pause a.ppm",           "boundary 02 pause b.ppm",
        "boundary 03 advance 1.ppm",         "boundary 04 advance 2.ppm",
        "boundary 05 advance 3.ppm",         "boundary 06 advance 4.ppm",
        "boundary 07 advance 5.ppm",         "boundary 08 load probe.ppm",
        "boundary 09 post load 2.ppm",       "boundary 10 post load 3.ppm",
        "boundary 11 resume forward.ppm",    "boundary 12 rewind coarse.ppm",
        "boundary 13 rewound advance 1.ppm", "boundary 14 rewound advance 2.ppm",
        "boundary 15 rewound advance 3.ppm", "boundary 16 runahead probe.ppm"};
    char rom_path[PROGRESSION_PATH_CAP], calibration_path[PROGRESSION_PATH_CAP];
    char original_path[PROGRESSION_PATH_CAP], rewound_path[PROGRESSION_PATH_CAP];
    char alternative_path[PROGRESSION_PATH_CAP], alternative_image_path[PROGRESSION_PATH_CAP];
    char wrong_path[PROGRESSION_PATH_CAP], record_path[PROGRESSION_PATH_CAP];
    char artifact_paths[PROGRESSION_REQUIRED_BOUNDARIES][PROGRESSION_PATH_CAP];
    if (!marker_progression_fixtures(work) ||
        !progression_make_path(rom_path, sizeof rom_path, work, "marker.nes") ||
        !progression_make_path(calibration_path, sizeof calibration_path, work,
                               "calibration.txt") ||
        !progression_make_path(original_path, sizeof original_path, work,
                               "retained original anchor.ra") ||
        !progression_make_path(rewound_path, sizeof rewound_path, work,
                               "retained rewound anchor.ra") ||
        !progression_make_path(alternative_path, sizeof alternative_path, work,
                               "alternative same content anchor.ra") ||
        !progression_make_path(alternative_image_path, sizeof alternative_image_path, work,
                               "alternative frame 101.ppm") ||
        !progression_make_path(wrong_path, sizeof wrong_path, work, "valid wrong origin.ra") ||
        !progression_make_path(record_path, sizeof record_path, work, "progression-complete.rec"))
        return 0;
    for (uint32_t i = 0; i < PROGRESSION_REQUIRED_BOUNDARIES; ++i)
        if (!progression_make_path(artifact_paths[i], sizeof artifact_paths[i], work, names[i]))
            return 0;

    marker_rom(rom);
    uint32_t offset = 0;
    if (!marker_offset(rom, &offset))
        return 0;
    char calibration[256];
    marker_calibration(calibration, sizeof calibration, offset);
    uint64_t original_identity = 0, wrong_identity = 0, rewound_identity = 0;
    uint64_t alternative_identity = 0;
    if (!progression_hash_file(original_path, &original_identity) ||
        !progression_hash_file(wrong_path, &wrong_identity))
        return 0;

    nesturbator *inst = marker_instance(rom);
    if (inst == NULL)
        return 0;
    size_t native_size = nesturbator_state_size(inst);
    uint8_t *original_native = (uint8_t *)malloc(native_size);
    uint8_t *rewound_native = (uint8_t *)malloc(native_size);
    uint8_t *alternative_native = (uint8_t *)malloc(native_size);
    if (original_native == NULL || rewound_native == NULL || alternative_native == NULL)
        return 0;
    uint64_t absolute = 0;
    for (uint64_t frame = 1u; frame <= 155u; ++frame) {
        if (!marker_frame(inst, rgb, &absolute) || absolute != frame)
            return 0;
        if (frame == 130u &&
            nesturbator_save_state(inst, rewound_native, native_size) != NESTURBATOR_OK)
            return 0;
        if (frame == 100u &&
            nesturbator_save_state(inst, alternative_native, native_size) != NESTURBATOR_OK)
            return 0;
        if (frame == 150u &&
            nesturbator_save_state(inst, original_native, native_size) != NESTURBATOR_OK)
            return 0;
        if ((frame >= 151u && frame <= 155u) || frame == 130u || frame == 150u || frame == 101u) {
            uint32_t palette[512], observed = 0;
            nesturbator_get_palette(inst, palette, 512u);
            const char *reason = marker_pixels(rgb, palette, offset, &observed);
            if (reason != NULL || observed != frame) {
                fprintf(stderr, "progression full fixture frame=%" PRIu64 " reason=%s\n", frame,
                        reason != NULL ? reason : "identity");
                return 0;
            }
            if (frame == 130u && !progression_write_ppm(artifact_paths[11], rgb))
                return 0;
            if (frame == 101u && !progression_write_ppm(alternative_image_path, rgb))
                return 0;
            if (frame == 150u && (!progression_write_ppm(artifact_paths[0], rgb) ||
                                  !progression_write_ppm(artifact_paths[1], rgb)))
                return 0;
            if (frame >= 151u && frame <= 155u &&
                !progression_write_ppm(artifact_paths[(uint32_t)(frame - 149u)], rgb))
                return 0;
        }
    }
    nesturbator_destroy(inst);
    if (!progression_write_anchor(rewound_path, rewound_native, native_size, &rewound_identity) ||
        !progression_write_anchor(alternative_path, alternative_native, native_size,
                                  &alternative_identity))
        return 0;

    nesturbator *validator = marker_instance(rom);
    if (validator == NULL)
        return 0;
    const char *origin_reason =
        progression_validate_anchor(validator, original_path, original_identity, 150u, offset);
    if (origin_reason == NULL)
        origin_reason =
            progression_validate_anchor(validator, rewound_path, rewound_identity, 130u, offset);
    if (origin_reason == NULL)
        origin_reason =
            progression_validate_anchor(validator, wrong_path, wrong_identity, 149u, offset);
    if (origin_reason == NULL)
        origin_reason = progression_validate_anchor(validator, alternative_path,
                                                    alternative_identity, 100u, offset);
    nesturbator_destroy(validator);
    if (origin_reason != NULL) {
        fprintf(stderr, "progression full fixture native precondition: %s\n", origin_reason);
        return 0;
    }

    nesturbator *probe = marker_instance(rom);
    if (probe == NULL || !progression_run_to(probe, 150u, rgb) ||
        nesturbator_load_state(probe, original_native, native_size) != NESTURBATOR_OK)
        return 0;
    for (uint32_t i = 0; i < 4u; ++i) {
        uint64_t expected = 151u + i;
        if (!marker_frame(probe, rgb, &absolute) || absolute != expected ||
            !progression_save_current_image(probe, rgb, artifact_paths[7u + i], expected, offset))
            return 0;
    }
    nesturbator_destroy(probe);

    nesturbator *rewind = marker_instance(rom);
    if (rewind == NULL || !progression_run_to(rewind, 130u, rgb) ||
        nesturbator_load_state(rewind, rewound_native, native_size) != NESTURBATOR_OK)
        return 0;
    for (uint32_t i = 0; i < 3u; ++i) {
        uint64_t expected = 131u + i;
        if (!marker_frame(rewind, rgb, &absolute) || absolute != expected ||
            !progression_save_current_image(rewind, rgb, artifact_paths[12u + i], expected, offset))
            return 0;
    }
    nesturbator_destroy(rewind);

    nesturbator *ahead = marker_instance(rom);
    if (ahead == NULL || !progression_run_to(ahead, 130u, rgb) ||
        nesturbator_load_state(ahead, rewound_native, native_size) != NESTURBATOR_OK ||
        !marker_frame(ahead, rgb, &absolute) || absolute != 131u ||
        !progression_check_current_image(ahead, rgb, 131u, offset) ||
        !marker_frame(ahead, rgb, &absolute) || absolute != 132u ||
        !progression_save_current_image(ahead, rgb, artifact_paths[15], 132u, offset))
        return 0;
    nesturbator_destroy(ahead);

    progression_session session;
    memset(&session, 0, sizeof session);
    session.mode = PROGRESSION_COMPLETE;
    session.content_identity = progression_hash64(rom, sizeof rom);
    session.calibration_identity = progression_hash64(calibration, strlen(calibration));
    session.terminal_exit = 0;
    snprintf(session.original_anchor_path, sizeof session.original_anchor_path, "%s",
             original_path);
    session.original_anchor_identity = original_identity;
    session.original_anchor_frame = 150u;
    snprintf(session.rewound_anchor_path, sizeof session.rewound_anchor_path, "%s", rewound_path);
    session.rewound_anchor_identity = rewound_identity;
    session.rewound_anchor_frame = 130u;
    session.boundary_count = PROGRESSION_REQUIRED_BOUNDARIES;
    static const progression_operation operations[PROGRESSION_REQUIRED_BOUNDARIES] = {
        PROGRESSION_CAPTURE,          PROGRESSION_CAPTURE,
        PROGRESSION_ADVANCE,          PROGRESSION_ADVANCE,
        PROGRESSION_ADVANCE,          PROGRESSION_ADVANCE,
        PROGRESSION_ADVANCE,          PROGRESSION_LOAD_PROBE,
        PROGRESSION_ADVANCE,          PROGRESSION_ADVANCE,
        PROGRESSION_RESUME_AND_PAUSE, PROGRESSION_REWIND_BURST,
        PROGRESSION_ADVANCE,          PROGRESSION_ADVANCE,
        PROGRESSION_ADVANCE,          PROGRESSION_RUNAHEAD_LOAD_PROBE};
    static const progression_relation relations[PROGRESSION_REQUIRED_BOUNDARIES] = {
        PROGRESSION_PAUSE_EQUAL_ORIGINAL, PROGRESSION_PAUSE_EQUAL_ORIGINAL,
        PROGRESSION_ORIGINAL_PLUS,        PROGRESSION_ORIGINAL_PLUS,
        PROGRESSION_ORIGINAL_PLUS,        PROGRESSION_ORIGINAL_PLUS,
        PROGRESSION_ORIGINAL_PLUS,        PROGRESSION_ORIGINAL_LOAD_PROBE,
        PROGRESSION_ORIGINAL_POST_LOAD,   PROGRESSION_ORIGINAL_POST_LOAD,
        PROGRESSION_RESUME_FORWARD,       PROGRESSION_REWIND_COARSE,
        PROGRESSION_REWOUND_PLUS,         PROGRESSION_REWOUND_PLUS,
        PROGRESSION_REWOUND_PLUS,         PROGRESSION_REWOUND_LOAD_PROBE};
    static const progression_origin origins[PROGRESSION_REQUIRED_BOUNDARIES] = {
        PROGRESSION_ORIGIN_ORIGINAL, PROGRESSION_ORIGIN_ORIGINAL, PROGRESSION_ORIGIN_ORIGINAL,
        PROGRESSION_ORIGIN_ORIGINAL, PROGRESSION_ORIGIN_ORIGINAL, PROGRESSION_ORIGIN_ORIGINAL,
        PROGRESSION_ORIGIN_ORIGINAL, PROGRESSION_ORIGIN_ORIGINAL, PROGRESSION_ORIGIN_ORIGINAL,
        PROGRESSION_ORIGIN_ORIGINAL, PROGRESSION_ORIGIN_ORIGINAL, PROGRESSION_ORIGIN_REWOUND,
        PROGRESSION_ORIGIN_REWOUND,  PROGRESSION_ORIGIN_REWOUND,  PROGRESSION_ORIGIN_REWOUND,
        PROGRESSION_ORIGIN_REWOUND};
    static const uint32_t commands[PROGRESSION_REQUIRED_BOUNDARIES] = {
        1u, 1u, 1u, 1u, 1u, 1u, 1u, 2u, 1u, 1u, 2u, 40u, 1u, 1u, 1u, 2u};
    static const uint32_t deltas[PROGRESSION_REQUIRED_BOUNDARIES] = {
        0u, 0u, 1u, 2u, 3u, 4u, 5u, 1u, 2u, 3u, 0u, 0u, 1u, 2u, 3u, 1u};
    for (uint32_t i = 0; i < PROGRESSION_REQUIRED_BOUNDARIES; ++i) {
        progression_boundary *b = &session.boundaries[i];
        b->id = i + 1u;
        b->operation = operations[i];
        b->relation = relations[i];
        b->origin = origins[i];
        b->command_count = commands[i];
        b->send_count = commands[i];
        b->artifact_identity = i + 1u;
        snprintf(b->artifact_path, sizeof b->artifact_path, "%s", artifact_paths[i]);
        if (!progression_hash_file(b->artifact_path, &b->artifact_hash))
            return 0;
        b->display_frame = frames[i];
        b->candidate_count = 1u;
        b->candidate_first = frames[i];
        b->candidate_last = frames[i];
        b->anchor_identity =
            origins[i] == PROGRESSION_ORIGIN_ORIGINAL ? original_identity : rewound_identity;
        b->anchor_frame = origins[i] == PROGRESSION_ORIGIN_ORIGINAL ? 150u : 130u;
        b->display_lead = i == 15u ? 1 : 0;
        b->expected_delta = deltas[i];
        b->elapsed_ms = 1u;
        b->deadline_ms = 1000u;
        b->process_alive = 1;
    }
    free(original_native);
    free(rewound_native);
    free(alternative_native);
    if (!progression_write_record(record_path, &session))
        return 0;
    printf(
        "progression full fixture: ready (16 decoded boundaries, retained anchors 150 and 130)\n");
    return 1;
}

static int progression_control_expect(const char *name, const progression_session *session,
                                      const char *expected, unsigned *count)
{
    return marker_expect(name, progression_verdict(session, 1), expected, count);
}

static int progression_control_exit_expect(const char *name, int actual, int expected,
                                           unsigned *count)
{
    char actual_text[32], expected_text[32];
    snprintf(actual_text, sizeof actual_text, "exit %d", actual);
    snprintf(expected_text, sizeof expected_text, "exit %d", expected);
    return marker_expect(name, actual_text, expected_text, count);
}

static int marker_progression_cli(const char *rom_path, const char *calibration_path,
                                  const char *record_path, int prefix)
{
    static uint8_t expected_rom[MARKER_ROM_SIZE];
    marker_rom(expected_rom);
    uint32_t offset = 0;
    if (!marker_offset(expected_rom, &offset)) {
        fprintf(stderr, "marker: IDENTITY (startup calibration)\n");
        return 2;
    }
    char expected_calibration[256];
    marker_calibration(expected_calibration, sizeof expected_calibration, offset);

    size_t rom_size = 0;
    uint8_t *rom = marker_read(rom_path, sizeof expected_rom, &rom_size);
    if (rom == NULL) {
        fprintf(stderr, "marker: IO (content)\n");
        return 2;
    }
    uint64_t content_identity = progression_hash64(rom, rom_size);
    int valid_rom =
        rom_size == sizeof expected_rom && memcmp(rom, expected_rom, sizeof expected_rom) == 0;
    free(rom);
    if (!valid_rom) {
        fprintf(stderr, "marker: IDENTITY (content)\n");
        return 1;
    }

    size_t calibration_size = 0;
    uint8_t *calibration =
        marker_read(calibration_path, sizeof expected_calibration, &calibration_size);
    if (calibration == NULL) {
        fprintf(stderr, "marker: IO (calibration)\n");
        return 2;
    }
    uint64_t calibration_identity = progression_hash64(calibration, calibration_size);
    int valid_calibration = calibration_size == strlen(expected_calibration) &&
                            memcmp(calibration, expected_calibration, calibration_size) == 0;
    free(calibration);
    if (!valid_calibration) {
        fprintf(stderr, "marker: IDENTITY (calibration)\n");
        return 1;
    }

    progression_session session;
    const char *reason = progression_read_record(record_path, &session);
    if (reason != NULL) {
        fprintf(stderr, "marker: %s (record)\n", reason);
        return strcmp(reason, "IO") == 0 || strcmp(reason, "PARSE") == 0 ? 2 : 1;
    }
    if (session.content_identity != content_identity ||
        session.calibration_identity != calibration_identity) {
        fprintf(stderr, "marker: IDENTITY (record binding)\n");
        return 1;
    }
    nesturbator *inst = marker_instance(expected_rom);
    if (inst == NULL) {
        fprintf(stderr, "marker: IO (core instance)\n");
        return 2;
    }
    reason = progression_validate_anchor(inst, session.original_anchor_path,
                                         session.original_anchor_identity,
                                         session.original_anchor_frame, offset);
    if (reason == NULL && session.boundary_count > 11u)
        reason = progression_validate_anchor(inst, session.rewound_anchor_path,
                                             session.rewound_anchor_identity,
                                             session.rewound_anchor_frame, offset);
    for (uint32_t i = 0; reason == NULL && i < session.boundary_count; ++i) {
        const progression_boundary *b = &session.boundaries[i];
        uint64_t actual_hash = 0;
        if (b->artifact_path[0] == '\0' || b->artifact_identity == 0u) {
            reason = "ATTRIBUTION";
            break;
        }
        if (!progression_hash_file(b->artifact_path, &actual_hash)) {
            reason = "IO";
            break;
        }
        if (actual_hash != b->artifact_hash) {
            reason = "ATTRIBUTION";
            break;
        }
        const char *anchor_path = b->origin == PROGRESSION_ORIGIN_REWOUND
                                      ? session.rewound_anchor_path
                                      : session.original_anchor_path;
        uint64_t anchor_identity = b->origin == PROGRESSION_ORIGIN_REWOUND
                                       ? session.rewound_anchor_identity
                                       : session.original_anchor_identity;
        uint64_t anchor_frame = b->origin == PROGRESSION_ORIGIN_REWOUND
                                    ? session.rewound_anchor_frame
                                    : session.original_anchor_frame;
        uint32_t palette[512];
        reason = progression_palette_for_frame(expected_rom, anchor_path, anchor_identity,
                                               anchor_frame, b->display_frame, offset, palette);
        if (reason != NULL)
            break;
        uint32_t decoded = 0;
        const char *image_reason = marker_image(b->artifact_path, palette, offset, &decoded);
        if (image_reason != NULL) {
            fprintf(stderr, "marker: image boundary=%u reason=%s\n", b->id, image_reason);
            reason = strcmp(image_reason, "image-geometry-or-format") == 0 ? "PARSE" : "IDENTITY";
        } else if (decoded != b->display_frame) {
            fprintf(stderr, "marker: image boundary=%u expected=%" PRIu64 " actual=%u\n", b->id,
                    b->display_frame, decoded);
            reason = "PROGRESSION";
        }
    }
    nesturbator_destroy(inst);
    if (reason == NULL)
        reason = progression_verdict(&session, !prefix);
    printf("progression boundaries=%u mode=%s verdict=%s\n", session.boundary_count,
           prefix ? "prefix" : "complete", reason);
    if (reason == NULL)
        return 2;
    if (strcmp(reason, "PREFIX_OK") == 0 || strcmp(reason, "VERDICT_OK") == 0)
        return 0;
    if (strcmp(reason, "IO") == 0 || strcmp(reason, "PARSE") == 0)
        return 2;
    return 1;
}

static int progression_complete_required_selftest(const char *work)
{
    char rom[PROGRESSION_PATH_CAP], calibration[PROGRESSION_PATH_CAP];
    char record[PROGRESSION_PATH_CAP];
    if (!progression_make_path(rom, sizeof rom, work, "marker.nes") ||
        !progression_make_path(calibration, sizeof calibration, work, "calibration.txt") ||
        !progression_make_path(record, sizeof record, work, "progression-prefix.rec"))
        return 2;
    progression_session session;
    if (progression_read_record(record, &session) != NULL)
        return 2;
    const char *expected_reason = progression_verdict(&session, 1);
    int result = marker_progression_cli(rom, calibration, record, 0);
    if (strcmp(expected_reason, "INCOMPLETE") != 0 || result != 1) {
        fprintf(stderr,
                "ASSERT complete verdict rejects a prefix: expected INCOMPLETE/exit 1, "
                "actual %s/exit %d\n",
                expected_reason, result);
        return 1;
    }
    printf("complete verdict control: prefix rejected as INCOMPLETE\n");
    return 0;
}

static int progression_replace_once(uint8_t *data, size_t size, const char *before,
                                    const char *after)
{
    size_t length = strlen(before);
    if (length != strlen(after) || length > size)
        return 0;
    for (size_t i = 0; i <= size - length; ++i) {
        if (memcmp(data + i, before, length) == 0) {
            memcpy(data + i, after, length);
            return 1;
        }
    }
    return 0;
}

static uint8_t *progression_insert_after(const uint8_t *data, size_t size, const char *needle,
                                         const char *insert, size_t *new_size)
{
    size_t needle_size = strlen(needle), insert_size = strlen(insert);
    if (needle_size == 0u || needle_size > size || size > PROGRESSION_RECORD_CAP - insert_size)
        return NULL;
    for (size_t i = 0; i <= size - needle_size; ++i) {
        if (memcmp(data + i, needle, needle_size) == 0) {
            size_t split = i + needle_size;
            uint8_t *result = (uint8_t *)malloc(size + insert_size);
            if (result == NULL)
                return NULL;
            memcpy(result, data, split);
            memcpy(result + split, insert, insert_size);
            memcpy(result + split + insert_size, data + split, size - split);
            *new_size = size + insert_size;
            return result;
        }
    }
    return NULL;
}

static int progression_controls_selftest(const char *work)
{
    char rom[PROGRESSION_PATH_CAP], calibration[PROGRESSION_PATH_CAP];
    char record[PROGRESSION_PATH_CAP], wrong_anchor[PROGRESSION_PATH_CAP];
    char alternative_anchor[PROGRESSION_PATH_CAP], alternative_image[PROGRESSION_PATH_CAP];
    char ambiguous_image[PROGRESSION_PATH_CAP], ambiguous_record[PROGRESSION_PATH_CAP];
    char malformed_record[PROGRESSION_PATH_CAP], missing_record[PROGRESSION_PATH_CAP];
    char missing_artifact[PROGRESSION_PATH_CAP], missing_artifact_record[PROGRESSION_PATH_CAP];
    char missing_anchor[PROGRESSION_PATH_CAP], missing_anchor_record[PROGRESSION_PATH_CAP];
    if (!progression_full_fixtures(work) ||
        !progression_make_path(rom, sizeof rom, work, "marker.nes") ||
        !progression_make_path(calibration, sizeof calibration, work, "calibration.txt") ||
        !progression_make_path(record, sizeof record, work, "progression-complete.rec") ||
        !progression_make_path(wrong_anchor, sizeof wrong_anchor, work, "valid wrong origin.ra") ||
        !progression_make_path(alternative_anchor, sizeof alternative_anchor, work,
                               "alternative same content anchor.ra") ||
        !progression_make_path(alternative_image, sizeof alternative_image, work,
                               "alternative frame 101.ppm") ||
        !progression_make_path(ambiguous_image, sizeof ambiguous_image, work,
                               "A A A actual image.ppm") ||
        !progression_make_path(ambiguous_record, sizeof ambiguous_record, work,
                               "A A B ambiguous.rec") ||
        !progression_make_path(malformed_record, sizeof malformed_record, work,
                               "malformed record control.rec") ||
        !progression_make_path(missing_record, sizeof missing_record, work,
                               "must remain absent record.rec") ||
        !progression_make_path(missing_artifact, sizeof missing_artifact, work,
                               "must remain absent image.ppm") ||
        !progression_make_path(missing_artifact_record, sizeof missing_artifact_record, work,
                               "missing artifact control.rec") ||
        !progression_make_path(missing_anchor, sizeof missing_anchor, work,
                               "must remain absent anchor.ra") ||
        !progression_make_path(missing_anchor_record, sizeof missing_anchor_record, work,
                               "missing anchor control.rec"))
        return 2;
    if (marker_progression_cli(rom, calibration, record, 0) != 0) {
        fprintf(stderr,
                "progression controls: actual complete fixture failed its CLI precondition\n");
        return 2;
    }
    progression_session baseline;
    if (progression_read_record(record, &baseline) != NULL)
        return 2;
    uint64_t wrong_identity = 0, alternative_identity = 0;
    if (!progression_hash_file(wrong_anchor, &wrong_identity) ||
        !progression_hash_file(alternative_anchor, &alternative_identity))
        return 2;
    static uint8_t marker[MARKER_ROM_SIZE];
    marker_rom(marker);
    uint32_t offset = 0;
    if (!marker_offset(marker, &offset))
        return 2;
    nesturbator *validator = marker_instance(marker);
    if (validator == NULL)
        return 2;
    const char *wrong_reason =
        progression_validate_anchor(validator, wrong_anchor, wrong_identity, 149u, offset);
    nesturbator_destroy(validator);
    if (wrong_reason != NULL) {
        fprintf(stderr, "progression controls: wrong-origin native precondition failed: %s\n",
                wrong_reason);
        return 2;
    }

    unsigned count = 0u;
    int ok = 1;
    ok &= progression_control_expect("intended-complete-history", &baseline, "VERDICT_OK", &count);

    progression_session bad = baseline;
    bad.boundaries[2].display_frame = 150u;
    bad.boundaries[2].candidate_first = 150u;
    bad.boundaries[2].candidate_last = 150u;
    ok &= progression_control_expect("isolated-drop-one", &bad, "PROGRESSION", &count);

    bad = baseline;
    bad.boundaries[2].display_frame = 152u;
    bad.boundaries[2].candidate_first = 152u;
    bad.boundaries[2].candidate_last = 152u;
    ok &= progression_control_expect("isolated-double-one", &bad, "PROGRESSION", &count);

    bad = baseline;
    bad.boundaries[2].display_frame = 150u;
    bad.boundaries[2].candidate_first = 150u;
    bad.boundaries[2].candidate_last = 150u;
    bad.boundaries[3].display_frame = 152u;
    bad.boundaries[3].candidate_first = 152u;
    bad.boundaries[3].candidate_last = 152u;
    ok &= progression_control_expect("compensated-drop-then-double", &bad, "PROGRESSION", &count);

    bad = baseline;
    bad.boundaries[7].display_frame = 156u;
    bad.boundaries[7].candidate_first = 156u;
    bad.boundaries[7].candidate_last = 156u;
    ok &= progression_control_expect("load-probe-dropped-load", &bad, "PROGRESSION", &count);

    bad = baseline;
    bad.boundaries[7].display_frame = 152u;
    bad.boundaries[7].candidate_first = 152u;
    bad.boundaries[7].candidate_last = 152u;
    ok &= progression_control_expect("load-probe-doubled-advance", &bad, "PROGRESSION", &count);

    bad = baseline;
    bad.boundaries[7].command_count = 1u;
    ok &= progression_control_expect("load-probe-command-count", &bad, "PROGRESSION", &count);

    bad = baseline;
    bad.boundaries[7].send_count = 1u;
    ok &= progression_control_expect("load-probe-send-count", &bad, "PROGRESSION", &count);

    bad = baseline;
    bad.boundaries[15].command_count = 3u;
    bad.boundaries[15].send_count = 3u;
    ok &= progression_control_expect("runahead-load-probe-extra-command", &bad, "PROGRESSION",
                                     &count);

    /* A,A,B: the intended 150,151,152 sequence saw A,A,B. A is both 150 and 151;
       reject it as ambiguous rather than choosing the candidate that fits the next row. */
    uint64_t a_hash = 0u, b_hash = 0u, duplicate_a_hash = 0u;
    size_t a_size = 0u;
    uint8_t *a_image =
        marker_read(baseline.boundaries[0].artifact_path, PROGRESSION_ARTIFACT_CAP, &a_size);
    if (a_image == NULL || !progression_hash_file(baseline.boundaries[0].artifact_path, &a_hash) ||
        !progression_hash_file(baseline.boundaries[1].artifact_path, &b_hash) || a_hash != b_hash ||
        !marker_write(ambiguous_image, a_image, a_size) ||
        !progression_hash_file(ambiguous_image, &duplicate_a_hash)) {
        free(a_image);
        return 2;
    }
    free(a_image);
    uint64_t final_picture_hash = 0u;
    if (!progression_hash_file(baseline.boundaries[3].artifact_path, &final_picture_hash) ||
        duplicate_a_hash != a_hash || duplicate_a_hash == final_picture_hash)
        return 2;
    bad = baseline;
    bad.boundaries[2].artifact_identity = 17u;
    snprintf(bad.boundaries[2].artifact_path, sizeof bad.boundaries[2].artifact_path, "%s",
             ambiguous_image);
    bad.boundaries[2].artifact_hash = duplicate_a_hash;
    bad.boundaries[2].display_frame = 150u;
    bad.boundaries[2].candidate_count = 2u;
    bad.boundaries[2].candidate_first = 150u;
    bad.boundaries[2].candidate_last = 151u;
    if (!progression_write_record(ambiguous_record, &bad))
        return 2;
    int ambiguous_cli = marker_progression_cli(rom, calibration, ambiguous_record, 0);
    ok &= marker_expect("A,A,B-actual-images-rejected",
                        ambiguous_cli == 1 ? "rejected" : "accepted", "rejected", &count);
    ok &= progression_control_expect("A,A,B-candidate-ambiguity", &bad, "AMBIGUOUS", &count);

    bad = baseline;
    bad.boundaries[2].candidate_count = 0u;
    ok &= progression_control_expect("zero-candidate-identity", &bad, "IDENTITY", &count);

    bad = baseline;
    bad.boundaries[2].candidate_first = 150u;
    bad.boundaries[2].candidate_last = 150u;
    ok &= progression_control_expect("single-candidate-frame-mismatch", &bad, "IDENTITY", &count);

    bad = baseline;
    bad.boundaries[4].id = 2u;
    ok &= progression_control_expect("stale-boundary-id", &bad, "ATTRIBUTION", &count);

    bad = baseline;
    bad.boundaries[2].id = 4u;
    ok &= progression_control_expect("out-of-order-boundary-id", &bad, "ATTRIBUTION", &count);

    bad = baseline;
    bad.boundaries[2].artifact_identity = bad.boundaries[1].artifact_identity;
    ok &= progression_control_expect("repeated-artifact-identity", &bad, "ATTRIBUTION", &count);

    bad = baseline;
    snprintf(bad.boundaries[2].artifact_path, sizeof bad.boundaries[2].artifact_path, "%s",
             bad.boundaries[1].artifact_path);
    ok &= progression_control_expect("repeated-artifact-path", &bad, "ATTRIBUTION", &count);

    bad = baseline;
    bad.boundary_count = 15u;
    ok &= progression_control_expect("missing-final-observation", &bad, "INCOMPLETE", &count);

    bad = baseline;
    bad.boundary_count = 17u;
    ok &= progression_control_expect("extra-observation", &bad, "ATTRIBUTION", &count);

    bad = baseline;
    bad.boundaries[7].artifact_identity = 17u;
    snprintf(bad.boundaries[7].artifact_path, sizeof bad.boundaries[7].artifact_path, "%s",
             alternative_image);
    bad.boundaries[7].anchor_identity = alternative_identity;
    bad.boundaries[7].anchor_frame = 100u;
    bad.boundaries[7].display_frame = 101u;
    bad.boundaries[7].candidate_first = 101u;
    bad.boundaries[7].candidate_last = 101u;
    if (!progression_hash_file(alternative_image, &bad.boundaries[7].artifact_hash))
        return 2;
    ok &= progression_control_expect("valid-same-content-wrong-load", &bad, "ORIGIN", &count);

    bad = baseline;
    snprintf(bad.original_anchor_path, sizeof bad.original_anchor_path, "%s", alternative_anchor);
    bad.original_anchor_identity = alternative_identity;
    bad.original_anchor_frame = 100u;
    ok &=
        progression_control_expect("original-branch-wrong-retained-anchor", &bad, "ORIGIN", &count);

    bad = baseline;
    bad.boundaries[11].anchor_identity = baseline.original_anchor_identity;
    bad.boundaries[11].anchor_frame = baseline.original_anchor_frame;
    ok &= progression_control_expect("original-rewound-anchor-swap", &bad, "ORIGIN", &count);

    bad = baseline;
    bad.boundaries[12].anchor_identity = alternative_identity;
    bad.boundaries[12].anchor_frame = 100u;
    ok &= progression_control_expect("incompatible-rewound-branch-origin", &bad, "ORIGIN", &count);

    bad = baseline;
    bad.boundaries[15].display_lead = 0;
    ok &= progression_control_expect("wrong-runahead-display-lead", &bad, "PROGRESSION", &count);

    bad = baseline;
    bad.boundaries[15].display_frame = 133u;
    bad.boundaries[15].candidate_first = 133u;
    bad.boundaries[15].candidate_last = 133u;
    ok &= progression_control_expect("wrong-runahead-displayed-frame", &bad, "PROGRESSION", &count);

    bad = baseline;
    bad.boundaries[10].display_frame = 153u;
    bad.boundaries[10].candidate_first = 153u;
    bad.boundaries[10].candidate_last = 153u;
    ok &= progression_control_expect("resume-not-beyond-load-three", &bad, "PROGRESSION", &count);

    bad = baseline;
    bad.boundaries[11].display_frame = 140u;
    bad.boundaries[11].candidate_first = 140u;
    bad.boundaries[11].candidate_last = 140u;
    ok &= progression_control_expect("rewind-under-twenty-frames", &bad, "PROGRESSION", &count);

    bad = baseline;
    bad.boundaries[11].command_count = 39u;
    bad.boundaries[11].send_count = 39u;
    ok &= progression_control_expect("rewind-burst-not-forty", &bad, "PROGRESSION", &count);

    bad = baseline;
    bad.boundaries[5].elapsed_ms = 1001u;
    ok &= progression_control_expect("deadline-overrun", &bad, "DEADLINE", &count);

    bad = baseline;
    bad.boundaries[5].deadline_ms = 0u;
    ok &= progression_control_expect("deadline-missing", &bad, "DEADLINE", &count);

    bad = baseline;
    bad.boundaries[5].process_alive = 0;
    ok &= progression_control_expect("host-process-dead", &bad, "PROCESS", &count);

    bad = baseline;
    bad.terminal_exit = 1;
    ok &= progression_control_expect("nonzero-terminal-exit", &bad, "PROCESS", &count);

    bad = baseline;
    bad.terminal_exit = -1;
    ok &= progression_control_expect("terminal-exit-unobserved", &bad, "INCOMPLETE", &count);

    bad = baseline;
    bad.mode = PROGRESSION_PREFIX;
    ok &= progression_control_expect("prefix-cannot-pass-complete", &bad, "INCOMPLETE", &count);

    bad = baseline;
    bad.content_identity = 0u;
    ok &= progression_control_expect("content-identity-missing", &bad, "IDENTITY", &count);

    bad = baseline;
    bad.calibration_identity = 0u;
    ok &= progression_control_expect("calibration-identity-missing", &bad, "IDENTITY", &count);

    size_t record_size = 0u;
    uint8_t *record_bytes = marker_read(record, PROGRESSION_RECORD_CAP, &record_size);
    if (record_bytes == NULL)
        return 2;
    progression_session parsed;
    int replaced = progression_replace_once(record_bytes, record_size, "operation=capture",
                                            "operation=unknown");
    const char *parse_reason =
        replaced ? progression_parse_record(record_bytes, record_size, &parsed) : "TEST_SETUP";
    ok &= marker_expect("unknown-operation-record", parse_reason, "PARSE", &count);
    free(record_bytes);

    record_bytes = marker_read(record, PROGRESSION_RECORD_CAP, &record_size);
    if (record_bytes == NULL)
        return 2;
    replaced = progression_replace_once(record_bytes, record_size, "NESTURBATOR-PROGRESSION 1",
                                        "NESTURBATOR-PROGRESSION 2");
    parse_reason =
        replaced ? progression_parse_record(record_bytes, record_size, &parsed) : "TEST_SETUP";
    ok &= marker_expect("unknown-record-version", parse_reason, "PARSE", &count);
    free(record_bytes);

    record_bytes = marker_read(record, PROGRESSION_RECORD_CAP, &record_size);
    if (record_bytes == NULL)
        return 2;
    replaced =
        progression_replace_once(record_bytes, record_size, "mode=complete", "xode=complete");
    parse_reason =
        replaced ? progression_parse_record(record_bytes, record_size, &parsed) : "TEST_SETUP";
    ok &= marker_expect("unknown-record-field", parse_reason, "PARSE", &count);
    free(record_bytes);

    record_bytes = marker_read(record, PROGRESSION_RECORD_CAP, &record_size);
    if (record_bytes == NULL)
        return 2;
    size_t duplicate_size = 0u;
    uint8_t *duplicate_record = progression_insert_after(
        record_bytes, record_size, "mode=complete\n", "mode=complete\n", &duplicate_size);
    parse_reason = duplicate_record != NULL
                       ? progression_parse_record(duplicate_record, duplicate_size, &parsed)
                       : "TEST_SETUP";
    ok &= marker_expect("duplicate-record-field", parse_reason, "PARSE", &count);
    free(duplicate_record);
    free(record_bytes);

    record_bytes = marker_read(record, PROGRESSION_RECORD_CAP, &record_size);
    if (record_bytes == NULL ||
        !progression_replace_once(record_bytes, record_size, "NESTURBATOR-PROGRESSION 1",
                                  "NESTURBATOR-PROGRESSION 2") ||
        !marker_write(malformed_record, record_bytes, record_size)) {
        free(record_bytes);
        return 2;
    }
    free(record_bytes);
    ok &= progression_control_exit_expect(
        "malformed-record-exit-status",
        marker_progression_cli(rom, calibration, malformed_record, 0), 2, &count);

    uint64_t absent_identity = 0u;
    if (progression_hash_file(missing_record, &absent_identity) ||
        progression_hash_file(missing_artifact, &absent_identity) ||
        progression_hash_file(missing_anchor, &absent_identity))
        return 2;
    ok &= progression_control_exit_expect(
        "missing-record-exit-status", marker_progression_cli(rom, calibration, missing_record, 0),
        2, &count);

    bad = baseline;
    bad.boundaries[0].artifact_identity = 17u;
    snprintf(bad.boundaries[0].artifact_path, sizeof bad.boundaries[0].artifact_path, "%s",
             missing_artifact);
    if (!progression_write_record(missing_artifact_record, &bad))
        return 2;
    ok &= progression_control_exit_expect(
        "missing-artifact-exit-status",
        marker_progression_cli(rom, calibration, missing_artifact_record, 0), 2, &count);

    bad = baseline;
    snprintf(bad.original_anchor_path, sizeof bad.original_anchor_path, "%s", missing_anchor);
    if (!progression_write_record(missing_anchor_record, &bad))
        return 2;
    ok &= progression_control_exit_expect(
        "missing-anchor-exit-status",
        marker_progression_cli(rom, calibration, missing_anchor_record, 0), 2, &count);

    printf("progression controls: %u cases\n", count);
    return ok ? 0 : 1;
}

static int progression_tracer_selftest(const char *work)
{
    char rom[PROGRESSION_PATH_CAP], calibration[PROGRESSION_PATH_CAP];
    char record[PROGRESSION_PATH_CAP];
    char wrong_anchor[PROGRESSION_PATH_CAP];
    if (!marker_progression_fixtures(work) ||
        !progression_make_path(rom, sizeof rom, work, "marker.nes") ||
        !progression_make_path(calibration, sizeof calibration, work, "calibration.txt") ||
        !progression_make_path(record, sizeof record, work, "progression-prefix.rec") ||
        !progression_make_path(wrong_anchor, sizeof wrong_anchor, work, "valid wrong origin.ra"))
        return 2;
    int result = marker_progression_cli(rom, calibration, record, 1);
    if (result != 0)
        return result;
    progression_session session;
    if (progression_read_record(record, &session) != NULL)
        return 2;
    uint64_t wrong_identity = 0;
    uint32_t offset = 0;
    static uint8_t marker[MARKER_ROM_SIZE];
    marker_rom(marker);
    if (!progression_hash_file(wrong_anchor, &wrong_identity) || !marker_offset(marker, &offset))
        return 2;
    nesturbator *inst = marker_instance(marker);
    if (inst == NULL)
        return 2;
    const char *anchor_reason =
        progression_validate_anchor(inst, wrong_anchor, wrong_identity, 149u, offset);
    nesturbator_destroy(inst);
    progression_session wrong_origin = session;
    wrong_origin.boundaries[2].anchor_identity = wrong_identity;
    wrong_origin.boundaries[2].anchor_frame = 149u;
    unsigned count = 0;
    int ok = marker_expect("valid-same-content-wrong-origin",
                           anchor_reason == NULL ? progression_verdict(&wrong_origin, 0)
                                                 : "NATIVE_INVALID",
                           "ORIGIN", &count);
    printf("progression origin controls: %u case\n", count);
    return ok ? 0 : 1;
}

static int marker_selftest(const char *work)
{
    if (marker_pixel_control() != 0)
        return 1;
    const char *duplicate = marker_progress(255u, 255u, 1u);
    if (duplicate == NULL || strcmp(duplicate, "duplicate-identity") != 0) {
        fprintf(stderr,
                "ASSERT duplicate qualification identity: expected duplicate-identity, "
                "actual %s\n",
                duplicate != NULL ? duplicate : "accepted");
        return 1;
    }
    static uint8_t rom[MARKER_ROM_SIZE];
    static uint8_t rgb[NESTURBATOR_TEST_RGB_SIZE];
    marker_rom(rom);
    nesturbator *inst = marker_instance(rom);
    uint32_t offset = 0;
    uint64_t absolute = 0;
    if (inst == NULL || !marker_offset(rom, &offset))
        return 2;
    uint32_t palette[512];
    nesturbator_get_palette(inst, palette, 512u);
    size_t size = nesturbator_state_size(inst);
    size_t padded = (size + 7u) & ~(size_t)7u;
    size_t length = 16u + padded + 8u;
    uint8_t *bytes = (uint8_t *)calloc(length + 1u, 1u);
    uint8_t *base = (uint8_t *)calloc(length + 1u, 1u);
    uint8_t *earlier = (uint8_t *)malloc(size);
    uint8_t *first = (uint8_t *)malloc(size);
    if (bytes == NULL || base == NULL || earlier == NULL || first == NULL)
        return 2;
    for (unsigned n = 1; n <= 150u; ++n) {
        if (!marker_frame(inst, rgb, &absolute))
            return 2;
        if (n == MARKER_FIRST && nesturbator_save_state(inst, first, size) != NESTURBATOR_OK)
            return 2;
        if (n == 149u && nesturbator_save_state(inst, earlier, size) != NESTURBATOR_OK)
            return 2;
    }
    memcpy(bytes, "RASTATE\001MEM ", 12u);
    put32(bytes + 12u, (uint32_t)size);
    memcpy(bytes + 16u + padded, "END ", 4u);
    if (nesturbator_save_state(inst, bytes + 16u, size) != NESTURBATOR_OK)
        return 2;
    memcpy(base, bytes, length);
    unsigned count = 1u;
    int ok = marker_expect("native-origin", marker_anchor(inst, bytes, length, 150u, offset), NULL,
                           &count);
    memcpy(bytes + 16u, first, size);
    ok &=
        marker_expect("first-anchor", marker_anchor(inst, bytes, length, 4u, offset), NULL, &count);
    memcpy(bytes, base, length);
    memcpy(bytes + 16u, earlier, size);
    /* A real saved same-ROM wrong origin must pass the native loader before semantic rejection. */
    if (nesturbator_load_state(inst, bytes + 16u, size) != NESTURBATOR_OK)
        return 2;
    ok &= marker_expect("valid-wrong-origin", marker_anchor(inst, bytes, length, 150u, offset),
                        "native-origin", &count);
    memcpy(bytes, base, length);
    put32(bytes + 32u, NESTURBATOR_BEHAVIOUR_REVISION - 1u);
    st_recrc(bytes + 16u, size);
    if (nesturbator_load_state(inst, bytes + 16u, size) != NESTURBATOR_OK)
        return 2;
    ok &= marker_expect("valid-stale-revision", marker_anchor(inst, bytes, length, 150u, offset),
                        "native-revision", &count);
    const char *native_names[] = {"wrong-content", "wrong-format", "crc-corruption"};
    const char *native_reasons[] = {"native-content", "native-format", "native-checksum"};
    const unsigned native_offsets[] = {24u, 8u, 64u};
    for (unsigned i = 0; i < 3u; ++i) {
        memcpy(bytes, base, length);
        bytes[16u + native_offsets[i]] ^= 1u;
        if (i < 2u)
            st_recrc(bytes + 16u, size);
        ok &= marker_expect(native_names[i], marker_anchor(inst, bytes, length, 150u, offset),
                            native_reasons[i], &count);
    }
    memcpy(bytes, base, length);
    ok &= marker_expect("truncated", marker_anchor(inst, bytes, length - 1u, 150u, offset),
                        "container-size", &count);
    ok &= marker_expect("trailer", marker_anchor(inst, bytes, length + 1u, 150u, offset),
                        "container-size", &count);
    ok &= marker_expect("oversized", marker_anchor(inst, bytes, SIZE_MAX, 150u, offset),
                        "container-size", &count);
    bytes[0] ^= 1u;
    ok &= marker_expect("compression-or-magic", marker_anchor(inst, bytes, length, 150u, offset),
                        "container-header", &count);
    memcpy(bytes, base, length);
    put32(bytes + 12u, UINT32_MAX);
    ok &= marker_expect("oversized-MEM", marker_anchor(inst, bytes, length, 150u, offset),
                        "container-header", &count);
    memcpy(bytes, base, length);
    if (padded == size)
        return 2;
    bytes[16u + size] = 1u;
    ok &= marker_expect("nonzero-padding", marker_anchor(inst, bytes, length, 150u, offset),
                        "container-padding", &count);
    memcpy(bytes, base, length);
    memcpy(bytes + 16u + padded, "MEM ", 4u);
    ok &= marker_expect("duplicate-MEM", marker_anchor(inst, bytes, length, 150u, offset),
                        "container-end", &count);
    memcpy(bytes, base, length);
    bytes[16u + padded + 4u] = 1u;
    ok &= marker_expect("nonzero-END", marker_anchor(inst, bytes, length, 150u, offset),
                        "container-end", &count);
    memcpy(bytes, base, length);
    bytes[16u + 10u] ^= 1u;
    st_recrc(bytes + 16u, size);
    ok &= marker_expect("CRC-valid-invalid-native",
                        marker_anchor(inst, bytes, length, 150u, offset), "native-invalid", &count);

    const uint32_t counters[] = {0u, 1u, 9997u, 9998u, 255u, 256u, 257u};
    const uint32_t frames[] = {0u, 4u, 10000u, 0u, 258u, 259u, 260u};
    const char *names[] = {"startup",      "first", "last",       "end-plus-one",
                           "carry-before", "carry", "carry-after"};
    for (unsigned i = 0; i < 7u; ++i) {
        uint32_t observed = 0;
        marker_pattern(rgb, palette, counters[i]);
        const char *reason = marker_pixels(rgb, palette, offset, &observed);
        ok &= marker_expect(names[i], reason, frames[i] == 0u ? "window" : NULL, &count);
        if (frames[i] != 0u && observed != frames[i])
            ok = 0;
    }
    marker_pattern(rgb, palette, 257u);
    /* Background RGB is irrelevant to identity. Complete marker pixels still must match. */
    rgb[0] = 127u;
    uint32_t observed = 0;
    ok &=
        marker_expect("harmless-RGB", marker_pixels(rgb, palette, offset, &observed), NULL, &count);
    size_t guard_pixel = ((MARKER_Y + 7u) * W_PX + 32u) * 3u;
    rgb[guard_pixel] ^= 1u;
    ok &= marker_expect("invalid-color", marker_pixels(rgb, palette, offset, &observed), "color",
                        &count);
    marker_pattern(rgb, palette, 257u);
    memset(rgb + guard_pixel, 0, 3u);
    ok &= marker_expect("partial-guard", marker_pixels(rgb, palette, offset, &observed), "guard",
                        &count);
    marker_pattern(rgb, palette, 257u);
    size_t cell_pixel = ((MARKER_Y + 7u) * W_PX + 48u) * 3u;
    memset(rgb + cell_pixel, 0, 3u);
    ok &= marker_expect("partial-cell", marker_pixels(rgb, palette, offset, &observed), "cell",
                        &count);
    const uint32_t before[] = {255u, 255u, 255u, 255u, 255u};
    const uint32_t after[] = {255u, 256u, 257u, 255u, 257u};
    const uint32_t steps[] = {0u, 1u, 2u, 1u, 1u};
    const char *progress_names[] = {"zero", "one", "two", "duplicate-identity", "double"};
    for (unsigned i = 0; i < 5u; ++i)
        ok &= marker_expect(progress_names[i], marker_progress(before[i], after[i], steps[i]),
                            i < 3u    ? NULL
                            : i == 3u ? "duplicate-identity"
                                      : "progression",
                            &count);
    char path[2048];
    int path_size = snprintf(path, sizeof path, "%s/marker-wrong-geometry.ppm", work);
    if (path_size < 0 || (size_t)path_size >= sizeof path ||
        !marker_write(path, "P6\n255 240\n255\n", 15u))
        return 2;
    ok &= marker_expect("wrong-dimensions", marker_image(path, palette, offset, &observed),
                        "image-geometry-or-format", &count);
    /* Independently encode equivalent BMP pixels: top-down 24-bit and bottom-up 32-bit,
       with arbitrary unused alpha. The existing reader normalizes both to the same RGB. */
    marker_pattern(rgb, palette, 257u);
    for (unsigned representation = 0; representation < 2u; ++representation) {
        size_t stride = representation == 0u ? 3u : 4u;
        size_t bmp_size = 54u + W_PX * H_PX * stride;
        uint8_t *bmp = (uint8_t *)calloc(bmp_size, 1u);
        if (bmp == NULL)
            return 2;
        bmp[0] = 'B';
        bmp[1] = 'M';
        put32(bmp + 2u, (uint32_t)bmp_size);
        put32(bmp + 10u, 54u);
        put32(bmp + 14u, 40u);
        put32(bmp + 18u, W_PX);
        put32(bmp + 22u, representation == 0u ? (uint32_t)(0u - H_PX) : H_PX);
        bmp[26] = 1u;
        bmp[28] = representation == 0u ? 24u : 32u;
        for (unsigned y = 0; y < H_PX; ++y) {
            unsigned source_y = representation == 0u ? y : H_PX - 1u - y;
            for (unsigned x = 0; x < W_PX; ++x) {
                size_t dst = 54u + ((size_t)y * W_PX + x) * stride;
                size_t src = ((size_t)source_y * W_PX + x) * 3u;
                bmp[dst] = rgb[src + 2u];
                bmp[dst + 1u] = rgb[src + 1u];
                bmp[dst + 2u] = rgb[src];
                if (stride == 4u)
                    bmp[dst + 3u] = 127u;
            }
        }
        path_size =
            snprintf(path, sizeof path, "%s/marker-representation-%u.bmp", work, representation);
        if (path_size < 0 || (size_t)path_size >= sizeof path || !marker_write(path, bmp, bmp_size))
            return 2;
        free(bmp);
        ok &= marker_expect(representation == 0u ? "BMP-top-down-24" : "BMP-bottom-up-32-alpha",
                            marker_image(path, palette, offset, &observed), NULL, &count);
        if (observed != 260u)
            ok = 0;
    }
    free(bytes);
    free(base);
    free(earlier);
    free(first);
    nesturbator_destroy(inst);
    if (ok && count == 36u) {
        printf("marker controls: %u cases\n", count);
        return 0;
    }
    fprintf(stderr, "marker controls failed after %u cases\n", count);
    return 1;
}

static int marker_cli(int argc, char **argv)
{
    static uint8_t expected_rom[MARKER_ROM_SIZE];
    marker_rom(expected_rom);
    if (argc == 3 && strcmp(argv[1], "--marker-generate") == 0)
        return marker_write(argv[2], expected_rom, sizeof expected_rom) ? 0 : 2;
    if (argc == 3 && strcmp(argv[1], "--marker-selftest") == 0)
        return marker_selftest(argv[2]);
    if (argc == 4 && strcmp(argv[1], "--marker-selftest") == 0 &&
        strcmp(argv[3], "--fixtures") == 0) {
        if (marker_selftest(argv[2]) != 0)
            return 1;
        return marker_progression_fixtures(argv[2]) ? 0 : 2;
    }
    if (argc == 5 && strcmp(argv[1], "--marker-verdict-prefix") == 0)
        return marker_progression_cli(argv[2], argv[3], argv[4], 1);
    if (argc == 5 && strcmp(argv[1], "--marker-verdict") == 0)
        return marker_progression_cli(argv[2], argv[3], argv[4], 0);
    if (argc < 4)
        return 2;
    size_t size = 0;
    uint8_t *rom = marker_read(argv[2], sizeof expected_rom, &size);
    int valid_rom =
        rom != NULL && size == sizeof expected_rom && memcmp(rom, expected_rom, size) == 0;
    free(rom);
    if (!valid_rom) {
        fprintf(stderr, "marker: content\n");
        return 1;
    }
    uint32_t offset = 0;
    if (!marker_offset(expected_rom, &offset)) {
        fprintf(stderr, "marker: startup calibration\n");
        return 1;
    }
    nesturbator *inst = marker_instance(expected_rom);
    if (inst == NULL)
        return 2;
    uint32_t palette[512];
    nesturbator_get_palette(inst, palette, 512u);
    char calibration[256];
    marker_calibration(calibration, sizeof calibration, offset);
    int result = 1;
    const char *reason = NULL;
    static uint8_t rgb[NESTURBATOR_TEST_RGB_SIZE];
    if (argc == 4 && strcmp(argv[1], "--marker-qualify") == 0) {
        uint32_t previous = 0, before_previous = 0;
        unsigned count = 0;
        for (unsigned n = 1; n <= MARKER_LAST; ++n) {
            uint64_t absolute;
            uint32_t value = 0;
            if (!marker_frame(inst, rgb, &absolute) || absolute != n) {
                reason = "frame";
                break;
            }
            reason = marker_decode(rgb, palette, &value);
            if (n < MARKER_FIRST) {
                if (reason == NULL && value != 0u) {
                    reason = "startup";
                    break;
                }
                reason = NULL;
                continue;
            }
            if (reason != NULL || value + offset != absolute ||
                (count > 0u && marker_progress(previous, value, 1u) != NULL) ||
                (count > 1u && marker_progress(before_previous, value, 2u) != NULL)) {
                fprintf(stderr, "marker: qualification frame %u counter %u reason %s offset %u\n",
                        n, value, reason != NULL ? reason : "mapping", offset);
                reason = "qualification";
                break;
            }
            before_previous = previous;
            previous = value;
            ++count;
        }
        if (reason == NULL && count == MARKER_LAST - MARKER_FIRST + 1u &&
            marker_write(argv[3], calibration, strlen(calibration))) {
            printf("marker qualification: %u frames, first=%u last=%u offset=%u "
                   "injective zero/one/two and carry transitions\n",
                   count, MARKER_FIRST, MARKER_LAST, offset);
            result = 0;
        }
    } else {
        size_t cal_size = 0;
        uint8_t *cal = marker_read(argv[3], sizeof calibration, &cal_size);
        int valid_cal = cal != NULL && cal_size == strlen(calibration) &&
                        memcmp(cal, calibration, cal_size) == 0;
        free(cal);
        if (!valid_cal)
            reason = "calibration";
        else if (argc == 5 && strcmp(argv[1], "--marker-observe") == 0) {
            uint32_t value = 0;
            reason = marker_image(argv[4], palette, offset, &value);
            if (reason == NULL) {
                printf("marker frame=%u guest=%u\n", value, value - offset);
                result = 0;
            }
        } else if (argc == 6 && strcmp(argv[1], "--marker-anchor") == 0) {
            char *end;
            unsigned long expected = strtoul(argv[5], &end, 10);
            if (*argv[5] == '\0' || *end != '\0' || expected > MARKER_LAST)
                reason = "expected-origin";
            else {
                /* Bound the read BEFORE allocating: exact state size determines the envelope. */
                FILE *f = fopen(argv[4], "rb");
                size_t limit = 16u + ((nesturbator_state_size(inst) + 7u) & ~(size_t)7u) + 8u;
                uint8_t *bytes = (uint8_t *)malloc(limit + 1u);
                size_t got = f != NULL && bytes != NULL ? fread(bytes, 1u, limit + 1u, f) : 0u;
                if (f != NULL)
                    fclose(f);
                reason =
                    got == 0u ? "state-read" : marker_anchor(inst, bytes, got, expected, offset);
                free(bytes);
                if (reason == NULL) {
                    printf("marker anchor frame=%lu native-valid format=%u revision=%u "
                           "envelope=RASTATE1/MEM/pad8/END continuation=%lu\n",
                           expected, NESTURBATOR_STATE_FORMAT_VERSION,
                           NESTURBATOR_BEHAVIOUR_REVISION, expected + 1u);
                    result = 0;
                }
            }
        } else
            reason = "usage";
    }
    if (result != 0)
        fprintf(stderr, "marker: %s\n", reason != NULL ? reason : "qualification-write");
    nesturbator_destroy(inst);
    return result;
}

/* FNV-1a over the RGB bytes: only used to find equal-frame candidates in --dups, which then
   compares the bytes. */
static uint64_t fnv1a(const uint8_t *p, size_t n)
{
    uint64_t h = 14695981039346656037ull;
    for (size_t i = 0; i < n; i++) {
        h ^= p[i];
        h *= 1099511628211ull;
    }
    return h;
}

int main(int argc, char **argv)
{
    if (argc == 3 && strcmp(argv[1], "--progression-selftest") == 0)
        return progression_tracer_selftest(argv[2]);
    if (argc == 4 && strcmp(argv[1], "--progression-selftest") == 0 &&
        strcmp(argv[3], "--controls") == 0)
        return progression_controls_selftest(argv[2]);
    if (argc == 3 && strcmp(argv[1], "--progression-complete-selftest") == 0)
        return progression_complete_required_selftest(argv[2]);
    if (argc >= 2 && strncmp(argv[1], "--marker-", 9u) == 0)
        return marker_cli(argc, argv);
    int dups = argc >= 2 && strcmp(argv[1], "--dups") == 0;
    int first = dups ? 2 : 1;
    if (argc < 4 || (dups && argc != 4)) {
        fprintf(stderr, "usage: frame_index ROM W IMAGE...\n       frame_index --dups ROM W\n");
        return 2;
    }
    const char *rom_path = argv[first];
    char *end = NULL;
    unsigned long frames = strtoul(argv[first + 1], &end, 10);
    if (end == argv[first + 1] || *end != '\0' || frames == 0ul || frames > MAX_W) {
        fprintf(stderr, "frame_index: W must be a positive count\n");
        return 2;
    }
    int image_count = dups ? 0 : argc - (first + 2);
    uint8_t *images = NULL;
    unsigned *matched = NULL;
    char **match_text = NULL;
    if (image_count > 0) {
        images = (uint8_t *)malloc((size_t)image_count * NESTURBATOR_TEST_RGB_SIZE);
        matched = (unsigned *)calloc((size_t)image_count, sizeof *matched);
        match_text = (char **)calloc((size_t)image_count, sizeof *match_text);
        if (images == NULL || matched == NULL || match_text == NULL) {
            fprintf(stderr, "frame_index: out of memory\n");
            return 2;
        }
        for (int k = 0; k < image_count; k++) {
            const char *name = argv[first + 2 + k];
            uint8_t *dst = images + (size_t)k * NESTURBATOR_TEST_RGB_SIZE;
            int rc;
            if (has_suffix(name, ".ppm")) {
                rc = nesturbator_test_read_ppm(name, dst);
            } else {
                uint32_t w = 0, h = 0;
                rc = nesturbator_test_read_bmp(name, &w, &h, dst, NESTURBATOR_TEST_RGB_SIZE);
                if (rc == 0 && (w != W_PX || h != H_PX))
                    rc = 1;
            }
            if (rc != 0) {
                fprintf(stderr, "frame_index: cannot read a 256x240 image from %s\n", name);
                return 2;
            }
            match_text[k] = (char *)malloc(16u * 1024u);
            if (match_text[k] == NULL) {
                fprintf(stderr, "frame_index: out of memory\n");
                return 2;
            }
            match_text[k][0] = '\0';
        }
    }

    size_t rom_size = 0;
    uint8_t *rom = read_file(rom_path, &rom_size);
    if (rom == NULL) {
        fprintf(stderr, "frame_index: cannot read %s\n", rom_path);
        return 2;
    }
    nesturbator_config cfg;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    nesturbator *inst = NULL;
    if (nesturbator_create(&cfg, &inst) != NESTURBATOR_OK ||
        nesturbator_load_cartridge(inst, rom, rom_size) != NESTURBATOR_OK) {
        fprintf(stderr, "frame_index: cannot load %s\n", rom_path);
        return 2;
    }
    free(rom);

    static uint16_t video[W_PX * H_PX];
    static int16_t audio[4096];
    static uint32_t xrgb[W_PX * H_PX];
    static uint8_t rgb[NESTURBATOR_TEST_RGB_SIZE];
    uint32_t palette[512];
    nesturbator_get_palette(inst, palette, 512u);

    uint64_t *hashes = dups ? (uint64_t *)malloc(frames * sizeof *hashes) : NULL;
    uint8_t *ring =
        dups ? (uint8_t *)malloc((MAX_PARTNER_GAP + 1u) * NESTURBATOR_TEST_RGB_SIZE) : NULL;
    if (dups && (hashes == NULL || ring == NULL)) {
        fprintf(stderr, "frame_index: out of memory\n");
        return 2;
    }
    unsigned long distinct = 0;
    int bad = 0;

    for (unsigned long n = 1; n <= frames; n++) {
        nesturbator_frame io;
        memset(&io, 0, sizeof io);
        io.size = (uint32_t)sizeof io;
        io.video = video;
        io.video_pitch = W_PX;
        io.audio = audio;
        io.audio_capacity = (uint32_t)(sizeof audio / sizeof audio[0]);
        if (nesturbator_run_frame(inst, &io) != NESTURBATOR_OK) {
            fprintf(stderr, "frame_index: frame %lu failed\n", n);
            return 2;
        }
        nesturbator_host_convert(palette, video, W_PX, xrgb, W_PX);
        for (size_t p = 0; p < (size_t)W_PX * H_PX; p++) {
            rgb[p * 3u + 0u] = (uint8_t)(xrgb[p] >> 16);
            rgb[p * 3u + 1u] = (uint8_t)(xrgb[p] >> 8);
            rgb[p * 3u + 2u] = (uint8_t)xrgb[p];
        }
        if (dups) {
            uint64_t h = fnv1a(rgb, sizeof rgb);
            int equal_earlier = 0;
            int partner_near = 0;
            hashes[n - 1u] = h;
            for (unsigned long j = 1; j < n; j++) {
                if (hashes[j - 1u] != h)
                    continue;
                equal_earlier = 1;
                if (n - j <= MAX_PARTNER_GAP) {
                    const uint8_t *prev =
                        ring + (size_t)(j % (MAX_PARTNER_GAP + 1u)) * NESTURBATOR_TEST_RGB_SIZE;
                    if (memcmp(prev, rgb, sizeof rgb) == 0)
                        partner_near = 1;
                }
            }
            if (!equal_earlier)
                distinct++;
            else if (!partner_near) {
                fprintf(stderr,
                        "frame_index: frame %lu repeats an earlier frame more than %u "
                        "frames back\n",
                        n, MAX_PARTNER_GAP);
                bad = 1;
            }
            memcpy(ring + (size_t)(n % (MAX_PARTNER_GAP + 1u)) * NESTURBATOR_TEST_RGB_SIZE, rgb,
                   sizeof rgb);
        } else {
            for (int k = 0; k < image_count; k++) {
                if (memcmp(images + (size_t)k * NESTURBATOR_TEST_RGB_SIZE, rgb, sizeof rgb) != 0)
                    continue;
                size_t used = strlen(match_text[k]);
                if (used + 24u < 16u * 1024u)
                    snprintf(match_text[k] + used, 24u, " %lu", n);
                matched[k]++;
            }
        }
    }
    nesturbator_destroy(inst);

    if (dups) {
        printf("%lu distinct frames in 1..%lu\n", distinct, frames);
        free(hashes);
        free(ring);
        return bad ? 1 : 0;
    }
    int status = 0;
    for (int k = 0; k < image_count; k++) {
        const char *name = argv[first + 2 + k];
        if (matched[k] == 0u) {
            printf("%s none\n", name);
            status = 1;
        } else {
            printf("%s%s\n", name, match_text[k]);
        }
        free(match_text[k]);
    }
    free(match_text);
    free(matched);
    free(images);
    return status;
}
