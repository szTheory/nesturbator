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
#include "state_test.h"

#include <stdint.h>
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
    (void)before;
    (void)after;
    (void)steps;
    return NULL;
}

static int marker_selftest(void)
{
    if (marker_pixel_control() != 0)
        return 1;
    const char *duplicate = marker_progress(255u, 255u, 1u);
    if (duplicate == NULL || strcmp(duplicate, "duplicate-identity") != 0) {
        fprintf(stderr, "ASSERT duplicate qualification identity: expected duplicate-identity, "
                        "actual %s\n", duplicate != NULL ? duplicate : "accepted");
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
    for (unsigned n = 1; n <= 150u; ++n)
        if (!marker_frame(inst, rgb, &absolute))
            return 2;
    size_t size = nesturbator_state_size(inst);
    size_t padded = (size + 7u) & ~(size_t)7u;
    size_t length = 16u + padded + 8u;
    uint8_t *bytes = (uint8_t *)calloc(length, 1u);
    if (bytes == NULL)
        return 2;
    memcpy(bytes, "RASTATE\001MEM ", 12u);
    put32(bytes + 12u, (uint32_t)size);
    memcpy(bytes + 16u + padded, "END ", 4u);
    if (nesturbator_save_state(inst, bytes + 16u, size) != NESTURBATOR_OK)
        return 2;
    const char *reason = marker_anchor(inst, bytes, length, 150u, offset);
    if (reason != NULL) {
        fprintf(stderr, "ASSERT valid native origin: %s\n", reason);
        return 1;
    }
    put32(bytes + 32u, NESTURBATOR_BEHAVIOUR_REVISION - 1u);
    st_recrc(bytes + 16u, size);
    if (nesturbator_load_state(inst, bytes + 16u, size) != NESTURBATOR_OK)
        return 2;
    reason = marker_anchor(inst, bytes, length, 150u, offset);
    if (reason == NULL || strcmp(reason, "native-revision") != 0) {
        fprintf(stderr, "ASSERT native-valid stale revision: expected native-revision, got %s\n",
                reason != NULL ? reason : "accepted");
        return 1;
    }
    free(bytes);
    nesturbator_destroy(inst);
    printf("marker controls: 3 cases (identity, native origin, native-valid stale revision)\n");
    return 0;
}

static int marker_cli(int argc, char **argv)
{
    static uint8_t expected_rom[MARKER_ROM_SIZE];
    marker_rom(expected_rom);
    if (argc == 3 && strcmp(argv[1], "--marker-generate") == 0)
        return marker_write(argv[2], expected_rom, sizeof expected_rom) ? 0 : 2;
    if (argc == 3 && strcmp(argv[1], "--marker-selftest") == 0)
        return marker_selftest();
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
                (count > 0u && value != previous + 1u) ||
                (count > 1u && value != before_previous + 2u)) {
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
            int read_result;
            if (has_suffix(argv[4], ".ppm"))
                read_result = nesturbator_test_read_ppm(argv[4], rgb);
            else {
                uint32_t w = 0, h = 0;
                read_result = nesturbator_test_read_bmp(argv[4], &w, &h, rgb, sizeof rgb);
                if (w != W_PX || h != H_PX)
                    read_result = 1;
            }
            uint32_t value = 0;
            reason =
                read_result != 0 ? "image-geometry-or-format" : marker_decode(rgb, palette, &value);
            if (reason == NULL && (value + offset < MARKER_FIRST || value + offset > MARKER_LAST))
                reason = "window";
            if (reason == NULL) {
                printf("marker frame=%u guest=%u\n", value + offset, value);
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
