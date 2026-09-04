/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  💬 git-ez : prompt.c                                       │
 *  │  "Listening patiently to the keyboard clatter"              │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#include "prompt.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void prompt_text(const char *label, const char *default_val, char *buffer, size_t max_len) {
    if (!buffer || max_len == 0) return;

    char input[1024];
    while (1) {
        if (default_val && *default_val) {
            printf("  " ANSI_BOLD "%s" ANSI_RESET " [" ANSI_CYAN "%s" ANSI_RESET "]: ", label, default_val);
        } else {
            printf("  " ANSI_BOLD "%s" ANSI_RESET ": ", label);
        }
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            // End-of-file or stream closed; fall back gracefully
            if (default_val) snprintf(buffer, max_len, "%s", default_val);
            else buffer[0] = '\0';
            return;
        }

        // Chop off newline characters
        size_t len = strlen(input);
        while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r')) {
            input[--len] = '\0';
        }

        // Did user just smash Enter? Use the default!
        if (len == 0) {
            if (default_val && *default_val) {
                snprintf(buffer, max_len, "%s", default_val);
                return;
            }
            // If empty and no default exists, politely ask again
            continue;
        }

        // Strip surrounding quotes in case the user dragged-and-dropped a folder with quotes
        char *start = input;
        while (isspace((unsigned char)*start)) start++;
        if (*start == '"' || *start == '\'') {
            start++;
            char *end = start + strlen(start) - 1;
            while (end > start && (*end == '"' || *end == '\'' || isspace((unsigned char)*end))) {
                *end = '\0';
                end--;
            }
        }

        snprintf(buffer, max_len, "%s", start);
        return;
    }
}

bool prompt_confirm(const char *label, bool default_yes) {
    char input[64];
    const char *hint = default_yes ? "(Y/n)" : "(y/N)";

    while (1) {
        printf("  " ANSI_BOLD "%s" ANSI_RESET " " ANSI_DIM "%s" ANSI_RESET ": ", label, hint);
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            return default_yes;
        }

        // Trim
        size_t len = strlen(input);
        while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r')) {
            input[--len] = '\0';
        }

        if (len == 0) {
            return default_yes;
        }

        char c = (char)tolower((unsigned char)input[0]);
        if (c == 'y') return true;
        if (c == 'n') return false;
        
        printf("  " ANSI_DIM "(Just type 'y' for yes or 'n' for no!)" ANSI_RESET "\n");
    }
}

size_t prompt_choice(const char *label, const char *options[], size_t option_count, size_t default_idx) {
    if (option_count == 0) return 0;
    if (default_idx >= option_count) default_idx = 0;

    printf("\n  " ANSI_BOLD "%s" ANSI_RESET "\n", label);
    for (size_t i = 0; i < option_count; ++i) {
        if (i == default_idx) {
            printf("    " ANSI_BRIGHT_CYAN ANSI_BOLD "[%zu] %s (Recommended)" ANSI_RESET "\n", i + 1, options[i]);
        } else {
            printf("    " ANSI_DIM "[%zu] %s" ANSI_RESET "\n", i + 1, options[i]);
        }
    }

    char input[64];
    while (1) {
        printf("  Pick a number [1-%zu, Enter for %zu]: ", option_count, default_idx + 1);
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            return default_idx;
        }

        size_t len = strlen(input);
        while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r')) {
            input[--len] = '\0';
        }

        if (len == 0) {
            return default_idx;
        }

        int val = atoi(input);
        if (val >= 1 && (size_t)val <= option_count) {
            return (size_t)(val - 1);
        }
        printf("  " ANSI_RED "Whoops! Please enter a number between 1 and %zu." ANSI_RESET "\n", option_count);
    }
}

void prompt_wait_key(const char *message) {
    if (message && *message) {
        printf("  %s", message);
    } else {
        printf("  " ANSI_DIM "Press Enter when ready..." ANSI_RESET);
    }
    fflush(stdout);

    char temp[64];
    if (fgets(temp, sizeof(temp), stdin) == NULL) {
        // Stream ended, moving along
    }
}
