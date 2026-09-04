#!/usr/bin/env bash
set -e

echo "==================================================="
echo "  Building git-ez (Pure C Git & GitHub Launcher)"
echo "==================================================="

mkdir -p bin

CC_BIN=""
if command -v gcc >/dev/null 2>&1; then
    CC_BIN="gcc"
elif command -v clang >/dev/null 2>&1; then
    CC_BIN="clang"
elif command -v zig >/dev/null 2>&1; then
    CC_BIN="zig cc"
elif command -v tcc >/dev/null 2>&1; then
    CC_BIN="tcc"
fi

if [ -z "$CC_BIN" ]; then
    echo "Error: No compatible C compiler (gcc, clang, zig cc, or tcc) found." >&2
    echo "Install one using: sudo apt install gcc (or pacman/dnf/brew)" >&2
    exit 1
fi

echo "[*] Compiling using $CC_BIN..."
$CC_BIN -std=c99 -Wall -Wextra -O2 -Isrc \
    src/main.c \
    src/sys/process.c \
    src/sys/fs_util.c \
    src/sys/clipboard.c \
    src/ui/ui.c \
    src/ui/prompt.c \
    src/git/git_ops.c \
    src/git/gitignore.c \
    src/gh/gh_ops.c \
    -o bin/git-ez

echo "[OK] Build successful: bin/git-ez"
echo "Run: ./bin/git-ez --help"
