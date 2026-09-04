/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  📁 git-ez : fs_util.h                                      │
 *  │  "Filesystem Detective & Path Scrubbing Wizardry"           │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#ifndef FS_UTIL_H
#define FS_UTIL_H

#include <stdbool.h>
#include <stddef.h>

// Does this folder actually exist, or is it a figment of our imagination?
bool fs_dir_exists(const char *path);

// Does this specific file exist?
bool fs_file_exists(const char *path);

// "Where in the world are we right now?" (Gets cwd)
bool fs_get_current_dir(char *buffer, size_t max_len);

// Clean up and normalize path: strip quotes/whitespace, resolve absolute path, strip trailing slashes
bool fs_normalize_path(const char *input, char *output, size_t max_len);

// Turn messy folder names with spaces and weird symbols into shiny GitHub-friendly slugs!
void fs_sanitize_repo_name(const char *input, char *output, size_t max_len);

// Snip out just the final folder name from a long sprawling path
void fs_extract_folder_name(const char *path, char *output, size_t max_len);

// Is there a .git folder hiding here?
bool fs_is_git_repo(const char *path);

// Turn boring raw byte counts into friendly "42.5 KB" or "1.2 MB" strings
void fs_format_size(size_t bytes, char *buffer, size_t max_len);

// Sherlock Holmes mode: count files, weigh folder, and sniff out the programming language!
void fs_scan_project(const char *path, size_t *out_file_count, size_t *out_bytes, char *detected_type, size_t type_max);

#endif // FS_UTIL_H
