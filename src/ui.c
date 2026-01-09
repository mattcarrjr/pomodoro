#include "ui.h"
#include "digits.h"
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

    attron(COLOR_PAIR(PAIR_PROGRESS));
    for (int i = 0; i < filled; i++) {
        mvaddch(bar_y, bar_x + i, ' ');
    }
    attroff(COLOR_PAIR(PAIR_PROGRESS));

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
