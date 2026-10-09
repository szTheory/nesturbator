/* Canonical little-endian serialization for the runner's audio hashes. */
#include "audio_hash.h"

void nesturbator_run_audio_hash_init(nesturbator_run_audio_hash *hash)
{
    nesturbator_run_sha256_init(&hash->transitions);
    nesturbator_run_sha256_init(&hash->pcm);
}

void nesturbator_run_audio_hash_transition(void *context, uint64_t cpu_cycle, int32_t level)
{
    nesturbator_run_audio_hash *hash = (nesturbator_run_audio_hash *)context;
    uint32_t bits = (uint32_t)level;
    uint8_t record[12];
    for (uint32_t i = 0u; i < 8u; i++)
        record[i] = (uint8_t)(cpu_cycle >> (8u * i));
    for (uint32_t i = 0u; i < 4u; i++)
        record[8u + i] = (uint8_t)(bits >> (8u * i));
    nesturbator_run_sha256_update(&hash->transitions, record, sizeof record);
}

void nesturbator_run_audio_hash_pcm(nesturbator_run_audio_hash *hash, const int16_t *samples,
                                    size_t count)
{
    uint8_t bytes[256];
    while (count != 0u) {
        size_t take = count < sizeof bytes / 2u ? count : sizeof bytes / 2u;
        for (size_t i = 0u; i < take; i++) {
            uint16_t bits = (uint16_t)samples[i];
            bytes[2u * i] = (uint8_t)bits;
            bytes[2u * i + 1u] = (uint8_t)(bits >> 8);
        }
        nesturbator_run_sha256_update(&hash->pcm, bytes, take * 2u);
        samples += take;
        count -= take;
    }
}

void nesturbator_run_audio_hash_final(nesturbator_run_audio_hash *hash, char transitions_hex[65],
                                      char pcm_hex[65])
{
    uint8_t digest[32];
    nesturbator_run_sha256_final(&hash->transitions, digest);
    nesturbator_run_sha256_hex(digest, transitions_hex);
    nesturbator_run_sha256_final(&hash->pcm, digest);
    nesturbator_run_sha256_hex(digest, pcm_hex);
}
