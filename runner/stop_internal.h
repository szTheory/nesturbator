/* Windows test seam for the stop module. Not part of the runner's interface. */
#ifndef NESTURBATOR_RUNNER_STOP_INTERNAL_H
#define NESTURBATOR_RUNNER_STOP_INTERNAL_H

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

/* The console control handler (HandlerRoutine in Microsoft Learn). */
BOOL WINAPI nesturbator_run_stop_handler(DWORD type);

/* Sets how long a console close event waits for the exit sequence: at most cap_ms, then, once the
   sequence is done, grace_ms more. */
void nesturbator_run_stop_set_close_wait(DWORD cap_ms, DWORD grace_ms);

/* Clears the requested and done flags and the event count. */
void nesturbator_run_stop_reset(void);
#endif

#endif
