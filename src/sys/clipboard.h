/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  📋 git-ez : clipboard.h                                    │
 *  │  "The Copy-Paste Conjurer"                                  │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <stdbool.h>

/**
 * Slip a string directly into the user's OS clipboard so Ctrl+V / Cmd+V just works!
 */
bool clipboard_copy(const char *text);

#endif // CLIPBOARD_H
