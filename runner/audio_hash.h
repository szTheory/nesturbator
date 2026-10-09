/* Canonical SHA-256 streams for APU transitions and signed PCM. */
#ifndef NESTURBATOR_RUN_AUDIO_HASH_H
#define NESTURBATOR_RUN_AUDIO_HASH_H

#include <stddef.h>
#include <stdint.h>

#include "sha256.h"

typedef struct nesturbator_run_audio_hash {
    nesturbator_run_sha256 transitions;
    nesturbator_run_sha256 pcm;
} nesturbator_run_audio_hash;

void nesturbator_run_audio_hash_init(nesturbator_run_audio_hash *hash);
void nesturbator_run_audio_hash_transition(void *context, uint64_t cpu_cycle, int32_t level);
void nesturbator_run_audio_hash_pcm(nesturbator_run_audio_hash *hash, const int16_t *samples,
                                    size_t count);
void nesturbator_run_audio_hash_final(nesturbator_run_audio_hash *hash, char transitions_hex[65],
                                      char pcm_hex[65]);

#endif /* NESTURBATOR_RUN_AUDIO_HASH_H */
