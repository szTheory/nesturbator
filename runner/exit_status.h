/* The exit statuses of nesturbator-run, defined once (LIBRETRO-AND-RUNNER section 5).
 *
 * This header is runner-internal: it is not installed and nothing from it is part of
 * nesturbator.h. --help prints the table below and the README section "Exit status" repeats it;
 * the test runner.exit.table keeps the three in step. 3 and 77 are reserved and have no
 * enumerator. Boolean helpers in the runner sources keep returning 0/1 as truth values; only a
 * value that becomes the process status uses these names.
 */
#ifndef NESTURBATOR_RUNNER_EXIT_STATUS_H
#define NESTURBATOR_RUNNER_EXIT_STATUS_H

/* X(enumerator, status, text), in ascending status order. */
#define RUNNER_EXIT_TABLE(X)                                                                       \
    X(RUNNER_EXIT_DONE, 0, "done")                                                                 \
    X(RUNNER_EXIT_FAILURE, 1, "failure")                                                           \
    X(RUNNER_EXIT_USAGE, 2, "usage error")                                                         \
    X(RUNNER_EXIT_SAVE_SIZE, 4, ".sav size mismatch")                                              \
    X(RUNNER_EXIT_STATE_REJECTED, 5, "state rejected")

#define RUNNER_EXIT_RESERVED_TEXT "3 and 77 are reserved"

#define RUNNER_EXIT_ENUM(name, value, text) name = value,
enum { RUNNER_EXIT_TABLE(RUNNER_EXIT_ENUM) };
#undef RUNNER_EXIT_ENUM

#endif
