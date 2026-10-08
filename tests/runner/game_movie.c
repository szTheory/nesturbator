/* Writes a fixed 180-frame DABG input script for native hash regression. */
#include <stdio.h>
#include <string.h>

#define FRAME_COUNT 180u
#define MOVIE_SIZE (16u + FRAME_COUNT * 4u)

static int write_movie(const char *path, const char *scenario)
{
    unsigned char bytes[MOVIE_SIZE];
    int port0 = strcmp(scenario, "p0") == 0 || strcmp(scenario, "both") == 0;
    int port1 = strcmp(scenario, "p1") == 0 || strcmp(scenario, "both") == 0;
    FILE *file;

    if (!port0 && !port1)
        return 2;
    memset(bytes, 0, sizeof bytes);
    memcpy(bytes, "NMOVIE1", 7u);
    bytes[8] = 1u;
    bytes[12] = (unsigned char)(FRAME_COUNT & 0xffu);
    bytes[13] = (unsigned char)((FRAME_COUNT >> 8) & 0xffu);
    bytes[14] = (unsigned char)((FRAME_COUNT >> 16) & 0xffu);
    bytes[15] = (unsigned char)((FRAME_COUNT >> 24) & 0xffu);
    for (unsigned frame = 0; frame < FRAME_COUNT; frame++) {
        unsigned char p0 = 0u, p1 = 0u;
        if (frame >= 24u && frame < 26u) {
            p0 = port0 ? 0x08u : 0u; /* Start */
            p1 = port1 ? 0x08u : 0u;
        } else if ((frame >= 48u && frame < 50u) || (frame >= 72u && frame < 74u) ||
                   (frame >= 96u && frame < 98u)) {
            p0 = port0 ? 0x01u : 0u; /* A */
            p1 = port1 ? 0x01u : 0u;
        } else if (frame >= 128u && frame < 160u) {
            p0 = port0 ? 0x80u : 0u; /* Right */
            p1 = port1 ? 0x40u : 0u; /* Left */
        }
        bytes[16u + frame * 4u] = p0;
        bytes[18u + frame * 4u] = p1;
    }
    file = fopen(path, "wb");
    if (file == NULL)
        return 1;
    int ok = fwrite(bytes, 1u, sizeof bytes, file) == sizeof bytes;
    if (fclose(file) != 0)
        ok = 0;
    return ok ? 0 : 1;
}

int main(int argc, char **argv)
{
    if (argc != 3)
        return 2;
    return write_movie(argv[1], argv[2]);
}
