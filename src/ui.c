#include "ui.h"
#include "digits.h"
#include "config.h"
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

/* Color pair definitions */
#define PAIR_NORMAL   1
#define PAIR_BORDER   2
#define PAIR_TITLE    3
#define PAIR_DIGITS   4
#define PAIR_PROGRESS 5
#define PAIR_ALERT    6
#define PAIR_DIM      7
#define PAIR_PROGRESS_GREEN  8
#define PAIR_PROGRESS_YELLOW 9
#define PAIR_PROGRESS_ORANGE 10
#define PAIR_PROGRESS_RED    11

/* Minimum terminal size */
#define MIN_WIDTH  60
#define MIN_HEIGHT 20

static void setup_colors(ColorTheme theme)
{
    start_color();
    use_default_colors();

    short fg;
    switch (theme) {
        case THEME_GREEN:
            fg = COLOR_GREEN;
            break;
        case THEME_AMBER:
            fg = COLOR_YELLOW;
            break;
        case THEME_CYAN:
            fg = COLOR_CYAN;
            break;
        case THEME_WHITE:
        default:
            fg = COLOR_WHITE;
            break;
    }

    init_pair(PAIR_NORMAL, fg, -1);
    init_pair(PAIR_BORDER, fg, -1);
    init_pair(PAIR_TITLE, fg, -1);
    init_pair(PAIR_DIGITS, fg, -1);
    init_pair(PAIR_PROGRESS, COLOR_BLACK, fg);
    init_pair(PAIR_ALERT, COLOR_BLACK, COLOR_RED);
    init_pair(PAIR_DIM, COLOR_BLACK, -1);

    /* Gradient progress bar colors (independent of theme) */
    init_pair(PAIR_PROGRESS_GREEN, COLOR_BLACK, COLOR_GREEN);
    init_pair(PAIR_PROGRESS_YELLOW, COLOR_BLACK, COLOR_YELLOW);
    init_pair(PAIR_PROGRESS_ORANGE, COLOR_BLACK, COLOR_YELLOW);  /* Will use bright attribute */
    init_pair(PAIR_PROGRESS_RED, COLOR_BLACK, COLOR_RED);
}

int ui_init(UIContext *ctx, ColorTheme theme)
{
    /* Set locale for UTF-8 support */
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);  /* non-blocking input */

    if (has_colors()) {
        setup_colors(theme);
    }

    ctx->theme = theme;
    ctx->needs_redraw = true;
    ctx->show_help = false;
    getmaxyx(stdscr, ctx->term_height, ctx->term_width);

    return 0;
}

void ui_cleanup(void)
{
    endwin();
}

void ui_handle_resize(UIContext *ctx)
{
    getmaxyx(stdscr, ctx->term_height, ctx->term_width);
    clear();
    ctx->needs_redraw = true;
}

void ui_set_theme(UIContext *ctx, ColorTheme theme)
{
    ctx->theme = theme;
    setup_colors(theme);
    ctx->needs_redraw = true;
}

static void draw_box(int y, int x, int h, int w)
{
    /* Draw corners */
    mvaddch(y, x, ACS_ULCORNER);
    mvaddch(y, x + w - 1, ACS_URCORNER);
    mvaddch(y + h - 1, x, ACS_LLCORNER);
    mvaddch(y + h - 1, x + w - 1, ACS_LRCORNER);

    /* Draw horizontal lines */
    for (int i = 1; i < w - 1; i++) {
        mvaddch(y, x + i, ACS_HLINE);
        mvaddch(y + h - 1, x + i, ACS_HLINE);
    }

    /* Draw vertical lines */
    for (int i = 1; i < h - 1; i++) {
        mvaddch(y + i, x, ACS_VLINE);
        mvaddch(y + i, x + w - 1, ACS_VLINE);
    }
}

static void draw_digit(int y, int x, int digit)
{
    const char **art = digit_get(digit);
    if (!art) return;

    attron(COLOR_PAIR(PAIR_DIGITS) | A_BOLD);
    for (int i = 0; i < digit_height(); i++) {
        mvprintw(y + i, x, "%s", art[i]);
    }
    attroff(COLOR_PAIR(PAIR_DIGITS) | A_BOLD);
}

static void draw_colon(int y, int x)
{
    const char **art = digit_get_colon();
    attron(COLOR_PAIR(PAIR_DIGITS) | A_BOLD);
    for (int i = 0; i < digit_height(); i++) {
        mvprintw(y + i, x, "%s", art[i]);
    }
    attroff(COLOR_PAIR(PAIR_DIGITS) | A_BOLD);
}

