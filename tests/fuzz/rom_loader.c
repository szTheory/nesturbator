/* Fuzz and replay the public cartridge load/unload lifecycle. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(__APPLE__) || defined(__linux__)
#include <sys/resource.h>
#endif
#include "nesturbator.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

#ifdef NESTURBATOR_FUZZ_METRICS
static uint64_t nt_metric_inputs;
static uint64_t nt_metric_accepted;
static uint64_t nt_metric_api_operations;

static uint64_t nt_metric_peak_rss_bytes(void)
{
#if defined(__APPLE__) || defined(__linux__)
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) != 0 || usage.ru_maxrss < 0)
        return 0u;
#if defined(__APPLE__)
    return (uint64_t)usage.ru_maxrss;
#else
    return (uint64_t)usage.ru_maxrss * 1024u;
#endif
#else
    return 0u;
#endif
}

void nt_replay_report_metrics(void);
void nt_replay_report_metrics(void)
{
    printf("METRICS target=rom-loader inputs=%llu accepted=%llu refused=%llu api_operations=%llu frame_calls=0 frame_ok=0 frame_jam=0 guest_ticks=0 peak_rss_bytes=%llu\n",
           (unsigned long long)nt_metric_inputs, (unsigned long long)nt_metric_accepted,
           (unsigned long long)(nt_metric_inputs - nt_metric_accepted),
           (unsigned long long)nt_metric_api_operations,
           (unsigned long long)nt_metric_peak_rss_bytes());
}
#endif

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    nesturbator *inst = NULL;
    nesturbator_config config = {0};
    config.size = (uint32_t)sizeof config;
    config.abi = NESTURBATOR_ABI_VERSION;
#ifdef NESTURBATOR_FUZZ_METRICS
    nt_metric_inputs++;
    nt_metric_api_operations++;
#endif
    if (nesturbator_create(&config, &inst) != NESTURBATOR_OK)
        return 0;
#ifdef NESTURBATOR_FUZZ_METRICS
    nt_metric_api_operations++;
    if (nesturbator_load_cartridge(inst, data, size) == NESTURBATOR_OK)
        nt_metric_accepted++;
#else
    (void)nesturbator_load_cartridge(inst, data, size);
#endif
#ifdef NESTURBATOR_FUZZ_METRICS
    nt_metric_api_operations++;
#endif
    nesturbator_unload_cartridge(inst);
#ifdef NESTURBATOR_FUZZ_METRICS
    nt_metric_api_operations++;
#endif
    nesturbator_destroy(inst);
    return 0;
}

#include "replay_main.h"
