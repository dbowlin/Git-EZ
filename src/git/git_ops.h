/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  🌿 git-ez : git_ops.h                                      │
 *  │  "The Local Git Sorcery & Time Machine Wrangler"            │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#ifndef GIT_OPS_H
#define GIT_OPS_H

#include <stdbool.h>
#include <stddef.h>

// Check if Git is awake and report its version string
bool git_check_installed(char *version_buf, size_t max_len);

// Breathe life into a new repo with 'git init' and set default branch (e.g. main)
bool git_init_repo(const char *dir, const char *branch_name, char *error_buf, size_t err_max);

// Gather all your wonderful files into the staging area (git add -A)
bool git_stage_all(const char *dir, char *error_buf, size_t err_max);

// Freeze time and create a commit snapshot with a descriptive message
bool git_commit(const char *dir, const char *message, char *error_buf, size_t err_max);

// Push your commits upstream to the cloud!
bool git_push(const char *dir, char *error_buf, size_t err_max);

// Has this repo made any commit snapshots yet?
bool git_has_commits(const char *dir);

// Find out which branch we are currently standing on
bool git_get_current_branch(const char *dir, char *branch_buf, size_t max_len);

// Tally up staged, untracked, and modified files
void git_get_status_summary(const char *dir, size_t *staged, size_t *untracked, size_t *modified);

#endif // GIT_OPS_H