static void draw_time(UIContext *ctx, int seconds)
{
    int minutes = seconds / 60;
    int secs = seconds % 60;

    int d1 = minutes / 10;
    int d2 = minutes % 10;
    int d3 = secs / 10;
    int d4 = secs % 10;

    /* Spacing between digits for better legibility */
    int spacing = 2;

    /* Calculate total width: 4 digits + 1 colon + spacing */
    int total_width = (4 * digit_width()) + colon_width() + (3 * spacing);
    int start_x = (ctx->term_width - total_width) / 2;
    int start_y = (ctx->term_height - digit_height()) / 2 - 2;

    int x = start_x;
    draw_digit(start_y, x, d1);
    x += digit_width() + spacing;
    draw_digit(start_y, x, d2);
    x += digit_width() + spacing;
    draw_colon(start_y, x);
    x += colon_width() + spacing;
    draw_digit(start_y, x, d3);
    x += digit_width() + spacing;
    draw_digit(start_y, x, d4);
}

static void draw_progress_bar(UIContext *ctx, int progress)
{
    int bar_width = ctx->term_width - 20;
    int bar_y = ctx->term_height / 2 + 5;
    int bar_x = (ctx->term_width - bar_width) / 2;

    int filled = (progress * bar_width) / 100;

    mvprintw(bar_y, bar_x - 1, "[");

    /* Draw gradient progress bar */
    for (int i = 0; i < filled; i++) {
        /* Calculate position percentage within the bar */
        int pos_percent = (i * 100) / bar_width;

        /* Select color based on position */
        int color_pair;
        int attr = 0;
        if (pos_percent < 50) {
            color_pair = PAIR_PROGRESS_GREEN;
        } else if (pos_percent < 75) {
            color_pair = PAIR_PROGRESS_YELLOW;
        } else if (pos_percent < 90) {
            color_pair = PAIR_PROGRESS_ORANGE;
            attr = A_BOLD;  /* Make orange brighter */
        } else {
            color_pair = PAIR_PROGRESS_RED;
        }

        attron(COLOR_PAIR(color_pair) | attr);
        mvaddch(bar_y, bar_x + i, ' ');
        attroff(COLOR_PAIR(color_pair) | attr);
    }

    attron(COLOR_PAIR(PAIR_DIM));
    for (int i = filled; i < bar_width; i++) {
        mvaddch(bar_y, bar_x + i, ACS_CKBOARD);
    }
    attroff(COLOR_PAIR(PAIR_DIM));

    mvprintw(bar_y, bar_x + bar_width, "] %3d%%", progress);
}

static void draw_header(UIContext *ctx, TimerContext *timer)
{
    attron(COLOR_PAIR(PAIR_TITLE) | A_BOLD);

    /* Title */
    const char *title = "POMODORO TIMER";
    mvprintw(2, (ctx->term_width - (int)strlen(title)) / 2, "%s", title);

    /* State indicator */
    const char *state = timer_state_string(timer->state);
    char status[64];
    snprintf(status, sizeof(status), "[%s] %d/%d",
             state, timer->current_cycle, timer->config.cycles_before_long);

    mvprintw(2, ctx->term_width - (int)strlen(status) - 3, "%s", status);

    attroff(COLOR_PAIR(PAIR_TITLE) | A_BOLD);

    /* Separator line */
    attron(COLOR_PAIR(PAIR_BORDER));
    mvhline(3, 1, ACS_HLINE, ctx->term_width - 2);
    attroff(COLOR_PAIR(PAIR_BORDER));
}

static void draw_footer(UIContext *ctx)
{
    int y = ctx->term_height - 2;

    attron(COLOR_PAIR(PAIR_BORDER));
    mvhline(y - 1, 1, ACS_HLINE, ctx->term_width - 2);
    attroff(COLOR_PAIR(PAIR_BORDER));

    attron(COLOR_PAIR(PAIR_NORMAL));
    const char *help = "[S]tart  [P]ause  [R]eset  [H]elp  [Q]uit";
    mvprintw(y, (ctx->term_width - (int)strlen(help)) / 2, "%s", help);
    attroff(COLOR_PAIR(PAIR_NORMAL));
}

