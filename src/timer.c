#include "timer.h"
#include <string.h>

static bool session_completed = false;

TimerConfig timer_default_config(void)
{
    TimerConfig config = {
        .work_duration = 25,
        .short_break = 5,
        .long_break = 60,
        .cycles_before_long = 4
    };
    return config;
}

void timer_init(TimerContext *ctx, TimerConfig *config)
{
    memset(ctx, 0, sizeof(TimerContext));

    if (config) {
        ctx->config = *config;
    } else {
        ctx->config = timer_default_config();
    }

    ctx->state = STATE_IDLE;
    ctx->current_cycle = 1;
    ctx->total_cycles_completed = 0;
}

void timer_reset(TimerContext *ctx)
{
    TimerConfig saved_config = ctx->config;
    timer_init(ctx, &saved_config);
}

void timer_start(TimerContext *ctx)
{
    if (ctx->state == STATE_IDLE) {
        ctx->state = STATE_WORKING;
        ctx->duration_seconds = ctx->config.work_duration * 60;
        ctx->start_time = time(NULL);
        ctx->elapsed_paused = 0;
        session_completed = false;
    }
}

void timer_toggle_pause(TimerContext *ctx)
{
    if (ctx->state == STATE_PAUSED) {
        /* Unpause - accumulate paused time */
        ctx->elapsed_paused += (int)(time(NULL) - ctx->pause_time);
        ctx->state = ctx->paused_state;
    } else if (ctx->state != STATE_IDLE) {
        /* Pause */
        ctx->paused_state = ctx->state;
        ctx->pause_time = time(NULL);
        ctx->state = STATE_PAUSED;
    }
}

int timer_get_remaining(TimerContext *ctx)
{
    if (ctx->state == STATE_IDLE) {
        return ctx->config.work_duration * 60;
    }

    if (ctx->state == STATE_PAUSED) {
        int elapsed = (int)(ctx->pause_time - ctx->start_time) - ctx->elapsed_paused;
        int remaining = ctx->duration_seconds - elapsed;
        return remaining > 0 ? remaining : 0;
    }

    time_t now = time(NULL);
    int elapsed = (int)(now - ctx->start_time) - ctx->elapsed_paused;
    int remaining = ctx->duration_seconds - elapsed;

    return remaining > 0 ? remaining : 0;
}

int timer_get_progress(TimerContext *ctx)
{
    if (ctx->state == STATE_IDLE || ctx->duration_seconds == 0) {
        return 0;
    }

    int remaining = timer_get_remaining(ctx);
    int elapsed = ctx->duration_seconds - remaining;

    return (elapsed * 100) / ctx->duration_seconds;
}

void timer_update(TimerContext *ctx)
{
    if (ctx->state == STATE_IDLE || ctx->state == STATE_PAUSED) {
        return;
    }

    int remaining = timer_get_remaining(ctx);

    if (remaining <= 0 && !session_completed) {
        session_completed = true;
    }
}

bool timer_just_completed(TimerContext *ctx)
{
    (void)ctx;  /* reserved for future use */
    return session_completed;
}

void timer_advance(TimerContext *ctx)
{
    session_completed = false;

    switch (ctx->state) {
        case STATE_WORKING:
            if (ctx->current_cycle >= ctx->config.cycles_before_long) {
                /* Time for long break */
                ctx->state = STATE_LONG_BREAK;
                ctx->duration_seconds = ctx->config.long_break * 60;
            } else {
                /* Short break */
                ctx->state = STATE_SHORT_BREAK;
                ctx->duration_seconds = ctx->config.short_break * 60;
            }
            ctx->total_cycles_completed++;
            break;

        case STATE_SHORT_BREAK:
            ctx->current_cycle++;
            ctx->state = STATE_WORKING;
            ctx->duration_seconds = ctx->config.work_duration * 60;
            break;

        case STATE_LONG_BREAK:
            ctx->current_cycle = 1;
            ctx->state = STATE_WORKING;
            ctx->duration_seconds = ctx->config.work_duration * 60;
            break;

        default:
            return;
    }

    ctx->start_time = time(NULL);
    ctx->elapsed_paused = 0;
}

const char *timer_state_string(TimerState state)
{
    switch (state) {
        case STATE_IDLE:        return "IDLE";
        case STATE_WORKING:     return "WORK";
        case STATE_SHORT_BREAK: return "SHORT BREAK";
        case STATE_LONG_BREAK:  return "LONG BREAK";
        case STATE_PAUSED:      return "PAUSED";
        default:                return "UNKNOWN";
    }
}
