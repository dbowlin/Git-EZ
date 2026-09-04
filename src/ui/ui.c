/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  🎨 git-ez : ui.c                                           │
 *  │  "Making terminals look like cozy retro spaceship consoles" │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#include "ui.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

void ui_print_banner(void) {
    printf("\n");
    printf(ANSI_CYAN ANSI_BOLD "  ╔═══════════════════════════════════════════════════════════════╗\n");
    printf("  ║" ANSI_BRIGHT_CYAN "   🚀 GIT-EZ  " ANSI_RESET ANSI_DIM "::" ANSI_RESET ANSI_WHITE " The Cozy Git & Private GitHub Repo Launcher   " ANSI_CYAN ANSI_BOLD "║\n");
    printf("  ╚═══════════════════════════════════════════════════════════════╝" ANSI_RESET "\n\n");
}

void ui_print_section(const char *title) {
    printf("\n" ANSI_BOLD ANSI_MAGENTA "▶ " ANSI_BRIGHT_CYAN "%s" ANSI_RESET "\n", title);
    printf(ANSI_DIM "  ─────────────────────────────────────────────────────────────" ANSI_RESET "\n");
}

void ui_success(const char *fmt, ...) {
    printf("  " ANSI_BRIGHT_GREEN "✔ " ANSI_RESET);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

void ui_error(const char *fmt, ...) {
    printf("  " ANSI_RED ANSI_BOLD "✖ " ANSI_RESET ANSI_RED);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf(ANSI_RESET "\n");
}

void ui_info(const char *fmt, ...) {
    printf("  " ANSI_CYAN "ℹ " ANSI_RESET);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

void ui_warn(const char *fmt, ...) {
    printf("  " ANSI_BRIGHT_YELLOW "⚠ " ANSI_RESET ANSI_YELLOW);
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf(ANSI_RESET "\n");
}

void ui_step(int step_num, int total_steps, const char *step_name) {
    printf("\n" ANSI_BOLD ANSI_BLUE "[%d/%d] " ANSI_BRIGHT_CYAN "%s" ANSI_RESET "\n", step_num, total_steps, step_name);
}

void ui_print_diagnostics(const SystemDiagnostics *diag) {
    ui_print_section("Preflight Environment Diagnostics");
    
    if (diag->git_installed) {
        ui_success("Git: %s (looking sharp!)", diag->git_version);
    } else {
        ui_error("Git is NOT installed. Grab it here: https://git-scm.com");
    }

    if (diag->gh_installed) {
        ui_success("GitHub CLI: %s (ready for action)", diag->gh_version);
    } else {
        ui_error("GitHub CLI (gh) is missing. Install via: winget install GitHub.cli (Win) or package manager (Linux)");
    }

    if (diag->gh_authenticated) {
        ui_success("GitHub Auth: Signed in as @" ANSI_BOLD "%s" ANSI_RESET " ✨", diag->gh_username);
    } else {
        ui_warn("GitHub Auth: Not signed in yet. (We can help you log in with 'gh auth login'!)");
    }
}

void ui_print_summary_card(const RepoConfig *config, const char *gh_username) {
    char size_str[64];
    if (config->total_bytes < 1024) {
        snprintf(size_str, sizeof(size_str), "%zu B", config->total_bytes);
    } else if (config->total_bytes < 1024 * 1024) {
        snprintf(size_str, sizeof(size_str), "%.1f KB", (double)config->total_bytes / 1024.0);
    } else {
        snprintf(size_str, sizeof(size_str), "%.1f MB", (double)config->total_bytes / (1024.0 * 1024.0));
    }

    const char *vis_str = (config->visibility == VISIBILITY_PRIVATE) 
        ? ANSI_BRIGHT_YELLOW ANSI_BOLD "🔒 PRIVATE (Safe & Secret)" ANSI_RESET 
        : ANSI_BRIGHT_GREEN ANSI_BOLD "🌐 PUBLIC (Open to All)" ANSI_RESET;

    printf("\n");
    printf("  " ANSI_CYAN "┌─" ANSI_BOLD " The Master Plan " ANSI_RESET ANSI_CYAN "────────────────────────────────────────┐" ANSI_RESET "\n");
    printf("  " ANSI_CYAN "│" ANSI_RESET "  Project Path    : " ANSI_BOLD "%-44s" ANSI_RESET ANSI_CYAN "│" ANSI_RESET "\n", config->target_path);
    printf("  " ANSI_CYAN "│" ANSI_RESET "  Repo Name       : " ANSI_BOLD "%-44s" ANSI_RESET ANSI_CYAN "│" ANSI_RESET "\n", config->repo_name);
    if (gh_username && *gh_username) {
        char full_target[MAX_NAME_LEN * 2];
        snprintf(full_target, sizeof(full_target), "%s/%s", gh_username, config->repo_name);
        printf("  " ANSI_CYAN "│" ANSI_RESET "  GitHub Remote   : " ANSI_CYAN "%-44s" ANSI_RESET ANSI_CYAN "│" ANSI_RESET "\n", full_target);
    }
    printf("  " ANSI_CYAN "│" ANSI_RESET "  Visibility      : %-64s" ANSI_CYAN "│" ANSI_RESET "\n", vis_str);
    printf("  " ANSI_CYAN "│" ANSI_RESET "  Main Branch     : %-44s" ANSI_CYAN "│" ANSI_RESET "\n", config->initial_branch);
    
    char staging_info[128];
    snprintf(staging_info, sizeof(staging_info), "%zu files (~%s)", config->file_count, size_str);
    printf("  " ANSI_CYAN "│" ANSI_RESET "  Files to Pack   : %-44s" ANSI_CYAN "│" ANSI_RESET "\n", staging_info);

    if (config->generate_gitignore) {
        char gi_info[128];
        snprintf(gi_info, sizeof(gi_info), "Yes (%s recipe)", config->gitignore_template);
        printf("  " ANSI_CYAN "│" ANSI_RESET "  Bake .gitignore : %-44s" ANSI_CYAN "│" ANSI_RESET "\n", gi_info);
    }

    printf("  " ANSI_CYAN "│" ANSI_RESET "  Commit Message  : \"" ANSI_DIM "%-42s" ANSI_RESET "\" " ANSI_CYAN "│" ANSI_RESET "\n", config->commit_message);
    printf("  " ANSI_CYAN "└─────────────────────────────────────────────────────────────┘" ANSI_RESET "\n\n");
}

void ui_print_success_card(const char *repo_url, const char *visibility_str) {
    printf("\n");
    printf("  " ANSI_BRIGHT_GREEN "╔═════════════════════════════════════════════════════════════╗\n");
    printf("  ║  🎉 " ANSI_BOLD "MISSION ACCOMPLISHED! CODE IS LIVE ON GITHUB!" ANSI_RESET ANSI_BRIGHT_GREEN "        ║\n");
    printf("  ╠═════════════════════════════════════════════════════════════╣\n");
    printf("  ║" ANSI_RESET "  Repository: " ANSI_BOLD "%-47s" ANSI_RESET ANSI_BRIGHT_GREEN "║\n", repo_url);
    printf("  ║" ANSI_RESET "  Visibility: %-47s" ANSI_BRIGHT_GREEN "║\n", visibility_str);
    printf("  ║" ANSI_RESET "  Status    : " ANSI_BRIGHT_GREEN "🔒 Pushed, locked, and secured" ANSI_RESET "               " ANSI_BRIGHT_GREEN "║\n");
    printf("  ╚═════════════════════════════════════════════════════════════╝" ANSI_RESET "\n\n");
}

void ui_spinner_start(const char *message) {
    printf("  " ANSI_CYAN "⏳ %s..." ANSI_RESET "\r", message);
    fflush(stdout);
}

void ui_spinner_stop(bool success, const char *final_message) {
    if (success) {
        printf("  " ANSI_BRIGHT_GREEN "✔ %s" ANSI_RESET "                          \n", final_message);
    } else {
        printf("  " ANSI_RED "✖ %s" ANSI_RESET "                          \n", final_message);
    }
    fflush(stdout);
}
