/* Replay main shared by the fuzz harnesses. Included at the end of a harness that defines
 * LLVMFuzzerTestOneInput. Built only with NESTURBATOR_FUZZ_REPLAY, so the libFuzzer binaries keep
 * libFuzzer's own main. A harness may define NESTURBATOR_REPLAY_EXTRA_MAIN and provide
 * nt_replay_extra_main(argc, argv) before including this file; it returns -1 when it does not
 * handle the arguments. NESTURBATOR_REPLAY_NAME prefixes the diagnostics. */
#ifndef NESTURBATOR_FUZZ_REPLAY_MAIN_H
#define NESTURBATOR_FUZZ_REPLAY_MAIN_H

#ifdef NESTURBATOR_FUZZ_REPLAY
#ifndef NESTURBATOR_REPLAY_NAME
#define NESTURBATOR_REPLAY_NAME "fuzz.regress"
#endif
#ifdef NESTURBATOR_FUZZ_METRICS
void nt_replay_report_metrics(void);
#endif

static int replay_file(const char *path)
{
    FILE *file = fopen(path, "rb");
    long length;
    uint8_t *bytes;
    if (file == NULL) {
        fprintf(stderr, NESTURBATOR_REPLAY_NAME ": cannot open corpus seed: %s\n", path);
        return 0;
    }
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0 || (unsigned long)length > 64ul * 1024ul * 1024ul) {
        fprintf(stderr, NESTURBATOR_REPLAY_NAME ": invalid corpus seed: %s\n", path);
        (void)fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc(length == 0 ? 1u : (size_t)length);
    if (bytes == NULL) {
        fprintf(stderr, NESTURBATOR_REPLAY_NAME ": out of memory reading seed: %s\n", path);
        (void)fclose(file);
        return 0;
    }
    if ((size_t)length != 0u && fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        fprintf(stderr, NESTURBATOR_REPLAY_NAME ": short read: %s\n", path);
        free(bytes);
        (void)fclose(file);
        return 0;
    }
    if (fclose(file) != 0) {
        fprintf(stderr, NESTURBATOR_REPLAY_NAME ": close failed: %s\n", path);
        free(bytes);
        return 0;
    }
    (void)LLVMFuzzerTestOneInput(bytes, (size_t)length);
    free(bytes);
    return 1;
}

int main(int argc, char **argv)
{
#ifdef NESTURBATOR_REPLAY_EXTRA_MAIN
    int extra = nt_replay_extra_main(argc, argv);
    if (extra >= 0)
        return extra;
#endif
    if (argc < 2) {
        fputs(NESTURBATOR_REPLAY_NAME ": no corpus seeds supplied\n", stderr);
        return 1;
    }
    for (int i = 1; i < argc; ++i) {
        if (!replay_file(argv[i]))
            return 1;
    }
#ifdef NESTURBATOR_FUZZ_METRICS
    nt_replay_report_metrics();
#endif
    return 0;
}
#endif

#endif
