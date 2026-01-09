# Pomodoro Timer

A cross-platform terminal-based Pomodoro timer application that works on Linux (X11 and Wayland) and macOS. This is an in-house tool to help manage time throughout the day based on the Pomodoro method.

## Basic Functionality
- The app runs for a user-specified amount of time (default 25 minutes) before playing a timer sound
- Once the timer sounds, the user is prompted to acknowledge it and start a break of a user-specified amount of time (default 5 minutes)
- This cycle runs 4 times before taking a much longer break (default 60 minutes, also user specified)
- The app features a retro-computing interface, reminiscent of UNIX applications of the 80's and 90's
- Custom sounds are stored in the `./sounds/` directory
- Written in C for performance and low resource usage

## Project Structure

```
pomodoro/
├── src/
│   ├── main.c      # Main application entry point
│   ├── timer.c/h   # Timer logic and state management
│   ├── ui.c/h      # ncurses-based UI rendering
│   ├── sound.c/h   # Sound playback (platform-specific)
│   ├── config.c/h  # Configuration file and argument parsing
│   └── digits.c/h  # Large ASCII digit rendering
├── sounds/         # Sound files (.wav, .ogg)
├── Makefile        # Build configuration
└── CLAUDE.md       # This file
```

## Building the Application

### Requirements
- **Linux**: gcc, ncurses-devel
- **macOS**: Xcode Command Line Tools or gcc, ncurses (via Homebrew if needed)

### Build Commands
```bash
# Build the application
make

# Clean build artifacts
make clean

# Install to /usr/local/bin
sudo make install

# Debug build with symbols
make debug

# Quick test with short durations
make test
```

### Rocky Linux 9 / RHEL Build
```bash
# Install dependencies
sudo dnf install gcc ncurses-devel

# Build
make clean && make
```

## Usage

```bash
# Run with defaults (25 min work, 5 min break, 60 min long break)
./pomodoro

# Customize durations
./pomodoro -w 30 -s 10 -l 90

# Change theme
./pomodoro -t amber    # Options: green, amber, cyan, white

# Disable sounds (use terminal bell)
./pomodoro -n

# Show help
./pomodoro -h
```

### Interactive Controls
- `s` - Start timer (prompts for task name if not set)
- `p` - Pause/unpause
- `r` - Reset timer
- `c` - Open config menu to change settings
- `SPACE` - Acknowledge completion and advance to next phase
- `q` - Quit

### Configuration File
Create `~/.pomodororc` to set default values:
```
work_duration=25
short_break=5
long_break=60
cycles_before_long=4
theme=green
sound_enabled=true
sound_dir=./sounds
```

## Features

### 1. Gradient Progress Bar
The progress bar dynamically changes colors as you work through your session, providing visual feedback about time remaining:

