/* Helpers for generated NMI guest evidence in the runner state tests. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio_hash.h"
#include "internal.h"
#include "nesturbator.h"
#include "nmi_fixture.h"

#define PIXELS (256u * 240u)
#define AUDIO_CAPACITY 1024u

static int read_file(const char *path, uint8_t **bytes, size_t *size)
{
    FILE *file = fopen(path, "rb");
    long length;
    uint8_t *buffer;
    if (file == NULL || fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        if (file != NULL)
            fclose(file);
        return 0;
    }
    buffer = (uint8_t *)malloc((size_t)length);
    if (buffer == NULL || fread(buffer, 1u, (size_t)length, file) != (size_t)length) {
        fclose(file);
        free(buffer);
        return 0;
    }
    if (fclose(file) != 0) {
        free(buffer);
        return 0;
    }
    *bytes = buffer;
    *size = (size_t)length;
    return 1;
}

static int write_fixture(const char *path)
{
    uint8_t image[NMI_FIXTURE_HOST_IMAGE_SIZE];
    size_t size = nmi_fixture_host_image(image, sizeof image);
    FILE *file;
    if (size != sizeof image || (file = fopen(path, "wb")) == NULL)
        return 0;
    if (fwrite(image, 1u, size, file) != size || fclose(file) != 0)
        return 0;
    return 1;
}

static nesturbator *load_instance(const char *rom_path, const char *state_path)
{
    nesturbator_config config;
    nesturbator *instance = NULL;
    uint8_t *rom = NULL;
    uint8_t *state = NULL;
    size_t rom_size = 0u;
    size_t state_size = 0u;
    if (!read_file(rom_path, &rom, &rom_size)) {
        fprintf(stderr, "nmi_fixture: cannot read ROM %s\n", rom_path);
        return NULL;
    }
    memset(&config, 0, sizeof config);
    config.size = (uint32_t)sizeof config;
    config.abi = NESTURBATOR_ABI_VERSION;
    if (nesturbator_create(&config, &instance) != NESTURBATOR_OK || instance == NULL ||
        nesturbator_load_cartridge(instance, rom, rom_size) != NESTURBATOR_OK) {
        fprintf(stderr, "nmi_fixture: cannot create instance or load ROM\n");
        free(rom);
        nesturbator_destroy(instance);
        return NULL;
    }
    free(rom);
    if (state_path != NULL) {
        if (!read_file(state_path, &state, &state_size)) {
            fprintf(stderr, "nmi_fixture: cannot read state %s\n", state_path);
            nesturbator_destroy(instance);
            return NULL;
        }
        if (nesturbator_load_state(instance, state, state_size) != NESTURBATOR_OK) {
            fprintf(stderr, "nmi_fixture: cannot load state %s\n", state_path);
            free(state);
            nesturbator_destroy(instance);
            return NULL;
        }
        free(state);
    }
    return instance;
}

static int read_ram(const nesturbator *instance, uint16_t address, uint8_t *value)
{
    return nesturbator_peek_cpu_ram(instance, address, value) == NESTURBATOR_OK;
}

static int check_marker(const nesturbator *instance, uint32_t expected)
{
    uint8_t count = 0u;
    if (!read_ram(instance, NMI_FIXTURE_HOST_COUNT, &count) || count != expected) {
        fprintf(stderr, "guest NMI marker mismatch: expected %u\n", expected);
        return 0;
    }
    for (uint32_t i = 1u; i <= expected; i++) {
        uint8_t event = 0u;
        if (!read_ram(instance, (uint16_t)(NMI_FIXTURE_HOST_EVENTS + i), &event) || event != i) {
            fprintf(stderr, "guest NMI event mismatch: expected sequence 1..%u\n", expected);
            return 0;
        }
    }
    return 1;
}

static void hash_video(const uint16_t *video, char hex[65])
{
    nesturbator_run_sha256 hash;
    uint8_t digest[32];
    uint8_t row[256u * 2u];
    nesturbator_run_sha256_init(&hash);
    for (uint32_t y = 0u; y < 240u; y++) {
        for (uint32_t x = 0u; x < 256u; x++) {
            uint16_t pixel = video[y * 256u + x];
            row[2u * x] = (uint8_t)pixel;
            row[2u * x + 1u] = (uint8_t)(pixel >> 8);
        }
        nesturbator_run_sha256_update(&hash, row, sizeof row);
    }
    nesturbator_run_sha256_final(&hash, digest);
    nesturbator_run_sha256_hex(digest, hex);
}

static int verify_state(const char *rom_path, const char *state_path, uint32_t expected)
{
    nesturbator *instance = load_instance(rom_path, state_path);
    int ok;
    if (instance == NULL)
        return 1;
    ok = check_marker(instance, expected);
    if (ok)
        printf("guest NMI marker %u; event sequence 1..%u\n", expected, expected);
    nesturbator_destroy(instance);
    return ok ? 0 : 9;
}

static int reference_suffix(const char *rom_path, uint32_t first, uint32_t last)
{
    nesturbator *instance = load_instance(rom_path, NULL);
    static uint16_t video[PIXELS];
    static int16_t audio[AUDIO_CAPACITY];
    nesturbator_frame frame;
    nesturbator_input input;
    nesturbator_run_audio_hash audio_hash;
    char video_hash[65], transitions_hash[65], pcm_hash[65];
    if (instance == NULL || first == 0u || first > last)
        return 1;
    nesturbator_run_audio_hash_init(&audio_hash);
    memset(&input, 0, sizeof input);
    input.size = (uint32_t)sizeof input;
    memset(&frame, 0, sizeof frame);
    frame.size = (uint32_t)sizeof frame;
    frame.video = video;
    frame.video_pitch = 256u;
    frame.audio = audio;
    frame.audio_capacity = AUDIO_CAPACITY;
    for (uint32_t n = 1u; n <= last; n++) {
        if (nesturbator_set_input(instance, &input) != NESTURBATOR_OK)
            goto fail;
        if (n == first)
            nesturbator__apu_set_transition_sink(instance, nesturbator_run_audio_hash_transition,
                                                 &audio_hash);
        if (nesturbator_run_frame(instance, &frame) != NESTURBATOR_OK || !check_marker(instance, n))
            goto fail;
        if (n >= first)
            nesturbator_run_audio_hash_pcm(&audio_hash, audio, frame.audio_count);
    }
    hash_video(video, video_hash);
    nesturbator_run_audio_hash_final(&audio_hash, transitions_hash, pcm_hash);
    printf("frame %u ticks %llu sha256 %s\n", last, (unsigned long long)frame.ticks, video_hash);
    printf("audio transitions sha256 %s\n", transitions_hash);
    printf("audio pcm sha256 %s\n", pcm_hash);
    nesturbator_destroy(instance);
    return 0;
fail:
    nesturbator_destroy(instance);
    return 1;
}

int main(int argc, char **argv)
{
    char *end = NULL;
    unsigned long expected;
    if (argc == 3 && strcmp(argv[1], "write") == 0)
        return write_fixture(argv[2]) ? 0 : 1;
    if (argc == 5 && strcmp(argv[1], "verify") == 0) {
        expected = strtoul(argv[4], &end, 10);
        if (end == argv[4] || *end != '\0' || expected > 255u)
            return 2;
        return verify_state(argv[2], argv[3], (uint32_t)expected);
    }
    if (argc == 5 && strcmp(argv[1], "reference") == 0) {
        unsigned long first = strtoul(argv[3], &end, 10);
        if (end == argv[3] || *end != '\0' || first > UINT32_MAX)
            return 2;
        unsigned long last = strtoul(argv[4], &end, 10);
        if (end == argv[4] || *end != '\0' || last > UINT32_MAX)
            return 2;
        return reference_suffix(argv[2], (uint32_t)first, (uint32_t)last);
    }
    fprintf(stderr, "usage: runner.nmi_fixture write <rom> | verify <rom> <state> <count> | "
                    "reference <rom> <first-frame> <last-frame>\n");
    return 2;
}
