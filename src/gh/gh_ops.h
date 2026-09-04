/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  🐙 git-ez : gh_ops.h                                       |
 *  │  "The GitHub Octocat Whisperer & Remote Repository Forger"  │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#ifndef GH_OPS_H
#define GH_OPS_H

#include <stdbool.h>
#include <stddef.h>
#include "../app_state.h"

// Make sure the GitHub CLI (gh) binary is installed and saying hello
bool gh_check_installed(char *version_buf, size_t max_len);

// Peek into our active GitHub session and find our username
bool gh_check_auth(char *username_buf, size_t max_len);

// Launch the interactive GitHub login dance if needed
bool gh_login_interactive(void);

// Forge the remote repo (Private by default!), wire up git remote, and push!
bool gh_create_and_push_repo(const char *dir, 
                            const char *repo_name, 
                            RepoVisibility visibility, 
                            const char *description, 
                            char *out_repo_url, 
                            size_t url_max, 
                            char *err_buf, 
                            size_t err_max);

// Ask gh CLI to open the repository in the browser
bool gh_open_in_browser(const char *dir);

#endif // GH_OPS_H
