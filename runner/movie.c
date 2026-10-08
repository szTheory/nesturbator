#include "movie.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MOVIE_HEADER_SIZE 16u
#define MOVIE_FRAME_LIMIT 1000000u

static uint32_t read_u32le(const uint8_t *bytes)
{
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) | ((uint32_t)bytes[2] << 16) |
           ((uint32_t)bytes[3] << 24);
}

void nesturbator_movie_free(nesturbator_movie *movie)
{
    if (movie != NULL) {
        free(movie->masks);
        movie->masks = NULL;
        movie->frame_count = 0u;
    }
}

int nesturbator_movie_read(const char *path, nesturbator_movie *movie)
{
    static const uint8_t magic[8] = {'N', 'M', 'O', 'V', 'I', 'E', '1', 0};
    FILE *file = NULL;
    uint8_t header[MOVIE_HEADER_SIZE];
    uint8_t *masks = NULL;
    long length;
    uint32_t count;
    uint64_t expected;
    int ok = 0;

    if (path == NULL || movie == NULL)
        return 0;
    movie->frame_count = 0u;
    movie->masks = NULL;
    file = fopen(path, "rb");
    if (file == NULL || fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0 || (uint64_t)length < MOVIE_HEADER_SIZE ||
        fread(header, 1, sizeof header, file) != sizeof header)
        goto done;
    if (memcmp(header, magic, sizeof magic) != 0 || read_u32le(header + 8u) != 1u)
        goto done;
    count = read_u32le(header + 12u);
    if (count > MOVIE_FRAME_LIMIT)
        goto done;
    expected = MOVIE_HEADER_SIZE + (uint64_t)count * 4u;
    if ((uint64_t)length != expected)
        goto done;
    if (count != 0u) {
        uint8_t record[4];
        masks = (uint8_t *)malloc((size_t)count * 2u);
        if (masks == NULL)
            goto done;
        for (uint32_t frame = 0; frame < count; frame++) {
            if (fread(record, 1, sizeof record, file) != sizeof record)
                goto done;
            if (record[1] != 0u || record[3] != 0u)
                goto done;
            masks[(size_t)frame * 2u] = record[0];
            masks[(size_t)frame * 2u + 1u] = record[2];
        }
    }
    if (fgetc(file) != EOF || ferror(file))
        goto done;
    movie->frame_count = count;
    movie->masks = masks;
    masks = NULL;
    ok = 1;

done:
    free(masks);
    if (file != NULL)
        fclose(file);
    return ok;
}
