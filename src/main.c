/*
 * Pomodoro Timer
 * A retro terminal-based Pomodoro timer using ncurses
 */

#define _XOPEN_SOURCE 700
#define _DARWIN_C_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <ncurses.h>

#include "timer.h"
#include "ui.h"
#include "sound.h"
#include "config.h"

/* Fallback for KEY_RESIZE if not defined */
#ifndef KEY_RESIZE
#define KEY_RESIZE 0632
#endif

static volatile sig_atomic_t running = 1;
static volatile sig_atomic_t resize_pending = 0;

static void handle_signal(int sig)
{
    if (sig == SIGINT || sig == SIGTERM) {
        running = 0;
    } else if (sig == SIGWINCH) {
        resize_pending = 1;
    }
}

static void setup_signals(void)
{
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGWINCH, &sa, NULL);
}

static SoundType get_sound_for_state(TimerState state)
{
    switch (state) {
        case STATE_WORKING:
            return SOUND_WORK_COMPLETE;
        case STATE_LONG_BREAK:
            return SOUND_LONG_BREAK_COMPLETE;
        default:
            return SOUND_BREAK_COMPLETE;
    }
}

int main(int argc, char **argv)
{
    AppConfig config;
    TimerContext timer;
    UIContext ui;
    SoundContext sound;

    /* Load configuration */
    if (config_load(&config, argc, argv) != 0) {
        fprintf(stderr, "Failed to load configuration\n");
        return 1;
    }

    /* Initialize subsystems */
    timer_init(&timer, &config.timer);
    sound_init(&sound, config.sound_dir);
    sound_set_enabled(&sound, config.sound_enabled);

    if (ui_init(&ui, config.theme) != 0) {
        fprintf(stderr, "Failed to initialize UI\n");
        return 1;
    }

    setup_signals();

    /* Main loop */
    while (running) {
        /* Handle terminal resize */
        if (resize_pending) {
            resize_pending = 0;
            ui_handle_resize(&ui);
        }

        /* Update timer state */
        timer_update(&timer);

        /* Draw UI */
        ui_draw(&ui, &timer);

        /* Check for completion */
        if (timer_just_completed(&timer)) {
            /* Play completion sound */
            sound_play(&sound, get_sound_for_state(timer.state));

            /* Show alert overlay */
            ui_draw_alert(&ui, &timer);

            /* Wait for spacebar to acknowledge */
            int ch;
            while (running && (ch = ui_get_input()) != POMO_KEY_ADVANCE) {
                if (ch == POMO_KEY_QUIT) {
                    running = 0;
                    break;
                }
                if (ch == KEY_RESIZE) {
                    ui_handle_resize(&ui);
                    ui_draw(&ui, &timer);
                    ui_draw_alert(&ui, &timer);
                }
                usleep(50000);  /* 50ms */
            }

            if (running) {
                /* Advance to next phase */
                timer_advance(&timer);
            }
        }

        /* Handle input */
        int ch = ui_get_input();
        switch (ch) {
            case POMO_KEY_QUIT:
                running = 0;
                break;

            case POMO_KEY_START:
                timer_start(&timer);
                break;

            case POMO_KEY_PAUSE:
                timer_toggle_pause(&timer);
                break;

            case POMO_KEY_RESET:
                timer_reset(&timer);
                break;

            case KEY_RESIZE:
                ui_handle_resize(&ui);
                break;

            default:
                break;
        }

        /* Small delay to prevent busy-waiting */
        usleep(100000);  /* 100ms */
    }

    /* Cleanup */
    ui_cleanup();

    printf("Pomodoro timer finished.\n");
    printf("Total work cycles completed: %d\n", timer.total_cycles_completed);

    return 0;
}
