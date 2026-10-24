/* nesturbator-run: the headless runner.
 *
 *   nesturbator-run --frames N [--rom FILE] [--hash-frame N]... [--hash-audio] [--dump-frame
 * N:FILE]... nesturbator-run --movie FILE [--rom FILE] [--hash-audio] [--dump-frame N:FILE]...
 *   nesturbator-run --accuracycoin-page N --rom FILE --scoreboard FILE
 *   nesturbator-run --accuracycoin-inventory --rom FILE
 *   nesturbator-run --accuracycoin-observe PAGE --rom FILE --frames N --accuracycoin-output DIR
 *   nesturbator-run --accuracycoin-collect DIR --rom FILE
 *   nesturbator-run --version
 *   nesturbator-run --help
 *
 * --help prints the usage text and the exit statuses to stdout and exits 0; a bad command line
 * prints the same text to stderr and exits 2. --version prints "nesturbator-run X.Y.Z" from
 * nesturbator_get_version and exits 0.
 *
 * Runs N frames, with optional cartridge content (mappers 0, 1, 2, 3, 4 and 7). A movie supplies
 * the two port masks for every frame and prints the native hash of every replayed frame. For each
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
 * With --save-dir DIR a cartridge that has battery RAM reads DIR/<name>.sav after it loads and
 * before the first frame, and writes the raw bytes back when they changed. <name> is the last
 * component of the --rom path, split at '/' or '\\', without its last extension. A write goes
 * to DIR/<name>.sav.tmp and is renamed over the save, so the save is the old one or the new one,
 * never torn. A failed write removes its .tmp and keeps the previous save. The runner writes at the
 * end of the run (also after a JAM) and, with --save-interval N, after every Nth frame; a write
 * that fails at an interval is tried once more at exit. A save whose size is not the battery RAM's
 * is never touched. Without --save-dir no save file is read or written, and neither is one for a
 * cartridge without battery RAM.
 *
 * Exit statuses are defined once in exit_status.h (0 done, 1 failure, 2 usage error, 4 .sav size
 * mismatch, 5 state rejected; 3 and 77 are reserved), printed by --help and listed in the README
 * section Exit status (LIBRETRO-AND-RUNNER section 5).
 *
 * Stopping: SIGINT and SIGTERM (POSIX signal) and, on Windows, Ctrl+C, Ctrl+Break and console
 * close (Microsoft Learn: HandlerRoutine, SetConsoleCtrlHandler) make the runner finish the
 * current frame, write the battery save, print a stop line on stderr and exit 0, or 1 if the save
 * failed. A second signal ends it at once. --raise-after N[:SIG[:K]] signals this process K times
 * after frame N (a test aid); SIG is int or term on POSIX (default term), int or break on Windows
 * (default break). --accuracycoin-page installs no stop handlers; a signal there has its default
 * action. A stop that missed a requested output (a --hash-frame, --hash-state, --state-at-frame
 * or --dump-frame beyond the last finished frame, --hash-audio, the end of a --movie) exits 1,
 * names the first such option on stderr after "not reached:" and prints no audio hash; a
 * --frames count not reached is not a failure. See runner/stop.h and the README section Stopping
 * the runner.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "convert.h"
#include "audio_hash.h"
#include "accuracycoin.h"
#include "exit_status.h"
#include "stop.h"
#include "bench.h"
#include "clock.h"
#include "internal.h"
#include "movie.h"
#include "nesturbator.h"
#include "ppm.h"
#include "save.h"
#include "sha256.h"
#include "state_file.h"

#define WIDTH 256u
#define HEIGHT 240u
#define AUDIO_CAPACITY 1024u

static void print_usage(FILE *out)
{
    fprintf(
        out,
        "usage: nesturbator-run (--frames N | --movie FILE | --bench N) [--rom FILE] [--hash-frame "
        "N]... "
        "[--hash-audio] [--dump-frame N:FILE]... [--save-dir DIR [--save-interval N]]\n"
        "                      [--state-at-frame N:FILE]... [--hash-state N]... "
        "[--load-state FILE]\n"
        "                      [--raise-after N[:SIG[:K]]]\n"
        "       nesturbator-run --accuracycoin-page N --rom FILE --scoreboard FILE\n"
        "       nesturbator-run --accuracycoin-inventory --rom FILE\n"
        "       nesturbator-run --accuracycoin-observe PAGE --rom FILE --frames N "
        "--accuracycoin-output DIR\n"
        "       nesturbator-run --accuracycoin-collect DIR --rom FILE\n"
        "       nesturbator-run --version\n"
        "       nesturbator-run --help\n"
        "  --frames N           run N frames (N >= 1)\n"
        "  --movie FILE         replay a validated two-port movie\n"
        "  --bench N            time N frames (1 to 1000000) and print frame-time statistics to "
        "stderr; needs --rom\n"
        "  --bench-state        with --bench, also save and load the state after every frame and "
        "time each\n"
        "  --rom FILE           load an iNES image (mapper 0, 1, 2, 3, 4 or 7)\n"
        "  --hash-frame N       print the SHA-256 of frame N (1 <= N <= --frames)\n"
        "  --hash-audio         print SHA-256 of canonical transitions and signed PCM\n"
        "  --dump-frame N:FILE  write frame N to FILE as a binary PPM (P6)\n"
        "  --save-dir DIR       load and keep DIR/<rom name>.sav (needs --rom)\n"
        "  --save-interval N    with --save-dir, also write a changed save every N frames\n"
        "  --state-at-frame N:FILE  write the save state after frame N (N >= 0) to FILE\n"
        "  --load-state FILE    resume from a save state; frame numbers stay absolute\n"
        "  --hash-state N       print the SHA-256 of the save state after frame N (N >= 0)\n"
        "  --raise-after N[:SIG[:K]]  test aid: after frame N, signal this process K times (1 "
        "or 2); SIG int or term (default term), on Windows int or break (default break)\n"
        "  --accuracycoin-page N  strictly check page N (2, 14 or 17) for 1800 frames; needs "
        "--rom and --scoreboard\n"
        "  --scoreboard FILE    with --accuracycoin-page, the scoreboard to compare against\n"
        "  --accuracycoin-inventory  print the selected ROM pin's verified inventory as TSV\n"
        "  --accuracycoin-observe PAGE  record bounded guest results without strict-pass gating\n"
        "  --accuracycoin-output DIR  with --accuracycoin-observe, write checked evidence files\n"
        "  --accuracycoin-boot-cap N  with observation, maximum cold-boot frames (default 120)\n"
        "  --accuracycoin-navigation-cap N  with observation, frames per page switch (default 5)\n"
        "  --accuracycoin-execution-cap N  with observation, post-release guest frames\n"
        "  --accuracycoin-collect DIR  validate and classify one 22-page campaign\n"
        "  --version            print the version and exit\n"
        "  --help               print this text and the exit statuses, and exit 0\n"
        "Exit status:\n");
#define RUNNER_EXIT_PRINT(name, value, text) fprintf(out, "  %d  %s\n", value, text);
    RUNNER_EXIT_TABLE(RUNNER_EXIT_PRINT)
#undef RUNNER_EXIT_PRINT
    fprintf(out, "  %s\n", RUNNER_EXIT_RESERVED_TEXT);
}

static int usage(const char *why)
{
    fprintf(stderr, "nesturbator-run: %s\n", why);
    print_usage(stderr);
    return RUNNER_EXIT_USAGE;
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

/* Like parse_count_n, but 0 is a valid frame index (the state at power-on). */
static int parse_frame_index(const char *text, size_t len, uint32_t *out)
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

