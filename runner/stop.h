/* The runner's graceful stop on a termination request (RUN-02).

   Runner-only: the core library links only the C memory functions and never sees a signal or a
   console event. The handler installed here does nothing but record the request. The main loop
   asks nesturbator_run_stop_requested() before it starts each frame, so the current frame and its
   outputs always finish, then the runner flushes and exits 0.

   POSIX: SIGINT and SIGTERM, through the C17 signal function (ISO C17 7.14.1.1; POSIX.1-2017
   signal). The handler resets the disposition to the default and stores a flag, so a second signal
   of the same kind ends the process at once.
   Windows: Ctrl+C, Ctrl+Break and the console close event, through SetConsoleCtrlHandler
   (Microsoft Learn: SetConsoleCtrlHandler, HandlerRoutine). The CRT signal and raise functions are
   not used there.

   The --raise-after test aid signals this process only, through the functions at the end. */
#ifndef NESTURBATOR_RUNNER_STOP_H
#define NESTURBATOR_RUNNER_STOP_H

#include <stddef.h>
#include <stdint.h>

#define NESTURBATOR_RUN_STOP_INT 1
#define NESTURBATOR_RUN_STOP_TERM 2  /* POSIX only */
#define NESTURBATOR_RUN_STOP_BREAK 3 /* Windows only */

/* Installs the handlers. Call once, after the command line is validated. */
void nesturbator_run_stop_install(void);

/* Returns 1 once a stop was requested, else 0. */
int nesturbator_run_stop_requested(void);

/* Tells the handler the exit sequence is complete (Windows: lets a console close event return). */
void nesturbator_run_stop_done(void);

/* Parses the first len characters of text as a signal name for this platform: "int" and "term" on
   POSIX, "int" and "break" on Windows. Stores a NESTURBATOR_RUN_STOP_* value and returns 1, or
   returns 0 for any other text. */
int nesturbator_run_stop_parse_kind(const char *text, size_t len, int *kind);

/* The kind used when --raise-after names none: term on POSIX, break on Windows. */
int nesturbator_run_stop_default_kind(void);

/* Signals this process count times with the given kind. */
void nesturbator_run_stop_raise(int kind, uint32_t count);

#endif
