@echo off
setlocal enabledelayedexpansion

echo ===================================================
echo   Building git-ez (Pure C Git ^& GitHub Launcher)
echo ===================================================

if not exist bin mkdir bin

REM Add temporary or local toolchains to PATH if present
if exist "%TEMP%\w64devkit_dir\w64devkit\bin\gcc.exe" (
    set "PATH=%TEMP%\w64devkit_dir\w64devkit\bin;%PATH%"
)
if exist "%LOCALAPPDATA%\Programs\Zig\zig.exe" (
    set "PATH=%LOCALAPPDATA%\Programs\Zig;%PATH%"
)

REM 1. Check for GCC (MinGW / w64devkit)
where gcc >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo [*] Found GCC compiler. Compiling with 'gcc'...
    set "RES_OBJ="
    where windres >nul 2>&1
    if !ERRORLEVEL! equ 0 (
        windres -i src/resources.rc -o bin/resources.o
        set "RES_OBJ=bin/resources.o"
    )
    gcc -std=c99 -Wall -Wextra -O2 -s -mwindows -Isrc src/main.c src/sys/process.c src/sys/fs_util.c src/sys/clipboard.c src/ui/ui.c src/ui/prompt.c src/ui/win_gui.c src/git/git_ops.c src/git/gitignore.c src/gh/gh_ops.c !RES_OBJ! -lcomctl32 -lshell32 -lole32 -luuid -luser32 -lgdi32 -ladvapi32 -ldwmapi -o bin/git-ez.exe
    if !ERRORLEVEL! equ 0 (
        echo [OK] Build successful: bin\git-ez.exe
        goto :done
    )
)

REM 2. Check for Zig (zig cc)
where zig >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo [*] Found Zig compiler. Compiling with 'zig cc'...
    zig cc -std=c99 -Wall -Wextra -O2 -s -mwindows -Isrc src/main.c src/sys/process.c src/sys/fs_util.c src/sys/clipboard.c src/ui/ui.c src/ui/prompt.c src/ui/win_gui.c src/git/git_ops.c src/git/gitignore.c src/gh/gh_ops.c -lcomctl32 -lshell32 -lole32 -luuid -luser32 -lgdi32 -ladvapi32 -ldwmapi -o bin/git-ez.exe
    if !ERRORLEVEL! equ 0 (
        echo [OK] Build successful: bin\git-ez.exe
        goto :done
    )
)

REM 3. Check for Clang
where clang >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo [*] Found Clang compiler. Compiling with 'clang'...
    clang -std=c99 -Wall -Wextra -O2 -s -mwindows -Isrc src/main.c src/sys/process.c src/sys/fs_util.c src/sys/clipboard.c src/ui/ui.c src/ui/prompt.c src/ui/win_gui.c src/git/git_ops.c src/git/gitignore.c src/gh/gh_ops.c -lcomctl32 -lshell32 -lole32 -luuid -luser32 -lgdi32 -ladvapi32 -ldwmapi -o bin/git-ez.exe
    if !ERRORLEVEL! equ 0 (
        echo [OK] Build successful: bin\git-ez.exe
        goto :done
    )
)

REM 4. Check for MSVC (cl.exe)
where cl >nul 2>&1
if %ERRORLEVEL% equ 0 (
    echo [*] Found MSVC compiler. Compiling with 'cl.exe'...
    rc /fo bin\resources.res src\resources.rc >nul 2>&1
    set "RES_ARG="
    if exist bin\resources.res set "RES_ARG=bin\resources.res"
    cl /nologo /O2 /W3 /Isrc src\main.c src\sys\process.c src\sys\fs_util.c src\sys\clipboard.c src\ui\ui.c src\ui\prompt.c src\ui\win_gui.c src\git\git_ops.c src\git\gitignore.c src\gh\gh_ops.c !RES_ARG! /link /SUBSYSTEM:WINDOWS comctl32.lib shell32.lib ole32.lib uuid.lib user32.lib gdi32.lib advapi32.lib dwmapi.lib /out:bin\git-ez.exe /Fo:bin\
    if !ERRORLEVEL! equ 0 (
        echo [OK] Build successful: bin\git-ez.exe
        goto :done
    )
)

echo [!] Error: No compatible C compiler (zig, gcc, clang, or cl) found in PATH.
echo [!] Please install a C compiler or add it to your PATH.
exit /b 1

:done
echo.
echo Ready to use! Run: bin\git-ez.exe