/* Parses N:FILE for --state-at-frame, where N may be 0. */
static int parse_state_at(const char *text, uint32_t *frame, const char **path)
{
    const char *colon = text != NULL ? strchr(text, ':') : NULL;
    if (colon == NULL || colon[1] == '\0') {
        return 0;
    }
    if (!parse_frame_index(text, (size_t)(colon - text), frame)) {
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

static void print_audio_hash(nesturbator_run_audio_hash *hash)
{
    char transitions_hex[65], pcm_hex[65];
    nesturbator_run_audio_hash_final(hash, transitions_hex, pcm_hex);
    printf("audio transitions sha256 %s\n", transitions_hex);
    printf("audio pcm sha256 %s\n", pcm_hex);
}

/* The runner's options. Each list has room for one entry per argument. */
typedef struct options {
    uint32_t frames;
    uint32_t bench_frames;
    uint64_t *bench_frame_ns;
    int bench_state;
    uint64_t *bench_save_ns;
    uint64_t *bench_load_ns;
    uint8_t *bench_state_buf;
    const char *movie_path;
    const char *rom_path;
    const char *scoreboard_path;
    uint32_t accuracy_page;
    uint32_t accuracy_observe_page;
    const char *accuracy_output_dir;
    const char *accuracy_collect_dir;
    uint32_t accuracy_boot_cap;
    uint32_t accuracy_navigation_cap;
    uint32_t accuracy_execution_cap;
    int accuracy_inventory;
    uint32_t *hash_frames;
    uint32_t hash_count;
    uint32_t *dump_frames;
    const char **dump_paths;
    uint32_t dump_count;
    uint32_t *state_frames;
    const char **state_paths;
    uint32_t state_count;
    uint32_t *hash_states;
    uint32_t hash_state_count;
    /* Command-line position of each request, so a stop names the first unmet one. */
    int *hash_frame_argi;
    int *dump_argi;
    int *state_argi;
    int *hash_state_argi;
    int hash_audio_argi;
    int movie_argi;
    const char *load_state_path;
    int hash_audio;
    int show_version;
    int show_help;
    const char *save_dir;
    uint32_t save_interval;
    int raise_after_set;
    uint32_t raise_after_frame;
    int raise_after_kind;
    uint32_t raise_after_count;
} options;

static void free_options(options *o)
{
    free(o->hash_frames);
    free(o->dump_frames);
    free((void *)o->dump_paths);
    free(o->state_frames);
    free((void *)o->state_paths);
    free(o->hash_states);
    free(o->hash_frame_argi);
    free(o->dump_argi);
    free(o->state_argi);
    free(o->hash_state_argi);
    free(o->bench_frame_ns);
    free(o->bench_save_ns);
    free(o->bench_load_ns);
    free(o->bench_state_buf);
}

/* Parses N[:SIG[:K]] for --raise-after into o. Returns 1, or 0 on any other text. */
static int parse_raise_after(const char *text, options *o)
{
    const char *c1;
    const char *c2 = NULL;
    size_t n_len;
    int kind = nesturbator_run_stop_default_kind();
    uint32_t count = 1u;
    uint32_t frame;
    if (text == NULL || o->raise_after_set != 0)
        return 0;
    c1 = strchr(text, ':');
    n_len = c1 != NULL ? (size_t)(c1 - text) : strlen(text);
    if (!parse_frame_index(text, n_len, &frame))
        return 0;
    if (c1 != NULL) {
        const char *sig = c1 + 1;
        c2 = strchr(sig, ':');
        if (!nesturbator_run_stop_parse_kind(sig, c2 != NULL ? (size_t)(c2 - sig) : strlen(sig),
                                             &kind))
            return 0;
    }
    if (c2 != NULL) {
        if (c2[1] < '1' || c2[1] > '2' || c2[2] != '\0')
            return 0;
        count = (uint32_t)(c2[1] - '0');
    }
    o->raise_after_set = 1;
    o->raise_after_frame = frame;
    o->raise_after_kind = kind;
    o->raise_after_count = count;
    return 1;
}

/* Returns 0 when the options are valid, 2 after a usage message, 1 when out of
   memory. */
static int parse_options(int argc, char **argv, options *o)
{
    size_t n_args = (size_t)argc;
    o->hash_frames = (uint32_t *)malloc(n_args * sizeof *o->hash_frames);
    o->dump_frames = (uint32_t *)malloc(n_args * sizeof *o->dump_frames);
    o->dump_paths = (const char **)malloc(n_args * sizeof *o->dump_paths);
    o->state_frames = (uint32_t *)malloc(n_args * sizeof *o->state_frames);
    o->state_paths = (const char **)malloc(n_args * sizeof *o->state_paths);
    o->hash_states = (uint32_t *)malloc(n_args * sizeof *o->hash_states);
    o->hash_frame_argi = (int *)malloc(n_args * sizeof *o->hash_frame_argi);
    o->dump_argi = (int *)malloc(n_args * sizeof *o->dump_argi);
    o->state_argi = (int *)malloc(n_args * sizeof *o->state_argi);
    o->hash_state_argi = (int *)malloc(n_args * sizeof *o->hash_state_argi);
    if (o->hash_states == NULL || o->hash_frames == NULL || o->dump_frames == NULL ||
        o->dump_paths == NULL || o->state_frames == NULL || o->state_paths == NULL ||
        o->hash_frame_argi == NULL || o->dump_argi == NULL || o->state_argi == NULL ||
        o->hash_state_argi == NULL) {
        fprintf(stderr, "nesturbator-run: out of memory\n");
        return RUNNER_EXIT_FAILURE;
    }
    /* Most options take a value; --hash-audio is a standalone switch. */
    for (int i = 1; i < argc;) {
        const char *arg = argv[i++];
        const char *value;
        uint32_t n;
        if (strcmp(arg, "--version") == 0) {
            o->show_version = 1;
            return RUNNER_EXIT_DONE;
        }
        if (strcmp(arg, "--help") == 0) {
            o->show_help = 1;
            return RUNNER_EXIT_DONE;
        }
        if (strcmp(arg, "--hash-audio") == 0) {
            if (o->hash_audio != 0)
                return usage("--hash-audio may be specified once");
            o->hash_audio = 1;
            o->hash_audio_argi = i - 1;
            continue;
        }
        if (strcmp(arg, "--accuracycoin-inventory") == 0) {
            if (o->accuracy_inventory != 0)
                return usage("--accuracycoin-inventory may be specified once");
            o->accuracy_inventory = 1;
            continue;
        }
        if (strcmp(arg, "--bench-state") == 0) {
            if (o->bench_state != 0)
                return usage("--bench-state may be specified once");
            o->bench_state = 1;
            continue;
        }
        value = i < argc ? argv[i++] : NULL;
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
        } else if (strcmp(arg, "--accuracycoin-observe") == 0) {
            if (!parse_count(value, &n) || o->accuracy_observe_page != 0u)
                return usage("--accuracycoin-observe needs one page number");
            o->accuracy_observe_page = n;
        } else if (strcmp(arg, "--accuracycoin-output") == 0) {
            if (value == NULL || value[0] == '\0' || o->accuracy_output_dir != NULL)
                return usage("--accuracycoin-output needs one directory");
            o->accuracy_output_dir = value;
        } else if (strcmp(arg, "--accuracycoin-collect") == 0) {
            if (value == NULL || value[0] == '\0' || o->accuracy_collect_dir != NULL)
                return usage("--accuracycoin-collect needs one campaign directory");
            o->accuracy_collect_dir = value;
        } else if (strcmp(arg, "--accuracycoin-boot-cap") == 0) {
            if (!parse_count(value, &n) || o->accuracy_boot_cap != 0u)
                return usage("--accuracycoin-boot-cap needs one positive frame count");
            o->accuracy_boot_cap = n;
        } else if (strcmp(arg, "--accuracycoin-navigation-cap") == 0) {
            if (!parse_count(value, &n) || o->accuracy_navigation_cap != 0u)
                return usage("--accuracycoin-navigation-cap needs one positive frame count");
            o->accuracy_navigation_cap = n;
        } else if (strcmp(arg, "--accuracycoin-execution-cap") == 0) {
            if (!parse_count(value, &n) || o->accuracy_execution_cap != 0u)
                return usage("--accuracycoin-execution-cap needs one positive frame count");
            o->accuracy_execution_cap = n;
        } else if (strcmp(arg, "--save-dir") == 0) {
            if (value == NULL || value[0] == '\0' || o->save_dir != NULL)
                return usage("--save-dir needs one directory");
            o->save_dir = value;
        } else if (strcmp(arg, "--save-interval") == 0) {
            if (!parse_count(value, &n) || o->save_interval != 0u)
                return usage("--save-interval needs one whole number of 1 or more");
            o->save_interval = n;
        } else if (strcmp(arg, "--bench") == 0) {
            if (o->bench_frames != 0u)
                return usage("--bench may be specified once");
            if (!parse_count(value, &n) || n > 1000000u)
                return usage("--bench needs one whole number from 1 to 1000000");
            o->bench_frames = n;
        } else if (strcmp(arg, "--movie") == 0) {
            if (value == NULL || value[0] == '\0' || o->movie_path != NULL)
                return usage("--movie needs one file path");
            o->movie_path = value;
            o->movie_argi = i - 2;
        } else if (strcmp(arg, "--dump-frame") == 0) {
            const char *path;
            if (!parse_dump(value, &n, &path)) {
                return usage("--dump-frame needs N:FILE with N of 1 or more");
            }
            o->dump_frames[o->dump_count] = n;
            o->dump_paths[o->dump_count] = path;
            o->dump_argi[o->dump_count] = i - 2;
            o->dump_count++;
        } else if (strcmp(arg, "--state-at-frame") == 0) {
            const char *path;
            if (!parse_state_at(value, &n, &path)) {
                return usage("--state-at-frame needs N:FILE with N of 0 or more");
            }
            for (uint32_t k = 0; k < o->state_count; k++) {
                if (o->state_frames[k] == n)
                    return usage("--state-at-frame N given twice");
            }
            o->state_frames[o->state_count] = n;
            o->state_paths[o->state_count] = path;
            o->state_argi[o->state_count] = i - 2;
            o->state_count++;
        } else if (strcmp(arg, "--hash-state") == 0) {
            if (value == NULL || !parse_frame_index(value, strlen(value), &n))
                return usage("--hash-state needs a frame number of 0 or more");
            for (uint32_t k = 0; k < o->hash_state_count; k++) {
                if (o->hash_states[k] == n)
                    return usage("--hash-state N given twice");
            }
            o->hash_state_argi[o->hash_state_count] = i - 2;
            o->hash_states[o->hash_state_count++] = n;
        } else if (strcmp(arg, "--load-state") == 0) {
            if (o->load_state_path != NULL)
                return usage("--load-state may be specified once");
            if (value == NULL || value[0] == '\0')
                return usage("--load-state needs a file path");
            o->load_state_path = value;
        } else if (strcmp(arg, "--raise-after") == 0) {
            if (!parse_raise_after(value, o))
                return usage("--raise-after needs N[:SIG[:K]]; SIG int or term (POSIX), int or "
                             "break (Windows); K 1 or 2");
        } else if (strcmp(arg, "--frames") == 0 || strcmp(arg, "--hash-frame") == 0) {
            if (!parse_count(value, &n)) {
                return usage("option needs a whole number of 1 or more");
            }
            if (strcmp(arg, "--frames") == 0) {
                o->frames = n;
            } else {
                o->hash_frame_argi[o->hash_count] = i - 2;
                o->hash_frames[o->hash_count++] = n;
            }
        } else {
            return usage(arg[0] == '-' ? "unknown option" : "unexpected argument");
        }
    }
    if (o->bench_state != 0 && o->bench_frames == 0u)
        return usage("--bench-state requires --bench");
    if (o->bench_frames != 0u) {
        if (o->frames != 0u || o->movie_path != NULL)
            return usage("--bench cannot be combined with --frames or --movie");
        if (o->rom_path == NULL)
            return usage("--bench needs --rom");
        if (o->accuracy_page != 0u || o->accuracy_observe_page != 0u ||
            o->accuracy_inventory != 0 || o->accuracy_collect_dir != NULL)
            return usage("--bench cannot be combined with --accuracycoin-page");
        if (o->raise_after_set != 0)
            return usage("--bench cannot be combined with --raise-after");
        if (o->dump_count != 0u)
            return usage("--bench cannot be combined with --dump-frame");
        if (o->state_count != 0u)
            return usage("--bench cannot be combined with --state-at-frame");
        if (o->hash_state_count != 0u)
            return usage("--bench cannot be combined with --hash-state");
        if (o->save_dir != NULL)
            return usage("--bench cannot be combined with --save-dir");
        if (o->save_interval != 0u)
            return usage("--bench cannot be combined with --save-interval");
        if (o->load_state_path != NULL)
            return usage("--bench cannot be combined with --load-state");
        o->frames = o->bench_frames;
    }
    if (o->accuracy_page != 0u) {
        if (o->accuracy_page != 2u && o->accuracy_page != 14u && o->accuracy_page != 17u)
            return usage("AccuracyCoin supports pages 2, 14 and 17");
        if (o->rom_path == NULL || o->scoreboard_path == NULL || o->movie_path != NULL ||
            o->frames != 0u)
            return usage("AccuracyCoin page mode needs --rom and --scoreboard only");
        o->frames = 1800u;
    } else if (o->scoreboard_path != NULL) {
        return usage("--scoreboard requires --accuracycoin-page");
    }
    if (o->accuracy_inventory != 0) {
        if (o->accuracy_page != 0u || o->accuracy_observe_page != 0u ||
            o->accuracy_collect_dir != NULL || o->rom_path == NULL || o->frames != 0u ||
            o->scoreboard_path != NULL || o->accuracy_output_dir != NULL || o->movie_path != NULL ||
            o->bench_frames != 0u || o->save_dir != NULL || o->save_interval != 0u ||
            o->raise_after_set != 0 || o->hash_audio != 0 || o->hash_count != 0u ||
            o->dump_count != 0u || o->state_count != 0u || o->hash_state_count != 0u ||
            o->load_state_path != NULL)
            return usage("AccuracyCoin inventory needs --rom only");
    }
    if (o->accuracy_collect_dir != NULL) {
        if (o->accuracy_page != 0u || o->accuracy_observe_page != 0u ||
            o->accuracy_inventory != 0 || o->rom_path == NULL || o->frames != 0u ||
            o->scoreboard_path != NULL || o->accuracy_output_dir != NULL || o->movie_path != NULL ||
            o->bench_frames != 0u || o->save_dir != NULL || o->save_interval != 0u ||
            o->raise_after_set != 0 || o->hash_audio != 0 || o->hash_count != 0u ||
            o->dump_count != 0u || o->state_count != 0u || o->hash_state_count != 0u ||
            o->load_state_path != NULL || o->accuracy_boot_cap != 0u ||
            o->accuracy_navigation_cap != 0u || o->accuracy_execution_cap != 0u)
            return usage("AccuracyCoin collection needs --rom and one campaign directory only");
    }
    if (o->accuracy_observe_page != 0u) {
        if (o->accuracy_observe_page > ACCURACYCOIN_PAGE_COUNT || o->rom_path == NULL ||
            o->frames == 0u || o->scoreboard_path != NULL || o->accuracy_output_dir == NULL ||
            o->accuracy_page != 0u || o->accuracy_inventory != 0 || o->movie_path != NULL ||
            o->bench_frames != 0u || o->save_dir != NULL || o->save_interval != 0u ||
            o->raise_after_set != 0 || o->hash_audio != 0 || o->hash_count != 0u ||
            o->dump_count != 0u || o->state_count != 0u || o->hash_state_count != 0u ||
            o->load_state_path != NULL || o->accuracy_collect_dir != NULL)
            return usage(
                "AccuracyCoin observation needs --rom, --frames and --accuracycoin-output only");
    } else if (o->accuracy_output_dir != NULL) {
        return usage("--accuracycoin-output requires --accuracycoin-observe");
    }
    if (o->accuracy_observe_page == 0u &&
        (o->accuracy_boot_cap != 0u || o->accuracy_navigation_cap != 0u ||
         o->accuracy_execution_cap != 0u))
        return usage("AccuracyCoin stage caps require --accuracycoin-observe");
    if (o->state_count != 0u || o->hash_state_count != 0u || o->load_state_path != NULL) {
        if (o->rom_path == NULL || o->accuracy_page != 0u || o->accuracy_observe_page != 0u ||
            o->accuracy_inventory != 0)
            return usage("--state-at-frame, --load-state and --hash-state need --rom");
        if (o->movie_path != NULL && o->load_state_path != NULL)
            return usage("--movie cannot be combined with --load-state");
    }
    if (o->save_dir != NULL && (o->rom_path == NULL || o->accuracy_page != 0u ||
                                o->accuracy_observe_page != 0u || o->accuracy_inventory != 0))
        return usage("--save-dir needs --rom and cannot be used with --accuracycoin-page");
    if (o->save_interval != 0u && o->save_dir == NULL)
        return usage("--save-interval requires --save-dir");
    if (o->raise_after_set != 0 && (o->accuracy_page != 0u || o->accuracy_observe_page != 0u))
        return usage("--raise-after cannot be used with --accuracycoin-page");
    if (o->frames == 0u && o->movie_path == NULL && o->accuracy_inventory == 0 &&
        o->accuracy_collect_dir == NULL) {
        return usage("--frames N is required");
    }
    if (o->accuracy_observe_page != 0u && o->frames < 16u)
        return usage("AccuracyCoin observation needs a --frames budget of at least 16");
    return RUNNER_EXIT_DONE;
}

/* The battery span of the loaded cartridge and what was last written for it. */
typedef struct battery_save {
    char path[4096];
    uint8_t *span;
    size_t size;
    uint8_t *shadow;
    uint64_t generation;
} battery_save;

/* Writes the span when a CPU write reached it since the last look and the
   bytes differ from the last written copy. Returns 0, or 1 after a failure.
   A failed write leaves the generation unrecorded, so the next look (the exit
   flush) writes again. */
static int save_if_changed(nesturbator *inst, battery_save *sv)
{
    uint64_t gen;
    if (sv->shadow == NULL)
        return 0;
    gen = nesturbator_save_generation(inst);
    if (gen == sv->generation)
        return 0;
    if (memcmp(sv->span, sv->shadow, sv->size) != 0 &&
        nesturbator_run_save_flush(sv->path, sv->span, sv->size, sv->shadow) != 0)
        return 1;
    sv->generation = gen;
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

/* Prints why a cartridge was refused (D-15). Reads only the 16 header bytes the caller already
 * holds, with the loader's NES 2.0 bit rule ((b[7] & 0x0c) == 0x08) for mapper and submapper.
 * Source: https://www.nesdev.org/wiki/NES_2.0 and https://www.nesdev.org/wiki/INES */
static void describe_rejection(const uint8_t *b, size_t len, int status)
{
    if (len >= 16u && b[0] == 'N' && b[1] == 'E' && b[2] == 'S' && b[3] == 0x1au) {
        int nes2 = (b[7] & 0x0cu) == 0x08u;
        uint32_t mapper = (uint32_t)(b[6] >> 4) | (uint32_t)(b[7] & 0xf0u);
        uint32_t sub = 0u;
        if (nes2) {
            mapper |= (uint32_t)(b[8] & 0x0fu) << 8;
            sub = (uint32_t)(b[8] >> 4);
        }
        if ((b[6] & 0x08u) != 0u) {
            fprintf(stderr, "nesturbator-run: four-screen cartridges are not supported\n");
            return;
        }
        if (mapper == 4u && sub == 1u) {
            fprintf(stderr, "nesturbator-run: MMC6 (mapper 4, submapper 1) is not supported\n");
            return;
        }
        if (mapper != 0u && mapper != 1u && mapper != 2u && mapper != 3u && mapper != 4u &&
            mapper != 7u) {
            fprintf(stderr,
                    "nesturbator-run: mapper %u is not supported; supported mappers are 0, 1, 2, "
                    "3, 4 and 7\n",
                    (unsigned)mapper);
            return;
        }
    }
    fprintf(stderr,
            "nesturbator-run: malformed or unsupported cartridge (status %d); supported "
            "mappers are 0, 1, 2, 3, 4 and 7\n",
            status);
}

/* The word for a rejected state (D-07), by nesturbator_status 8 to 12. */
static const char *state_rejection_word(int st)
{
    switch (st) {
    case NESTURBATOR_ERR_STATE_FORMAT:
        return "another format version";
    case NESTURBATOR_ERR_STATE_CARTRIDGE:
        return "wrong game";
    case NESTURBATOR_ERR_STATE_SIZE:
        return "wrong length";
    case NESTURBATOR_ERR_STATE_CHECKSUM:
        return "damaged";
    default:
        return "invalid contents";
    }
}

/* Loads the state file into the instance. On success *frame is the state's
   frame number K (header offset 32, little-endian). Returns 0, 1 when the file
   cannot be read or memory runs out, or 5 when the library rejects the state
   (one line says why). */
static int load_state_file(nesturbator *inst, const char *path, uint64_t *frame, uint64_t *ticks)
{
    size_t size = nesturbator_state_size(inst);
    size_t got = 0;
    uint8_t *buf = (uint8_t *)malloc(size + 1u);
    nesturbator_status st;
    if (buf == NULL) {
        fprintf(stderr, "nesturbator-run: out of memory\n");
        return RUNNER_EXIT_FAILURE;
    }
    /* One byte beyond the state size, so a longer file is a length rejection. */
    if (nesturbator_run_state_read(path, buf, size + 1u, &got) != 0) {
        free(buf);
        return RUNNER_EXIT_FAILURE;
    }
    st = nesturbator_load_state(inst, buf, got);
    if (st != NESTURBATOR_OK) {
        fprintf(stderr, "nesturbator-run: cannot load state %s (%s)\n", path,
                state_rejection_word((int)st));
        free(buf);
        return RUNNER_EXIT_STATE_REJECTED;
    }
    *frame = 0u;
    for (unsigned k = 0; k < 8u; k++)
        *frame |= (uint64_t)buf[32u + k] << (8u * k);
    *ticks = 0u;
    for (unsigned k = 0; k < 8u; k++)
        *ticks |= (uint64_t)buf[40u + k] << (8u * k);
    free(buf);
    return RUNNER_EXIT_DONE;
}

/* Prints the state line for every --hash-state whose N is frame. Returns 0, or 1 after a
   message. ticks is the emulated time after that frame (0 at power-on). */
static int print_state_hashes(const nesturbator *inst, const options *o, uint64_t frame,
                              uint64_t ticks)
{
    size_t size = nesturbator_state_size(inst);
    uint8_t *buf;
    nesturbator_run_sha256 s;
    uint8_t digest[32];
    char hex[65];
    if (!listed(o->hash_states, o->hash_state_count, (uint32_t)frame))
        return 0;
    buf = size != 0u ? (uint8_t *)malloc(size) : NULL;
    if (buf == NULL || nesturbator_save_state(inst, buf, size) != NESTURBATOR_OK) {
        free(buf);
        fprintf(stderr, "nesturbator-run: cannot save state at frame %lu\n", (unsigned long)frame);
        return 1;
    }
    nesturbator_run_sha256_init(&s);
    nesturbator_run_sha256_update(&s, buf, size);
    nesturbator_run_sha256_final(&s, digest);
    nesturbator_run_sha256_hex(digest, hex);
    free(buf);
    printf("state %lu ticks %llu size %lu sha256 %s\n", (unsigned long)frame,
           (unsigned long long)ticks, (unsigned long)size, hex);
    return 0;
}

/* Writes the state for every --state-at-frame whose N is frame. Returns 0, or
   1 after a message. Each file is replaced atomically and no other path is
   written (PROH-12-08-01). */
static int write_states(const nesturbator *inst, const options *o, uint64_t frame)
{
    size_t size = nesturbator_state_size(inst);
    uint8_t *buf = NULL;
    int rc = 0;
    for (uint32_t k = 0; k < o->state_count && rc == 0; k++) {
        if (o->state_frames[k] != frame)
            continue;
        if (buf == NULL) {
            buf = size != 0u ? (uint8_t *)malloc(size) : NULL;
            if (buf == NULL || nesturbator_save_state(inst, buf, size) != NESTURBATOR_OK) {
                fprintf(stderr, "nesturbator-run: cannot save state at frame %lu\n",
                        (unsigned long)frame);
                rc = 1;
                break;
            }
        }
        if (nesturbator_run_file_replace(o->state_paths[k], buf, size) != 0)
            rc = 1;
    }
    free(buf);
    return rc;
}

/* The range checks that do not depend on a loaded state (D-09): no frame index may lie beyond
   --frames. They run before any file is touched, so a bad command line exits 2 whatever else is
   wrong with the ROM, the battery save or the state. Returns 0, or 2 after a usage message. */
static int check_frame_limits(const options *o)
{
    char why[160];
    if (o->raise_after_set != 0 && o->raise_after_frame > o->frames)
        return usage("--raise-after N is beyond --frames");
    for (uint32_t k = 0; k < o->hash_count + o->dump_count; k++) {
        int is_hash = k < o->hash_count;
        uint32_t n = is_hash ? o->hash_frames[k] : o->dump_frames[k - o->hash_count];
        if (n > o->frames) {
            snprintf(why, sizeof why, "%s %lu is beyond the requested frame count",
                     is_hash ? "--hash-frame" : "--dump-frame", (unsigned long)n);
            return usage(why);
        }
    }
    for (uint32_t k = 0; k < o->state_count + o->hash_state_count; k++) {
        int is_file = k < o->state_count;
        uint32_t n = is_file ? o->state_frames[k] : o->hash_states[k - o->state_count];
        if (n > o->frames) {
            snprintf(why, sizeof why, "%s %lu is beyond the requested frame count",
                     is_file ? "--state-at-frame" : "--hash-state", (unsigned long)n);
            return usage(why);
        }
    }
    return RUNNER_EXIT_DONE;
}

/* The checks that need K, the loaded state's frame (0 without a load): frame indices are
   absolute from power-on. Run after the state load. Returns 0, or 2 after a usage message. */
static int check_ranges(const options *o, uint64_t k_frame)
{
    char why[160];
    if (o->raise_after_set != 0 && o->raise_after_frame < k_frame) {
        snprintf(why, sizeof why, "--raise-after %lu is before the loaded state's frame %lu",
                 (unsigned long)o->raise_after_frame, (unsigned long)k_frame);
        return usage(why);
    }
    if (o->frames < k_frame) {
        snprintf(why, sizeof why, "--frames %lu is before the loaded state's frame %lu",
                 (unsigned long)o->frames, (unsigned long)k_frame);
        return usage(why);
    }
    for (uint32_t k = 0; k < o->hash_count + o->dump_count; k++) {
        int is_hash = k < o->hash_count;
        uint32_t n = is_hash ? o->hash_frames[k] : o->dump_frames[k - o->hash_count];
        if (n <= k_frame) {
            snprintf(why, sizeof why, "%s %lu is not after the loaded state's frame %lu",
                     is_hash ? "--hash-frame" : "--dump-frame", (unsigned long)n,
                     (unsigned long)k_frame);
            return usage(why);
        }
    }
    for (uint32_t k = 0; k < o->state_count + o->hash_state_count; k++) {
        int is_file = k < o->state_count;
        uint32_t n = is_file ? o->state_frames[k] : o->hash_states[k - o->state_count];
        if (n < k_frame) {
            snprintf(why, sizeof why, "%s %lu is before the loaded state's frame %lu",
                     is_file ? "--state-at-frame" : "--hash-state", (unsigned long)n,
                     (unsigned long)k_frame);
            return usage(why);
        }
    }
    return RUNNER_EXIT_DONE;
}

/* D-08, D-10: after a stop, returns 1 and names the first request (by command-line position)
   that was not produced, or returns 0 when everything requested was. A --frames count not
   reached is not a request. */
static int report_unmet(const options *o, uint64_t done_frame)
{
    int best = -1;
    uint32_t k;
    char text[1024];
    text[0] = '\0';
#define UNMET_PICK(argi, ...)                                                                      \
    do {                                                                                           \
        if (best < 0 || (argi) < best) {                                                           \
            best = (argi);                                                                         \
            (void)snprintf(text, sizeof text, __VA_ARGS__);                                        \
        }                                                                                          \
    } while (0)
    for (k = 0; k < o->hash_count; k++) {
        if (o->hash_frames[k] > done_frame)
            UNMET_PICK(o->hash_frame_argi[k], "--hash-frame %lu", (unsigned long)o->hash_frames[k]);
    }
    for (k = 0; k < o->hash_state_count; k++) {
        if (o->hash_states[k] > done_frame)
            UNMET_PICK(o->hash_state_argi[k], "--hash-state %lu", (unsigned long)o->hash_states[k]);
    }
    for (k = 0; k < o->state_count; k++) {
        if (o->state_frames[k] > done_frame)
            UNMET_PICK(o->state_argi[k], "--state-at-frame %lu:%s",
                       (unsigned long)o->state_frames[k], o->state_paths[k]);
    }
    for (k = 0; k < o->dump_count; k++) {
        if (o->dump_frames[k] > done_frame)
            UNMET_PICK(o->dump_argi[k], "--dump-frame %lu:%s", (unsigned long)o->dump_frames[k],
                       o->dump_paths[k]);
    }
    if (o->hash_audio != 0)
        UNMET_PICK(o->hash_audio_argi, "--hash-audio");
    if (o->movie_path != NULL && done_frame < o->frames)
        UNMET_PICK(o->movie_argi, "--movie %s", o->movie_path);
#undef UNMET_PICK
    if (best < 0)
        return 0;
    fprintf(stderr, "nesturbator-run: not reached: %s\n", text);
    return 1;
}

static int read_input_file(const char *path, uint8_t **out_bytes, size_t *out_size)
{
    FILE *file = fopen(path, "rb");
    long length;
    uint8_t *bytes;
    int ok = 0;
    if (file == NULL || fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0 || (unsigned long long)length > (unsigned long long)SIZE_MAX)
        goto done;
    bytes = (uint8_t *)malloc((size_t)length);
    if (bytes == NULL)
        goto done;
    if (fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(bytes);
        goto done;
    }
    *out_bytes = bytes;
    *out_size = (size_t)length;
    ok = 1;
done:
    if (file != NULL && fclose(file) != 0)
        ok = 0;
    if (ok == 0 && *out_bytes != NULL) {
        free(*out_bytes);
        *out_bytes = NULL;
        *out_size = 0u;
    }
    return ok;
}

int main(int argc, char **argv)
{
    static uint16_t video[WIDTH * HEIGHT];
    static int16_t audio[AUDIO_CAPACITY];
    static uint32_t palette[512];
    static uint32_t image[WIDTH * HEIGHT];
    options opt;
    accuracycoin_directory accuracy_directory;
    nesturbator_movie movie;
    nesturbator_run_audio_hash audio_hash;
    battery_save save;
    uint64_t start_frame = 0u;
    uint64_t cur_ticks = 0u;
    uint64_t done_frame;
    int stopped = 0;
    int status;

    memset(&opt, 0, sizeof opt);
    memset(&accuracy_directory, 0, sizeof accuracy_directory);
    memset(&movie, 0, sizeof movie);
    memset(&save, 0, sizeof save);
    status = parse_options(argc, argv, &opt);
    if (status != 0) {
        free_options(&opt);
        return status;
    }
    if (opt.show_version) {
        nesturbator_version ver;
        memset(&ver, 0, sizeof ver);
        ver.size = (uint32_t)sizeof ver;
        nesturbator_get_version(&ver);
        printf("nesturbator-run %u.%u.%u\n", (unsigned)ver.major, (unsigned)ver.minor,
               (unsigned)ver.patch);
        free_options(&opt);
        return fflush(stdout) == 0 ? 0 : 1;
    }
    if (opt.show_help) {
        print_usage(stdout);
        free_options(&opt);
        return fflush(stdout) == 0 ? RUNNER_EXIT_DONE : RUNNER_EXIT_FAILURE;
    }
    if (opt.accuracy_inventory != 0) {
        uint8_t *bytes = NULL;
        size_t size = 0u;
        accuracycoin_reason reason = ACCURACYCOIN_REASON_OK;
        if (!read_input_file(opt.rom_path, &bytes, &size)) {
            fprintf(stderr, "nesturbator-run: cannot read AccuracyCoin ROM %s\n", opt.rom_path);
            free_options(&opt);
            return RUNNER_EXIT_FAILURE;
        }
        if (!accuracycoin_parse_pinned(bytes, size, &accuracy_directory, &reason)) {
            fprintf(stderr, "nesturbator-run: invalid AccuracyCoin inventory: %s\n",
                    accuracycoin_reason_text(reason));
            free(bytes);
            free_options(&opt);
            return RUNNER_EXIT_FAILURE;
        }
        free(bytes);
        status = accuracycoin_write_inventory(stdout, &accuracy_directory) ? RUNNER_EXIT_DONE
                                                                           : RUNNER_EXIT_FAILURE;
        free_options(&opt);
        return status;
    }
    if (opt.accuracy_collect_dir != NULL) {
        uint8_t *bytes = NULL;
        size_t size = 0u;
        accuracycoin_reason reason = ACCURACYCOIN_REASON_OK;
        if (!read_input_file(opt.rom_path, &bytes, &size)) {
            fprintf(stderr, "nesturbator-run: cannot read AccuracyCoin ROM %s\n", opt.rom_path);
            free_options(&opt);
            return RUNNER_EXIT_FAILURE;
        }
        if (!accuracycoin_parse_pinned(bytes, size, &accuracy_directory, &reason)) {
            fprintf(stderr, "nesturbator-run: invalid AccuracyCoin inventory: %s\n",
                    accuracycoin_reason_text(reason));
            free(bytes);
            free_options(&opt);
            return RUNNER_EXIT_FAILURE;
        }
        free(bytes);
        status = accuracycoin_collect_campaign(&accuracy_directory, opt.accuracy_collect_dir);
        free_options(&opt);
        return status;
    }
    nesturbator_run_audio_hash_init(&audio_hash);

    if (opt.movie_path != NULL) {
        if (!nesturbator_movie_read(opt.movie_path, &movie)) {
            fprintf(stderr, "nesturbator-run: malformed or unreadable movie %s\n", opt.movie_path);
            free_options(&opt);
            return RUNNER_EXIT_FAILURE;
        }
        if (opt.frames != 0u && opt.frames != movie.frame_count) {
            fprintf(stderr, "nesturbator-run: --frames must match the movie frame count\n");
            nesturbator_movie_free(&movie);
            free_options(&opt);
            return RUNNER_EXIT_USAGE;
        }
        opt.frames = movie.frame_count;
    }
    status = check_frame_limits(&opt);
    if (status != 0) {
        nesturbator_movie_free(&movie);
        free_options(&opt);
        return status;
    }
    if (opt.accuracy_page == 0u && opt.accuracy_observe_page == 0u)
        nesturbator_run_stop_install();
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
        return RUNNER_EXIT_FAILURE;
    }
    if (opt.rom_path != NULL) {
        uint8_t *bytes = NULL;
        size_t size = 0u;
        if (!read_input_file(opt.rom_path, &bytes, &size)) {
            fprintf(stderr, "nesturbator-run: cannot read cartridge %s\n", opt.rom_path);
            nesturbator_destroy(inst);
            nesturbator_movie_free(&movie);
            free_options(&opt);
            return RUNNER_EXIT_FAILURE;
        }
        if (opt.accuracy_page != 0u || opt.accuracy_observe_page != 0u) {
            accuracycoin_reason reason = ACCURACYCOIN_REASON_OK;
            if (!accuracycoin_parse_pinned(bytes, size, &accuracy_directory, &reason)) {
                fprintf(stderr, "nesturbator-run: invalid AccuracyCoin inventory: %s\n",
                        accuracycoin_reason_text(reason));
                free(bytes);
                nesturbator_destroy(inst);
                nesturbator_movie_free(&movie);
                free_options(&opt);
                return RUNNER_EXIT_FAILURE;
            }
        }
        st = nesturbator_load_cartridge(inst, bytes, size);
        if (st != NESTURBATOR_OK) {
            describe_rejection(bytes, size, (int)st);
            free(bytes);
            nesturbator_destroy(inst);
            nesturbator_movie_free(&movie);
            free_options(&opt);
            return RUNNER_EXIT_FAILURE;
        }
        free(bytes);
        if (opt.save_dir != NULL) {
            nesturbator_status mst =
                nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &save.span, &save.size);
            if (mst == NESTURBATOR_OK && save.span != NULL && save.size != 0u) {
                enum nesturbator_run_save_status ls = NESTURBATOR_RUN_SAVE_ERROR;
                if (!nesturbator_run_save_path(save.path, sizeof save.path, opt.save_dir,
                                               opt.rom_path)) {
                    fprintf(stderr, "nesturbator-run: save path is too long\n");
                } else {
                    ls = nesturbator_run_save_load(save.path, save.span, save.size);
                }
                if (ls == NESTURBATOR_RUN_SAVE_MISMATCH || ls == NESTURBATOR_RUN_SAVE_ERROR) {
                    nesturbator_destroy(inst);
                    nesturbator_movie_free(&movie);
                    free_options(&opt);
                    return ls == NESTURBATOR_RUN_SAVE_MISMATCH ? RUNNER_EXIT_SAVE_SIZE
                                                               : RUNNER_EXIT_FAILURE;
                }
                save.shadow = (uint8_t *)malloc(save.size);
                if (save.shadow == NULL) {
                    fprintf(stderr, "nesturbator-run: out of memory\n");
                    nesturbator_destroy(inst);
                    nesturbator_movie_free(&movie);
                    free_options(&opt);
                    return RUNNER_EXIT_FAILURE;
                }
                memcpy(save.shadow, save.span, save.size);
                save.generation = nesturbator_save_generation(inst);
            }
        }
    }
    /* D-10: the state load follows the battery load, so the .sav rule below sees the state's
       battery bytes as a change. */
    if (opt.load_state_path != NULL) {
        status = load_state_file(inst, opt.load_state_path, &start_frame, &cur_ticks);
        if (status != 0) {
            free(save.shadow);
            nesturbator_destroy(inst);
            nesturbator_movie_free(&movie);
            free_options(&opt);
            return status;
        }
    }
    status = check_ranges(&opt, start_frame);
    if (status != 0) {
        free(save.shadow);
        nesturbator_destroy(inst);
        nesturbator_movie_free(&movie);
        free_options(&opt);
        return status;
    }
    if (opt.dump_count > 0u) {
        nesturbator_get_palette(inst, palette, 512u);
    }

    if (opt.hash_audio != 0)
        nesturbator__apu_set_transition_sink(inst, nesturbator_run_audio_hash_transition,
                                             &audio_hash);

    if (opt.accuracy_page != 0u || opt.accuracy_observe_page != 0u) {
        if (opt.accuracy_page != 0u) {
            status = accuracycoin_run_strict_page(&accuracy_directory, opt.accuracy_page,
                                                  opt.scoreboard_path, inst, video, audio,
                                                  opt.hash_audio != 0 ? &audio_hash : NULL);
        } else {
            accuracycoin_run_budgets budgets;
            uint64_t navigation_max = 0u;
            uint64_t required_frames;
            budgets.boot_frames = opt.accuracy_boot_cap != 0u ? opt.accuracy_boot_cap : 120u;
            budgets.navigation_frames_per_page =
                opt.accuracy_navigation_cap != 0u ? opt.accuracy_navigation_cap : 5u;
            navigation_max =
                (uint64_t)(opt.accuracy_observe_page - 1u) * budgets.navigation_frames_per_page;
            required_frames = (uint64_t)budgets.boot_frames + navigation_max + 2u;
            if (opt.accuracy_execution_cap != 0u) {
                budgets.execution_frames = opt.accuracy_execution_cap;
            } else if ((uint64_t)opt.frames > required_frames) {
                budgets.execution_frames = (uint32_t)((uint64_t)opt.frames - required_frames);
            } else {
                budgets.execution_frames = 0u;
            }
            budgets.total_frames = opt.frames;
            if (budgets.execution_frames == 0u ||
                (uint64_t)budgets.execution_frames + required_frames > budgets.total_frames) {
                fprintf(stderr,
                        "nesturbator-run: AccuracyCoin stage caps exceed the total frame bound\n");
                status = RUNNER_EXIT_USAGE;
            } else {
                status = accuracycoin_run_observe_page(&accuracy_directory,
                                                       opt.accuracy_observe_page, &budgets,
                                                       opt.accuracy_output_dir, inst, video, audio);
            }
        }
        if (status == 0 && opt.hash_audio != 0)
            print_audio_hash(&audio_hash);
        nesturbator_destroy(inst);
        nesturbator_movie_free(&movie);
        free_options(&opt);
        return status;
    }

    /* A 64-bit count, so the loop ends when --frames is 4294967295. */
    done_frame = start_frame;
    if (print_state_hashes(inst, &opt, start_frame, cur_ticks) != 0 ||
        write_states(inst, &opt, start_frame) != 0)
        status = RUNNER_EXIT_FAILURE;
    if (status == RUNNER_EXIT_DONE && opt.raise_after_set != 0 &&
        opt.raise_after_frame == start_frame)
        nesturbator_run_stop_raise(opt.raise_after_kind, opt.raise_after_count);
    uint32_t bench_timed = 0u;
    if (opt.bench_frames != 0u) {
        opt.bench_frame_ns = (uint64_t *)malloc((size_t)opt.bench_frames * sizeof(uint64_t));
        if (opt.bench_frame_ns == NULL) {
            fprintf(stderr, "nesturbator-run: out of memory\n");
            status = RUNNER_EXIT_FAILURE;
        }
    }
    size_t bench_state_size = 0u;
    if (opt.bench_state != 0 && status == 0) {
        bench_state_size = nesturbator_state_size(inst);
        opt.bench_state_buf = (uint8_t *)malloc(bench_state_size);
        opt.bench_save_ns = (uint64_t *)malloc((size_t)opt.bench_frames * sizeof(uint64_t));
        opt.bench_load_ns = (uint64_t *)malloc((size_t)opt.bench_frames * sizeof(uint64_t));
        if (opt.bench_state_buf == NULL || opt.bench_save_ns == NULL || opt.bench_load_ns == NULL) {
            fprintf(stderr, "nesturbator-run: out of memory\n");
            status = RUNNER_EXIT_FAILURE;
        }
    }
    for (uint64_t n = start_frame + 1u; n <= opt.frames && status == 0; n++) {
        uint32_t f = (uint32_t)n;
        if (nesturbator_run_stop_requested()) {
            stopped = 1;
            break;
        }
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
                status = RUNNER_EXIT_FAILURE;
                break;
            }
        }
        if (opt.bench_frames != 0u) {
            uint64_t t1;
            uint64_t t0 = nesturbator_run_clock_now_ns();
            st = nesturbator_run_frame(inst, &io);
            t1 = nesturbator_run_clock_now_ns();
            if (t0 == 0u || t1 == 0u) {
                fprintf(stderr, "nesturbator-run: clock unavailable\n");
                status = RUNNER_EXIT_FAILURE;
                break;
            }
            if (st == NESTURBATOR_OK)
                opt.bench_frame_ns[bench_timed++] = t1 - t0;
        } else {
            st = nesturbator_run_frame(inst, &io);
        }
        if (st != NESTURBATOR_OK) {
            if (st == NESTURBATOR_STOP_JAM)
                fprintf(stderr, "nesturbator-run: JAM stopped replay at frame %u\n", f);
            else
                fprintf(stderr, "nesturbator-run: frame %u failed with status %d\n", f, (int)st);
            status = RUNNER_EXIT_FAILURE;
            break;
        }
        cur_ticks = io.ticks;
        if (opt.hash_audio != 0)
            nesturbator_run_audio_hash_pcm(&audio_hash, audio, io.audio_count);
        if ((opt.movie_path != NULL || listed(opt.hash_frames, opt.hash_count, f))) {
            char hex[65];
            hash_frame(video, hex);
            printf("frame %lu ticks %llu sha256 %s\n", (unsigned long)f,
                   (unsigned long long)io.ticks, hex);
            /* D-03, D-04: under --bench each line is flushed as it is printed, after the timed
               call, so a reader of a pipe sees it at once; bytes are unchanged. */
            if (opt.bench_frames != 0u && fflush(stdout) != 0) {
                fprintf(stderr, "nesturbator-run: cannot write output\n");
                status = RUNNER_EXIT_FAILURE;
            }
        }
        if (print_state_hashes(inst, &opt, n, cur_ticks) != 0)
            status = RUNNER_EXIT_FAILURE;
        if (status == 0 && listed(opt.dump_frames, opt.dump_count, f)) {
            nesturbator_host_convert(palette, video, WIDTH, image, WIDTH);
            for (uint32_t k = 0; k < opt.dump_count; k++) {
                if (opt.dump_frames[k] == f &&
                    nesturbator_run_write_ppm(opt.dump_paths[k], image, WIDTH) != 0) {
                    fprintf(stderr, "nesturbator-run: cannot write %s\n", opt.dump_paths[k]);
                    status = RUNNER_EXIT_FAILURE;
                }
            }
        }
        if (status == 0 && write_states(inst, &opt, n) != 0)
            status = RUNNER_EXIT_FAILURE;
        /* After frame f has completed and before frame f + 1 starts. */
        if (status == 0 && opt.save_interval != 0u && f % opt.save_interval == 0u &&
            save_if_changed(inst, &save) != 0)
            status = RUNNER_EXIT_FAILURE;
        /* D-02: the state round trip is timed on its own, after the frame's work. */
        if (status == 0 && opt.bench_state != 0) {
            uint64_t s0 = nesturbator_run_clock_now_ns();
            st = nesturbator_save_state(inst, opt.bench_state_buf, bench_state_size);
            uint64_t s1 = nesturbator_run_clock_now_ns();
            if (st != NESTURBATOR_OK) {
                fprintf(stderr, "nesturbator-run: bench save failed at frame %u\n", f);
                status = RUNNER_EXIT_FAILURE;
            } else {
                uint64_t l0 = nesturbator_run_clock_now_ns();
                st = nesturbator_load_state(inst, opt.bench_state_buf, bench_state_size);
                uint64_t l1 = nesturbator_run_clock_now_ns();
                if (st != NESTURBATOR_OK) {
                    fprintf(stderr, "nesturbator-run: bench load failed at frame %u\n", f);
                    status = RUNNER_EXIT_FAILURE;
                } else {
                    opt.bench_save_ns[bench_timed - 1u] = s1 - s0;
                    opt.bench_load_ns[bench_timed - 1u] = l1 - l0;
                }
            }
        }
        done_frame = n;
        if (status == RUNNER_EXIT_DONE && opt.raise_after_set != 0 && n == opt.raise_after_frame)
            nesturbator_run_stop_raise(opt.raise_after_kind, opt.raise_after_count);
    }

    /* D-06, D-03: the statistics come first, before any stop output. The count goes straight to
       the helper, which alone decides what zero timed frames print. */
    if (opt.bench_frames != 0u && status == 0) {
        char report[512];
        nesturbator_run_bench_report(report, sizeof report, bench_timed, opt.bench_frame_ns,
                                     opt.bench_state != 0 ? opt.bench_save_ns : NULL,
                                     opt.bench_state != 0 ? opt.bench_load_ns : NULL);
        fputs(report, stderr);
    }
    /* D-07: on a stop, stdout is flushed first, then the exit flush runs, then the stop line. */
    if (stopped != 0 && fflush(stdout) != 0) {
        fprintf(stderr, "nesturbator-run: cannot write output\n");
        if (status == RUNNER_EXIT_DONE)
            status = RUNNER_EXIT_FAILURE;
    }
    /* The exit flush runs whatever ended the loop, including a JAM. */
    if (save_if_changed(inst, &save) != 0 && status == 0)
        status = RUNNER_EXIT_FAILURE;
    if (stopped != 0) {
        fprintf(stderr, "nesturbator-run: stopped by signal after frame %llu\n",
                (unsigned long long)done_frame);
        if (report_unmet(&opt, done_frame) != 0 && status == RUNNER_EXIT_DONE)
            status = RUNNER_EXIT_FAILURE;
    } else if (status == 0 && opt.hash_audio != 0) {
        print_audio_hash(&audio_hash);
    }
    nesturbator_run_stop_done();

    free(save.shadow);
    nesturbator_destroy(inst);
    nesturbator_movie_free(&movie);
    free_options(&opt);
    if (fflush(stdout) != 0) {
        fprintf(stderr, "nesturbator-run: cannot write output\n");
        return RUNNER_EXIT_FAILURE;
    }
    return status;
}
