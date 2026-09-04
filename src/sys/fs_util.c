/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  📁 git-ez : fs_util.c                                      │
 *  │  "Poking around folders, inspecting files, sniffing code"   │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#include "fs_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#include <dirent.h>
#endif

bool fs_dir_exists(const char *path) {
    if (!path || !*path) return false;

#ifdef _WIN32
    // Windows API: check if attributes exist and it's a genuine folder
    DWORD dwAttrib = GetFileAttributesA(path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
           (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    // POSIX stat: simple, classic, dependable
    struct stat st;
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    return false;
#endif
}

bool fs_file_exists(const char *path) {
    if (!path || !*path) return false;
    struct stat st;
    return (stat(path, &st) == 0 && !S_ISDIR(st.st_mode));
}

bool fs_get_current_dir(char *buffer, size_t max_len) {
    if (!buffer || max_len == 0) return false;
    return (getcwd(buffer, (int)max_len) != NULL);
}

bool fs_normalize_path(const char *input, char *output, size_t max_len) {
    if (!input || !output || max_len == 0) return false;

    // 1. Skip leading whitespace
    const char *start = input;
    while (*start && isspace((unsigned char)*start)) start++;

    // 2. Trim quotes if wrapped
    char temp[1024];
    if (*start == '"' || *start == '\'') {
        char quote = *start++;
        size_t len = strlen(start);
        if (len >= sizeof(temp)) len = sizeof(temp) - 1;
        memcpy(temp, start, len);
        temp[len] = '\0';
        while (len > 0 && (temp[len - 1] == quote || isspace((unsigned char)temp[len - 1]))) {
            temp[--len] = '\0';
        }
        start = temp;
    }

    if (*start == '\0') return false;

    // 3. Resolve canonical absolute path
#ifdef _WIN32
    char full_path[1024];
    DWORD len = GetFullPathNameA(start, (DWORD)sizeof(full_path), full_path, NULL);
    if (len == 0 || len >= sizeof(full_path)) {
        snprintf(output, max_len, "%s", start);
    } else {
        // Strip trailing slash unless it's a root drive like "C:\"
        while (len > 3 && (full_path[len - 1] == '\\' || full_path[len - 1] == '/')) {
            full_path[--len] = '\0';
        }
        snprintf(output, max_len, "%s", full_path);
    }
#else
    char full_path[1024];
    if (realpath(start, full_path) != NULL) {
        size_t len = strlen(full_path);
        while (len > 1 && full_path[len - 1] == '/') {
            full_path[--len] = '\0';
        }
        snprintf(output, max_len, "%s", full_path);
    } else {
        snprintf(output, max_len, "%s", start);
    }
#endif

    return true;
}

void fs_extract_folder_name(const char *path, char *output, size_t max_len) {
    if (!path || !output || max_len == 0) return;
    output[0] = '\0';

    size_t len = strlen(path);
    if (len == 0) return;

    // First: shave off any pesky trailing slashes or backslashes
    char temp[1024];
    snprintf(temp, sizeof(temp), "%s", path);
    len = strlen(temp);
    while (len > 0 && (temp[len - 1] == '/' || temp[len - 1] == '\\')) {
        temp[--len] = '\0';
    }

    // Check if it's a drive root like "C:"
    if (len == 2 && isalpha((unsigned char)temp[0]) && temp[1] == ':') {
        snprintf(output, max_len, "drive-%c", (char)tolower((unsigned char)temp[0]));
        return;
    }

    // Find the rightmost slash to isolate the base folder
    const char *last_slash = strrchr(temp, '/');
    const char *last_bslash = strrchr(temp, '\\');
    const char *name_start = temp;

    if (last_slash && last_slash > name_start) name_start = last_slash + 1;
    if (last_bslash && last_bslash > name_start) name_start = last_bslash + 1;

    snprintf(output, max_len, "%s", name_start);
}

void fs_sanitize_repo_name(const char *input, char *output, size_t max_len) {
    if (!input || !output || max_len == 0) return;

    // GitHub repo names like lowercase letters, numbers, hyphens, and dots.
    // Spaces become cute hyphens!
    size_t out_idx = 0;
    bool last_was_dash = false;

    for (size_t i = 0; input[i] != '\0' && out_idx < max_len - 1; ++i) {
        char c = input[i];
        if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.') {
            output[out_idx++] = (char)tolower((unsigned char)c);
            last_was_dash = (c == '-');
        } else if (isspace((unsigned char)c) || c == '/') {
            if (!last_was_dash && out_idx > 0) {
                output[out_idx++] = '-';
                last_was_dash = true;
            }
        }
    }

    // Clean off dangling punctuation at the end
    while (out_idx > 0 && (output[out_idx - 1] == '-' || output[out_idx - 1] == '.')) {
        out_idx--;
    }

    output[out_idx] = '\0';

    // If everything was stripped away, give it a friendly fallback name
    if (out_idx == 0) {
        snprintf(output, max_len, "my-repo");
    }
}

bool fs_is_git_repo(const char *path) {
    if (!path || !*path) return false;
    char git_path[1024];
    size_t plen = strlen(path);
    if (plen > 0 && (path[plen - 1] == '\\' || path[plen - 1] == '/')) {
        snprintf(git_path, sizeof(git_path), "%s.git", path);
    } else {
        snprintf(git_path, sizeof(git_path), "%s\\.git", path);
    }
    return fs_dir_exists(git_path);
}

void fs_format_size(size_t bytes, char *buffer, size_t max_len) {
    if (!buffer || max_len == 0) return;
    if (bytes < 1024) {
        snprintf(buffer, max_len, "%zu B", bytes);
    } else if (bytes < 1024 * 1024) {
        snprintf(buffer, max_len, "%.1f KB", (double)bytes / 1024.0);
    } else if (bytes < 1024 * 1024 * 1024) {
        snprintf(buffer, max_len, "%.1f MB", (double)bytes / (1024.0 * 1024.0));
    } else {
        snprintf(buffer, max_len, "%.2f GB", (double)bytes / (1024.0 * 1024.0 * 1024.0));
    }
}

void fs_scan_project(const char *path, size_t *out_file_count, size_t *out_bytes, char *detected_type, size_t type_max) {
    if (out_file_count) *out_file_count = 0;
    if (out_bytes) *out_bytes = 0;
    if (detected_type && type_max > 0) snprintf(detected_type, type_max, "General");

    if (!path || !*path) return;

    bool has_c = false;
    bool has_py = false;
    bool has_js = false;
    bool has_rust = false;
    bool has_go = false;

#ifdef _WIN32
    // Windows directory traversing with FindFirstFile
    char search_pattern[1024];
    size_t plen = strlen(path);
    if (plen > 0 && (path[plen - 1] == '\\' || path[plen - 1] == '/')) {
        snprintf(search_pattern, sizeof(search_pattern), "%s*.*", path);
    } else {
        snprintf(search_pattern, sizeof(search_pattern), "%s\\*.*", path);
    }
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search_pattern, &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0 || strcmp(fd.cFileName, ".git") == 0) {
                continue;
            }
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                if (out_file_count) (*out_file_count)++;
                ULARGE_INTEGER fileSize;
                fileSize.LowPart = fd.nFileSizeLow;
                fileSize.HighPart = fd.nFileSizeHigh;
                if (out_bytes) *out_bytes += (size_t)fileSize.QuadPart;

                const char *ext = strrchr(fd.cFileName, '.');
                if (ext) {
                    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0 || strcmp(ext, ".cpp") == 0) has_c = true;
                    else if (strcmp(ext, ".py") == 0) has_py = true;
                    else if (strcmp(ext, ".js") == 0 || strcmp(ext, ".ts") == 0 || strcmp(ext, ".json") == 0) has_js = true;
                    else if (strcmp(ext, ".rs") == 0) has_rust = true;
                    else if (strcmp(ext, ".go") == 0) has_go = true;
                }
            }
        } while (FindNextFileA(hFind, &fd));
        FindClose(hFind);
    }
