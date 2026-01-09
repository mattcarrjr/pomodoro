# Pomodoro Timer Makefile

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
LDFLAGS = -lncurses

# macOS with Homebrew ncurses (if system ncurses is too old)
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # Check if Homebrew ncurses exists
    ifneq ($(wildcard /opt/homebrew/opt/ncurses/.),)
        CFLAGS += -I/opt/homebrew/opt/ncurses/include
        LDFLAGS = -L/opt/homebrew/opt/ncurses/lib -lncurses
    else ifneq ($(wildcard /usr/local/opt/ncurses/.),)
        CFLAGS += -I/usr/local/opt/ncurses/include
        LDFLAGS = -L/usr/local/opt/ncurses/lib -lncurses
    endif
endif

# Source files
SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)

# Output binary
TARGET = pomodoro

# Default target
all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dependencies
$(SRCDIR)/main.o: $(SRCDIR)/timer.h $(SRCDIR)/ui.h $(SRCDIR)/sound.h $(SRCDIR)/config.h
$(SRCDIR)/timer.o: $(SRCDIR)/timer.h
$(SRCDIR)/ui.o: $(SRCDIR)/ui.h $(SRCDIR)/timer.h $(SRCDIR)/digits.h
$(SRCDIR)/digits.o: $(SRCDIR)/digits.h
$(SRCDIR)/sound.o: $(SRCDIR)/sound.h
$(SRCDIR)/config.o: $(SRCDIR)/config.h $(SRCDIR)/timer.h $(SRCDIR)/ui.h

clean:
	rm -f $(SRCDIR)/*.o $(TARGET)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

uninstall:
	rm -f /usr/local/bin/$(TARGET)

# Debug build
debug: CFLAGS += -g -O0 -DDEBUG
debug: clean $(TARGET)

# Run the timer (for quick testing)
run: $(TARGET)
	./$(TARGET)

# Quick test with short durations
test: $(TARGET)
	./$(TARGET) -w 1 -s 1 -l 1 -c 2

.PHONY: all clean install uninstall debug run test
