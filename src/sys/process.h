/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  ✨ git-ez : process.h                                      │
 *  │  "The Command Summoner & Terminal Whisperer"                │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <stdbool.h>
#include <stddef.h>

// What came back from the operating system depths?
typedef struct {
    int exit_code;       // 0 means happiness! Anything else is a learning opportunity.
    char output[4096];   // The chatter from stdout and stderr bundled up
    bool success;        // True if the command exited with 0
} ProcessResult;

/**
 * Callback for receiving real-time output lines from child processes.
 */
typedef void (*ProcessLogCallback)(const char *line, void *user_data);

/**
 * Run a command silently with zero console windows.
 * - dir: working directory (or NULL for current directory)
 * - command: command string to execute
 * - log_cb: optional callback invoked for each line of output
 * - user_data: contextual pointer passed to log_cb
 */
ProcessResult process_run_ex(const char *dir, const char *command, ProcessLogCallback log_cb, void *user_data);

/**
 * Politely ask the OS to run a command and capture what it mutters back.
 */
ProcessResult process_run(const char *command);

/**
 * Same as process_run, but we first wander into the specified directory.
 */
ProcessResult process_run_in_dir(const char *dir, const char *command);

/**
 * Hand the steering wheel over to an interactive command (e.g. gh auth login).
 */
int process_run_interactive(const char *command);

/**
 * Summon the user's default web browser to view their shiny new repository.
 */
bool sys_open_browser(const char *url);

/**
 * Cast a spell on the terminal so UTF-8 characters and vibrant ANSI colors work!
 */
void sys_init_console(void);

#endif // PROCESS_H
