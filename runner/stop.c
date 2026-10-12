#define _DEFAULT_SOURCE
/* _DEFAULT_SOURCE is a feature-test macro: it only makes POSIX and BSD declarations visible in the
   system headers, so the compiler mode stays -std=c17 without extensions. glibc gives signal()
   BSD (SA_RESTART) semantics only with it (signal(2)); without it a blocking call can fail with
   EINTR instead of resuming. */
#include "stop.h"

#include "stop_internal.h"

#include <signal.h>
#include <string.h>

#ifndef _WIN32

/* The handler restores the default disposition so a second signal ends the process (D-02), then
   stores the flag. Both are async-signal-safe (C17 7.14.1.1, POSIX signal-safety). */
static volatile sig_atomic_t stop_flag;

static void on_signal(int sig)
{
    (void)signal(sig, SIG_DFL);
    stop_flag = 1;
}

void nesturbator_run_stop_install(void)
{
    (void)signal(SIGINT, on_signal);
    (void)signal(SIGTERM, on_signal);
}

int nesturbator_run_stop_requested(void)
{
    return stop_flag != 0 ? 1 : 0;
}

void nesturbator_run_stop_done(void)
{
}

int nesturbator_run_stop_parse_kind(const char *text, size_t len, int *kind)
{
    if (text == NULL)
        return 0;
    if (len == 3u && memcmp(text, "int", 3u) == 0) {
        *kind = NESTURBATOR_RUN_STOP_INT;
        return 1;
    }
    if (len == 4u && memcmp(text, "term", 4u) == 0) {
        *kind = NESTURBATOR_RUN_STOP_TERM;
        return 1;
    }
    return 0;
}

int nesturbator_run_stop_default_kind(void)
{
    return NESTURBATOR_RUN_STOP_TERM;
}

void nesturbator_run_stop_raise(int kind, uint32_t count)
{
    int sig = kind == NESTURBATOR_RUN_STOP_INT ? SIGINT : SIGTERM;
    for (uint32_t k = 0; k < count; k++)
        (void)raise(sig);
}

#else

/* STATUS_CONTROL_C_EXIT: the status the default handler leaves when Ctrl+C ends a process. */
#define STOP_CONTROL_C_EXIT 0xC000013Au

/* The handler runs on a thread the system creates. It touches only these three values, through
   Interlocked functions, and never calls the runner (D-03). */
static volatile LONG requested_flag;
static volatile LONG done_flag;
static volatile LONG event_count;
static DWORD close_cap_ms = 4500u;
static DWORD close_grace_ms = 1000u;

static LONG read_flag(volatile LONG *flag)
{
    return InterlockedCompareExchange(flag, 0, 0);
}

/* Waits for the exit sequence on a close event: until done or the cap, then, if done was seen, a
   fixed grace measured from that instant (so the process can finish exiting), all bounded by the
   cap counted from the start of the event. */
static void wait_for_exit_sequence(void)
{
    ULONGLONG start = GetTickCount64();
    ULONGLONG seen = 0u;
    int have_seen = 0;
    for (;;) {
        ULONGLONG now = GetTickCount64();
        if (now - start >= (ULONGLONG)close_cap_ms)
            return;
        if (have_seen == 0 && read_flag(&done_flag) != 0) {
            have_seen = 1;
            seen = now;
        }
        if (have_seen != 0 && now - seen >= (ULONGLONG)close_grace_ms)
            return;
        Sleep(10u);
    }
}

BOOL WINAPI nesturbator_run_stop_handler(DWORD type)
{
    if (type == CTRL_C_EVENT || type == CTRL_BREAK_EVENT) {
        (void)InterlockedExchange(&requested_flag, 1);
        return InterlockedIncrement(&event_count) == 1 ? TRUE : FALSE;
    }
    if (type == CTRL_CLOSE_EVENT) {
        (void)InterlockedExchange(&requested_flag, 1);
        wait_for_exit_sequence();
        return TRUE;
    }
    return FALSE;
}

void nesturbator_run_stop_set_close_wait(DWORD cap_ms, DWORD grace_ms)
{
    close_cap_ms = cap_ms;
    close_grace_ms = grace_ms;
}

void nesturbator_run_stop_reset(void)
{
    (void)InterlockedExchange(&requested_flag, 0);
    (void)InterlockedExchange(&done_flag, 0);
    (void)InterlockedExchange(&event_count, 0);
}

void nesturbator_run_stop_install(void)
{
    (void)SetConsoleCtrlHandler(nesturbator_run_stop_handler, TRUE);
}

int nesturbator_run_stop_requested(void)
{
    return read_flag(&requested_flag) != 0 ? 1 : 0;
}

void nesturbator_run_stop_done(void)
{
    (void)InterlockedExchange(&done_flag, 1);
}

int nesturbator_run_stop_parse_kind(const char *text, size_t len, int *kind)
{
    if (text == NULL)
        return 0;
    if (len == 3u && memcmp(text, "int", 3u) == 0) {
        *kind = NESTURBATOR_RUN_STOP_INT;
        return 1;
    }
    if (len == 5u && memcmp(text, "break", 5u) == 0) {
        *kind = NESTURBATOR_RUN_STOP_BREAK;
        return 1;
    }
    return 0;
}

int nesturbator_run_stop_default_kind(void)
{
    return NESTURBATOR_RUN_STOP_BREAK;
}

/* A repeated event that the handler declines (returns FALSE) would reach the default handler,
   which ends the process; this reproduces that outcome in-process. */
void nesturbator_run_stop_raise(int kind, uint32_t count)
{
    DWORD type = kind == NESTURBATOR_RUN_STOP_INT ? CTRL_C_EVENT : CTRL_BREAK_EVENT;
    for (uint32_t k = 0; k < count; k++) {
        if (nesturbator_run_stop_handler(type) == FALSE)
            ExitProcess(STOP_CONTROL_C_EXIT);
    }
}

#endif
