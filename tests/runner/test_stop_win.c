/* runner.stop.win_handler: the Windows console control handler called directly (D-14).
   Needs no console. CTRL_C_EVENT and CTRL_BREAK_EVENT are handled once and declined after, logoff
   and shutdown are declined, and a close event holds its thread until the exit sequence is done or
   the cap passes (SetConsoleCtrlHandler and HandlerRoutine, Microsoft Learn). */
#include "../check.h"

#include "stop.h"
#include "stop_internal.h"

typedef struct {
    volatile LONG finished;
    volatile LONG result;
    volatile ULONGLONG returned_at;
} close_case;

static DWORD WINAPI close_thread(LPVOID argument)
{
    close_case *state = (close_case *)argument;
    BOOL result = nesturbator_run_stop_handler(CTRL_CLOSE_EVENT);
    state->returned_at = GetTickCount64();
    (void)InterlockedExchange(&state->result, result != FALSE ? 1 : 0);
    (void)InterlockedExchange(&state->finished, 1);
    return 0u;
}

/* Starts the close event on its own thread; returns the thread handle or NULL. */
static HANDLE start_close(close_case *state)
{
    state->finished = 0;
    state->result = 0;
    state->returned_at = 0u;
    return CreateThread(NULL, 0u, close_thread, state, 0u, NULL);
}

/* Waits up to 2000 ms for the handler to record the request. Returns 1 if it did. */
static int wait_requested(void)
{
    ULONGLONG start = GetTickCount64();
    while (nesturbator_run_stop_requested() == 0) {
        if (GetTickCount64() - start >= 2000u)
            return 0;
        Sleep(5u);
    }
    return 1;
}

static void test_event_pair(DWORD type)
{
    nesturbator_run_stop_reset();
    CHECK(nesturbator_run_stop_requested() == 0);
    CHECK(nesturbator_run_stop_handler(type) == TRUE);
    CHECK(nesturbator_run_stop_requested() == 1);
    CHECK(nesturbator_run_stop_handler(type) == FALSE);
    CHECK(nesturbator_run_stop_requested() == 1);
}

static void test_declined(DWORD type)
{
    nesturbator_run_stop_reset();
    CHECK(nesturbator_run_stop_handler(type) == FALSE);
    CHECK(nesturbator_run_stop_requested() == 0);
}

/* Close holds until done is called, then returns within the cap. */
static void test_close_waits_for_done(void)
{
    close_case state;
    HANDLE thread;
    ULONGLONG start;
    ULONGLONG t_done;

    nesturbator_run_stop_reset();
    nesturbator_run_stop_set_close_wait(3000u, 0u);
    start = GetTickCount64();
    thread = start_close(&state);
    CHECK(thread != NULL);
    if (thread == NULL)
        return;
    CHECK(wait_requested() == 1);
    CHECK(WaitForSingleObject(thread, 200u) == WAIT_TIMEOUT);
    t_done = GetTickCount64();
    nesturbator_run_stop_done();
    CHECK(WaitForSingleObject(thread, 2000u) == WAIT_OBJECT_0);
    CHECK(state.finished == 1);
    CHECK(state.result == 1);
    CHECK(state.returned_at >= t_done);
    CHECK(state.returned_at - start < 3000u);
    CloseHandle(thread);
}

/* Close returns at the cap when done never comes. */
static void test_close_cap(void)
{
    close_case state;
    HANDLE thread;
    ULONGLONG start;
    ULONGLONG elapsed;

    nesturbator_run_stop_reset();
    nesturbator_run_stop_set_close_wait(300u, 0u);
    start = GetTickCount64();
    thread = start_close(&state);
    CHECK(thread != NULL);
    if (thread == NULL)
        return;
    CHECK(WaitForSingleObject(thread, 5000u) == WAIT_OBJECT_0);
    CHECK(state.finished == 1);
    CHECK(state.result == 1);
    elapsed = state.returned_at - start;
    CHECK(elapsed >= 250u);
    CHECK(elapsed < 2000u);
    CloseHandle(thread);
}

/* After done, close waits the grace period before it returns. */
static void test_close_grace(void)
{
    close_case state;
    HANDLE thread;
    ULONGLONG t_done;

    nesturbator_run_stop_reset();
    nesturbator_run_stop_set_close_wait(3000u, 300u);
    thread = start_close(&state);
    CHECK(thread != NULL);
    if (thread == NULL)
        return;
    CHECK(wait_requested() == 1);
    t_done = GetTickCount64();
    nesturbator_run_stop_done();
    CHECK(WaitForSingleObject(thread, 5000u) == WAIT_OBJECT_0);
    CHECK(state.finished == 1);
    CHECK(state.result == 1);
    CHECK(state.returned_at >= t_done);
    CHECK(state.returned_at - t_done >= 250u);
    CloseHandle(thread);
}

int main(void)
{
    test_event_pair(CTRL_C_EVENT);
    test_event_pair(CTRL_BREAK_EVENT);
    test_declined(CTRL_LOGOFF_EVENT);
    test_declined(CTRL_SHUTDOWN_EVENT);
    test_close_waits_for_done();
    test_close_cap();
    test_close_grace();
    CHECK_DONE();
}