void ui_draw(UIContext *ctx, TimerContext *timer)
{
    erase();

    attron(COLOR_PAIR(PAIR_BORDER));
    draw_box(0, 0, ctx->term_height, ctx->term_width);
    attroff(COLOR_PAIR(PAIR_BORDER));

    draw_header(ctx, timer);
    draw_footer(ctx);

    int remaining = timer_get_remaining(timer);
    int progress = timer_get_progress(timer);

    /* Display task name above timer */
    int task_y = (ctx->term_height - digit_height()) / 2 - 4;
    attron(COLOR_PAIR(PAIR_NORMAL));
    if (timer->current_task[0] != '\0') {
        char task_label[300];
        snprintf(task_label, sizeof(task_label), "Task: %s", timer->current_task);
        mvprintw(task_y, (ctx->term_width - (int)strlen(task_label)) / 2, "%s", task_label);
    } else {
        const char *no_task = "Task: (none)";
        mvprintw(task_y, (ctx->term_width - (int)strlen(no_task)) / 2, "%s", no_task);
    }
    attroff(COLOR_PAIR(PAIR_NORMAL));

    draw_time(ctx, remaining);
    draw_progress_bar(ctx, progress);

    /* Show total completed cycles */
    char cycles_str[32];
    snprintf(cycles_str, sizeof(cycles_str), "Total completed: %d",
             timer->total_cycles_completed);
    attron(COLOR_PAIR(PAIR_NORMAL));
    mvprintw(ctx->term_height / 2 + 7,
             (ctx->term_width - (int)strlen(cycles_str)) / 2, "%s", cycles_str);
    attroff(COLOR_PAIR(PAIR_NORMAL));

    refresh();
    ctx->needs_redraw = false;
}

void ui_draw_alert(UIContext *ctx, TimerContext *timer)
{
    int box_h = 7;
    int box_w = 40;
    int box_y = (ctx->term_height - box_h) / 2;
    int box_x = (ctx->term_width - box_w) / 2;

    attron(COLOR_PAIR(PAIR_ALERT) | A_BOLD);

    /* Fill alert box */
    for (int i = 0; i < box_h; i++) {
        mvhline(box_y + i, box_x, ' ', box_w);
    }

    const char *line1 = "TIME'S UP!";
    const char *line2;

    switch (timer->state) {
        case STATE_WORKING:
            line2 = "Work session complete. Take a break!";
            break;
        case STATE_SHORT_BREAK:
        case STATE_LONG_BREAK:
            line2 = "Break over. Ready to work?";
            break;
        default:
            line2 = "Press SPACE to continue";
            break;
    }

    const char *line3 = "Press SPACE to continue";

    mvprintw(box_y + 2, box_x + (box_w - (int)strlen(line1)) / 2, "%s", line1);
    mvprintw(box_y + 3, box_x + (box_w - (int)strlen(line2)) / 2, "%s", line2);
    mvprintw(box_y + 5, box_x + (box_w - (int)strlen(line3)) / 2, "%s", line3);

    attroff(COLOR_PAIR(PAIR_ALERT) | A_BOLD);

    refresh();
}

