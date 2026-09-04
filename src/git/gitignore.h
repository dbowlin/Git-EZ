/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  🛡️ git-ez : gitignore.h                                    │
 *  │  "The Junk Shield : Keeping .DS_Store & .exe out of GitHub" │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#ifndef GITIGNORE_H
#define GITIGNORE_H

#include <stdbool.h>

/**
 * Bake a freshly tailored .gitignore file into the project folder.
 * Templates supported: "C/C++", "Python", "Node/Web", "Rust", "Go", "General".
 */
bool gitignore_write(const char *project_dir, const char *template_name);

#endif // GITIGNORE_H
