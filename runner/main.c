/* nesturbator-run: the headless runner.
 *
 *   nesturbator-run --frames N [--rom FILE] [--hash-frame N]... [--dump-frame N:FILE]...
 *   nesturbator-run --movie FILE [--rom FILE] [--dump-frame N:FILE]...
 *   nesturbator-run --accuracycoin-page N --rom FILE --scoreboard FILE
 *
 * Runs N frames, with optional mapper-0 cartridge content. A movie supplies the
 * two port masks for every frame and prints the native hash of every replayed frame.
 * For each --hash-frame N it prints, after frame N has run:
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
#include "movie.h"
#include "nesturbator.h"
#include "ppm.h"
#include "sha256.h"

#define WIDTH 256u
#define HEIGHT 240u
#define AUDIO_CAPACITY 1024u

static int usage(const char *why)
{
    fprintf(stderr, "nesturbator-run: %s\n", why);
    fprintf(stderr,
            "usage: nesturbator-run (--frames N | --movie FILE) [--rom FILE] [--hash-frame N]... "
            "[--dump-frame N:FILE]...\n"
            "  --frames N           run N frames (N >= 1)\n"
            "  --movie FILE         replay a validated two-port movie\n"
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
    const char *movie_path;
    const char *rom_path;
    const char *scoreboard_path;
    uint32_t accuracy_page;
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
        } else if (strcmp(arg, "--scoreboard") == 0) {
            if (value == NULL || value[0] == '\0' || o->scoreboard_path != NULL)
                return usage("--scoreboard needs one file path");
            o->scoreboard_path = value;
        } else if (strcmp(arg, "--accuracycoin-page") == 0) {
            if (!parse_count(value, &n) || o->accuracy_page != 0u)
                return usage("--accuracycoin-page needs one page number");
            o->accuracy_page = n;
        } else if (strcmp(arg, "--movie") == 0) {
            if (value == NULL || value[0] == '\0' || o->movie_path != NULL)
                return usage("--movie needs one file path");
            o->movie_path = value;
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
    if (o->accuracy_page != 0u) {
        if (o->accuracy_page != 2u && o->accuracy_page != 17u)
            return usage("AccuracyCoin supports pages 2 and 17");
        if (o->rom_path == NULL || o->scoreboard_path == NULL || o->movie_path != NULL ||
            o->frames != 0u)
            return usage("AccuracyCoin page mode needs --rom and --scoreboard only");
        o->frames = 1800u;
    } else if (o->scoreboard_path != NULL) {
        return usage("--scoreboard requires --accuracycoin-page");
    }
    if (o->frames == 0u && o->movie_path == NULL) {
        return usage("--frames N is required");
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

#define ACCURACY_MAX_TESTS 32u
typedef struct accuracy_test {
    char name[64];
    uint16_t result_address;
} accuracy_test;

static uint16_t read_le16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static int compare_accuracy_test(const void *left, const void *right)
{
    const accuracy_test *a = (const accuracy_test *)left;
    const accuracy_test *b = (const accuracy_test *)right;
    return strcmp(a->name, b->name);
}

static int accuracy_rom_offset(uint16_t address, size_t size, size_t *offset)
{
    if (address < 0x8000u)
        return 0;
    size_t result = 16u + (size_t)(address - 0x8000u);
    if (result >= size)
        return 0;
    *offset = result;
    return 1;
}

/* Read the page directory embedded in the pinned ROM; names and result
   addresses are never duplicated in project code. */
static int accuracy_page_tests(const char *path, uint32_t page, accuracy_test *tests,
                               size_t *test_count)
{
    FILE *file = fopen(path, "rb");
    uint8_t *bytes = NULL;
    long length;
    size_t first_page_offset;
    size_t count;
    int ok = 0;
    if (file == NULL || fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0)
        goto done;
    bytes = (uint8_t *)malloc((size_t)length);
    if (bytes == NULL || fread(bytes, 1, (size_t)length, file) != (size_t)length ||
        length < 0x140 || memcmp(bytes, "NES\x1a", 4u) != 0)
        goto done;
    if (!accuracy_rom_offset(read_le16(bytes + 0x110), (size_t)length, &first_page_offset) ||
        first_page_offset < 0x110u || (first_page_offset - 0x110u) % 2u != 0u)
        goto done;
    count = (first_page_offset - 0x110u) / 2u;
    if (count != 22u || page == 0u || page > count)
        goto done;
    uint16_t address = read_le16(bytes + 0x110u + (page - 1u) * 2u);
    size_t at;
    if (!accuracy_rom_offset(address, (size_t)length, &at))
        goto done;
    while (at < (size_t)length && bytes[at] != 0xffu)
        at++;
    if (at == (size_t)length)
        goto done;
    at++;
    *test_count = 0u;
    while (at < (size_t)length && bytes[at] != 0xffu) {
        size_t begin = at;
        while (at < (size_t)length && bytes[at] != 0xffu)
            at++;
        size_t name_len = at - begin;
        if (at == (size_t)length || name_len == 0u || name_len >= sizeof tests[0].name ||
            *test_count >= ACCURACY_MAX_TESTS || at + 5u >= (size_t)length)
            goto done;
        memcpy(tests[*test_count].name, bytes + begin, name_len);
        tests[*test_count].name[name_len] = '\0';
        at++;
        tests[*test_count].result_address = read_le16(bytes + at);
        at += 4u; /* result address and test routine pointer */
        if (tests[*test_count].result_address >= 0x2000u)
            goto done;
        (*test_count)++;
    }
    ok = at < (size_t)length && *test_count > 0u;
done:
    free(bytes);
    if (file != NULL)
        fclose(file);
    return ok;
}

