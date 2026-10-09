/* Canonical transition and PCM byte contracts, pinned against SHA-256 answers. */
#include <stdint.h>
#include <string.h>

#include "audio_hash.h"
#include "../check.h"

static int digests_are(nesturbator_run_audio_hash *hash, const char *transitions, const char *pcm)
{
    char transitions_hex[65], pcm_hex[65];
    nesturbator_run_audio_hash_final(hash, transitions_hex, pcm_hex);
    return strcmp(transitions_hex, transitions) == 0 && strcmp(pcm_hex, pcm) == 0;
}

int main(void)
{
    static const char empty[] = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    static const int16_t pcm_samples[] = {0x1234, -2, INT16_MIN, INT16_MAX};
    static const int16_t silence[] = {0, 0, 0, 0};
    nesturbator_run_audio_hash hash;

    nesturbator_run_audio_hash_init(&hash);
    CHECK(digests_are(&hash, empty, empty));

    nesturbator_run_audio_hash_init(&hash);
    nesturbator_run_audio_hash_transition(&hash, UINT64_C(0x0102030405060708), -2);
    CHECK(digests_are(&hash, "9592b5dd6c49d204359728848c79f7554bdf310a062bb406c9bbdfafde9bf6b1",
                      empty));

    nesturbator_run_audio_hash_init(&hash);
    nesturbator_run_audio_hash_transition(&hash, 7u, 10);
    nesturbator_run_audio_hash_transition(&hash, 7u, -10);
    CHECK(digests_are(&hash, "c0b92c25e030490a160529f739ad385eb65bd1101146cc01825e55bab9603a38",
                      empty));

    nesturbator_run_audio_hash_init(&hash);
    nesturbator_run_audio_hash_pcm(&hash, pcm_samples, sizeof pcm_samples / sizeof pcm_samples[0]);
    CHECK(digests_are(&hash, empty,
                      "e5639acff82e7c69e5f71c8fead7bcb4769dafb5c975797fafcbf5da868b5b4f"));

    nesturbator_run_audio_hash_init(&hash);
    nesturbator_run_audio_hash_pcm(&hash, silence, sizeof silence / sizeof silence[0]);
    CHECK(digests_are(&hash, empty,
                      "af5570f5a1810b7af78caf4bc70a660f0df51e42baf91d4de5b2328de0e83dfc"));
    CHECK_DONE();
}
