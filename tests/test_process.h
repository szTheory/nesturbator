#ifndef NESTURBATOR_TEST_PROCESS_H
#define NESTURBATOR_TEST_PROCESS_H

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

/* Run a child without a command shell; capture stdout and stderr when asked. */
static int test_process_run(const char *const arguments[], const char *output_path)
{
#ifdef _WIN32
    STARTUPINFOA startup;
    PROCESS_INFORMATION process;
    SECURITY_ATTRIBUTES security;
    HANDLE output = INVALID_HANDLE_VALUE;
    HANDLE child_output;
    char command_line[32768];
    size_t used = 0u;
    size_t argument;
    DWORD wait_result;
    DWORD exit_code = 1u;

    for (argument = 0u; arguments[argument] != NULL; ++argument) {
        const char *text = arguments[argument];
        size_t slashes = 0u;

        if (argument != 0u) {
            if (used + 1u >= sizeof command_line)
                return -1;
            command_line[used++] = ' ';
        }
        if (used + 1u >= sizeof command_line)
            return -1;
        command_line[used++] = '"';
        while (*text != '\0') {
            size_t count;
            char value = *text++;
            if (value == '\\') {
                ++slashes;
                continue;
            }
            count = value == '"' ? slashes * 2u + 1u : slashes;
            if (count > sizeof command_line - used - 1u)
                return -1;
            while (count-- != 0u)
                command_line[used++] = '\\';
            if (used + 1u >= sizeof command_line)
                return -1;
            command_line[used++] = value;
            slashes = 0u;
        }
        if (slashes > (sizeof command_line - used - 2u) / 2u)
            return -1;
        {
            size_t trailing = slashes * 2u;
            while (trailing-- != 0u)
                command_line[used++] = '\\';
        }
        if (used + 1u >= sizeof command_line)
            return -1;
        command_line[used++] = '"';
    }
    if (used == 0u || used >= sizeof command_line)
        return -1;
    command_line[used] = '\0';

    if (output_path != NULL) {
        security.nLength = (DWORD)sizeof security;
        security.lpSecurityDescriptor = NULL;
        security.bInheritHandle = TRUE;
        output = CreateFileA(output_path, GENERIC_WRITE, FILE_SHARE_READ, &security, CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL, NULL);
        if (output == INVALID_HANDLE_VALUE)
            return -1;
    }
    child_output = output_path != NULL ? output : GetStdHandle(STD_OUTPUT_HANDLE);
    memset(&startup, 0, sizeof startup);
    memset(&process, 0, sizeof process);
    startup.cb = (DWORD)sizeof startup;
    startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startup.hStdOutput = child_output;
    startup.hStdError = output_path != NULL ? output : GetStdHandle(STD_ERROR_HANDLE);
    if (!CreateProcessA(arguments[0], command_line, NULL, NULL, TRUE, 0u, NULL, NULL, &startup,
                        &process)) {
        if (output != INVALID_HANDLE_VALUE)
            CloseHandle(output);
        return -1;
    }
    wait_result = WaitForSingleObject(process.hProcess, INFINITE);
    if (wait_result == WAIT_OBJECT_0)
        (void)GetExitCodeProcess(process.hProcess, &exit_code);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    if (output != INVALID_HANDLE_VALUE)
        CloseHandle(output);
    return wait_result == WAIT_OBJECT_0 && exit_code == 0u ? 0 : 1;
#else
    pid_t child;
    pid_t waited;
    int status;

    (void)fflush(NULL);
    child = fork();
    if (child < 0)
        return -1;
    if (child == 0) {
        if (output_path != NULL) {
            if (freopen(output_path, "wb", stdout) == NULL ||
                dup2(STDOUT_FILENO, STDERR_FILENO) < 0)
                _exit(126);
        }
        execv(arguments[0], (char *const *)arguments);
        _exit(127);
    }
    do {
        waited = waitpid(child, &status, 0);
    } while (waited < 0 && errno == EINTR);
    if (waited < 0)
        return -1;
    return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : 1;
#endif
}

#endif