static int accuracy_ram(const nesturbator *inst, uint16_t address, uint8_t *out)
{
    return nesturbator_peek_cpu_ram(inst, address, out) == NESTURBATOR_OK;
}

static nesturbator_status accuracy_frame(nesturbator *inst, uint16_t buttons, uint16_t *video,
                                         int16_t *audio)
{
    nesturbator_input input;
    nesturbator_frame frame;
    memset(&input, 0, sizeof input);
    input.size = (uint32_t)sizeof input;
    input.buttons[0] = (uint8_t)buttons;
    nesturbator_status st = nesturbator_set_input(inst, &input);
    if (st != NESTURBATOR_OK)
        return st;
    memset(&frame, 0, sizeof frame);
    frame.size = (uint32_t)sizeof frame;
    frame.video = video;
    frame.video_pitch = WIDTH;
    frame.audio = audio;
    frame.audio_capacity = AUDIO_CAPACITY;
    return nesturbator_run_frame(inst, &frame);
}

static int accuracy_compare_scoreboard(const char *path, const accuracy_test *tests,
                                       const uint8_t *results, size_t count, uint32_t page)
{
    FILE *file = fopen(path, "r");
    char line[256];
    size_t found = 0u;
    if (file == NULL)
        return 0;
    while (fgets(line, sizeof line, file) != NULL) {
        if (strchr(line, '\n') == NULL && !feof(file)) {
            fclose(file);
            return 0;
        }
        for (size_t i = 0; i < count; i++) {
            char key[128];
            char expected[256];
            (void)snprintf(key, sizeof key, "accuracycoin/%s\t", tests[i].name);
            if (strncmp(line, key, strlen(key)) == 0) {
                uint8_t result = results[i];
                const char *status =
                    (result & 3u) == 1u ? "pass" : (result == 0xffu ? "skip" : "fail");
                (void)snprintf(expected, sizeof expected, "accuracycoin/%s\t%s\t0x%02x\t-\t-\n",
                               tests[i].name, status, result);
                if (strcmp(line, expected) != 0 || (found & ((size_t)1u << i)) != 0u) {
                    fclose(file);
                    return 0;
                }
                found |= (size_t)1u << i;
            }
        }
    }
    fclose(file);
    if (count > sizeof(size_t) * 8u || found != (((size_t)1u << count) - 1u))
        return 0;
    (void)page;
    return 1;
}

