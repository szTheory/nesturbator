/* Fuzz and replay the public cartridge load/unload lifecycle. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "nesturbator.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    nesturbator *inst = NULL;
    nesturbator_config config = {0};
    config.size = (uint32_t)sizeof config;
    config.abi = NESTURBATOR_ABI_VERSION;
    if (nesturbator_create(&config, &inst) != NESTURBATOR_OK)
        return 0;
    (void)nesturbator_load_cartridge(inst, data, size);
    nesturbator_unload_cartridge(inst);
    nesturbator_destroy(inst);
    return 0;
}

#ifdef NESTURBATOR_FUZZ_REPLAY
static int replay_file(const char *path)
{
    FILE *file = fopen(path, "rb");
    long length;
    uint8_t *bytes;
    if (file == NULL) {
        fprintf(stderr, "fuzz.regress: cannot open corpus seed: %s\n", path);
        return 0;
    }
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0 || (unsigned long)length > 64ul * 1024ul * 1024ul) {
        fprintf(stderr, "fuzz.regress: invalid corpus seed: %s\n", path);
        (void)fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc(length == 0 ? 1u : (size_t)length);
    if (bytes == NULL) {
        fprintf(stderr, "fuzz.regress: out of memory reading seed: %s\n", path);
        (void)fclose(file);
        return 0;
    }
    if ((size_t)length != 0u && fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        fprintf(stderr, "fuzz.regress: short read: %s\n", path);
        free(bytes);
        (void)fclose(file);
        return 0;
    }
    if (fclose(file) != 0) {
        fprintf(stderr, "fuzz.regress: close failed: %s\n", path);
        free(bytes);
        return 0;
    }
    (void)LLVMFuzzerTestOneInput(bytes, (size_t)length);
    free(bytes);
    return 1;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fputs("fuzz.regress: no corpus seeds supplied\n", stderr);
        return 1;
    }
    for (int i = 1; i < argc; ++i) {
        if (!replay_file(argv[i]))
            return 1;
    }
    return 0;
}
#endif
