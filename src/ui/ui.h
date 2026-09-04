/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  🎨 git-ez : ui.h                                           │
 *  │  "The Painter's Palette : ANSI Rainbows & Box Art"          │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <stddef.h>
#include "../app_state.h"

// ANSI Color and Style Codes (our trusty box of crayons)
#define ANSI_RESET      "\x1b[0m"
#define ANSI_BOLD       "\x1b[1m"
#define ANSI_DIM        "\x1b[2m"
#define ANSI_UNDERLINE  "\x1b[4m"

#define ANSI_BLACK      "\x1b[30m"
#define ANSI_RED        "\x1b[31m"
#define ANSI_GREEN      "\x1b[32m"
#define ANSI_YELLOW     "\x1b[33m"
#define ANSI_BLUE       "\x1b[34m"
#define ANSI_MAGENTA    "\x1b[35m"
#define ANSI_CYAN       "\x1b[36m"
#define ANSI_WHITE      "\x1b[37m"

#define ANSI_BG_BLUE    "\x1b[44m"
#define ANSI_BG_MAGENTA "\x1b[45m"
#define ANSI_BG_DARK    "\x1b[48;5;235m"

#define ANSI_BRIGHT_CYAN    "\x1b[96m"
#define ANSI_BRIGHT_GREEN   "\x1b[92m"
#define ANSI_BRIGHT_YELLOW  "\x1b[93m"
#define ANSI_BRIGHT_MAGENTA "\x1b[95m"

// The visual wizardry functions
void ui_print_banner(void);
void ui_print_section(const char *title);
void ui_success(const char *fmt, ...);
void ui_error(const char *fmt, ...);
void ui_info(const char *fmt, ...);
void ui_warn(const char *fmt, ...);
void ui_step(int step_num, int total_steps, const char *step_name);

void ui_print_diagnostics(const SystemDiagnostics *diag);
void ui_print_summary_card(const RepoConfig *config, const char *gh_username);
void ui_print_success_card(const char *repo_url, const char *visibility_str);

void ui_spinner_start(const char *message);
void ui_spinner_stop(bool success, const char *final_message);

#endif // UI_H
