/* runner.stop.win_console: a real Ctrl+Break reaches the runner through its own console (D-16).

   runner.stop_console RUNNER FIXTURE_DIR WORK_DIR

   The runner starts in a new console with its output in files. The test waits for a whole
   battery.sav (the handler is installed before frame 1, so no sleep is needed), detaches from its
   own console, attaches to the runner's, registers a handler that accepts Ctrl+Break (attaching
   resets the handler list, so registration must follow it), and sends CTRL_BREAK_EVENT to group 0,
   which reaches every process on that console, this one included. CTRL_C_EVENT is never sent. See
   GenerateConsoleCtrlEvent, AttachConsole, FreeConsole and SetConsoleCtrlHandler in Microsoft
   Learn. Every API failure prints the function name and GetLastError text. */
#include "../check.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <stdio.h>
#include <string.h>

#define SAVE_SIZE 8192u
#define SAVE_WAIT_MS 20000u
#define ATTACH_WAIT_MS 2000u
#define EXIT_WAIT_MS 5000u

static BOOL WINAPI sender_handler(DWORD type)
{
    return type == CTRL_BREAK_EVENT ? TRUE : FALSE;
}

/* Prints the failed function, its error code and the system's text for it. */
static void report(const char *function)
{
    DWORD error = GetLastError();
    char text[512];
    DWORD length = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                                  error, 0u, text, (DWORD)sizeof text, NULL);
    if (length == 0u)
        text[0] = '\0';
    fprintf(stderr, "test_console: %s failed, GetLastError %lu: %s\n", function,
            (unsigned long)error, text);
}

/* Ends the child and closes its handles. */
static void abandon(PROCESS_INFORMATION *process)
{
    (void)TerminateProcess(process->hProcess, 1u);
    (void)WaitForSingleObject(process->hProcess, 2000u);
    CloseHandle(process->hThread);
    CloseHandle(process->hProcess);
}

/* Builds the quoted command line with the quoting rule of tests/test_process.h. Returns 1 on
   success. */
static int build_command_line(char *line, size_t capacity, const char *const arguments[])
{
    size_t used = 0u;
    size_t argument;
    for (argument = 0u; arguments[argument] != NULL; ++argument) {
        const char *text = arguments[argument];
        size_t slashes = 0u;

        if (argument != 0u) {
            if (used + 1u >= capacity)
                return 0;
            line[used++] = ' ';
        }
        if (used + 1u >= capacity)
            return 0;
        line[used++] = '"';
        while (*text != '\0') {
            size_t count;
            char value = *text++;
            if (value == '\\') {
                ++slashes;
                continue;
            }
            count = value == '"' ? slashes * 2u + 1u : slashes;
            if (count > capacity - used - 1u)
                return 0;
            while (count-- != 0u)
                line[used++] = '\\';
            if (used + 1u >= capacity)
                return 0;
            line[used++] = value;
            slashes = 0u;
        }
        if (slashes > (capacity - used - 2u) / 2u)
            return 0;
        {
            size_t trailing = slashes * 2u;
            while (trailing-- != 0u)
                line[used++] = '\\';
        }
        if (used + 1u >= capacity)
            return 0;
        line[used++] = '"';
    }
    if (used == 0u || used >= capacity)
        return 0;
    line[used] = '\0';
    return 1;
}

static int save_is_whole(const char *path)
{
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &data))
        return 0;
    return data.nFileSizeHigh == 0u && data.nFileSizeLow == SAVE_SIZE;
}

static int exists(const char *path)
{
    return GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
}

static HANDLE open_output(const char *path)
{
    SECURITY_ATTRIBUTES security;
    security.nLength = (DWORD)sizeof security;
    security.lpSecurityDescriptor = NULL;
    security.bInheritHandle = TRUE;
    return CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, &security, CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL, NULL);
}

static void check_save_content(const char *path)
{
    unsigned char head[4];
    FILE *file = fopen(path, "rb");
    CHECK(file != NULL);
    if (file == NULL)
        return;
    CHECK(fread(head, 1u, sizeof head, file) == sizeof head);
    CHECK(memcmp(head, "SAVE", 4u) == 0);
    (void)fclose(file);
}

