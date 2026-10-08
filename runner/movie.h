/* Owned binary input movies for nesturbator-run. */
#ifndef NESTURBATOR_RUN_MOVIE_H
#define NESTURBATOR_RUN_MOVIE_H

#include <stdint.h>

typedef struct nesturbator_movie {
    uint32_t frame_count;
    uint8_t *masks; /* two uint8_t button masks per frame, port 0 then port 1 */
} nesturbator_movie;

/* Reads and validates the complete file before returning. */
int nesturbator_movie_read(const char *path, nesturbator_movie *movie);
void nesturbator_movie_free(nesturbator_movie *movie);

#endif
