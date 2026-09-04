/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  ✨ git-ez : process.c                                      │
 *  │  "Where we talk to the OS and hope it talks nicely back"    │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

void sys_init_console(void) {
#ifdef _WIN32
    // Windows console defaults to old code pages; let's dial it into the 21st century!
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // Turn on ANSI virtual terminal magic so our colors look fabulous
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
    // Linux and macOS users are already living in ANSI UTF-8 wonderland by default :)
}

ProcessResult process_run_ex(const char *dir, const char *command, ProcessLogCallback log_cb, void *user_data) {
    ProcessResult res;
    memset(&res, 0, sizeof(res));
    res.exit_code = -1;
    res.success = false;

    if (!command || !*command) {
        return res;
    }

    size_t offset = 0;

#ifdef _WIN32
    HANDLE hReadPipe = NULL;
    HANDLE hWritePipe = NULL;
    SECURITY_ATTRIBUTES sa;
    memset(&sa, 0, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        snprintf(res.output, sizeof(res.output), "Failed to create communication pipe.");
        return res;
    }
    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;
    si.hStdInput = INVALID_HANDLE_VALUE;

    PROCESS_INFORMATION pi;
    memset(&pi, 0, sizeof(pi));

    char cmd_buf[4096];
    snprintf(cmd_buf, sizeof(cmd_buf), "%s", command);

    const char *work_dir = (dir && *dir) ? dir : NULL;

    BOOL created = CreateProcessA(
        NULL,
        cmd_buf,
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        work_dir,
        &si,
        &pi
    );

    CloseHandle(hWritePipe);

    if (!created) {
        CloseHandle(hReadPipe);
        snprintf(res.output, sizeof(res.output), "Failed to spawn process: %s", command);
        return res;
    }

    char buffer[256];
    DWORD bytesRead = 0;
    char line_buf[1024];
    size_t line_len = 0;

    while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';

        if (offset + bytesRead < sizeof(res.output) - 1) {
            memcpy(res.output + offset, buffer, bytesRead);
            offset += bytesRead;
            res.output[offset] = '\0';
        }

        if (log_cb) {
            for (DWORD i = 0; i < bytesRead; i++) {
                char ch = buffer[i];
                if (ch == '\r') continue;
                if (ch == '\n') {
                    line_buf[line_len] = '\0';
                    if (line_len > 0) {
                        log_cb(line_buf, user_data);
                    }
                    line_len = 0;
                } else {
                    if (line_len < sizeof(line_buf) - 1) {
                        line_buf[line_len++] = ch;
                    }
                }
            }
        }
    }
    if (log_cb && line_len > 0) {
        line_buf[line_len] = '\0';
        log_cb(line_buf, user_data);
    }

    CloseHandle(hReadPipe);

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code = 0;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    res.exit_code = (int)exit_code;
    res.success = (exit_code == 0);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

#else
    char full_cmd[8192];
    if (dir && *dir) {
        snprintf(full_cmd, sizeof(full_cmd), "cd \"%s\" && %s 2>&1", dir, command);
    } else {
        snprintf(full_cmd, sizeof(full_cmd), "%s 2>&1", command);
    }
    FILE *fp = popen(full_cmd, "r");
    if (!fp) {
        snprintf(res.output, sizeof(res.output), "Failed to spawn process.");
        return res;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t len = strlen(buffer);
        if (offset + len < sizeof(res.output) - 1) {
            memcpy(res.output + offset, buffer, len);
            offset += len;
            res.output[offset] = '\0';
        }
        if (log_cb) {
            char trimmed[512];
            snprintf(trimmed, sizeof(trimmed), "%s", buffer);
            size_t tlen = strlen(trimmed);
            while (tlen > 0 && (trimmed[tlen - 1] == '\n' || trimmed[tlen - 1] == '\r')) {
                trimmed[--tlen] = '\0';
            }
            if (tlen > 0) log_cb(trimmed, user_data);
        }
    }

    int status = pclose(fp);
    if (WIFEXITED(status)) {
        res.exit_code = WEXITSTATUS(status);
    } else {
        res.exit_code = status;
    }
    res.success = (res.exit_code == 0);
#endif

    while (offset > 0 && (res.output[offset - 1] == '\n' || res.output[offset - 1] == '\r')) {
        res.output[--offset] = '\0';
    }

    return res;
}

ProcessResult process_run(const char *command) {
    return process_run_ex(NULL, command, NULL, NULL);
}

ProcessResult process_run_in_dir(const char *dir, const char *command) {
    return process_run_ex(dir, command, NULL, NULL);
}

int process_run_interactive(const char *command) {
    if (!command || !*command) {
        return -1;
    }
#ifdef _WIN32
    char cmd[512];
    if (strcmp(command, "gh auth login") == 0) {
        snprintf(cmd, sizeof(cmd), "gh auth login --web");
    } else {
        snprintf(cmd, sizeof(cmd), "%s", command);
    }
    ProcessResult res = process_run_ex(NULL, cmd, NULL, NULL);
    return res.exit_code;
#else
    return system(command);
#endif
}

bool sys_open_browser(const char *url) {
    if (!url || !*url) {
        return false;
    }

#ifdef _WIN32
    // Windows: ShellExecute knows what browser the user adores
    HINSTANCE hInst = ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
    return ((INT_PTR)hInst > 32);
#elif defined(__APPLE__)
    // macOS: the charming 'open' utility
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "open \"%s\"", url);
    return (system(cmd) == 0);
#else
    // Linux: xdg-open to the rescue!
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "xdg-open \"%s\" > /dev/null 2>&1 &", url);
    return (system(cmd) == 0);
#endif
}
