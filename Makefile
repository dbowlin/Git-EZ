CC ?= gcc
CFLAGS ?= -std=c99 -Wall -Wextra -O2 -s -mwindows -Isrc
LIBS = -lcomctl32 -lshell32 -lole32 -luuid -luser32 -lgdi32 -ladvapi32 -ldwmapi
TARGET = bin/git-ez

SRCS = src/main.c \
       src/sys/process.c \
       src/sys/fs_util.c \
       src/sys/clipboard.c \
       src/ui/ui.c \
       src/ui/prompt.c \
       src/ui/win_gui.c \
       src/git/git_ops.c \
       src/git/gitignore.c \
       src/gh/gh_ops.c

OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe
