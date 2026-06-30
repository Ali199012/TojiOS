CC      = gcc
CFLAGS  = -Wall -Wextra -g `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0` -lpthread -lrt

SRCS =  main.c \
        kernel/boot.c \
        kernel/memory.c \
        kernel/process.c \
        kernel/scheduler.c \
        lib/ipc.c \
        lib/sync.c \
        lib/utils.c \
        apps/sysmonitor.c \
        apps/procmanager.c \
        apps/fileexplorer.c \
        apps/terminal.c \
        apps/settings.c \
        apps/notepad.c \
        apps/calculator.c \
        apps/calendar.c \
        apps/taskscheduler.c \
        apps/fileops.c \
        apps/sysinfo.c \
        apps/search.c \
        apps/minesweeper.c \
        apps/musicplayer.c \
        apps/clock.c \
        apps/snake.c \
        apps/kernelconsole.c

OBJS   = $(SRCS:.c=.o)
TARGET = tojios

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo ""
	@echo "  ╔══════════════════════════════════╗"
	@echo "  ║   Toji OS  built successfully!   ║"
	@echo "  ╚══════════════════════════════════╝"
	@echo "  Run with:  ./tojios"
	@echo ""

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
