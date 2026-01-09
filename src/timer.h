#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include <stdbool.h>

/* Timer states */
typedef enum {
    STATE_IDLE,
    STATE_WORKING,
    STATE_SHORT_BREAK,
    STATE_LONG_BREAK,
    STATE_PAUSED
} TimerState;

/* Timer configuration */
typedef struct {
    int work_duration;      /* in minutes */
    int short_break;        /* in minutes */
    int long_break;         /* in minutes */
    int cycles_before_long; /* number of work sessions before long break */
} TimerConfig;

/* Timer context */
typedef struct {
    TimerState state;
    TimerState paused_state;    /* state before pause */
    int current_cycle;          /* 1 to cycles_before_long */
    int total_cycles_completed;
    time_t start_time;
    time_t pause_time;
    int duration_seconds;       /* current session duration */
    int elapsed_paused;         /* accumulated pause time */
    TimerConfig config;
    char current_task[256];     /* current task being worked on */
} TimerContext;

/* Initialize timer with default or custom config */
void timer_init(TimerContext *ctx, TimerConfig *config);

/* Reset timer to initial state */
void timer_reset(TimerContext *ctx);

/* Start the timer (begin work session) */
void timer_start(TimerContext *ctx);

/* Pause/unpause the timer */
void timer_toggle_pause(TimerContext *ctx);

/* Update timer state - call this in main loop */
void timer_update(TimerContext *ctx);

/* Get remaining seconds in current session */
int timer_get_remaining(TimerContext *ctx);

/* Get progress as percentage (0-100) */
int timer_get_progress(TimerContext *ctx);

/* Check if timer just completed (transitioned) */
bool timer_just_completed(TimerContext *ctx);

/* Acknowledge completion and advance to next state */
void timer_advance(TimerContext *ctx);

/* Get string representation of current state */
const char *timer_state_string(TimerState state);

/* Get default configuration */
TimerConfig timer_default_config(void);

/* Set current task name */
void timer_set_task(TimerContext *ctx, const char *task);

#endif /* TIMER_H */