- **0-50% complete**: Green (you're doing great, plenty of time)
- **50-75% complete**: Yellow (halfway there, keep going)
- **75-90% complete**: Orange (final stretch, stay focused)
- **90-100% complete**: Red (almost done!)

The gradient colors are **independent of your chosen theme** - they always use the same progression to provide consistent visual cues regardless of whether you're using green, amber, cyan, or white theme.

### 2. Task Name Field
Track what you're working on during each Pomodoro session:

- When you press **`s`** to start from IDLE, you'll be prompted: "Enter Task Name"
- Type your task description (e.g., "Write documentation", "Fix bug #123")
- Press **Enter** to confirm or **Esc** to skip
- The task name displays prominently **above the timer digits** during your work session
- Task name persists through work/break cycles
- Shows "Task: (none)" if no task was entered
- Task name is cleared when you reset the timer with **`r`**

**Future Enhancement**: Task logging for Obsidian integration is planned. This will automatically log completed sessions with timestamps and task names to a markdown file that can be imported into your Obsidian vault for tracking productivity.

### 3. In-App Config Menu
Change all your settings without leaving the application:

**Access**: Press **`c`** at any time to open the settings menu

**Available Settings**:
- Work Duration (minutes)
- Short Break (minutes)
- Long Break (minutes)
- Cycles Before Long Break (number)
- Theme (green, amber, cyan, white)

**Navigation**:
- **↑/↓ or j/k**: Move between settings
- **Enter**: Start editing the selected field
- **Numbers**: Type to set duration values (when editing)
- **←/→ or h/l**: Cycle through theme options (when editing theme)
- **s**: Save changes to `~/.pomodororc` and exit
- **Esc**: Cancel changes and exit without saving

**Behavior**:
- **Theme changes** apply immediately when you save
- **Time setting changes** while timer is IDLE: Applied to next session
- **Time setting changes** while timer is ACTIVE: Timer automatically resets to apply new durations
- All saved settings persist between application runs via `~/.pomodororc`

### Usage Example

```bash
# Start the app
./pomodoro

# Press 's' to start
# You'll see: "Enter Task Name"
# Type: "Implement new feature"
# Press Enter

# Timer starts, showing:
# Task: Implement new feature
# 25:00 (with large block digits)
# [=========>          ] (gradient progress bar)

# Mid-session, press 'c' to adjust settings
# Change work duration from 25 to 30 minutes
# Press 's' to save
# Timer resets to apply new 30-minute duration

# Press 's' again to restart with updated settings
```

## Cross-Platform Development Notes

### Important: Binary Compatibility
This application is written in C and compiled to native machine code. **Binaries are not portable between operating systems or CPU architectures.**

If you see an error like:
```
zsh: exec format error: ./pomodoro
```

This means you're trying to run a binary compiled for a different platform. For example:
- macOS ARM64 (Apple Silicon) binaries cannot run on Linux x86_64
- Linux binaries cannot run on macOS

**Solution**: Always run `make clean && make` when switching between systems.

### POSIX Compliance
The code requires POSIX functions (getopt, fork, usleep, pid_t). When compiling with `-std=c99`, these require feature test macros:

- `config.c`: Has `#define _POSIX_C_SOURCE 200809L` at the top
- `sound.c`: Has `#define _POSIX_C_SOURCE 200809L` at the top
- `main.c`: Has `#define _XOPEN_SOURCE 700` for extended POSIX features

These macros must be defined **before any #include statements** to enable POSIX extensions.

### Platform-Specific Sound Commands
The `sound.c` file uses conditional compilation:
- **macOS**: Uses `afplay` (built-in)
- **Linux**: Uses `aplay` (ALSA, typically pre-installed)

## Troubleshooting

### "exec format error" on Rocky Linux
**Problem**: The `pomodoro` binary was compiled on macOS and won't run on Linux.

**Solution**: Rebuild for Linux:
```bash
make clean && make
```

Verify the binary type:
```bash
file pomodoro
# Should show: "ELF 64-bit LSB executable, x86-64" (Linux)
# NOT: "Mach-O 64-bit arm64 executable" (macOS)
```

### Compilation errors with getopt/fork/pid_t
**Problem**: Missing POSIX feature test macros when using `-std=c99`.

**Solution**: Ensure `#define _POSIX_C_SOURCE 200809L` is at the top of source files before includes.

### ncurses linking errors on macOS
**Problem**: System ncurses may be outdated or Xcode version incompatible.

**Solution**: Install Homebrew ncurses:
```bash
brew install ncurses
```
The Makefile automatically detects and uses Homebrew ncurses if available.

### No sound on Linux
**Problem**: `aplay` not installed or not in PATH.

**Solution**: Install ALSA utilities:
```bash
# Rocky Linux / RHEL
sudo dnf install alsa-utils

# Debian / Ubuntu
sudo apt install alsa-utils
```

## Technical Implementation Notes

### Gradient Progress Bar
- **Files modified**: `src/ui.c`
- **Implementation**: Added four new color pairs (PAIR_PROGRESS_GREEN, PAIR_PROGRESS_YELLOW, PAIR_PROGRESS_ORANGE, PAIR_PROGRESS_RED) initialized with fixed colors
- **Logic**: `draw_progress_bar()` function calculates position percentage for each character and applies the appropriate color based on thresholds (50%, 75%, 90%)
- **Note**: Uses A_BOLD attribute for orange to make it visually distinct from yellow

### Task Name Field
- **Files modified**: `src/timer.h`, `src/timer.c`, `src/ui.h`, `src/ui.c`, `src/main.c`
- **Storage**: Added `char current_task[256]` field to `TimerContext` struct
- **Input**: `ui_prompt_task_name()` function creates a centered dialog box with text input
- **Display**: Task name rendered in `ui_draw()` above the timer digits
- **Behavior**: Task cleared on reset, persists through work/break transitions

### In-App Config Menu
- **Files modified**: `src/config.h`, `src/config.c`, `src/ui.h`, `src/ui.c`, `src/main.c`
- **Config saving**: `config_save()` function writes settings to `~/.pomodororc` in key=value format
- **UI components**:
  - `ui_draw_config_menu()`: Renders full-screen settings menu with navigation
  - `ui_handle_config_input()`: Processes keyboard input for menu navigation and editing
- **State management**: Main loop handles config menu mode, saves original settings, compares changes to determine if timer reset is needed
- **Type compatibility**: Used forward declaration `typedef struct AppConfig AppConfig;` in `ui.h` to avoid circular dependency with `config.h`

### ncurses Wide Character Support
- **Makefile change**: Changed from `-lncurses` to `-lncursesw` to enable UTF-8 character rendering
- **Applies to**: Both Linux and macOS builds
- **Result**: Unicode block characters (█) and box-drawing characters (╔═╗║╚╝) render correctly in timer digits

## Development History

- **2026-01-09**: Initial cross-platform build fixes
  - Added POSIX feature test macros to `config.c` and `sound.c`
  - Documented binary compatibility requirements
  - Verified successful build on Rocky Linux 9.7 with gcc 11.5.0

- **2026-01-09**: Major feature additions
  - **Gradient Progress Bar**: Implemented color transitions (green→yellow→orange→red) based on progress percentage
  - **Task Name Field**: Added task input prompt on timer start, displays above timer digits, provides foundation for future logging
  - **In-App Config Menu**: Full settings editor accessible with 'c' key, saves to `~/.pomodororc`, handles theme changes and timer resets intelligently
  - **Unicode Fix**: Switched to ncursesw for proper UTF-8 rendering of timer digits
  - All features tested and integrated on Rocky Linux 9.7
