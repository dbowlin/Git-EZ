/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  🎨 git-ez : win_gui.h                                      │
 *  │  "Native Windows Graphical User Interface for git-ez"       │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#ifndef WIN_GUI_H
#define WIN_GUI_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../app_state.h"

/**
 * Initialize and run the git-ez Win32 GUI application loop.
 */
int gui_run(AppState *state, HINSTANCE hInstance, int nCmdShow);

#endif // _WIN32
#endif // WIN_GUI_H
