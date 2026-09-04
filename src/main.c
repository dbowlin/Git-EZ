/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  🚀 git-ez : main.c                                         │
 *  │  "The Grand Conductor of the Git Symphony"                  │
 *  │                                                             │
 *  │  Written with boundless enthusiasm, late-night tea, and     │
 *  │  the stubborn belief that Git shouldn't give you headaches. │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "app_state.h"
#include "sys/process.h"
#include "sys/fs_util.h"
#include "sys/clipboard.h"
#include "ui/win_gui.h"
#include "ui/ui.h"
#include "ui/prompt.h"
#include "git/git_ops.h"
#include "git/gitignore.h"
#include "gh/gh_ops.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <objbase.h>
#endif

static void print_help(const char *prog_name) {
    ui_print_banner();
    printf("HOW TO SUMMON THE MAGIC:\n");
    printf("  %s [OPTIONS]\n\n", prog_name);
    printf("OPTIONS & SPELLS:\n");
    printf("  -d, --dir <path>       Target project folder (defaults to right here!)\n");
    printf("  -n, --name <name>      GitHub repository name (defaults to folder name)\n");
    printf("  -m, --message <msg>    Initial commit message (default: \"Initial commit\")\n");
    printf("  -p, --private          Create a PRIVATE repo (Default - keep it secret!)\n");
    printf("      --public           Create a PUBLIC repo (Shout it to the world!)\n");
    printf("  -s, --sync             Quick sync & ship edits for an existing repo\n");
    printf("  -y, --yes              Express mode: accept all defaults, no questions asked\n");
    printf("      --no-browser       Don't automatically pop open the browser\n");
    printf("      --no-copy          Don't copy the shiny new URL to clipboard\n");
    printf("  -v, --version          Show version & build information\n");
    printf("  -h, --help             Show this friendly help scroll\n\n");
    printf("FAVORITE EXAMPLES:\n");
    printf("  %s                     # Cozy interactive wizard in the current folder\n", prog_name);
    printf("  %s -y                  # Instant express publish with smart defaults\n", prog_name);
    printf("  %s -d \"C:\\MyProject\"   # Initialize & publish a specific folder\n", prog_name);
    printf("  %s --sync              # Quick commit & push for daily edits\n\n", prog_name);
}

int main(int argc, char *argv[]) {
    // Parse incoming CLI flags for help/version
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            sys_init_console();
            print_help(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            sys_init_console();
            printf("%s version %s (GUI edition)\n", APP_NAME, APP_VERSION);
            return 0;
        }
    }

#ifdef _WIN32
    return WinMain(GetModuleHandle(NULL), NULL, "", SW_SHOWNORMAL);
#else
    sys_init_console();
    AppState state;
    memset(&state, 0, sizeof(state));
    state.mode = MODE_WIZARD;
    state.config.visibility = VISIBILITY_PRIVATE;
    snprintf(state.config.initial_branch, sizeof(state.config.initial_branch), "main");
    snprintf(state.config.commit_message, sizeof(state.config.commit_message), "Initial commit");
    state.config.auto_open_browser = true;
    state.config.auto_copy_url = true;
    state.config.generate_gitignore = true;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dir") == 0) {
            if (i + 1 < argc) snprintf(state.config.target_path, sizeof(state.config.target_path), "%s", argv[++i]);
        }
    }
    return 0;
#endif
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    AppState state;
    memset(&state, 0, sizeof(state));
    state.config.visibility = VISIBILITY_PRIVATE;
    snprintf(state.config.initial_branch, sizeof(state.config.initial_branch), "main");
    snprintf(state.config.commit_message, sizeof(state.config.commit_message), "Initial commit");
    state.config.auto_open_browser = true;
    state.config.auto_copy_url = true;
    state.config.generate_gitignore = true;

    int argc = 0;
    LPWSTR *argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argvW) {
        for (int i = 1; i < argc; ++i) {
            char arg[MAX_PATH_LEN];
            WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, arg, sizeof(arg), NULL, NULL);
            if (strcmp(arg, "-d") == 0 || strcmp(arg, "--dir") == 0) {
                if (i + 1 < argc) {
                    WideCharToMultiByte(CP_UTF8, 0, argvW[++i], -1, state.config.target_path, sizeof(state.config.target_path), NULL, NULL);
                }
            } else if (strcmp(arg, "-n") == 0 || strcmp(arg, "--name") == 0) {
                if (i + 1 < argc) {
                    WideCharToMultiByte(CP_UTF8, 0, argvW[++i], -1, state.config.repo_name, sizeof(state.config.repo_name), NULL, NULL);
                }
            }
        }
        LocalFree(argvW);
    }

    int ret = gui_run(&state, hInstance, nCmdShow);
    CoUninitialize();
    return ret;
}
#endif
