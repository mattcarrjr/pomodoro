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
│   ├── main.c      # Main application entry point and event loop
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
./pomodoro -t purple    # Options: green, amber, cyan, white, purple, red

# Disable sounds (use terminal bell)
./pomodoro -n

# Show help
./pomodoro -h
```

### Interactive Controls
- `SPACE` - Start timer / Pause/Resume / Acknowledge completion alerts
- `s` - Start timer (legacy, same as spacebar)
- `p` - Pause/unpause (legacy, same as spacebar)
- `r` - Reset timer
- `c` - Open config menu to change settings
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

### 1. Color Themes
Six color themes available, each applying a consistent color across all UI elements:

| Theme  | Color   | Best For |
|--------|---------|----------|
| green  | Green   | Default, easy on eyes |
| amber  | Yellow  | Warm, retro feel |
| cyan   | Cyan    | Cool, calming |
| white  | White   | High contrast |
| purple | Magenta | Unique, stylish |
| red    | Red     | Bold, urgent |

Theme colors apply to:
- Border and box-drawing characters
- Title and status text
- Large digit display
- Progress bar (solid fill)
- Percentage indicator
- Task name and cycle counter

### 2. Progress Bar
A solid-color progress bar shows session completion:
- Width matches the digit counter for visual alignment
- Filled portion uses theme color as background
- Unfilled portion uses dimmed checkerboard pattern
- Percentage displayed in theme color to the right

### 3. Task Name Field
Track what you're working on during each Pomodoro session:

- When you press **SPACE** to start from IDLE, you'll be prompted: "Enter Task Name"
- Type your task description (e.g., "Write documentation", "Fix bug #123")
- Press **Enter** to confirm or **Esc** to skip
- The task name displays centered between the header and timer digits
- Task name persists through work/break cycles
- Shows "Task: (none)" if no task was entered
- Task name is cleared when you reset the timer with **`r`**

### 4. In-App Config Menu
Change all your settings without leaving the application:

**Access**: Press **`c`** at any time to open the settings menu

**Available Settings**:
- Work Duration (minutes)
- Short Break (minutes)
- Long Break (minutes)
- Cycles Before Long Break (number)
- Theme (green, amber, cyan, white, purple, red)

**Navigation**:
- **↑/↓ or j/k**: Move between settings
- **Enter**: Start editing the selected field
- **Numbers**: Type to set duration values (when editing)
- **←/→ or h/l**: Cycle through theme options (when editing theme)
- **s**: Save changes to `~/.pomodororc` and exit
- **Esc**: Cancel changes and exit without saving

**Behavior**:
- **Theme changes preview immediately** as you cycle through options
- Canceling with Esc restores the original theme
- **Time setting changes** while timer is IDLE: Applied to next session
- **Time setting changes** while timer is ACTIVE: Timer automatically resets to apply new durations
- All saved settings persist between application runs via `~/.pomodororc`

### 5. Responsive Terminal Resize
The UI properly handles terminal window resizing:
- All elements reposition and redraw when the terminal is resized
- Works in both main view and config menu
- Uses proper ncurses resize pattern (endwin/refresh) for reliable updates

### 6. UI Layout
The interface is carefully laid out for readability:

```
┌─────────────────────────────────────────────────────────────────┐
│  POMODORO TIMER                              [WORKING] 1/4      │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│                     Task: Write documentation                   │
│                                                                 │
│                        ██  ██     ██  ██                        │
│                       █  █   █   █  █ ██                        │
│                       ████  █    ████  █                        │
│                       █  █ █     █  █  █                        │
│                       ██  █████  ██  █████                      │
│                                                                 │
│                       ████████████░░░░░░░░  75%                 │
│                                                                 │
│                        Total completed: 3                       │
│                                                                 │
├─────────────────────────────────────────────────────────────────┤
│            [SPACE] Start/Pause  [R]eset  [C]onfig  [Q]uit       │
└─────────────────────────────────────────────────────────────────┘
```

- **Header**: Title on left, state and cycle count on right
- **Task**: Centered between header line and digit display
- **Digits**: Large ASCII art timer, centered
- **Progress bar**: Same width as digits, centered below
- **Cycles counter**: Centered between progress bar and footer
- **Footer**: Key bindings centered at bottom

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

### Cross-Platform Distribution
To distribute for multiple platforms, you must build separately:
- Build on Linux → `pomodoro-linux`
- Build on macOS → `pomodoro-macos`

Consider using GitHub Actions to automate builds for both platforms on each release.

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

### UI becomes garbled on terminal resize
**Problem**: Older versions had incomplete resize handling.

**Solution**: Current version uses proper ncurses resize pattern with `endwin()` + `refresh()` before getting new dimensions. Ensure you have the latest code.

## Technical Implementation Notes

### Color System
- **Files**: `src/ui.c`, `src/ui.h`
- **Implementation**: Uses ncurses 8-color palette with 7 color pairs
- **Color pairs**:
  - `PAIR_NORMAL` - Theme foreground on default background
  - `PAIR_BORDER` - Theme color for borders
  - `PAIR_TITLE` - Theme color for titles
  - `PAIR_DIGITS` - Theme color for large digits
  - `PAIR_PROGRESS` - Black text on theme color background (for filled bar)
  - `PAIR_ALERT` - Red for alert dialogs
  - `PAIR_DIM` - Black for unfilled progress bar
- **Theme switching**: `ui_set_theme()` reinitializes all color pairs

### Progress Bar
- **Width calculation**: Matches digit display width: `(4 * digit_width()) + colon_width() + (3 * spacing)`
- **Fill calculation**: `filled = (progress * bar_width) / 100`
- **Filled portion**: Space characters with `PAIR_PROGRESS` (theme background)
- **Unfilled portion**: `ACS_CKBOARD` (checkerboard) with `PAIR_DIM`

### Terminal Resize Handling
- **Signal**: `SIGWINCH` sets `resize_pending` flag
- **KEY_RESIZE**: Also handled from `getch()` return value
- **Handler pattern**:
  ```c
  void ui_handle_resize(UIContext *ctx) {
      endwin();    // Suspend ncurses
      refresh();   // Reinitialize ncurses with new size
      getmaxyx(stdscr, ctx->term_height, ctx->term_width);
      clear();
      ctx->needs_redraw = true;
  }
  ```

### Task Name Field
- **Files**: `src/timer.h`, `src/timer.c`, `src/ui.c`, `src/main.c`
- **Storage**: `char current_task[256]` field in `TimerContext` struct
- **Input**: `ui_prompt_task_name()` creates modal dialog with text input
- **Display**: Rendered in `ui_draw()` centered between header (y=4) and digit start

### Config Menu
- **Files**: `src/config.h`, `src/config.c`, `src/ui.c`, `src/main.c`
- **Theme preview**: Calls `ui_set_theme()` immediately when theme changes in edit mode
- **Cancel behavior**: Restores original config and theme on Esc
- **Config saving**: `config_save()` writes to `~/.pomodororc` in key=value format

### ncurses Wide Character Support
- **Makefile**: Links with `-lncursesw` for UTF-8 support
- **Locale**: `setlocale(LC_ALL, "")` called in `ui_init()`
- **Result**: Proper rendering of Unicode box-drawing and block characters

## Future Enhancements

Potential features for future development:

1. **Task logging for Obsidian** - Log completed sessions with timestamps and task names to markdown for productivity tracking

2. **Sound customization** - Allow users to specify custom sound files in config

3. **Statistics view** - Show daily/weekly productivity stats

4. **Notification integration** - System notifications on timer completion

5. **GitHub Actions CI** - Automated builds for Linux and macOS releases

## Development History

- **2026-01-09**: Initial cross-platform build fixes
  - Added POSIX feature test macros to `config.c` and `sound.c`
  - Documented binary compatibility requirements
  - Verified successful build on Rocky Linux 9.7 with gcc 11.5.0

- **2026-01-09**: Major feature additions
  - Gradient progress bar (later simplified)
  - Task name field with input prompt
  - In-app config menu with live editing
  - Unicode fix with ncursesw

- **2026-01-10**: UI refinements and simplification
  - Replaced gradient progress bar with solid theme-colored bar
  - Simplified to 6 color themes: green, amber, cyan, white, purple, red
  - Spacebar as primary start/pause/acknowledge control
  - Progress bar width matches digit counter, brackets removed
  - Percentage indicator in theme color
  - Task header centered between top UI and digits
  - Cycles counter centered between progress bar and footer
  - Immediate theme preview in config menu
  - Proper terminal resize handling with endwin/refresh pattern
  - Footer updated: `[SPACE] Start/Pause  [R]eset  [C]onfig  [Q]uit`
