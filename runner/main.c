/* nesturbator-run: the headless runner.
 *
 *   nesturbator-run --frames N [--rom FILE] [--hash-frame N]... [--dump-frame N:FILE]...
 *
 * Runs N frames, with optional mapper-0 cartridge content; --frames is required. For each
 * --hash-frame N it prints, after frame N has run:
 *
 *   frame <N> ticks <ticks> sha256 <64 lowercase hex digits>
 *
 * The hash is SHA-256 over the 256x240 native pixels as little-endian
 * uint16_t, row-major, top row first; never over the image (D-16).
 *
 * For each --dump-frame N:FILE it writes frame N to FILE as a binary PPM (P6),
 * 256x240, in the RGB of nesturbator_get_palette, converted by the loop the
 * libretro adapter also uses (D-15).
 *
 * Exit status (LIBRETRO-AND-RUNNER section 5): 0 done, 1 failure, 2 usage.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "convert.h"
#include "nesturbator.h"
#include "ppm.h"
#include "sha256.h"

#define WIDTH 256u
#define HEIGHT 240u
#define AUDIO_CAPACITY 1024u

static int usage(const char *why)
{
    fprintf(stderr, "nesturbator-run: %s\n", why);
    fprintf(stderr, "usage: nesturbator-run --frames N [--rom FILE] [--hash-frame N]... "
                    "[--dump-frame N:FILE]...\n"
                    "  --frames N           run N frames (N >= 1)\n"
                    "  --rom FILE           load a mapper-0 iNES image\n"
                    "  --hash-frame N       print the SHA-256 of frame N (1 <= N <= --frames)\n"
                    "  --dump-frame N:FILE  write frame N to FILE as a binary PPM (P6)\n");
    return 2;
}

/* Parses the first len characters of text as a decimal number from 1 to
   4294967295. Returns 0 on any other text. */
static int parse_count_n(const char *text, size_t len, uint32_t *out)
{
    uint64_t v = 0;
    if (text == NULL || len == 0u) {
        return 0;
    }
    for (size_t k = 0; k < len; k++) {
        char c = text[k];
        if (c < '0' || c > '9') {
            return 0;
        }
        v = v * 10u + (uint64_t)(c - '0');
        if (v > 0xffffffffu) {
            return 0;
        }
    }
    if (v == 0u) {
        return 0;
    }
    *out = (uint32_t)v;
    return 1;
}

static int parse_count(const char *text, uint32_t *out)
{
    return text != NULL && parse_count_n(text, strlen(text), out);
}

/* Parses N:FILE, splitting at the first colon so FILE may hold more. */
static int parse_dump(const char *text, uint32_t *frame, const char **path)
{
    const char *colon = text != NULL ? strchr(text, ':') : NULL;
    if (colon == NULL || colon[1] == '\0') {
        return 0;
    }
    if (!parse_count_n(text, (size_t)(colon - text), frame)) {
        return 0;
    }
    *path = colon + 1;
    return 1;
}

/* SHA-256 of the native pixels as little-endian bytes, written byte by byte
   so the result does not depend on the host's byte order. */
static void hash_frame(const uint16_t *video, char hex[65])
{
    nesturbator_run_sha256 s;
    uint8_t row[WIDTH * 2u];
    uint8_t digest[32];
    nesturbator_run_sha256_init(&s);
    for (uint32_t y = 0; y < HEIGHT; y++) {
        for (uint32_t x = 0; x < WIDTH; x++) {
            uint16_t px = video[y * WIDTH + x];
            row[2u * x] = (uint8_t)(px & 0xffu);
            row[2u * x + 1u] = (uint8_t)(px >> 8);
        }
        nesturbator_run_sha256_update(&s, row, sizeof row);
    }
    nesturbator_run_sha256_final(&s, digest);
    nesturbator_run_sha256_hex(digest, hex);
}

/* The runner's options. Each list has room for one entry per argument. */
typedef struct options {
    uint32_t frames;
    const char *rom_path;
    uint32_t *hash_frames;
    uint32_t hash_count;
    uint32_t *dump_frames;
    const char **dump_paths;
    uint32_t dump_count;
} options;

static void free_options(options *o)
{
    free(o->hash_frames);
    free(o->dump_frames);
    free((void *)o->dump_paths);
}

/* Returns 0 when the options are valid, 2 after a usage message, 1 when out of
   memory. */