static int run_accuracycoin(nesturbator *inst, const options *opt, uint16_t *video, int16_t *audio)
{
    accuracy_test tests[ACCURACY_MAX_TESTS];
    uint8_t results[ACCURACY_MAX_TESTS];
    size_t count = 0u;
    uint32_t frames = 0u;
    uint8_t value = 0u;
    if (!accuracy_page_tests(opt->rom_path, opt->accuracy_page, tests, &count)) {
        fprintf(stderr, "nesturbator-run: invalid or missing AccuracyCoin directory\n");
        return 1;
    }
    qsort(tests, count, sizeof tests[0], compare_accuracy_test);
    for (size_t i = 1u; i < count; i++) {
        if (strcmp(tests[i - 1u].name, tests[i].name) == 0) {
            fprintf(stderr, "nesturbator-run: duplicate AccuracyCoin test name '%s'\n",
                    tests[i].name);
            return 1;
        }
    }
    /* The menu reports ready at $00EC. Boot and every input pulse are bounded. */
    while (frames < 120u) {
        nesturbator_status st = accuracy_frame(inst, 0u, video, audio);
        frames++;
        if (st != NESTURBATOR_OK || !accuracy_ram(inst, 0x00ecu, &value))
            return 1;
        if (value == 0x0au)
            break;
    }
    if (value != 0x0au) {
        fprintf(stderr,
                "nesturbator-run: AccuracyCoin menu did not become ready within 120 frames\n");
        return 1;
    }
    for (uint32_t p = 1u; p < opt->accuracy_page; p++) {
        if (accuracy_frame(inst, NESTURBATOR_BUTTON_RIGHT, video, audio) != NESTURBATOR_OK ||
            accuracy_frame(inst, 0u, video, audio) != NESTURBATOR_OK) {
            fprintf(stderr, "nesturbator-run: AccuracyCoin page navigation failed\n");
            return 1;
        }
        frames += 2u;
        /* DrawNewSuiteTable disables NMI while it replaces the page. Give it
           several frame boundaries before sending the next edge-sensitive key. */
        for (uint32_t settle = 0u; settle < 3u; settle++) {
            if (accuracy_frame(inst, 0u, video, audio) != NESTURBATOR_OK)
                return 1;
            frames++;
        }
    }
    uint8_t selected_page = 0xffu;
    uint8_t cursor = 0u;
    if (!accuracy_ram(inst, 0x0014u, &selected_page) || !accuracy_ram(inst, 0x0016u, &cursor) ||
        selected_page != (uint8_t)(opt->accuracy_page - 1u) || cursor != 0xffu) {
        fprintf(
            stderr,
            "nesturbator-run: AccuracyCoin menu selection mismatch (page-index=%u cursor=0x%02x)\n",
            selected_page, cursor);
        return 1;
    }
    if (accuracy_frame(inst, NESTURBATOR_BUTTON_A, video, audio) != NESTURBATOR_OK ||
        accuracy_frame(inst, NESTURBATOR_BUTTON_A, video, audio) != NESTURBATOR_OK) {
        fprintf(stderr, "nesturbator-run: AccuracyCoin page start press failed\n");
        return 1;
    }
    if (accuracy_frame(inst, 0u, video, audio) != NESTURBATOR_OK) {
        fprintf(stderr, "nesturbator-run: AccuracyCoin page start failed\n");
        return 1;
    }
    frames += 3u;
    int complete = 0;
    while (frames < opt->frames) {
        complete = 1;
        for (size_t i = 0; i < count; i++) {
            if (!accuracy_ram(inst, tests[i].result_address, &results[i]))
                return 1;
            if (results[i] == 0u || (results[i] & 3u) == 3u)
                complete = 0;
        }
        if (complete)
            break;
        if (accuracy_frame(inst, 0u, video, audio) != NESTURBATOR_OK)
            return 1;
        frames++;
    }
    if (!complete || frames >= opt->frames) {
        for (size_t i = 0; i < count; i++) {
            (void)accuracy_ram(inst, tests[i].result_address, &results[i]);
            fprintf(stderr, "nesturbator-run: timeout result '%s'=0x%02x\n", tests[i].name,
                    results[i]);
        }
        (void)accuracy_ram(inst, 0x00ecu, &value);
        selected_page = 0u;
        cursor = 0u;
        uint8_t page_running = 0u;
        (void)accuracy_ram(inst, 0x0014u, &selected_page);
        (void)accuracy_ram(inst, 0x0016u, &cursor);
        (void)accuracy_ram(inst, 0x0034u, &page_running);
        fprintf(stderr, "nesturbator-run: AccuracyCoin page %u timed out after %u frames\n",
                opt->accuracy_page, frames);
        fprintf(stderr,
                "nesturbator-run: menu $00ec=0x%02x page-index=%u cursor=0x%02x suite-running=%u\n",
                value, selected_page, cursor, page_running);
        return 1;
    }
    int all_passed = 1;
    for (size_t i = 0; i < count; i++) {
        if (!accuracy_ram(inst, tests[i].result_address, &results[i]))
            return 1;
        uint8_t code = results[i] & 3u;
        const char *status = code == 1u ? "pass" : (results[i] == 0xffu ? "skip" : "fail");
        printf("accuracycoin/%s\t%s\t0x%02x\t-\t-\n", tests[i].name, status, results[i]);
        if (code != 1u) {
            fprintf(stderr, "nesturbator-run: AccuracyCoin page %u test '%s' returned 0x%02x\n",
                    opt->accuracy_page, tests[i].name, results[i]);
            all_passed = 0;
        }
    }
    if (!all_passed)
        return 1;
    if (!accuracy_compare_scoreboard(opt->scoreboard_path, tests, results, count,
                                     opt->accuracy_page)) {
        fprintf(stderr,
                "nesturbator-run: AccuracyCoin page %u RAM results differ from scoreboard\n",
                opt->accuracy_page);
        return 1;
    }
    printf("accuracycoin page %u PASS (%lu tests, %u frames)\n", opt->accuracy_page,
           (unsigned long)count, frames);
    return 0;
}

