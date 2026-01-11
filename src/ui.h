#ifndef UI_H
#define UI_H

#include "timer.h"
#include <stdbool.h>

/* Forward declaration to avoid circular dependency */
typedef struct AppConfig AppConfig;

/* Color themes */
typedef enum {
    THEME_GREEN,
    THEME_AMBER,
    THEME_CYAN,
    THEME_WHITE,
    THEME_PURPLE,
    THEME_RED
} ColorTheme;

/* UI context */
typedef struct {
    ColorTheme theme;
    int term_width;
    int term_height;
    bool needs_redraw;
    bool show_help;
    bool in_config_menu;
} UIContext;

/* Initialize ncurses and UI */
int ui_init(UIContext *ctx, ColorTheme theme);

/* Cleanup ncurses */
void ui_cleanup(void);

/* Handle terminal resize */
void ui_handle_resize(UIContext *ctx);

/* Draw the complete interface */
void ui_draw(UIContext *ctx, TimerContext *timer);

/* Draw completion alert overlay */
void ui_draw_alert(UIContext *ctx, TimerContext *timer);

/* Get keyboard input (non-blocking) */
int ui_get_input(void);

/* Set color theme */
void ui_set_theme(UIContext *ctx, ColorTheme theme);

/* Prompt user for task name */
bool ui_prompt_task_name(char *task_buffer, size_t buffer_size);

/* Config menu functions */
void ui_draw_config_menu(UIContext *ctx, AppConfig *config, int selected_item, bool editing);
int ui_handle_config_input(int ch, AppConfig *config, int *selected_item, bool *editing, bool *needs_save);

/* Key constants (prefixed to avoid ncurses conflicts) */
#define POMO_KEY_QUIT    'q'
#define POMO_KEY_START   's'
#define POMO_KEY_PAUSE   'p'
#define POMO_KEY_RESET   'r'
#define POMO_KEY_CONFIG  'c'
#define POMO_KEY_HELP    'h'
#define POMO_KEY_TOGGLE  ' '  /* spacebar to start/pause and acknowledge */

#endif /* UI_H */
