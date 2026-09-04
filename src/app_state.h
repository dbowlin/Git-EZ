/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  🚀 git-ez : app_state.h                                    │
 *  │  "The Grand Notebook of Everything We Need to Remember"     │
 *  │  Crafted with care, caffeine, and mild confusion.           │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdbool.h>
#include <stddef.h>

#define APP_NAME "git-ez"
#define APP_VERSION "1.0.0-whimsical"
#define APP_DESCRIPTION "The cozy, no-sweat way to toss private repos onto GitHub!"

// Generous buffer sizes because memory is cheap and buffer overflows are not fun
#define MAX_PATH_LEN 1024
#define MAX_NAME_LEN 256
#define MAX_MSG_LEN 512
#define MAX_URL_LEN 512
#define MAX_OUTPUT_LEN 4096

// How do you want to play today?
typedef enum {
    MODE_WIZARD = 0,     // The cozy guided tour (default!)
    MODE_EXPRESS,        // "Just do it, I trust you!" mode (-y)
    MODE_SYNC,           // "Ship my latest edits!" mode (--sync)
    MODE_STATUS,         // Curious about what's going on
    MODE_HELP,           // Show the friendly instruction manual
    MODE_VERSION         // Brag about the version number
} AppMode;

// Privacy settings (spoiler: private is best for top-secret world domination plans)
typedef enum {
    VISIBILITY_PRIVATE = 0, // 🔒 Shhh, it's a secret
    VISIBILITY_PUBLIC       // 🌐 Hello, world! Look at my code!
} RepoVisibility;

// Health report of our trusty tools
typedef struct {
    bool git_installed;
    char git_version[MAX_NAME_LEN];
    bool gh_installed;
    char gh_version[MAX_NAME_LEN];
    bool gh_authenticated;
    char gh_username[MAX_NAME_LEN];
} SystemDiagnostics;

// The recipe for the repository we are about to bake
typedef struct {
    char target_path[MAX_PATH_LEN];      // Where on disk does the project live?
    char repo_name[MAX_NAME_LEN];        // What shall we name our creation?
    char repo_description[MAX_MSG_LEN];  // A lovely little tagline
    RepoVisibility visibility;           // Lock or megaphone?
    char initial_branch[64];             // Almost always "main" these days
    char commit_message[MAX_MSG_LEN];    // "Initial commit" or poetry of your choice
    bool generate_gitignore;             // Should we conjure a .gitignore?
    char gitignore_template[64];         // C, Python, Node, or something mystical
    bool auto_open_browser;              // Pop open the browser for a victory glance
    bool auto_copy_url;                  // Pop URL straight into clipboard
    bool is_already_git_repo;            // Did someone already run git init here?
    size_t file_count;                   // How many files are we wrangling?
    size_t total_bytes;                  // Weight of the project in bytes
} RepoConfig;

// The whole shebang in one handy container
typedef struct {
    AppMode mode;
    bool non_interactive;
    bool verbose;
    bool dry_run;
    SystemDiagnostics diag;
    RepoConfig config;
    char created_repo_url[MAX_URL_LEN];
} AppState;

#endif // APP_STATE_H
