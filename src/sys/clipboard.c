/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  📋 git-ez : clipboard.c                                    │
 *  │  "Because typing URLs by hand is so 1995"                   │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#include "clipboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

bool clipboard_copy(const char *text) {
    if (!text) return false;
    size_t len = strlen(text);
    if (len == 0) return false;

    // Knock on the Win32 clipboard door
    if (!OpenClipboard(NULL)) return false;
    EmptyClipboard();

    // Allocate movable global memory (the classic Windows dance)
    HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, len + 1);
    if (!hGlob) {
        CloseClipboard();
        return false;
    }

    char *pGlob = (char *)GlobalLock(hGlob);
    if (!pGlob) {
        GlobalFree(hGlob);
        CloseClipboard();
        return false;
    }

    memcpy(pGlob, text, len + 1);
    GlobalUnlock(hGlob);

    // Hand it to Windows and close the door
    SetClipboardData(CF_TEXT, hGlob);
    CloseClipboard();
    return true;
}

#else

bool clipboard_copy(const char *text) {
    if (!text) return false;

    // Check if Wayland's wl-copy is around
    FILE *pipe = popen("wl-copy 2>/dev/null", "w");
    if (pipe) {
        fputs(text, pipe);
        if (pclose(pipe) == 0) return true;
    }

    // Check if classic X11 xclip is hanging out
    pipe = popen("xclip -selection clipboard 2>/dev/null", "w");
    if (pipe) {
        fputs(text, pipe);
        if (pclose(pipe) == 0) return true;
    }

    // Check if macOS pbcopy is ready
    pipe = popen("pbcopy 2>/dev/null", "w");
    if (pipe) {
        fputs(text, pipe);
        if (pclose(pipe) == 0) return true;
    }

    // If none of those worked, no worries, user can still copy from the screen
    return false;
}

#endif
