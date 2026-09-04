/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  🌿 git-ez : git_ops.c                                      │
 *  │  "Speaking fluent Git so you never have to remember flags"  │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#include "git_ops.h"
#include "../sys/process.h"
#include <stdio.h>
#include <string.h>

bool git_check_installed(char *version_buf, size_t max_len) {
    ProcessResult res = process_run("git --version");
    if (!res.success) return false;

    if (version_buf && max_len > 0) {
        snprintf(version_buf, max_len, "%s", res.output);
    }
    return true;
}

bool git_init_repo(const char *dir, const char *branch_name, char *error_buf, size_t err_max) {
    const char *branch = (branch_name && *branch_name) ? branch_name : "main";

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "git init -b %s", branch);
    ProcessResult res = process_run_in_dir(dir, cmd);
    if (!res.success) {
        // Fallback for older Git versions without -b flag
        res = process_run_in_dir(dir, "git init");
        if (!res.success) {
            if (error_buf && err_max > 0) snprintf(error_buf, err_max, "%s", res.output);
            return false;
        }
        snprintf(cmd, sizeof(cmd), "git branch -M %s", branch);
        process_run_in_dir(dir, cmd);
    }
    return true;
}

bool git_stage_all(const char *dir, char *error_buf, size_t err_max) {
    // Pack all the things! (git add -A handles additions, modifications, and deletions)
    ProcessResult res = process_run_in_dir(dir, "git add -A");
    if (!res.success) {
        if (error_buf && err_max > 0) snprintf(error_buf, err_max, "%s", res.output);
        return false;
    }
    return true;
}

bool git_commit(const char *dir, const char *message, char *error_buf, size_t err_max) {
    if (!message || !*message) message = "Initial commit";

    // Say cheese! Taking a permanent snapshot of the project
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "git commit -m \"%s\"", message);
    ProcessResult res = process_run_in_dir(dir, cmd);
    if (!res.success) {
        if (error_buf && err_max > 0) snprintf(error_buf, err_max, "%s", res.output);
        return false;
    }
    return true;
}

bool git_push(const char *dir, char *error_buf, size_t err_max) {
    // Sending bytes across the ether
    ProcessResult res = process_run_in_dir(dir, "git push");
    if (!res.success) {
        if (error_buf && err_max > 0) snprintf(error_buf, err_max, "%s", res.output);
        return false;
    }
    return true;
}

bool git_has_commits(const char *dir) {
    // Ask HEAD if anyone is home
    ProcessResult res = process_run_in_dir(dir, "git rev-parse --verify HEAD");
    return res.success;
}

bool git_get_current_branch(const char *dir, char *branch_buf, size_t max_len) {
    if (!branch_buf || max_len == 0) return false;
    ProcessResult res = process_run_in_dir(dir, "git branch --show-current");
    if (res.success && strlen(res.output) > 0) {
        snprintf(branch_buf, max_len, "%s", res.output);
        return true;
    }
    // Safe default fallback
    snprintf(branch_buf, max_len, "main");
    return false;
}

void git_get_status_summary(const char *dir, size_t *staged, size_t *untracked, size_t *modified) {
    if (staged) *staged = 0;
    if (untracked) *untracked = 0;
    if (modified) *modified = 0;

    // git status --porcelain is super easy for programs to parse!
    ProcessResult res = process_run_in_dir(dir, "git status --porcelain");
    if (!res.success) return;

    char *line = res.output;
    while (line && *line) {
        char *next = strchr(line, '\n');
        if (next) *next = '\0';

        if (strlen(line) >= 2) {
            char x = line[0];
            char y = line[1];

            if (x == '?' && y == '?') {
                if (untracked) (*untracked)++;
            } else {
                if (x == 'A' || x == 'M' || x == 'D' || x == 'R' || x == 'C') {
                    if (staged) (*staged)++;
                }
                if (y == 'M' || y == 'D') {
                    if (modified) (*modified)++;
                }
            }
        }

        line = next ? next + 1 : NULL;
    }
}
