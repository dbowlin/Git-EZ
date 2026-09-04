cmake_minimum_required(VERSION 3.10)
project(git-ez C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(src)

set(SOURCES
    src/main.c
    src/sys/process.c
    src/sys/fs_util.c
    src/sys/clipboard.c
    src/ui/ui.c
    src/ui/prompt.c
    src/ui/win_gui.c
    src/git/git_ops.c
    src/git/gitignore.c
    src/gh/gh_ops.c
)

if (WIN32)
    add_executable(git-ez WIN32 ${SOURCES})
    target_link_libraries(git-ez PRIVATE comctl32 shell32 ole32 uuid user32 gdi32 advapi32 dwmapi)
else()
    add_executable(git-ez ${SOURCES})
endif()