static int parse_options(int argc, char **argv, options *o)
{
    size_t n_args = (size_t)argc;
    o->hash_frames = (uint32_t *)malloc(n_args * sizeof *o->hash_frames);
    o->dump_frames = (uint32_t *)malloc(n_args * sizeof *o->dump_frames);
    o->dump_paths = (const char **)malloc(n_args * sizeof *o->dump_paths);
    if (o->hash_frames == NULL || o->dump_frames == NULL || o->dump_paths == NULL) {
        fprintf(stderr, "nesturbator-run: out of memory\n");
        return 1;
    }
    /* Every option takes one value, so options sit at odd positions. */
    for (int i = 1; i < argc; i += 2) {
        const char *arg = argv[i];
        const char *value = i + 1 < argc ? argv[i + 1] : NULL;
        uint32_t n;
        if (strcmp(arg, "--rom") == 0) {
            if (value == NULL || value[0] == '\0')
                return usage("--rom needs a file path");
            o->rom_path = value;
        } else if (strcmp(arg, "--dump-frame") == 0) {
            const char *path;
            if (!parse_dump(value, &n, &path)) {
                return usage("--dump-frame needs N:FILE with N of 1 or more");
            }
            o->dump_frames[o->dump_count] = n;
            o->dump_paths[o->dump_count] = path;
            o->dump_count++;
        } else if (strcmp(arg, "--frames") == 0 || strcmp(arg, "--hash-frame") == 0) {
            if (!parse_count(value, &n)) {
                return usage("option needs a whole number of 1 or more");
            }
            if (strcmp(arg, "--frames") == 0) {
                o->frames = n;
            } else {
                o->hash_frames[o->hash_count++] = n;
            }
        } else {
            return usage(arg[0] == '-' ? "unknown option" : "unexpected argument");
        }
    }
    if (o->frames == 0u) {
        return usage("--frames N is required");
    }
    for (uint32_t k = 0; k < o->hash_count; k++) {
        if (o->hash_frames[k] > o->frames) {
            return usage("--hash-frame is beyond --frames");
        }
    }
    for (uint32_t k = 0; k < o->dump_count; k++) {
        if (o->dump_frames[k] > o->frames) {
            return usage("--dump-frame is beyond --frames");
        }
    }
    return 0;
}

static int listed(const uint32_t *list, uint32_t count, uint32_t f)
{
    for (uint32_t k = 0; k < count; k++) {
        if (list[k] == f) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    static uint16_t video[WIDTH * HEIGHT];
    static int16_t audio[AUDIO_CAPACITY];
    static uint32_t palette[512];
    static uint32_t image[WIDTH * HEIGHT];
    options opt;
    int status;

    memset(&opt, 0, sizeof opt);
    status = parse_options(argc, argv, &opt);
    if (status != 0) {
        free_options(&opt);
        return status;
    }

    nesturbator_config cfg;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    nesturbator *inst = NULL;
    nesturbator_status st = nesturbator_create(&cfg, &inst);
    if (st != NESTURBATOR_OK) {
        fprintf(stderr, "nesturbator-run: nesturbator_create failed with status %d\n", (int)st);
        free_options(&opt);
        return 1;
    }
    if (opt.rom_path != NULL) {
        FILE *rom = fopen(opt.rom_path, "rb");
        long length;
        uint8_t *bytes;
        if (rom == NULL || fseek(rom, 0, SEEK_END) != 0 || (length = ftell(rom)) <= 0 ||
            fseek(rom, 0, SEEK_SET) != 0) {
            if (rom != NULL)
                fclose(rom);
            fprintf(stderr, "nesturbator-run: cannot read cartridge %s\n", opt.rom_path);
            nesturbator_destroy(inst);
            free_options(&opt);
            return 1;
        }
        bytes = (uint8_t *)malloc((size_t)length);
        if (bytes == NULL || fread(bytes, 1, (size_t)length, rom) != (size_t)length) {
            free(bytes);
            fclose(rom);
            fprintf(stderr, "nesturbator-run: cannot read cartridge %s\n", opt.rom_path);
            nesturbator_destroy(inst);
            free_options(&opt);
            return 1;
        }
        fclose(rom);
        st = nesturbator_load_cartridge(inst, bytes, (size_t)length);
        free(bytes);
        if (st != NESTURBATOR_OK) {
            fprintf(stderr, "nesturbator-run: unsupported cartridge (status %d)\n", (int)st);
            nesturbator_destroy(inst);
            free_options(&opt);
            return 1;
        }
    }
    if (opt.dump_count > 0u) {
        nesturbator_get_palette(inst, palette, 512u);
    }

    /* A 64-bit count, so the loop ends when --frames is 4294967295. */
    for (uint64_t n = 1; n <= opt.frames && status == 0; n++) {
        uint32_t f = (uint32_t)n;
        nesturbator_frame io;
        memset(&io, 0, sizeof io);
        io.size = (uint32_t)sizeof io;
        io.video = video;
        io.video_pitch = WIDTH;
        io.audio = audio;
        io.audio_capacity = AUDIO_CAPACITY;
        st = nesturbator_run_frame(inst, &io);
        if (st != NESTURBATOR_OK) {
            fprintf(stderr, "nesturbator-run: nesturbator_run_frame failed with status %d\n",
                    (int)st);
            status = 1;
            break;
        }
        if (listed(opt.hash_frames, opt.hash_count, f)) {
            char hex[65];
            hash_frame(video, hex);
            printf("frame %lu ticks %llu sha256 %s\n", (unsigned long)f,
                   (unsigned long long)io.ticks, hex);
        }
        if (listed(opt.dump_frames, opt.dump_count, f)) {
            nesturbator_host_convert(palette, video, WIDTH, image, WIDTH);
            for (uint32_t k = 0; k < opt.dump_count; k++) {
                if (opt.dump_frames[k] == f &&
                    nesturbator_run_write_ppm(opt.dump_paths[k], image, WIDTH) != 0) {
                    fprintf(stderr, "nesturbator-run: cannot write %s\n", opt.dump_paths[k]);
                    status = 1;
                }
            }
        }
    }

    nesturbator_destroy(inst);
    free_options(&opt);
    if (fflush(stdout) != 0) {
        fprintf(stderr, "nesturbator-run: cannot write output\n");
        return 1;
    }
    return status;
}