bool ui_prompt_task_name(char *task_buffer, size_t buffer_size)
{
    int box_h = 7;
    int box_w = 60;
    int term_height, term_width;
    getmaxyx(stdscr, term_height, term_width);
    int box_y = (term_height - box_h) / 2;
    int box_x = (term_width - box_w) / 2;

    /* Draw input box */
    attron(COLOR_PAIR(PAIR_BORDER) | A_BOLD);
    for (int i = 0; i < box_h; i++) {
        mvhline(box_y + i, box_x, ' ', box_w);
    }

    /* Draw border */
    mvhline(box_y, box_x, ACS_HLINE, box_w);
    mvhline(box_y + box_h - 1, box_x, ACS_HLINE, box_w);
    mvvline(box_y, box_x, ACS_VLINE, box_h);
    mvvline(box_y, box_x + box_w - 1, ACS_VLINE, box_h);
    mvaddch(box_y, box_x, ACS_ULCORNER);
    mvaddch(box_y, box_x + box_w - 1, ACS_URCORNER);
    mvaddch(box_y + box_h - 1, box_x, ACS_LLCORNER);
    mvaddch(box_y + box_h - 1, box_x + box_w - 1, ACS_LRCORNER);

    const char *title = "Enter Task Name";
    mvprintw(box_y + 1, box_x + (box_w - (int)strlen(title)) / 2, "%s", title);

    const char *instructions = "[Enter] Confirm  [Esc] Cancel";
    mvprintw(box_y + box_h - 2, box_x + (box_w - (int)strlen(instructions)) / 2, "%s", instructions);

    attroff(COLOR_PAIR(PAIR_BORDER) | A_BOLD);

    /* Input field */
    int input_y = box_y + 3;
    int input_x = box_x + 2;
    int input_width = box_w - 4;

    mvhline(input_y, input_x, '_', input_width);

    /* Enable input mode */
    curs_set(1);
    echo();
    nodelay(stdscr, FALSE);

    /* Move cursor to input position */
    move(input_y, input_x);
    refresh();

    /* Read input with limited length */
    char input[256] = {0};
    int pos = 0;
    bool confirmed = false;

    while (1) {
        int ch = getch();

        if (ch == '\n' || ch == KEY_ENTER) {
            /* Confirm */
            confirmed = true;
            break;
        } else if (ch == 27) {  /* ESC */
            /* Cancel */
            confirmed = false;
            break;
        } else if ((ch == KEY_BACKSPACE || ch == 127 || ch == '\b') && pos > 0) {
            /* Backspace */
            pos--;
            input[pos] = '\0';
            mvaddch(input_y, input_x + pos, '_');
            move(input_y, input_x + pos);
        } else if (ch >= 32 && ch < 127 && pos < input_width - 1 && pos < 254) {
            /* Printable character */
            input[pos] = (char)ch;
            pos++;
            input[pos] = '\0';
        }

        /* Redraw input field */
        mvhline(input_y, input_x, '_', input_width);
        attron(COLOR_PAIR(PAIR_NORMAL) | A_BOLD);
        mvprintw(input_y, input_x, "%s", input);
        attroff(COLOR_PAIR(PAIR_NORMAL) | A_BOLD);
        move(input_y, input_x + pos);
        refresh();
    }

    /* Restore input mode */
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);

    /* Copy result if confirmed */
    if (confirmed && pos > 0) {
        snprintf(task_buffer, buffer_size, "%s", input);
        return true;
    }

    return false;
}

void ui_draw_config_menu(UIContext *ctx, AppConfig *config, int selected_item, bool editing)
{
    int menu_h = 16;
    int menu_w = 60;
    int menu_y = (ctx->term_height - menu_h) / 2;
    int menu_x = (ctx->term_width - menu_w) / 2;

    /* Draw menu box */
    erase();
    attron(COLOR_PAIR(PAIR_BORDER));
    draw_box(0, 0, ctx->term_height, ctx->term_width);
    attroff(COLOR_PAIR(PAIR_BORDER));

    attron(COLOR_PAIR(PAIR_BORDER) | A_BOLD);
    for (int i = 0; i < menu_h; i++) {
        mvhline(menu_y + i, menu_x, ' ', menu_w);
    }

    /* Draw border */
    mvhline(menu_y, menu_x, ACS_HLINE, menu_w);
    mvhline(menu_y + menu_h - 1, menu_x, ACS_HLINE, menu_w);
    mvvline(menu_y, menu_x, ACS_VLINE, menu_h);
    mvvline(menu_y, menu_x + menu_w - 1, ACS_VLINE, menu_h);
    mvaddch(menu_y, menu_x, ACS_ULCORNER);
    mvaddch(menu_y, menu_x + menu_w - 1, ACS_URCORNER);
    mvaddch(menu_y + menu_h - 1, menu_x, ACS_LLCORNER);
    mvaddch(menu_y + menu_h - 1, menu_x + menu_w - 1, ACS_LRCORNER);

    const char *title = "SETTINGS";
    mvprintw(menu_y + 1, menu_x + (menu_w - (int)strlen(title)) / 2, "%s", title);
    attroff(COLOR_PAIR(PAIR_BORDER) | A_BOLD);

    /* Menu items */
    const char *items[] = {
        "Work Duration",
        "Short Break",
        "Long Break",
        "Cycles Before Long Break",
        "Theme"
    };
    int num_items = 5;

    int item_y = menu_y + 3;
    for (int i = 0; i < num_items; i++) {
        if (i == selected_item) {
            attron(COLOR_PAIR(PAIR_PROGRESS) | A_BOLD);
            mvprintw(item_y + (i * 2), menu_x + 2, "> ");
        } else {
            attron(COLOR_PAIR(PAIR_NORMAL));
            mvprintw(item_y + (i * 2), menu_x + 2, "  ");
        }

        /* Display item name and value */
        char value_str[32];
        if (i == 0) {
            snprintf(value_str, sizeof(value_str), "%d min", config->timer.work_duration);
        } else if (i == 1) {
            snprintf(value_str, sizeof(value_str), "%d min", config->timer.short_break);
        } else if (i == 2) {
            snprintf(value_str, sizeof(value_str), "%d min", config->timer.long_break);
        } else if (i == 3) {
            snprintf(value_str, sizeof(value_str), "%d", config->timer.cycles_before_long);
        } else if (i == 4) {
            const char *theme_names[] = {"green", "amber", "cyan", "white"};
            snprintf(value_str, sizeof(value_str), "%s", theme_names[config->theme]);
        }

        mvprintw(item_y + (i * 2), menu_x + 4, "%-28s : %s", items[i], value_str);

        if (i == selected_item && editing) {
            mvprintw(item_y + (i * 2), menu_x + menu_w - 10, "[EDITING]");
        }

        attroff(COLOR_PAIR(PAIR_PROGRESS) | A_BOLD);
        attroff(COLOR_PAIR(PAIR_NORMAL));
    }

    /* Instructions */
    const char *instructions = "[↑↓] Navigate  [Enter] Edit  [S] Save  [Esc] Cancel";
    attron(COLOR_PAIR(PAIR_NORMAL));
    mvprintw(menu_y + menu_h - 2, menu_x + (menu_w - (int)strlen(instructions)) / 2, "%s", instructions);
    attroff(COLOR_PAIR(PAIR_NORMAL));

    refresh();
}

