/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  🛡️ git-ez : gitignore.c                                    │
 *  │  "Because nobody wants to commit 500MB of node_modules"     │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#include "gitignore.h"
#include <stdio.h>
#include <string.h>

bool gitignore_write(const char *project_dir, const char *template_name) {
    if (!project_dir || !*project_dir) return false;

    char filepath[1024];
    snprintf(filepath, sizeof(filepath), "%s/.gitignore", project_dir);

    FILE *f = fopen(filepath, "w");
    if (!f) return false;

    // Friendly header for anyone opening .gitignore in an editor
    fprintf(f, "# ------------------------------------------------------------------------------\n");
    fprintf(f, "# Generated with love by git-ez ✨\n");
    fprintf(f, "# ------------------------------------------------------------------------------\n\n");

    // Standard OS & Editor artifacts that nobody likes seeing in git diffs
    fprintf(f, "# Operating System & Editor Curiosities\n");
    fprintf(f, ".DS_Store\n");
    fprintf(f, "Thumbs.db\n");
    fprintf(f, "desktop.ini\n");
    fprintf(f, "*.swp\n");
    fprintf(f, "*.swo\n");
    fprintf(f, "*~\n");
    fprintf(f, ".vscode/\n");
    fprintf(f, ".idea/\n\n");

    if (template_name && (strcmp(template_name, "C/C++") == 0 || strcmp(template_name, "C") == 0)) {
        fprintf(f, "# C & C++ Build Artifacts\n");
        fprintf(f, "*.o\n");
        fprintf(f, "*.obj\n");
        fprintf(f, "*.exe\n");
        fprintf(f, "*.out\n");
        fprintf(f, "*.app\n");
        fprintf(f, "*.dll\n");
        fprintf(f, "*.so\n");
        fprintf(f, "*.dylib\n");
        fprintf(f, "*.a\n");
        fprintf(f, "*.lib\n");
        fprintf(f, "*.pdb\n");
        fprintf(f, "*.ilk\n");
        fprintf(f, "bin/\n");
        fprintf(f, "build/\n");
        fprintf(f, ".vs/\n");
        fprintf(f, "compile_commands.json\n");
    } else if (template_name && strcmp(template_name, "Python") == 0) {
        fprintf(f, "# Python Bytecode & Virtual Environments\n");
        fprintf(f, "__pycache__/\n");
        fprintf(f, "*.py[cod]\n");
        fprintf(f, "*$py.class\n");
        fprintf(f, ".env\n");
        fprintf(f, ".venv\n");
        fprintf(f, "env/\n");
        fprintf(f, "venv/\n");
        fprintf(f, "ENV/\n");
        fprintf(f, "dist/\n");
        fprintf(f, "build/\n");
        fprintf(f, "*.egg-info/\n");
        fprintf(f, ".pytest_cache/\n");
    } else if (template_name && strcmp(template_name, "Node/Web") == 0) {
        fprintf(f, "# Node.js & Web Artifacts (Farewell, heavy node_modules!)\n");
        fprintf(f, "node_modules/\n");
        fprintf(f, "dist/\n");
        fprintf(f, "build/\n");
        fprintf(f, ".next/\n");
        fprintf(f, ".nuxt/\n");
        fprintf(f, ".env\n");
        fprintf(f, ".env.local\n");
        fprintf(f, "*.log\n");
        fprintf(f, "npm-debug.log*\n");
        fprintf(f, "yarn-debug.log*\n");
    } else if (template_name && strcmp(template_name, "Rust") == 0) {
        fprintf(f, "# Rust / Cargo Artifacts\n");
        fprintf(f, "/target/\n");
        fprintf(f, "**/*.rs.bk\n");
    } else if (template_name && strcmp(template_name, "Go") == 0) {
        fprintf(f, "# Go Binaries & Vendor\n");
        fprintf(f, "/bin/\n");
        fprintf(f, "/dist/\n");
        fprintf(f, "/vendor/\n");
    }

    fclose(f);
    return true;
}