int main(int argc, char **argv)
{
    char save_path[1024];
    char tmp_path[1030];
    char rom_path[1024];
    char out_path[1024];
    char err_path[1024];
    char command_line[4096];
    const char *arguments[12];
    STARTUPINFOA startup;
    PROCESS_INFORMATION process;
    HANDLE out_file;
    HANDLE err_file;
    ULONGLONG start;
    DWORD exit_code = 1u;

    if (argc != 4) {
        fprintf(stderr, "usage: runner.stop_console RUNNER FIXTURE_DIR WORK_DIR\n");
        return 2;
    }
    if (snprintf(save_path, sizeof save_path, "%s/battery.sav", argv[3]) >= (int)sizeof save_path ||
        snprintf(tmp_path, sizeof tmp_path, "%s.tmp", save_path) >= (int)sizeof tmp_path ||
        snprintf(rom_path, sizeof rom_path, "%s/battery.nes", argv[2]) >= (int)sizeof rom_path ||
        snprintf(out_path, sizeof out_path, "%s/runner.out", argv[3]) >= (int)sizeof out_path ||
        snprintf(err_path, sizeof err_path, "%s/runner.err", argv[3]) >= (int)sizeof err_path) {
        fprintf(stderr, "test_console: path too long\n");
        return 2;
    }

    if (!CreateDirectoryA(argv[3], NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        report("CreateDirectoryA");
        return 1;
    }
    (void)DeleteFileA(save_path);
    (void)DeleteFileA(tmp_path);

    arguments[0] = argv[1];
    arguments[1] = "--rom";
    arguments[2] = rom_path;
    arguments[3] = "--frames";
    arguments[4] = "4294967295";
    arguments[5] = "--save-dir";
    arguments[6] = argv[3];
    arguments[7] = "--save-interval";
    arguments[8] = "1";
    arguments[9] = NULL;
    if (!build_command_line(command_line, sizeof command_line, arguments)) {
        fprintf(stderr, "test_console: command line too long\n");
        return 2;
    }

    out_file = open_output(out_path);
    if (out_file == INVALID_HANDLE_VALUE) {
        report("CreateFileA runner.out");
        return 1;
    }
    err_file = open_output(err_path);
    if (err_file == INVALID_HANDLE_VALUE) {
        report("CreateFileA runner.err");
        CloseHandle(out_file);
        return 1;
    }

    memset(&startup, 0, sizeof startup);
    memset(&process, 0, sizeof process);
    startup.cb = (DWORD)sizeof startup;
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startup.hStdOutput = out_file;
    startup.hStdError = err_file;
    if (!CreateProcessA(argv[1], command_line, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL,
                        &startup, &process)) {
        report("CreateProcessA");
        CloseHandle(out_file);
        CloseHandle(err_file);
        return 1;
    }
    CloseHandle(out_file);
    CloseHandle(err_file);

    start = GetTickCount64();
    while (!save_is_whole(save_path)) {
        if (GetTickCount64() - start >= SAVE_WAIT_MS) {
            fprintf(stderr, "test_console: no whole battery.sav within %u ms\n", SAVE_WAIT_MS);
            abandon(&process);
            return 1;
        }
        if (WaitForSingleObject(process.hProcess, 0u) == WAIT_OBJECT_0) {
            (void)GetExitCodeProcess(process.hProcess, &exit_code);
            fprintf(stderr, "test_console: the runner exited early with status %lu\n",
                    (unsigned long)exit_code);
            CloseHandle(process.hThread);
            CloseHandle(process.hProcess);
            return 1;
        }
        Sleep(20u);
    }

    /* Detach first: AttachConsole fails with ERROR_ACCESS_DENIED while a console is attached. */
    (void)FreeConsole();
    start = GetTickCount64();
    while (!AttachConsole(process.dwProcessId)) {
        if (GetTickCount64() - start >= ATTACH_WAIT_MS) {
            report("AttachConsole");
            abandon(&process);
            return 1;
        }
        Sleep(20u);
    }
    /* Attaching reset the handler list, so this must come after it. */
    if (!SetConsoleCtrlHandler(sender_handler, TRUE)) {
        report("SetConsoleCtrlHandler");
        abandon(&process);
        return 1;
    }
    if (!GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0u)) {
        report("GenerateConsoleCtrlEvent");
        abandon(&process);
        return 1;
    }

    if (WaitForSingleObject(process.hProcess, EXIT_WAIT_MS) != WAIT_OBJECT_0) {
        fprintf(stderr, "test_console: the runner did not exit within %u ms\n", EXIT_WAIT_MS);
        abandon(&process);
        return 1;
    }
    if (!GetExitCodeProcess(process.hProcess, &exit_code)) {
        report("GetExitCodeProcess");
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
        return 1;
    }
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);

    CHECK_EQ_HEX(exit_code, 0u);
    CHECK(save_is_whole(save_path));
    check_save_content(save_path);
    CHECK(!exists(tmp_path));
    CHECK_DONE();
}
