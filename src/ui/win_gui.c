/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  🎨 git-ez : win_gui.c                                      │
 *  │  "The Cozy, Silent Native Win32 GUI for Git & GitHub"       │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0600

#include "win_gui.h"
#include "../sys/process.h"
#include "../sys/fs_util.h"
#include "../sys/clipboard.h"
#include "../git/git_ops.h"
#include "../git/gitignore.h"
#include "../gh/gh_ops.h"

#include <windows.h>
#include <commctrl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <dwmapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

// Control IDs
enum {
    ID_LBL_HEADER = 100,
    ID_LBL_DIAG_GIT,
    ID_LBL_DIAG_GH,
    ID_LBL_DIAG_AUTH,
    ID_BTN_AUTH_LOGIN,
    
    ID_GRP_FOLDER,
    ID_LBL_FOLDER,
    ID_EDIT_FOLDER,
    ID_BTN_BROWSE,
    ID_BTN_EXPLORE,
    ID_LBL_FOLDER_BADGE,

    ID_GRP_CONFIG,
    ID_LBL_REPO_NAME,
    ID_EDIT_REPO_NAME,
    ID_LBL_VISIBILITY,
    ID_RADIO_PRIVATE,
    ID_RADIO_PUBLIC,
    ID_CHECK_GITIGNORE,
    ID_COMBO_GITIGNORE,
    ID_LBL_BRANCH,
    ID_EDIT_BRANCH,
    ID_LBL_COMMIT,
    ID_EDIT_COMMIT,

    ID_CHECK_BROWSER,
    ID_CHECK_COPY,

    ID_BTN_ACTION,
    ID_PROGRESS,
    ID_LBL_STATUS,

    ID_GRP_LOG,
    ID_EDIT_LOG,

    ID_BTN_OPEN_URL,
    ID_BTN_COPY_URL
};

// Custom messages from worker thread
#define WM_APP_LOG          (WM_APP + 1)
#define WM_APP_PROGRESS     (WM_APP + 2)
#define WM_APP_COMPLETE     (WM_APP + 3)
#define WM_APP_AUTH_REFRESH (WM_APP + 4)

typedef struct {
    HWND hWnd;
    HFONT hFontUI;
    HFONT hFontBold;
    HFONT hFontTitle;
    HFONT hFontLog;
    HBRUSH hBrushBg;
    HBRUSH hBrushCard;

    HWND hEditFolder;
    HWND hBtnBrowse;
    HWND hBtnExplore;
    HWND hLblFolderBadge;

    HWND hEditRepoName;
    HWND hRadioPrivate;
    HWND hRadioPublic;
    HWND hCheckGitignore;
    HWND hComboGitignore;
    HWND hEditBranch;
    HWND hEditCommit;

    HWND hCheckBrowser;
    HWND hCheckCopy;

    HWND hBtnAction;
    HWND hProgress;
    HWND hLblStatus;

    HWND hEditLog;
    HWND hBtnOpenUrl;
    HWND hBtnCopyUrl;

    HWND hLblDiagGit;
    HWND hLblDiagGh;
    HWND hLblDiagAuth;
    HWND hBtnAuthLogin;

    AppState state;
    bool is_busy;
    bool sync_mode;
    char last_inspected_path[MAX_PATH_LEN];
} GuiContext;

static GuiContext g_ctx;

