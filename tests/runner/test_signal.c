#define _POSIX_C_SOURCE 200809L
/* macOS hides FIONREAD under _POSIX_C_SOURCE alone; this has no effect elsewhere. */
#define _DARWIN_C_SOURCE

/* External signals against the real nesturbator-run process (RUN-02, D-15).
   Both macros above are feature-test macros: they only make POSIX (and, on macOS,
   Darwin) declarations visible in the system headers. The compiler mode stays -std=c17.

   Usage: runner.signal RUNNER FIXTURE_DIR WORK_DIR CASE

   Cases: term, int, term_exitflush, second, load_error, second_in_flush.

   Rules of the helper: it never sleeps to wait for readiness (a sleep appears only as the
   poll interval inside a deadline loop), no wait is unbounded, and every failure SIGKILLs
   and reaps the child first so a hung runner cannot hang CI. Each timed stop prints
   "signal-to-exit <ms> ms" to stderr so the hosted latency shows in the CI log (D-17). */
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "../check.h"

#define PATH_LEN 1024u
#define SAVE_SIZE 8192
#define READY_DEADLINE_MS 10000
#define EXIT_DEADLINE_MS 5000
#define STOP_LIMIT_MS 2000
#define ERR_CAP 4096u

static char *g_runner;
static char *g_fixture;
static char *g_work;
static char g_err[PATH_LEN];

static int64_t now_ms(void)
{
    struct timespec ts;
    (void)clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000 + (int64_t)(ts.tv_nsec / 1000000L);
}

/* A poll interval inside a deadline loop; never a readiness wait on its own. */
static void pause_ms(long ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    (void)nanosleep(&ts, NULL);
}

static void join(char *out, const char *dir, const char *name)
{
    if (snprintf(out, PATH_LEN, "%s/%s", dir, name) >= (int)PATH_LEN) {
        fprintf(stderr, "path too long: %s/%s\n", dir, name);
        exit(2);
    }
}

static void in_work(char *out, const char *name)
{
    join(out, g_work, name);
}

static void in_fixture(char *out, const char *name)
{
    join(out, g_fixture, name);
}

static void make_dirs(const char *path)
{
    char tmp[PATH_LEN];
    size_t i;
    if (strlen(path) >= sizeof tmp) {
        exit(2);
    }
    strcpy(tmp, path);
    for (i = 1; tmp[i] != '\0'; i++) {
        if (tmp[i] == '/') {
            tmp[i] = '\0';
            (void)mkdir(tmp, 0755);
            tmp[i] = '/';
        }
    }
    (void)mkdir(tmp, 0755);
}

/* Returns 1 and the size when path is a regular file. */
static int regular_size(const char *path, int64_t *size)
{
    struct stat st;
    if (stat(path, &st) != 0 || !S_ISREG(st.st_mode)) {
        return 0;
    }
    *size = (int64_t)st.st_size;
    return 1;
}

static int exists(const char *path)
{
    struct stat st;
    return lstat(path, &st) == 0;
}

static size_t read_file(const char *path, uint8_t *buf, size_t cap)
{
    FILE *f = fopen(path, "rb");
    size_t got;
    if (f == NULL) {
        return 0;
    }
    got = fread(buf, 1, cap, f);
    fclose(f);
    return got;
}

static int copy_file(const char *src, const char *dst)
{
    static uint8_t buf[SAVE_SIZE + 16];
    size_t n = read_file(src, buf, sizeof buf);
    FILE *f;
    int ok;
    if (n == 0) {
        return 0;
    }
    f = fopen(dst, "wb");
    if (f == NULL) {
        return 0;
    }
    ok = fwrite(buf, 1, n, f) == n;
    return fclose(f) == 0 && ok;
}

/* The file is exactly the seed, byte for byte. */
static int same_as(const char *a, const char *b)
{
    static uint8_t x[SAVE_SIZE + 16];
    static uint8_t y[SAVE_SIZE + 16];
    size_t nx = read_file(a, x, sizeof x);
    size_t ny = read_file(b, y, sizeof y);
    return nx != 0 && nx == ny && memcmp(x, y, nx) == 0;
}

