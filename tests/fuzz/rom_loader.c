/* Fuzz and replay the public cartridge load/unload lifecycle. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(__APPLE__) || defined(__linux__)
#include <sys/resource.h>
#endif
#include "nesturbator.h"
#include "failure_control.h"

#define NESTURBATOR_REPLAY_NAME "fuzz.rom_loader"
#define NESTURBATOR_FUZZ_ACCEPTANCE 1

static int nt_last_accepted;
int nt_replay_last_accepted(void);

int nt_replay_last_accepted(void)
{
    return nt_last_accepted;
}

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
    printf("METRICS target=rom-loader inputs=%llu accepted=%llu refused=%llu api_operations=%llu "
           "frame_calls=0 frame_ok=0 frame_jam=0 guest_ticks=0 peak_rss_bytes=%llu\n",
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
    nesturbator_status load_status;
    nt_last_accepted = 0;
    config.size = (uint32_t)sizeof config;
    config.abi = NESTURBATOR_ABI_VERSION;
#ifdef NESTURBATOR_FUZZ_METRICS
    nt_metric_inputs++;
    nt_metric_api_operations++;
#endif
    nt_fuzz_trace_operation("nesturbator_create");
    if (nesturbator_create(&config, &inst) != NESTURBATOR_OK)
        return 0;
#ifdef NESTURBATOR_FUZZ_METRICS
    nt_metric_api_operations++;
#endif
    nt_fuzz_trace_operation("nesturbator_load_cartridge");
#ifdef NESTURBATOR_FUZZ_METRICS
    load_status = nesturbator_load_cartridge(inst, data, size);
    if (load_status == NESTURBATOR_OK)
        nt_metric_accepted++;
#else
    load_status = nesturbator_load_cartridge(inst, data, size);
#endif
    nt_last_accepted = load_status == NESTURBATOR_OK;
    if (nt_last_accepted && nt_fuzz_control_matches("rom-loader", data, size, 16u, 0x4cu))
        nt_fuzz_control_crash("rom-loader");
#ifdef NESTURBATOR_FUZZ_METRICS
    nt_metric_api_operations++;
#endif
    nt_fuzz_trace_operation("nesturbator_unload_cartridge");
    nesturbator_unload_cartridge(inst);
#ifdef NESTURBATOR_FUZZ_METRICS
    nt_metric_api_operations++;
#endif
    nt_fuzz_trace_operation("nesturbator_destroy");
    nesturbator_destroy(inst);
    return 0;
}

#include "replay_main.h"