int ui_handle_config_input(int ch, AppConfig *config, int *selected_item, bool *editing, bool *needs_save)
{
    /* Return values: 0 = continue, 1 = save, -1 = cancel */

    if (*editing) {
        /* In editing mode */
        if (ch == '\n' || ch == KEY_ENTER || ch == 27) {  /* Enter or ESC to finish editing */
            *editing = false;
            return 0;
        }

        /* Handle numeric input for numeric fields */
        if (*selected_item <= 3) {  /* Numeric fields */
            int *value = NULL;
            if (*selected_item == 0) value = &config->timer.work_duration;
            else if (*selected_item == 1) value = &config->timer.short_break;
            else if (*selected_item == 2) value = &config->timer.long_break;
            else if (*selected_item == 3) value = &config->timer.cycles_before_long;

            if (ch >= '0' && ch <= '9') {
                int digit = ch - '0';
                *value = (*value * 10) + digit;
                if (*value > 999) *value = 999;  /* Cap at 999 */
            } else if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
                *value = *value / 10;
            }
        } else if (*selected_item == 4) {  /* Theme field */
            if (ch == KEY_LEFT || ch == 'h') {
                if (config->theme > 0) config->theme--;
                else config->theme = THEME_WHITE;
            } else if (ch == KEY_RIGHT || ch == 'l') {
                if (config->theme < THEME_WHITE) config->theme++;
                else config->theme = THEME_GREEN;
            }
        }
    } else {
        /* In navigation mode */
        if (ch == KEY_UP || ch == 'k') {
            if (*selected_item > 0) (*selected_item)--;
        } else if (ch == KEY_DOWN || ch == 'j') {
            if (*selected_item < 4) (*selected_item)++;
        } else if (ch == '\n' || ch == KEY_ENTER) {
            /* Start editing */
            *editing = true;
            /* Clear numeric value if starting to edit */
            if (*selected_item <= 3) {
                int *value = NULL;
                if (*selected_item == 0) value = &config->timer.work_duration;
                else if (*selected_item == 1) value = &config->timer.short_break;
                else if (*selected_item == 2) value = &config->timer.long_break;
                else if (*selected_item == 3) value = &config->timer.cycles_before_long;
                *value = 0;
            }
        } else if (ch == 's' || ch == 'S') {
            *needs_save = true;
            return 1;  /* Save and exit */
        } else if (ch == 27) {  /* ESC */
            return -1;  /* Cancel */
        }
    }

    return 0;  /* Continue */
}

int ui_get_input(void)
{
    int ch = getch();
    if (ch == ERR) {
        return -1;
    }
    /* Handle resize */
    if (ch == KEY_RESIZE) {
        return KEY_RESIZE;
    }
    /* Convert to lowercase */
    if (ch >= 'A' && ch <= 'Z') {
        ch = ch + ('a' - 'A');
    }
    return ch;
}