/* Whole battery save: 8192 bytes, first four bytes "SAVE". */
static int save_is_whole(const char *path)
{
    static uint8_t buf[SAVE_SIZE + 16];
    int64_t size = 0;
    if (!regular_size(path, &size) || size != SAVE_SIZE) {
        return 0;
    }
    return read_file(path, buf, sizeof buf) == SAVE_SIZE && memcmp(buf, "SAVE", 4) == 0;
}

/* runner.err as a NUL-terminated string in a static buffer. */
static const char *err_text(void)
{
    static char buf[ERR_CAP];
    size_t n = read_file(g_err, (uint8_t *)buf, sizeof buf - 1u);
    buf[n] = '\0';
    return buf;
}

static void reset_child_signals(void)
{
    sigset_t empty;
    sigemptyset(&empty);
    (void)signal(SIGINT, SIG_DFL);
    (void)signal(SIGTERM, SIG_DFL);
    (void)sigprocmask(SIG_SETMASK, &empty, NULL);
}

/* fork + execv. Dispositions and the mask are reset first: they survive exec, and a
   background job of a non-interactive shell has SIGINT ignored (research Pitfall 5). */
static pid_t spawn(char *const argv[], int stdout_fd)
{
    pid_t pid = fork();
    if (pid == 0) {
        int out = stdout_fd;
        int err = open(g_err, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        reset_child_signals();
        if (out < 0) {
            out = open("/dev/null", O_WRONLY);
        }
        if (out < 0 || err < 0 || dup2(out, 1) < 0 || dup2(err, 2) < 0) {
            _exit(126);
        }
        execv(argv[0], argv);
        _exit(127);
    }
    return pid;
}

/* Polls waitpid(WNOHANG) every 5 ms. Returns 1 when reaped, 0 at the deadline. */
static int reap(pid_t pid, int64_t deadline, int *status)
{
    for (;;) {
        pid_t r = waitpid(pid, status, WNOHANG);
        if (r == pid) {
            return 1;
        }
        if (r < 0 && errno != EINTR) {
            return 0;
        }
        if (now_ms() >= deadline) {
            return 0;
        }
        pause_ms(5);
    }
}

/* Records the failure, SIGKILLs and reaps the child (the only unbounded-looking wait, and
   SIGKILL cannot be caught), and shows what the runner said. */
static void kill_and_fail(pid_t pid, const char *why)
{
    int st;
    fprintf(stderr, "FAIL: %s\n--- runner stderr ---\n%s\n---\n", why, err_text());
    check_failures++;
    (void)kill(pid, SIGKILL);
    (void)waitpid(pid, &st, 0);
}

/* Waits for a regular file of the given size (0 means any non-zero size). */
static int wait_file(const char *path, int64_t size, int64_t deadline)
{
    for (;;) {
        int64_t got = 0;
        if (regular_size(path, &got) && (size == 0 ? got > 0 : got == size)) {
            return 1;
        }
        if (now_ms() >= deadline) {
            return 0;
        }
        pause_ms(20);
    }
}

/* Stalled: bytes sit in the pipe and the count is the same on three polls 50 ms apart. */
static int wait_stalled(int read_fd, int64_t deadline)
{
    int last = -1;
    int same = 0;
    for (;;) {
        int n = 0;
        if (ioctl(read_fd, FIONREAD, &n) == 0 && n > 0 && n == last) {
            same++;
        } else {
            same = 0;
        }
        last = n;
        if (same >= 2) {
            return 1;
        }
        if (now_ms() >= deadline) {
            return 0;
        }
        pause_ms(50);
    }
}

/* Polls for ms milliseconds; returns 1 when the child is still running afterwards. */
static int still_alive_for(pid_t pid, long ms, int *status)
{
    return !reap(pid, now_ms() + ms, status);
}

/* After a first signal was delivered, send SIGTERM every 100 ms until the child dies or the
   deadline passes. Repeats are harmless: the handler already restored the default
   disposition, and repeating defeats coalescing of pending identical signals (Pitfall 6). */
static int term_until_dead(pid_t pid, int *status)
{
    int64_t deadline = now_ms() + EXIT_DEADLINE_MS;
    while (now_ms() < deadline) {
        (void)kill(pid, SIGTERM);
        if (reap(pid, now_ms() + 100, status)) {
            return 1;
        }
    }
    return 0;
}

static void remove_stale(void)
{
    const char *names[] = {"battery.sav", "battery.sav.tmp", "ready.state", "runner.err",
                           "state.fifo"};
    size_t i;
    for (i = 0; i < sizeof names / sizeof names[0]; i++) {
        char p[PATH_LEN];
        in_work(p, names[i]);
        (void)unlink(p);
    }
}

/* term, int and term_exitflush. With interval the save is written during the run; without
   it only the exit flush writes it, so fsync runs inside the 2 s window (Pitfall 3). */
static void case_graceful(int sig, int interval)
{
    char rom[PATH_LEN], sav[PATH_LEN], tmp[PATH_LEN], ready[PATH_LEN], state_arg[PATH_LEN + 8u];
    char *argv_interval[] = {g_runner,     "--rom",      rom,    "--frames",
                             "4294967295", "--save-dir", g_work, "--save-interval",
                             "1",          NULL};
    char *argv_flush[] = {g_runner,     "--rom",      rom,    "--frames",
                          "4294967295", "--save-dir", g_work, "--state-at-frame",
                          state_arg,    NULL};
    int status = 0;
    int64_t t0;
    int64_t ms;
    pid_t pid;
    in_fixture(rom, "battery.nes");
    in_work(sav, "battery.sav");
    in_work(tmp, "battery.sav.tmp");
    in_work(ready, "ready.state");
    (void)snprintf(state_arg, sizeof state_arg, "1:%s", ready);
    pid = spawn(interval != 0 ? argv_interval : argv_flush, -1);
    CHECK(pid > 0);
    if (pid <= 0) {
        return;
    }
    /* Both readiness files appear after frame 1, which is after the handlers are installed
       (Pitfall 4), so no signal can reach a runner without its handler. */
    if (!wait_file(interval != 0 ? sav : ready, interval != 0 ? SAVE_SIZE : 0,
                   now_ms() + READY_DEADLINE_MS)) {
        kill_and_fail(pid, "runner never reached frame 1");
        return;
    }
    if (interval == 0) {
        CHECK(!exists(sav)); /* only the exit flush may write it */
    }
    t0 = now_ms();
    (void)kill(pid, sig);
    if (!reap(pid, t0 + EXIT_DEADLINE_MS, &status)) {
        kill_and_fail(pid, "runner did not exit after the signal");
        return;
    }
    ms = now_ms() - t0;
    fprintf(stderr, "signal-to-exit %lld ms\n", (long long)ms);
    CHECK(WIFEXITED(status));
    if (WIFEXITED(status)) {
        CHECK_EQ_U64(WEXITSTATUS(status), 0);
    } else {
        fprintf(stderr, "runner ended by signal %d\n%s\n", WTERMSIG(status), err_text());
    }
    CHECK(ms <= STOP_LIMIT_MS);
    CHECK(save_is_whole(sav));
    CHECK(!exists(tmp));
}

/* A first SIGTERM to a runner blocked in a write to a full pipe changes nothing (the write
   restarts); a later SIGTERM ends it by the default action. */
static void case_second(void)
{
    char rom[PATH_LEN], movie[PATH_LEN], seed[PATH_LEN], sav[PATH_LEN], tmp[PATH_LEN];
    char *argv[] = {g_runner, "--movie", movie, "--rom", rom, "--save-dir", g_work, NULL};
    int fds[2];
    int status = 0;
    pid_t pid;
    in_fixture(rom, "battery.nes");
    in_fixture(movie, "stall.nmovie");
    in_fixture(seed, "seed.sav");
    in_work(sav, "battery.sav");
    in_work(tmp, "battery.sav.tmp");
    if (!copy_file(seed, sav) || pipe(fds) != 0) {
        fprintf(stderr, "cannot prepare the seed save or the pipe\n");
        check_failures++;
        return;
    }
    pid = spawn(argv, fds[1]);
    (void)close(fds[1]);
    CHECK(pid > 0);
    if (pid <= 0) {
        return;
    }
    if (!wait_stalled(fds[0], now_ms() + READY_DEADLINE_MS)) {
        kill_and_fail(pid, "the runner never stalled on a full stdout pipe");
        (void)close(fds[0]);
        return;
    }
    (void)kill(pid, SIGTERM);
    if (!still_alive_for(pid, 500, &status)) {
        kill_and_fail(pid, "the first SIGTERM ended a runner blocked on stdout");
        (void)close(fds[0]);
        return;
    }
    if (!term_until_dead(pid, &status)) {
        kill_and_fail(pid, "a later SIGTERM did not end the stalled runner");
        (void)close(fds[0]);
        return;
    }
    (void)close(fds[0]);
    CHECK(WIFSIGNALED(status));
    if (WIFSIGNALED(status)) {
        CHECK_EQ_U64(WTERMSIG(status), SIGTERM);
    }
    CHECK(same_as(sav, seed));
    CHECK(!exists(tmp));
}

/* D-05: a SIGTERM that arrives while --load-state is being read only sets the flag. The
   read completes, the rejected state gives exit 5, and nothing is stopped or written. */
static void case_load_error(void)
{
    char rom[PATH_LEN], seed[PATH_LEN], sav[PATH_LEN], tmp[PATH_LEN], fifo[PATH_LEN];
    char *argv[] = {g_runner, "--rom",        rom,  "--frames", "4294967295", "--save-dir",
                    g_work,   "--load-state", fifo, NULL};
    int status = 0;
    int wfd = -1;
    int64_t deadline;
    pid_t pid;
    in_fixture(rom, "battery.nes");
    in_fixture(seed, "seed.sav");
    in_work(sav, "battery.sav");
    in_work(tmp, "battery.sav.tmp");
    in_work(fifo, "state.fifo");
    if (!copy_file(seed, sav) || mkfifo(fifo, 0600) != 0) {
        fprintf(stderr, "cannot prepare the seed save or the fifo\n");
        check_failures++;
        return;
    }
    pid = spawn(argv, -1);
    CHECK(pid > 0);
    if (pid <= 0) {
        (void)unlink(fifo);
        return;
    }
    /* A non-blocking open for writing fails with ENXIO until the runner has the fifo open for
       reading, which is after handler install and the battery load, inside the state read. */
    deadline = now_ms() + READY_DEADLINE_MS;
    while (wfd < 0 && now_ms() < deadline) {
        wfd = open(fifo, O_WRONLY | O_NONBLOCK);
        if (wfd < 0) {
            pause_ms(20);
        }
    }
    if (wfd < 0) {
        kill_and_fail(pid, "the runner never opened the state fifo");
        (void)unlink(fifo);
        return;
    }
    (void)fcntl(wfd, F_SETFL, fcntl(wfd, F_GETFL) & ~O_NONBLOCK);
    (void)kill(pid, SIGTERM);
    if (!still_alive_for(pid, 300, &status)) {
        (void)close(wfd);
        kill_and_fail(pid, "the signal ended the runner while it was reading the state");
        (void)unlink(fifo);
        return;
    }
    {
        char junk[16];
        memset(junk, 'X', sizeof junk);
        (void)!write(wfd, junk, sizeof junk);
    }
    (void)close(wfd);
    if (!reap(pid, now_ms() + EXIT_DEADLINE_MS, &status)) {
        kill_and_fail(pid, "the runner did not exit after the state was rejected");
        (void)unlink(fifo);
        return;
    }
    CHECK(WIFEXITED(status));
    if (WIFEXITED(status)) {
        CHECK_EQ_U64(WEXITSTATUS(status), 5);
    }
    {
        const char *e = err_text();
        CHECK(strstr(e, "cannot load state") != NULL);
        CHECK(strstr(e, "stopped by signal") == NULL);
        CHECK(strstr(e, "read error") == NULL);
    }
    CHECK(same_as(sav, seed));
    CHECK(!exists(tmp));
    (void)unlink(fifo);
}

/* Fills the pipe's write end with non-blocking writes; returns bytes queued, 0 on trouble. */
static int fill_pipe(int wfd, int rfd)
{
    char block[4096];
    int n = 0;
    memset(block, 'F', sizeof block);
    (void)fcntl(wfd, F_SETFL, fcntl(wfd, F_GETFL) | O_NONBLOCK);
    while (write(wfd, block, sizeof block) > 0) {
    }
    while (write(wfd, block, 1) > 0) {
    }
    if (ioctl(rfd, FIONREAD, &n) != 0) {
        n = 0;
    }
    /* O_NONBLOCK lives on the open file description, which the child will share. */
    (void)fcntl(wfd, F_SETFL, fcntl(wfd, F_GETFL) & ~O_NONBLOCK);
    return n;
}

/* D-06: after a first SIGTERM has started the stop, a runner held in the exit sequence's
   stdout flush is ended by a later SIGTERM, with the interval-flushed save whole. */
static void case_second_in_flush(void)
{
    char rom[PATH_LEN], sav[PATH_LEN], tmp[PATH_LEN];
    char *argv[] = {g_runner, "--rom",      rom,    "--frames",        "4294967295", "--hash-frame",
                    "1",      "--save-dir", g_work, "--save-interval", "1",          NULL};
    int fds[2];
    int status = 0;
    pid_t pid;
    in_fixture(rom, "battery.nes");
    in_work(sav, "battery.sav");
    in_work(tmp, "battery.sav.tmp");
    if (pipe(fds) != 0) {
        check_failures++;
        return;
    }
    if (fill_pipe(fds[1], fds[0]) <= 0) {
        fprintf(stderr, "the pipe could not be filled\n");
        check_failures++;
        return;
    }
    /* The frame-1 hash line stays in the runner's stdio buffer (stdout is a fully buffered
       pipe), so nothing is written in the loop; only the exit flush blocks. */
    pid = spawn(argv, fds[1]);
    (void)close(fds[1]);
    CHECK(pid > 0);
    if (pid <= 0) {
        return;
    }
    if (!wait_file(sav, SAVE_SIZE, now_ms() + READY_DEADLINE_MS)) {
        kill_and_fail(pid, "runner never wrote the interval save");
        (void)close(fds[0]);
        return;
    }
    (void)kill(pid, SIGTERM);
    if (!still_alive_for(pid, 500, &status)) {
        kill_and_fail(pid, "pipe was not full: the runner finished the stop");
        (void)close(fds[0]);
        return;
    }
    if (strstr(err_text(), "stopped by signal") != NULL) {
        kill_and_fail(pid, "the stop line was printed before the stdout flush finished");
        (void)close(fds[0]);
        return;
    }
    if (!term_until_dead(pid, &status)) {
        kill_and_fail(pid, "a later SIGTERM did not end the runner held in its flush");
        (void)close(fds[0]);
        return;
    }
    (void)close(fds[0]);
    CHECK(WIFSIGNALED(status));
    if (WIFSIGNALED(status)) {
        CHECK_EQ_U64(WTERMSIG(status), SIGTERM);
    }
    CHECK(save_is_whole(sav));
    CHECK(!exists(tmp));
    CHECK(strstr(err_text(), "stopped by signal") == NULL);
}

int main(int argc, char **argv)
{
    const char *name;
    if (argc != 5) {
        fprintf(stderr, "usage: runner.signal RUNNER FIXTURE_DIR WORK_DIR CASE\n");
        return 2;
    }
    g_runner = argv[1];
    g_fixture = argv[2];
    g_work = argv[3];
    name = argv[4];
    make_dirs(g_work);
    in_work(g_err, "runner.err");
    remove_stale();
    if (strcmp(name, "term") == 0) {
        case_graceful(SIGTERM, 1);
    } else if (strcmp(name, "int") == 0) {
        case_graceful(SIGINT, 1);
    } else if (strcmp(name, "term_exitflush") == 0) {
        case_graceful(SIGTERM, 0);
    } else if (strcmp(name, "second") == 0) {
        case_second();
    } else if (strcmp(name, "load_error") == 0) {
        case_load_error();
    } else if (strcmp(name, "second_in_flush") == 0) {
        case_second_in_flush();
    } else {
        fprintf(stderr, "unknown case %s\n", name);
        return 2;
    }
    CHECK_DONE();
}
