#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "check.h"
#include "movie_fixture.h"

static void header(unsigned char bytes[16], uint32_t count)
{
    memset(bytes, 0, 16u);
    memcpy(bytes, "NMOVIE1", 7u);
    bytes[8] = 1u;
    bytes[12] = (unsigned char)(count & 0xffu);
    bytes[13] = (unsigned char)((count >> 8) & 0xffu);
    bytes[14] = (unsigned char)((count >> 16) & 0xffu);
    bytes[15] = (unsigned char)((count >> 24) & 0xffu);
}

static int write_bytes(const char *path, const unsigned char *bytes, size_t size)
{
    FILE *file = fopen(path, "wb");
    int ok;
    if (file == NULL)
        return 0;
    ok = fwrite(bytes, 1, size, file) == size;
    if (fclose(file) != 0)
        ok = 0;
    return ok;
}

static int run(const char *runner, const char *movie, const char *output)
{
    char command[8192];
    if (snprintf(command, sizeof command, "\"%s\" --movie \"%s\" > \"%s\" 2>&1", runner,
                 movie, output) <= 0)
        return -1;
    return system(command);
}

static int run_jam(const char *runner, const char *movie, const char *rom, const char *output)
{
    char command[8192];
    if (snprintf(command, sizeof command,
                 "\"%s\" --movie \"%s\" --rom \"%s\" > \"%s\" 2>&1", runner,
                 movie, rom, output) <= 0)
        return -1;
    return system(command);
}

static int rejected_before_replay(const char *runner, const char *movie, const char *output)
{
    char contents[1024];
    size_t used;
    FILE *file;
    if (run(runner, movie, output) == 0)
        return 0;
    file = fopen(output, "rb");
    if (file == NULL)
        return 0;
    used = fread(contents, 1, sizeof contents - 1u, file);
    fclose(file);
    contents[used] = '\0';
    return strstr(contents, "frame ") == NULL;
}

static int same_file(const char *a, const char *b)
{
    unsigned char left[2048], right[2048];
    size_t left_n, right_n;
    FILE *fa = fopen(a, "rb");
    FILE *fb = fopen(b, "rb");
    if (fa == NULL || fb == NULL) {
        if (fa != NULL)
            fclose(fa);
        if (fb != NULL)
            fclose(fb);
        return 0;
    }
    left_n = fread(left, 1, sizeof left, fa);
    right_n = fread(right, 1, sizeof right, fb);
    fclose(fa);
    fclose(fb);
    return left_n == right_n && left_n < sizeof left && memcmp(left, right, left_n) == 0;
}

int main(int argc, char **argv)
{
    unsigned char bytes[28];
    unsigned char *rom_bytes;
    FILE *file;
    char line[256];

    CHECK(argc == 6);
    if (argc != 6)
        CHECK_DONE();

    header(bytes, 0u);
    CHECK(write_bytes(argv[2], bytes, 16u));
    CHECK_EQ_U64(run(argv[1], argv[2], argv[3]), 0);
    file = fopen(argv[3], "rb");
    CHECK(file != NULL);
    if (file != NULL) {
        CHECK(fgets(line, sizeof line, file) == NULL);
        fclose(file);
    }

    header(bytes, 3u);
    for (uint32_t frame = 0; frame < 3u; frame++) {
        bytes[16u + frame * 4u] = MOVIE_FIXTURE_PORT0;
        bytes[17u + frame * 4u] = 0u;
        bytes[18u + frame * 4u] = MOVIE_FIXTURE_PORT1;
        bytes[19u + frame * 4u] = 0u;
    }
    CHECK(write_bytes(argv[2], bytes, 28u));
    CHECK_EQ_U64(run(argv[1], argv[2], argv[3]), 0);
    CHECK_EQ_U64(run(argv[1], argv[2], argv[4]), 0);
    CHECK(same_file(argv[3], argv[4]));
    file = fopen(argv[3], "rb");
    CHECK(file != NULL);
    if (file != NULL) {
        CHECK(fgets(line, sizeof line, file) != NULL);
        CHECK(strstr(line, "frame 1 ticks ") == line);
        CHECK(fgets(line, sizeof line, file) != NULL);
        CHECK(strstr(line, "frame 2 ticks ") == line);
        CHECK(fgets(line, sizeof line, file) != NULL);
        CHECK(strstr(line, "frame 3 ticks ") == line);
        CHECK(fgets(line, sizeof line, file) == NULL);
        fclose(file);
    }

    /* Every malformed encoding is rejected before a frame is emitted. */
    header(bytes, 1u);
    CHECK(write_bytes(argv[2], bytes, 15u));
    CHECK(rejected_before_replay(argv[1], argv[2], argv[3]));
    header(bytes, 0u);
    bytes[8] = 2u;
    CHECK(write_bytes(argv[2], bytes, 16u));
    CHECK(rejected_before_replay(argv[1], argv[2], argv[3]));
    header(bytes, 1u);
    bytes[16] = 0u;
    bytes[17] = 1u; /* mask exceeds the defined eight controller bits */
    CHECK(write_bytes(argv[2], bytes, 20u));
    CHECK(rejected_before_replay(argv[1], argv[2], argv[3]));
    header(bytes, 1u);
    CHECK(write_bytes(argv[2], bytes, 16u));
    CHECK(rejected_before_replay(argv[1], argv[2], argv[3]));
    header(bytes, 0u);
    bytes[16] = 0u;
    CHECK(write_bytes(argv[2], bytes, 17u));
    CHECK(rejected_before_replay(argv[1], argv[2], argv[3]));
    header(bytes, 1000001u);
    CHECK(write_bytes(argv[2], bytes, 16u));
    CHECK(rejected_before_replay(argv[1], argv[2], argv[3]));

    /* The first JAM stops movie playback and names the one-based frame. */
    rom_bytes = (unsigned char *)calloc(1u, 16u + 16384u + 8192u);
    CHECK(rom_bytes != NULL);
    if (rom_bytes != NULL) {
        rom_bytes[0] = 'N';
        rom_bytes[1] = 'E';
        rom_bytes[2] = 'S';
        rom_bytes[3] = 0x1au;
        rom_bytes[4] = 1u;
        rom_bytes[5] = 1u;
        rom_bytes[16] = 0x02u;
        rom_bytes[16u + 0x3ffcu] = 0x00u;
        rom_bytes[16u + 0x3ffdu] = 0x80u;
        CHECK(write_bytes(argv[5], rom_bytes, 16u + 16384u + 8192u));
        free(rom_bytes);
        header(bytes, 1u);
        bytes[16] = MOVIE_FIXTURE_PORT0;
        bytes[17] = 0u;
        bytes[18] = MOVIE_FIXTURE_PORT1;
        bytes[19] = 0u;
        CHECK(write_bytes(argv[2], bytes, 20u));
        CHECK(run_jam(argv[1], argv[2], argv[5], argv[3]) != 0);
        file = fopen(argv[3], "rb");
        CHECK(file != NULL);
        if (file != NULL) {
            CHECK(fgets(line, sizeof line, file) != NULL);
            CHECK(strstr(line, "JAM stopped replay at frame 1") != NULL);
            fclose(file);
        }
    }
    remove(argv[2]);
    CHECK(rejected_before_replay(argv[1], argv[2], argv[3]));

    CHECK_DONE();
}