static void append_log(const char *text) {
    if (!g_ctx.hEditLog || !text) return;
    int len = GetWindowTextLengthA(g_ctx.hEditLog);
    SendMessageA(g_ctx.hEditLog, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessageA(g_ctx.hEditLog, EM_REPLACESEL, FALSE, (LPARAM)text);
    SendMessageA(g_ctx.hEditLog, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
    SendMessageA(g_ctx.hEditLog, EM_SCROLLCARET, 0, 0);
}

static void log_callback(const char *line, void *user_data) {
    HWND hWnd = (HWND)user_data;
    if (hWnd && line && *line) {
        char *copy = _strdup(line);
        if (copy) {
            PostMessageA(hWnd, WM_APP_LOG, 0, (LPARAM)copy);
        }
    }
}

static void refresh_diagnostics(void) {
    g_ctx.state.diag.git_installed = git_check_installed(g_ctx.state.diag.git_version, sizeof(g_ctx.state.diag.git_version));
    g_ctx.state.diag.gh_installed = gh_check_installed(g_ctx.state.diag.gh_version, sizeof(g_ctx.state.diag.gh_version));
    if (g_ctx.state.diag.gh_installed) {
        g_ctx.state.diag.gh_authenticated = gh_check_auth(g_ctx.state.diag.gh_username, sizeof(g_ctx.state.diag.gh_username));
    } else {
        g_ctx.state.diag.gh_authenticated = false;
        g_ctx.state.diag.gh_username[0] = '\0';
    }

    char buf[256];
    if (g_ctx.state.diag.git_installed) {
        snprintf(buf, sizeof(buf), "Git: OK (%s)", g_ctx.state.diag.git_version);
    } else {
        snprintf(buf, sizeof(buf), "Git: NOT installed!");
    }
    SetWindowTextA(g_ctx.hLblDiagGit, buf);

    if (g_ctx.state.diag.gh_installed) {
        snprintf(buf, sizeof(buf), "GitHub CLI: OK (%s)", g_ctx.state.diag.gh_version);
    } else {
        snprintf(buf, sizeof(buf), "GitHub CLI: NOT installed!");
    }
    SetWindowTextA(g_ctx.hLblDiagGh, buf);

    if (g_ctx.state.diag.gh_authenticated) {
        snprintf(buf, sizeof(buf), "Signed in as @%s", g_ctx.state.diag.gh_username);
        ShowWindow(g_ctx.hBtnAuthLogin, SW_HIDE);
    } else {
        snprintf(buf, sizeof(buf), "GitHub: Not signed in");
        if (g_ctx.state.diag.gh_installed) {
            ShowWindow(g_ctx.hBtnAuthLogin, SW_SHOW);
        }
    }
    SetWindowTextA(g_ctx.hLblDiagAuth, buf);
}

static void inspect_folder(const char *raw_path) {
    if (!raw_path || !*raw_path) return;

    char normalized[MAX_PATH_LEN];
    if (!fs_normalize_path(raw_path, normalized, sizeof(normalized))) {
        SetWindowTextA(g_ctx.hLblFolderBadge, "Invalid folder path.");
        return;
    }

    if (strcmp(normalized, g_ctx.last_inspected_path) == 0) {
        return;
    }
    snprintf(g_ctx.last_inspected_path, sizeof(g_ctx.last_inspected_path), "%s", normalized);
    snprintf(g_ctx.state.config.target_path, sizeof(g_ctx.state.config.target_path), "%s", normalized);

    if (!fs_dir_exists(normalized)) {
        SetWindowTextA(g_ctx.hLblFolderBadge, "Folder does not exist yet.");
        SetWindowTextA(g_ctx.hBtnAction, "Create Folder & Publish");
        return;
    }

    bool is_repo = fs_is_git_repo(normalized);
    g_ctx.sync_mode = is_repo;

    char badge[512];
    if (is_repo) {
        char branch[64] = "main";
        git_get_current_branch(normalized, branch, sizeof(branch));
        size_t staged = 0, untracked = 0, modified = 0;
        git_get_status_summary(normalized, &staged, &untracked, &modified);

        snprintf(badge, sizeof(badge),
            "Existing Git repo (branch: %s)  *  Changes: %zu staged, %zu modified, %zu untracked",
            branch, staged, modified, untracked);
        SetWindowTextA(g_ctx.hLblFolderBadge, badge);

        SetWindowTextA(g_ctx.hBtnAction, "Sync & Push to GitHub");
        SetWindowTextA(g_ctx.hEditCommit, "Update project files");

        // Keep repo settings disabled or optional in sync mode
        EnableWindow(g_ctx.hEditRepoName, FALSE);
        EnableWindow(g_ctx.hRadioPrivate, FALSE);
        EnableWindow(g_ctx.hRadioPublic, FALSE);
        EnableWindow(g_ctx.hCheckGitignore, FALSE);
        EnableWindow(g_ctx.hComboGitignore, FALSE);
    } else {
        size_t file_count = 0;
        size_t total_bytes = 0;
        char detected[64] = "General";
        fs_scan_project(normalized, &file_count, &total_bytes, detected, sizeof(detected));

        char size_buf[64];
        fs_format_size(total_bytes, size_buf, sizeof(size_buf));

        snprintf(badge, sizeof(badge),
            "New project: %zu files (%s)  *  Detected Tech: %s",
            file_count, size_buf, detected);
        SetWindowTextA(g_ctx.hLblFolderBadge, badge);

        char folder_name[MAX_NAME_LEN];
        char repo_slug[MAX_NAME_LEN];
        fs_extract_folder_name(normalized, folder_name, sizeof(folder_name));
        fs_sanitize_repo_name(folder_name, repo_slug, sizeof(repo_slug));

        SetWindowTextA(g_ctx.hEditRepoName, repo_slug);
        SetWindowTextA(g_ctx.hBtnAction, "Publish to GitHub");
        SetWindowTextA(g_ctx.hEditCommit, "Initial commit");

        EnableWindow(g_ctx.hEditRepoName, TRUE);
        EnableWindow(g_ctx.hRadioPrivate, TRUE);
        EnableWindow(g_ctx.hRadioPublic, TRUE);
        EnableWindow(g_ctx.hCheckGitignore, TRUE);
        EnableWindow(g_ctx.hComboGitignore, TRUE);

        // Select matching template in combobox
        int idx = (int)SendMessageA(g_ctx.hComboGitignore, CB_FINDSTRINGEXACT, -1, (LPARAM)detected);
        if (idx != CB_ERR) {
            SendMessageA(g_ctx.hComboGitignore, CB_SETCURSEL, (WPARAM)idx, 0);
        }

        // Check if .gitignore already exists
        char gi_path[MAX_PATH_LEN];
        snprintf(gi_path, sizeof(gi_path), "%s/.gitignore", normalized);
        if (fs_file_exists(gi_path)) {
            SendMessageA(g_ctx.hCheckGitignore, BM_SETCHECK, BST_UNCHECKED, 0);
            EnableWindow(g_ctx.hCheckGitignore, FALSE);
            EnableWindow(g_ctx.hComboGitignore, FALSE);
        } else {
            SendMessageA(g_ctx.hCheckGitignore, BM_SETCHECK, BST_CHECKED, 0);
        }
    }
}

static void select_folder_dialog(HWND hWnd) {
    BROWSEINFOA bi;
    memset(&bi, 0, sizeof(bi));
    bi.hwndOwner = hWnd;
    bi.lpszTitle = "Select Project Folder for git-ez:";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;

    PIDLIST_ABSOLUTE pidl = SHBrowseForFolderA(&bi);
    if (pidl) {
        char selected_path[MAX_PATH_LEN];
        if (SHGetPathFromIDListA(pidl, selected_path)) {
            SetWindowTextA(g_ctx.hEditFolder, selected_path);
            inspect_folder(selected_path);
        }
        CoTaskMemFree(pidl);
    }
}

// Background Worker Thread Proc
typedef struct {
    HWND hWnd;
    AppState state;
    bool sync_mode;
} WorkerParams;

static DWORD WINAPI worker_thread_proc(LPVOID lpParam) {
    WorkerParams *wp = (WorkerParams *)lpParam;
    HWND hWnd = wp->hWnd;
    AppState state = wp->state;
    bool sync_mode = wp->sync_mode;
    free(wp);

    char err_buf[1024] = "";

    if (sync_mode) {
        PostMessageA(hWnd, WM_APP_PROGRESS, 20, (LPARAM)_strdup("Staging modified files..."));
        PostMessageA(hWnd, WM_APP_LOG, 0, (LPARAM)_strdup("[*] Staging all files (git add -A)..."));
        
        process_run_ex(state.config.target_path, "git add -A", log_callback, (void*)hWnd);

        PostMessageA(hWnd, WM_APP_PROGRESS, 50, (LPARAM)_strdup("Creating commit snapshot..."));
        PostMessageA(hWnd, WM_APP_LOG, 0, (LPARAM)_strdup("[*] Sealing commit snapshot..."));
        
        char commit_cmd[1024];
        snprintf(commit_cmd, sizeof(commit_cmd), "git commit -m \"%s\"", state.config.commit_message);
        process_run_ex(state.config.target_path, commit_cmd, log_callback, (void*)hWnd);

        PostMessageA(hWnd, WM_APP_PROGRESS, 80, (LPARAM)_strdup("Pushing to GitHub remote..."));
        PostMessageA(hWnd, WM_APP_LOG, 0, (LPARAM)_strdup("[*] Pushing to GitHub..."));

        ProcessResult push_res = process_run_ex(state.config.target_path, "git push", log_callback, (void*)hWnd);
        if (!push_res.success) {
            PostMessageA(hWnd, WM_APP_LOG, 0, (LPARAM)_strdup("[!] Push failed. See output above."));
            PostMessageA(hWnd, WM_APP_COMPLETE, FALSE, (LPARAM)_strdup("Push failed. Check output log."));
            return 0;
        }

        // Get repo url
        ProcessResult url_res = process_run_in_dir(state.config.target_path, "gh repo view --json url --jq .url");
        char final_url[MAX_URL_LEN] = "";
        if (url_res.success && *url_res.output) {
            snprintf(final_url, sizeof(final_url), "%s", url_res.output);
        }

        PostMessageA(hWnd, WM_APP_PROGRESS, 100, (LPARAM)_strdup("Sync & push complete!"));
        PostMessageA(hWnd, WM_APP_COMPLETE, TRUE, (LPARAM)_strdup(final_url));

    } else {
        // Publish Mode
        // 1. Create directory if missing
        if (!fs_dir_exists(state.config.target_path)) {
            CreateDirectoryA(state.config.target_path, NULL);
        }

        // 2. .gitignore
        if (state.config.generate_gitignore) {
            PostMessageA(hWnd, WM_APP_PROGRESS, 15, (LPARAM)_strdup("Writing .gitignore..."));
            PostMessageA(hWnd, WM_APP_LOG, 0, (LPARAM)_strdup("[*] Writing .gitignore file..."));
            gitignore_write(state.config.target_path, state.config.gitignore_template);
        }

        // 3. git init
        PostMessageA(hWnd, WM_APP_PROGRESS, 30, (LPARAM)_strdup("Initializing Git repository..."));
        PostMessageA(hWnd, WM_APP_LOG, 0, (LPARAM)_strdup("[*] Initializing local Git repository..."));
        if (!git_init_repo(state.config.target_path, state.config.initial_branch, err_buf, sizeof(err_buf))) {
            PostMessageA(hWnd, WM_APP_LOG, 0, (LPARAM)_strdup("[!] Failed to initialize git."));
            PostMessageA(hWnd, WM_APP_COMPLETE, FALSE, (LPARAM)_strdup("Git init failed."));
            return 0;
        }

        // 4. git add
        PostMessageA(hWnd, WM_APP_PROGRESS, 50, (LPARAM)_strdup("Staging project files..."));
        PostMessageA(hWnd, WM_APP_LOG, 0, (LPARAM)_strdup("[*] Staging files (git add -A)..."));
        process_run_ex(state.config.target_path, "git add -A", log_callback, (void*)hWnd);

        // 5. git commit
        PostMessageA(hWnd, WM_APP_PROGRESS, 70, (LPARAM)_strdup("Sealing initial commit..."));
        PostMessageA(hWnd, WM_APP_LOG, 0, (LPARAM)_strdup("[*] Committing files..."));
        char commit_cmd[1024];
        snprintf(commit_cmd, sizeof(commit_cmd), "git commit -m \"%s\"", state.config.commit_message);
        process_run_ex(state.config.target_path, commit_cmd, log_callback, (void*)hWnd);

        // 6. gh repo create
        PostMessageA(hWnd, WM_APP_PROGRESS, 85, (LPARAM)_strdup("Creating GitHub repository & uploading..."));
        PostMessageA(hWnd, WM_APP_LOG, 0, (LPARAM)_strdup("[*] Creating repository on GitHub with gh repo create..."));

        char created_url[MAX_URL_LEN] = "";
        bool ok = gh_create_and_push_repo(
            state.config.target_path,
            state.config.repo_name,
            state.config.visibility,
            state.config.repo_description,
            created_url,
            sizeof(created_url),
            err_buf,
            sizeof(err_buf)
        );

        if (!ok) {
            PostMessageA(hWnd, WM_APP_LOG, 0, (LPARAM)_strdup("[!] GitHub repo creation/push encountered an error."));
            if (*err_buf) {
                PostMessageA(hWnd, WM_APP_LOG, 0, (LPARAM)_strdup(err_buf));
            }
            PostMessageA(hWnd, WM_APP_COMPLETE, FALSE, (LPARAM)_strdup("Failed to publish to GitHub."));
            return 0;
        }

        PostMessageA(hWnd, WM_APP_PROGRESS, 100, (LPARAM)_strdup("Successfully published to GitHub!"));
        PostMessageA(hWnd, WM_APP_COMPLETE, TRUE, (LPARAM)_strdup(created_url));
    }

    return 0;
}

static void on_action_button_clicked(void) {
    if (g_ctx.is_busy) return;

    char folder[MAX_PATH_LEN];
    GetWindowTextA(g_ctx.hEditFolder, folder, sizeof(folder));
    if (!fs_normalize_path(folder, g_ctx.state.config.target_path, sizeof(g_ctx.state.config.target_path))) {
        MessageBoxA(g_ctx.hWnd, "Please specify a valid folder path.", "git-ez", MB_ICONWARNING);
        return;
    }

    if (!g_ctx.sync_mode) {
        GetWindowTextA(g_ctx.hEditRepoName, g_ctx.state.config.repo_name, sizeof(g_ctx.state.config.repo_name));
        if (strlen(g_ctx.state.config.repo_name) == 0) {
            MessageBoxA(g_ctx.hWnd, "Please enter a repository name.", "git-ez", MB_ICONWARNING);
            return;
        }

        g_ctx.state.config.visibility = (SendMessageA(g_ctx.hRadioPrivate, BM_GETCHECK, 0, 0) == BST_CHECKED)
            ? VISIBILITY_PRIVATE : VISIBILITY_PUBLIC;

        g_ctx.state.config.generate_gitignore = (SendMessageA(g_ctx.hCheckGitignore, BM_GETCHECK, 0, 0) == BST_CHECKED);
        GetWindowTextA(g_ctx.hComboGitignore, g_ctx.state.config.gitignore_template, sizeof(g_ctx.state.config.gitignore_template));
    }

    GetWindowTextA(g_ctx.hEditBranch, g_ctx.state.config.initial_branch, sizeof(g_ctx.state.config.initial_branch));
    GetWindowTextA(g_ctx.hEditCommit, g_ctx.state.config.commit_message, sizeof(g_ctx.state.config.commit_message));

    g_ctx.state.config.auto_open_browser = (SendMessageA(g_ctx.hCheckBrowser, BM_GETCHECK, 0, 0) == BST_CHECKED);
    g_ctx.state.config.auto_copy_url = (SendMessageA(g_ctx.hCheckCopy, BM_GETCHECK, 0, 0) == BST_CHECKED);

    g_ctx.is_busy = true;
    EnableWindow(g_ctx.hBtnAction, FALSE);
    EnableWindow(g_ctx.hBtnBrowse, FALSE);
    EnableWindow(g_ctx.hEditFolder, FALSE);
    SendMessageA(g_ctx.hProgress, PBM_SETPOS, 0, 0);
    SetWindowTextA(g_ctx.hLblStatus, "Preparing operations...");

    SetWindowTextA(g_ctx.hEditLog, "");
    append_log("=== git-ez starting operations ===");

    WorkerParams *wp = (WorkerParams *)malloc(sizeof(WorkerParams));
    wp->hWnd = g_ctx.hWnd;
    wp->state = g_ctx.state;
    wp->sync_mode = g_ctx.sync_mode;

    HANDLE hThread = CreateThread(NULL, 0, worker_thread_proc, wp, 0, NULL);
    if (hThread) {
        CloseHandle(hThread);
    }
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        int code = HIWORD(wParam);

        if (id == ID_BTN_BROWSE && code == BN_CLICKED) {
            select_folder_dialog(hWnd);
        } else if (id == ID_BTN_EXPLORE && code == BN_CLICKED) {
            char folder[MAX_PATH_LEN];
            GetWindowTextA(g_ctx.hEditFolder, folder, sizeof(folder));
            if (fs_dir_exists(folder)) {
                ShellExecuteA(NULL, "explore", folder, NULL, NULL, SW_SHOWNORMAL);
            }
        } else if (id == ID_EDIT_FOLDER && code == EN_KILLFOCUS) {
            char folder[MAX_PATH_LEN];
            GetWindowTextA(g_ctx.hEditFolder, folder, sizeof(folder));
            inspect_folder(folder);
        } else if (id == ID_BTN_ACTION && code == BN_CLICKED) {
            on_action_button_clicked();
        } else if (id == ID_BTN_AUTH_LOGIN && code == BN_CLICKED) {
            append_log("[*] Opening GitHub authentication in browser...");
            sys_open_browser("https://github.com/login");
            process_run_interactive("gh auth login");
            refresh_diagnostics();
        } else if (id == ID_BTN_OPEN_URL && code == BN_CLICKED) {
            if (*g_ctx.state.created_repo_url) {
                sys_open_browser(g_ctx.state.created_repo_url);
            }
        } else if (id == ID_BTN_COPY_URL && code == BN_CLICKED) {
            if (*g_ctx.state.created_repo_url) {
                clipboard_copy(g_ctx.state.created_repo_url);
                MessageBoxA(hWnd, "Repository URL copied to clipboard!", "git-ez", MB_OK | MB_ICONINFORMATION);
            }
        }
        return 0;
    }

    case WM_APP_LOG: {
        char *line = (char *)lParam;
        if (line) {
            append_log(line);
            free(line);
        }
        return 0;
    }

    case WM_APP_PROGRESS: {
        int pos = (int)wParam;
        char *status = (char *)lParam;
        SendMessageA(g_ctx.hProgress, PBM_SETPOS, (WPARAM)pos, 0);
        if (status) {
            SetWindowTextA(g_ctx.hLblStatus, status);
            free(status);
        }
        return 0;
    }

    case WM_APP_COMPLETE: {
        bool success = (bool)wParam;
        char *msg_data = (char *)lParam;

        g_ctx.is_busy = false;
        EnableWindow(g_ctx.hBtnAction, TRUE);
        EnableWindow(g_ctx.hBtnBrowse, TRUE);
        EnableWindow(g_ctx.hEditFolder, TRUE);

        if (success) {
            SendMessageA(g_ctx.hProgress, PBM_SETPOS, 100, 0);
            SetWindowTextA(g_ctx.hLblStatus, "Done! Operation completed successfully.");

            if (msg_data && *msg_data) {
                snprintf(g_ctx.state.created_repo_url, sizeof(g_ctx.state.created_repo_url), "%s", msg_data);
                ShowWindow(g_ctx.hBtnOpenUrl, SW_SHOW);
                ShowWindow(g_ctx.hBtnCopyUrl, SW_SHOW);

                if (g_ctx.state.config.auto_copy_url) {
                    clipboard_copy(g_ctx.state.created_repo_url);
                }
                if (g_ctx.state.config.auto_open_browser) {
                    sys_open_browser(g_ctx.state.created_repo_url);
                }

                char congrats[1024];
                snprintf(congrats, sizeof(congrats), "Mission accomplished! Your code is live:\n\n%s", g_ctx.state.created_repo_url);
                MessageBoxA(hWnd, congrats, "git-ez: Success!", MB_OK | MB_ICONINFORMATION);
            } else {
                MessageBoxA(hWnd, "Changes successfully synced and pushed to GitHub!", "git-ez: Success!", MB_OK | MB_ICONINFORMATION);
            }
        } else {
            SetWindowTextA(g_ctx.hLblStatus, "Operation encountered an error. Check log.");
            char err[512];
            snprintf(err, sizeof(err), "Operation encountered a hiccup: %s\nCheck the output log for details.", msg_data ? msg_data : "Unknown error");
            MessageBoxA(hWnd, err, "git-ez: Notice", MB_OK | MB_ICONWARNING);
        }

        if (msg_data) free(msg_data);

        // Re-inspect current folder to reflect new status
        char current_f[MAX_PATH_LEN];
        GetWindowTextA(g_ctx.hEditFolder, current_f, sizeof(current_f));
        g_ctx.last_inspected_path[0] = '\0';
        inspect_folder(current_f);
        return 0;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        SetBkMode(hdcStatic, TRANSPARENT);
        return (LRESULT)g_ctx.hBrushBg;
    }

    case WM_DESTROY: {
        DeleteObject(g_ctx.hFontUI);
        DeleteObject(g_ctx.hFontBold);
        DeleteObject(g_ctx.hFontTitle);
        DeleteObject(g_ctx.hFontLog);
        DeleteObject(g_ctx.hBrushBg);
        DeleteObject(g_ctx.hBrushCard);
        PostQuitMessage(0);
        return 0;
    }
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}

int gui_run(AppState *state, HINSTANCE hInstance, int nCmdShow) {
    memset(&g_ctx, 0, sizeof(g_ctx));
    if (state) g_ctx.state = *state;

    // Init Common Controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_WIN95_CLASSES | ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    // Register Window Class
    WNDCLASSEXA wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "GitEzGuiClass";

    RegisterClassExA(&wc);

    int win_w = 680;
    int win_h = 690;
    int screen_w = GetSystemMetrics(SM_CXSCREEN);
    int screen_h = GetSystemMetrics(SM_CYSCREEN);
    int pos_x = (screen_w - win_w) / 2;
    int pos_y = (screen_h - win_h) / 2;

    HWND hWnd = CreateWindowExA(
        WS_EX_APPWINDOW,
        "GitEzGuiClass",
        "git-ez  ::  Cozy Git & GitHub Launcher",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        pos_x, pos_y, win_w, win_h,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) return 1;
    g_ctx.hWnd = hWnd;

    // Dark Mode Support for Titlebar (Windows 10/11)
    BOOL darkMode = TRUE;
    DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));

    // Create Fonts
    g_ctx.hFontUI = CreateFontA(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    g_ctx.hFontBold = CreateFontA(-13, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    g_ctx.hFontTitle = CreateFontA(-17, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    g_ctx.hFontLog = CreateFontA(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    g_ctx.hBrushBg = GetSysColorBrush(COLOR_BTNFACE);

    // Header & Diagnostics
    HWND hTitle = CreateWindowExA(0, "STATIC", "git-ez", WS_CHILD | WS_VISIBLE, 20, 12, 100, 24, hWnd, (HMENU)ID_LBL_HEADER, hInstance, NULL);
    SendMessageA(hTitle, WM_SETFONT, (WPARAM)g_ctx.hFontTitle, TRUE);

    HWND hSub = CreateWindowExA(0, "STATIC", "The cozy, no-sweat way to toss private repos onto GitHub", WS_CHILD | WS_VISIBLE, 110, 16, 400, 20, hWnd, NULL, hInstance, NULL);
    SendMessageA(hSub, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    g_ctx.hLblDiagGit = CreateWindowExA(0, "STATIC", "Git: Checking...", WS_CHILD | WS_VISIBLE, 20, 42, 210, 18, hWnd, (HMENU)ID_LBL_DIAG_GIT, hInstance, NULL);
    SendMessageA(g_ctx.hLblDiagGit, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    g_ctx.hLblDiagGh = CreateWindowExA(0, "STATIC", "GitHub CLI: Checking...", WS_CHILD | WS_VISIBLE, 235, 42, 230, 18, hWnd, (HMENU)ID_LBL_DIAG_GH, hInstance, NULL);
    SendMessageA(g_ctx.hLblDiagGh, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    g_ctx.hLblDiagAuth = CreateWindowExA(0, "STATIC", "Auth: Checking...", WS_CHILD | WS_VISIBLE, 470, 42, 190, 18, hWnd, (HMENU)ID_LBL_DIAG_AUTH, hInstance, NULL);
    SendMessageA(g_ctx.hLblDiagAuth, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    g_ctx.hBtnAuthLogin = CreateWindowExA(0, "BUTTON", "Sign In", WS_CHILD | BS_PUSHBUTTON, 580, 12, 70, 24, hWnd, (HMENU)ID_BTN_AUTH_LOGIN, hInstance, NULL);
    SendMessageA(g_ctx.hBtnAuthLogin, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    // Group 1: Project Folder Selection
    HWND hGrpFolder = CreateWindowExA(0, "BUTTON", " Project Folder ", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 15, 68, 640, 88, hWnd, (HMENU)ID_GRP_FOLDER, hInstance, NULL);
    SendMessageA(hGrpFolder, WM_SETFONT, (WPARAM)g_ctx.hFontBold, TRUE);

    g_ctx.hEditFolder = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 28, 92, 450, 24, hWnd, (HMENU)ID_EDIT_FOLDER, hInstance, NULL);
    SendMessageA(g_ctx.hEditFolder, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    g_ctx.hBtnBrowse = CreateWindowExA(0, "BUTTON", "Browse...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 485, 92, 80, 24, hWnd, (HMENU)ID_BTN_BROWSE, hInstance, NULL);
    SendMessageA(g_ctx.hBtnBrowse, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    g_ctx.hBtnExplore = CreateWindowExA(0, "BUTTON", "Explorer", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 570, 92, 70, 24, hWnd, (HMENU)ID_BTN_EXPLORE, hInstance, NULL);
    SendMessageA(g_ctx.hBtnExplore, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    g_ctx.hLblFolderBadge = CreateWindowExA(0, "STATIC", "Analyzing folder...", WS_CHILD | WS_VISIBLE, 28, 125, 615, 20, hWnd, (HMENU)ID_LBL_FOLDER_BADGE, hInstance, NULL);
    SendMessageA(g_ctx.hLblFolderBadge, WM_SETFONT, (WPARAM)g_ctx.hFontBold, TRUE);

    // Group 2: Repository Settings
    HWND hGrpConfig = CreateWindowExA(0, "BUTTON", " Repository Settings ", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 15, 164, 640, 160, hWnd, (HMENU)ID_GRP_CONFIG, hInstance, NULL);
    SendMessageA(hGrpConfig, WM_SETFONT, (WPARAM)g_ctx.hFontBold, TRUE);

    HWND hLblRepo = CreateWindowExA(0, "STATIC", "Repo Name:", WS_CHILD | WS_VISIBLE, 28, 192, 90, 20, hWnd, NULL, hInstance, NULL);
    SendMessageA(hLblRepo, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    g_ctx.hEditRepoName = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "my-repo", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 120, 189, 210, 24, hWnd, (HMENU)ID_EDIT_REPO_NAME, hInstance, NULL);
    SendMessageA(g_ctx.hEditRepoName, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    HWND hLblVis = CreateWindowExA(0, "STATIC", "Visibility:", WS_CHILD | WS_VISIBLE, 350, 192, 70, 20, hWnd, NULL, hInstance, NULL);
    SendMessageA(hLblVis, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    g_ctx.hRadioPrivate = CreateWindowExA(0, "BUTTON", "Private (Safe)", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP, 425, 190, 110, 22, hWnd, (HMENU)ID_RADIO_PRIVATE, hInstance, NULL);
    SendMessageA(g_ctx.hRadioPrivate, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);
    SendMessageA(g_ctx.hRadioPrivate, BM_SETCHECK, BST_CHECKED, 0);

    g_ctx.hRadioPublic = CreateWindowExA(0, "BUTTON", "Public", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, 545, 190, 80, 22, hWnd, (HMENU)ID_RADIO_PUBLIC, hInstance, NULL);
    SendMessageA(g_ctx.hRadioPublic, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    g_ctx.hCheckGitignore = CreateWindowExA(0, "BUTTON", "Generate .gitignore for:", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 28, 225, 170, 22, hWnd, (HMENU)ID_CHECK_GITIGNORE, hInstance, NULL);
    SendMessageA(g_ctx.hCheckGitignore, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);
    SendMessageA(g_ctx.hCheckGitignore, BM_SETCHECK, BST_CHECKED, 0);

    g_ctx.hComboGitignore = CreateWindowExA(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 205, 224, 125, 180, hWnd, (HMENU)ID_COMBO_GITIGNORE, hInstance, NULL);
    SendMessageA(g_ctx.hComboGitignore, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);
    const char *templates[] = { "General", "C/C++", "Python", "Node/Web", "Rust", "Go" };
    for (size_t i = 0; i < sizeof(templates) / sizeof(templates[0]); ++i) {
        SendMessageA(g_ctx.hComboGitignore, CB_ADDSTRING, 0, (LPARAM)templates[i]);
    }
    SendMessageA(g_ctx.hComboGitignore, CB_SETCURSEL, 0, 0);

    HWND hLblBranch = CreateWindowExA(0, "STATIC", "Branch:", WS_CHILD | WS_VISIBLE, 350, 226, 60, 20, hWnd, NULL, hInstance, NULL);
    SendMessageA(hLblBranch, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    g_ctx.hEditBranch = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "main", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 425, 224, 200, 24, hWnd, (HMENU)ID_EDIT_BRANCH, hInstance, NULL);
    SendMessageA(g_ctx.hEditBranch, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    HWND hLblCommit = CreateWindowExA(0, "STATIC", "Commit Msg:", WS_CHILD | WS_VISIBLE, 28, 260, 90, 20, hWnd, NULL, hInstance, NULL);
    SendMessageA(hLblCommit, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    g_ctx.hEditCommit = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Initial commit", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 120, 258, 505, 24, hWnd, (HMENU)ID_EDIT_COMMIT, hInstance, NULL);
    SendMessageA(g_ctx.hEditCommit, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    g_ctx.hCheckBrowser = CreateWindowExA(0, "BUTTON", "Auto-open in browser upon completion", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 28, 292, 280, 22, hWnd, (HMENU)ID_CHECK_BROWSER, hInstance, NULL);
    SendMessageA(g_ctx.hCheckBrowser, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);
    SendMessageA(g_ctx.hCheckBrowser, BM_SETCHECK, BST_CHECKED, 0);

    g_ctx.hCheckCopy = CreateWindowExA(0, "BUTTON", "Auto-copy repository URL to clipboard", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 340, 292, 280, 22, hWnd, (HMENU)ID_CHECK_COPY, hInstance, NULL);
    SendMessageA(g_ctx.hCheckCopy, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);
    SendMessageA(g_ctx.hCheckCopy, BM_SETCHECK, BST_CHECKED, 0);

    // Action Section
    g_ctx.hBtnAction = CreateWindowExA(0, "BUTTON", "Publish to GitHub", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 15, 332, 240, 36, hWnd, (HMENU)ID_BTN_ACTION, hInstance, NULL);
    SendMessageA(g_ctx.hBtnAction, WM_SETFONT, (WPARAM)g_ctx.hFontTitle, TRUE);

    g_ctx.hProgress = CreateWindowExA(0, PROGRESS_CLASSA, "", WS_CHILD | WS_VISIBLE | PBS_SMOOTH, 265, 335, 390, 16, hWnd, (HMENU)ID_PROGRESS, hInstance, NULL);
    SendMessageA(g_ctx.hProgress, PBM_SETRANGE32, 0, 100);

    g_ctx.hLblStatus = CreateWindowExA(0, "STATIC", "Ready", WS_CHILD | WS_VISIBLE, 265, 354, 390, 16, hWnd, (HMENU)ID_LBL_STATUS, hInstance, NULL);
    SendMessageA(g_ctx.hLblStatus, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    // Group 3: Operation Log & Results
    HWND hGrpLog = CreateWindowExA(0, "BUTTON", " Operation Output Log ", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 15, 375, 640, 225, hWnd, (HMENU)ID_GRP_LOG, hInstance, NULL);
    SendMessageA(hGrpLog, WM_SETFONT, (WPARAM)g_ctx.hFontBold, TRUE);

    g_ctx.hEditLog = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, 28, 398, 615, 192, hWnd, (HMENU)ID_EDIT_LOG, hInstance, NULL);
    SendMessageA(g_ctx.hEditLog, WM_SETFONT, (WPARAM)g_ctx.hFontLog, TRUE);

    // Bottom Result Buttons
    g_ctx.hBtnOpenUrl = CreateWindowExA(0, "BUTTON", "Open in Web Browser", WS_CHILD | BS_PUSHBUTTON, 360, 610, 145, 28, hWnd, (HMENU)ID_BTN_OPEN_URL, hInstance, NULL);
    SendMessageA(g_ctx.hBtnOpenUrl, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    g_ctx.hBtnCopyUrl = CreateWindowExA(0, "BUTTON", "Copy Repo URL", WS_CHILD | BS_PUSHBUTTON, 515, 610, 140, 28, hWnd, (HMENU)ID_BTN_COPY_URL, hInstance, NULL);
    SendMessageA(g_ctx.hBtnCopyUrl, WM_SETFONT, (WPARAM)g_ctx.hFontUI, TRUE);

    // Initial populate
    refresh_diagnostics();

    char init_dir[MAX_PATH_LEN] = "";
    if (*state->config.target_path) {
        snprintf(init_dir, sizeof(init_dir), "%s", state->config.target_path);
    } else {
        fs_get_current_dir(init_dir, sizeof(init_dir));
    }
    SetWindowTextA(g_ctx.hEditFolder, init_dir);
    inspect_folder(init_dir);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Message Loop
    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}

#endif // _WIN32
