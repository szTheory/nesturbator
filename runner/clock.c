#define _POSIX_C_SOURCE 199309L
/* _POSIX_C_SOURCE only makes clock_gettime visible under -std=c17; it adds no extension. */
#include "clock.h"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

/* QueryPerformanceCounter is a monotonic tick count and QueryPerformanceFrequency its rate
   (Microsoft Learn, "Acquiring high-resolution time stamps"). The conversion splits whole seconds
   from the remainder so the multiplication cannot overflow. */
uint64_t nesturbator_run_clock_now_ns(void)
{
    static uint64_t freq;
    LARGE_INTEGER t;
    uint64_t ticks;
    if (freq == 0u) {
        LARGE_INTEGER f;
        if (!QueryPerformanceFrequency(&f) || f.QuadPart <= 0) {
            return 0u;
        }
        freq = (uint64_t)f.QuadPart;
    }
    if (!QueryPerformanceCounter(&t)) {
        return 0u;
    }
    ticks = (uint64_t)t.QuadPart;
    return (ticks / freq) * 1000000000u + (ticks % freq) * 1000000000u / freq;
}

#else

#include <time.h>

/* CLOCK_MONOTONIC never jumps with the wall clock (POSIX.1-2017 clock_getres, clock_gettime). */
uint64_t nesturbator_run_clock_now_ns(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0u;
    }
    return (uint64_t)ts.tv_sec * 1000000000u + (uint64_t)ts.tv_nsec;
}

#endif
