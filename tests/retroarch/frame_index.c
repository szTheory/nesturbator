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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define W_PX 256u
#define H_PX 240u
#define MAX_PARTNER_GAP 3u
#define MAX_W 100000u

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
