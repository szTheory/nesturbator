/* nesturbator-run: the headless runner.
 *
 *   nesturbator-run [--frames N] [--hash-frame N]...
 *
 * Runs N frames of one instance with no cartridge. For each --hash-frame N it
 * prints, after frame N has run:
 *
 *   frame <N> ticks <ticks> sha256 <64 lowercase hex digits>
 *
 * The hash is SHA-256 over the 256x240 native pixels as little-endian
 * uint16_t, row-major, top row first.
 *
 * Exit status (LIBRETRO-AND-RUNNER section 5): 0 done, 1 failure, 2 usage.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nesturbator.h"
#include "sha256.h"

#define WIDTH 256u
#define HEIGHT 240u
#define AUDIO_CAPACITY 1024u

static int usage(const char *why)
{
    fprintf(stderr, "nesturbator-run: %s\n", why);
    fprintf(stderr, "usage: nesturbator-run [--frames N] [--hash-frame N]...\n"
                    "  --frames N      run N frames (N >= 1)\n"
                    "  --hash-frame N  print the SHA-256 of frame N (1 <= N <= --frames)\n");
    return 2;
}

/* Parses a decimal number from 1 to 4294967295. Returns 0 on any other text. */
static int parse_count(const char *text, uint32_t *out)
{
    uint64_t v = 0;
    if (text == NULL || *text == '\0') {
        return 0;
    }
    for (const char *p = text; *p != '\0'; p++) {
        if (*p < '0' || *p > '9') {
            return 0;
        }
        v = v * 10u + (uint64_t)(*p - '0');
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

int main(int argc, char **argv)
{
    static uint16_t video[WIDTH * HEIGHT];
    static int16_t audio[AUDIO_CAPACITY];
    uint32_t frames = 0;
    uint32_t *hash_frames = NULL;
    uint32_t hash_count = 0;
    int status = 0;

    if (argc > 1) {
        hash_frames = (uint32_t *)malloc((size_t)argc * sizeof *hash_frames);
        if (hash_frames == NULL) {
            fprintf(stderr, "nesturbator-run: out of memory\n");
            return 1;
        }
    }
    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        int is_frames = strcmp(arg, "--frames") == 0;
        int is_hash = strcmp(arg, "--hash-frame") == 0;
        if (!is_frames && !is_hash) {
            free(hash_frames);
            return usage(arg[0] == '-' ? "unknown option" : "unexpected argument");
        }
        uint32_t n;
        if (i + 1 >= argc || !parse_count(argv[i + 1], &n)) {
            free(hash_frames);
            return usage("option needs a whole number of 1 or more");
        }
        i++;
        if (is_frames) {
            frames = n;
        } else {
            hash_frames[hash_count++] = n;
        }
    }
    for (uint32_t k = 0; k < hash_count; k++) {
        if (hash_frames[k] > frames) {
            free(hash_frames);
            return usage("--hash-frame is beyond --frames");
        }
    }

    nesturbator_config cfg = NESTURBATOR_CONFIG_INIT;
    nesturbator *inst = NULL;
    nesturbator_status st = nesturbator_create(&cfg, &inst);
    if (st != NESTURBATOR_OK) {
        fprintf(stderr, "nesturbator-run: nesturbator_create failed with status %d\n", (int)st);
        free(hash_frames);
        return 1;
    }

    for (uint32_t f = 1; f <= frames && status == 0; f++) {
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
        int wanted = 0;
        for (uint32_t k = 0; k < hash_count; k++) {
            if (hash_frames[k] == f) {
                wanted = 1;
            }
        }
        if (wanted) {
            char hex[65];
            hash_frame(video, hex);
            printf("frame %lu ticks %llu sha256 %s\n", (unsigned long)f,
                   (unsigned long long)io.ticks, hex);
        }
    }

    nesturbator_destroy(inst);
    free(hash_frames);
    if (fflush(stdout) != 0) {
        fprintf(stderr, "nesturbator-run: cannot write output\n");
        return 1;
    }
    return status;
}