int main(int argc, char **argv)
{
    static uint16_t video[WIDTH * HEIGHT];
    static int16_t audio[AUDIO_CAPACITY];
    static uint32_t palette[512];
    static uint32_t image[WIDTH * HEIGHT];
    options opt;
    nesturbator_movie movie;
    int status;

    memset(&opt, 0, sizeof opt);
    memset(&movie, 0, sizeof movie);
    status = parse_options(argc, argv, &opt);
    if (status != 0) {
        free_options(&opt);
        return status;
    }

    if (opt.movie_path != NULL) {
        if (!nesturbator_movie_read(opt.movie_path, &movie)) {
            fprintf(stderr, "nesturbator-run: malformed or unreadable movie %s\n", opt.movie_path);
            free_options(&opt);
            return 1;
        }
        if (opt.frames != 0u && opt.frames != movie.frame_count) {
            fprintf(stderr, "nesturbator-run: --frames must match the movie frame count\n");
            nesturbator_movie_free(&movie);
            free_options(&opt);
            return 2;
        }
        opt.frames = movie.frame_count;
    }
    for (uint32_t k = 0; k < opt.hash_count; k++) {
        if (opt.hash_frames[k] > opt.frames) {
            status = usage("--hash-frame is beyond the requested frame count");
            nesturbator_movie_free(&movie);
            free_options(&opt);
            return status;
        }
    }
    for (uint32_t k = 0; k < opt.dump_count; k++) {
        if (opt.dump_frames[k] > opt.frames) {
            status = usage("--dump-frame is beyond the requested frame count");
            nesturbator_movie_free(&movie);
            free_options(&opt);
            return status;
        }
    }

    nesturbator_config cfg;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    nesturbator *inst = NULL;
    nesturbator_status st = nesturbator_create(&cfg, &inst);
    if (st != NESTURBATOR_OK) {
        fprintf(stderr, "nesturbator-run: nesturbator_create failed with status %d\n", (int)st);
        nesturbator_movie_free(&movie);
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
            nesturbator_movie_free(&movie);
            free_options(&opt);
            return 1;
        }
        bytes = (uint8_t *)malloc((size_t)length);
        if (bytes == NULL || fread(bytes, 1, (size_t)length, rom) != (size_t)length) {
            free(bytes);
            fclose(rom);
            fprintf(stderr, "nesturbator-run: cannot read cartridge %s\n", opt.rom_path);
            nesturbator_destroy(inst);
            nesturbator_movie_free(&movie);
            free_options(&opt);
            return 1;
        }
        fclose(rom);
        st = nesturbator_load_cartridge(inst, bytes, (size_t)length);
        free(bytes);
        if (st != NESTURBATOR_OK) {
            fprintf(stderr,
                    "nesturbator-run: malformed or unsupported mapper-0 cartridge (status %d)\n",
                    (int)st);
            nesturbator_destroy(inst);
            nesturbator_movie_free(&movie);
            free_options(&opt);
            return 1;
        }
    }
    if (opt.dump_count > 0u) {
        nesturbator_get_palette(inst, palette, 512u);
    }

    if (opt.accuracy_page != 0u) {
        status = run_accuracycoin(inst, &opt, video, audio);
        nesturbator_destroy(inst);
        nesturbator_movie_free(&movie);
        free_options(&opt);
        return status;
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
        if (opt.movie_path != NULL) {
            nesturbator_input input;
            memset(&input, 0, sizeof input);
            input.size = (uint32_t)sizeof input;
            input.buttons[0] = movie.masks[(size_t)(f - 1u) * 2u];
            input.buttons[1] = movie.masks[(size_t)(f - 1u) * 2u + 1u];
            st = nesturbator_set_input(inst, &input);
            if (st != NESTURBATOR_OK) {
                fprintf(stderr, "nesturbator-run: cannot set movie input at frame %u\n", f);
                status = 1;
                break;
            }
        }
        st = nesturbator_run_frame(inst, &io);
        if (st != NESTURBATOR_OK) {
            if (st == NESTURBATOR_STOP_JAM)
                fprintf(stderr, "nesturbator-run: JAM stopped replay at frame %u\n", f);
            else
                fprintf(stderr, "nesturbator-run: frame %u failed with status %d\n", f, (int)st);
            status = 1;
            break;
        }
        if ((opt.movie_path != NULL || listed(opt.hash_frames, opt.hash_count, f))) {
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
    nesturbator_movie_free(&movie);
    free_options(&opt);
    if (fflush(stdout) != 0) {
        fprintf(stderr, "nesturbator-run: cannot write output\n");
        return 1;
    }
    return status;
}