#else
    // POSIX opendir / readdir loop
    DIR *d = opendir(path);
    if (d) {
        struct dirent *dir;
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0 || strcmp(dir->d_name, ".git") == 0) {
                continue;
            }
            char full_file[1024];
            snprintf(full_file, sizeof(full_file), "%s/%s", path, dir->d_name);
            struct stat st;
            if (stat(full_file, &st) == 0 && !S_ISDIR(st.st_mode)) {
                if (out_file_count) (*out_file_count)++;
                if (out_bytes) *out_bytes += (size_t)st.st_size;

                const char *ext = strrchr(dir->d_name, '.');
                if (ext) {
                    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".h") == 0 || strcmp(ext, ".cpp") == 0) has_c = true;
                    else if (strcmp(ext, ".py") == 0) has_py = true;
                    else if (strcmp(ext, ".js") == 0 || strcmp(ext, ".ts") == 0 || strcmp(ext, ".json") == 0) has_js = true;
                    else if (strcmp(ext, ".rs") == 0) has_rust = true;
                    else if (strcmp(ext, ".go") == 0) has_go = true;
                }
            }
        }
        closedir(d);
    }
#endif

    // Make an educated guess about the project's primary language
    if (detected_type && type_max > 0) {
        if (has_c) snprintf(detected_type, type_max, "C/C++");
        else if (has_py) snprintf(detected_type, type_max, "Python");
        else if (has_js) snprintf(detected_type, type_max, "Node/Web");
        else if (has_rust) snprintf(detected_type, type_max, "Rust");
        else if (has_go) snprintf(detected_type, type_max, "Go");
        else snprintf(detected_type, type_max, "General");
    }
}
