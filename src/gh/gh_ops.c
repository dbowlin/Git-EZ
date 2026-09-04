/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  🐙 git-ez : gh_ops.c                                       │
 *  │  "Turning local folders into shiny GitHub treasures"        │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#include "gh_ops.h"
#include "../sys/process.h"
#include <stdio.h>
#include <string.h>

bool gh_check_installed(char *version_buf, size_t max_len) {
    ProcessResult res = process_run("gh --version");
    if (!res.success) return false;

    if (version_buf && max_len > 0) {
        // First line contains the version string, e.g. "gh version 2.x.x"
        char *newline = strchr(res.output, '\n');
        if (newline) *newline = '\0';
        snprintf(version_buf, max_len, "%s", res.output);
    }
    return true;
}

bool gh_check_auth(char *username_buf, size_t max_len) {
    ProcessResult res = process_run("gh auth status");
    if (!res.success) {
        if (username_buf && max_len > 0) username_buf[0] = '\0';
        return false;
    }

    if (username_buf && max_len > 0) {
        // Output commonly says: "Logged in to github.com account <username>"
        const char *account_str = strstr(res.output, "account ");
        if (account_str) {
            account_str += 8;
            size_t idx = 0;
            while (*account_str && *account_str != ' ' && *account_str != '\n' && *account_str != '\r' && *account_str != '(' && idx < max_len - 1) {
                username_buf[idx++] = *account_str++;
            }
            username_buf[idx] = '\0';
        } else {
            // Clever backup: ask the GitHub API directly for the logged in handle
            ProcessResult user_res = process_run("gh api user --jq .login");
            if (user_res.success && strlen(user_res.output) > 0) {
                snprintf(username_buf, max_len, "%s", user_res.output);
            } else {
                snprintf(username_buf, max_len, "authenticated-user");
            }
        }
    }
    return true;
}

bool gh_login_interactive(void) {
    // Handing over the stage to GitHub CLI's official browser/device auth flow
    int code = process_run_interactive("gh auth login");
    return (code == 0);
}

bool gh_create_and_push_repo(const char *dir, 
                            const char *repo_name, 
                            RepoVisibility visibility, 
                            const char *description, 
                            char *out_repo_url, 
                            size_t url_max, 
                            char *err_buf, 
                            size_t err_max) {
    if (!repo_name || !*repo_name) {
        if (err_buf && err_max > 0) snprintf(err_buf, err_max, "Oops! The repository name cannot be empty.");
        return false;
    }

    // Default to --private for safety!
    const char *vis_flag = (visibility == VISIBILITY_PRIVATE) ? "--private" : "--public";

    char cmd[2048];
    if (description && *description) {
        snprintf(cmd, sizeof(cmd), "gh repo create %s %s --description \"%s\" --source=. --push",
                 repo_name, vis_flag, description);
    } else {
        snprintf(cmd, sizeof(cmd), "gh repo create %s %s --source=. --push",
                 repo_name, vis_flag);
    }

    ProcessResult res = process_run_in_dir(dir, cmd);
    if (!res.success) {
        if (err_buf && err_max > 0) {
            snprintf(err_buf, err_max, "%s", res.output);
        }
        return false;
    }

    // Ask gh for the clean canonical web URL
    ProcessResult url_res = process_run_in_dir(dir, "gh repo view --json url --jq .url");
    if (url_res.success && strlen(url_res.output) > 0) {
        if (out_repo_url && url_max > 0) {
            snprintf(out_repo_url, url_max, "%s", url_res.output);
        }
    } else {
        // Construct the URL manually if jq query wasn't supported
        char username[128] = "";
        gh_check_auth(username, sizeof(username));
        if (out_repo_url && url_max > 0) {
            if (*username) {
                snprintf(out_repo_url, url_max, "https://github.com/%s/%s", username, repo_name);
            } else {
                snprintf(out_repo_url, url_max, "https://github.com/%s", repo_name);
            }
        }
    }

    return true;
}

bool gh_open_in_browser(const char *dir) {
    ProcessResult res = process_run_in_dir(dir, "gh repo view --web");
    return res.success;
}
