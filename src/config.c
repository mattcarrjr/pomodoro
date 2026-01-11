#define _POSIX_C_SOURCE 200809L
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

AppConfig config_default(void)
{
    AppConfig config = {
        .timer = {
            .work_duration = 25,
            .short_break = 5,
            .long_break = 60,
            .cycles_before_long = 4
        },
        .theme = THEME_GREEN,
        .sound_enabled = true,
        .sound_dir = "./sounds"
    };
    return config;
}

static char *trim(char *str)
{
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;

    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';

    return str;
}

static int parse_config_file(AppConfig *config, const char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp) {
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        /* Skip comments and empty lines */
        char *trimmed = trim(line);
        if (trimmed[0] == '#' || trimmed[0] == '\0') {
            continue;
        }

        char *eq = strchr(trimmed, '=');
        if (!eq) continue;

        *eq = '\0';
        char *key = trim(trimmed);
        char *value = trim(eq + 1);

        if (strcmp(key, "work_duration") == 0) {
            config->timer.work_duration = atoi(value);
        } else if (strcmp(key, "short_break") == 0) {
            config->timer.short_break = atoi(value);
        } else if (strcmp(key, "long_break") == 0) {
            config->timer.long_break = atoi(value);
        } else if (strcmp(key, "cycles") == 0) {
            config->timer.cycles_before_long = atoi(value);
        } else if (strcmp(key, "theme") == 0) {
            if (strcmp(value, "green") == 0) {
                config->theme = THEME_GREEN;
            } else if (strcmp(value, "amber") == 0) {
                config->theme = THEME_AMBER;
            } else if (strcmp(value, "cyan") == 0) {
                config->theme = THEME_CYAN;
            } else if (strcmp(value, "white") == 0) {
                config->theme = THEME_WHITE;
            } else if (strcmp(value, "purple") == 0) {
                config->theme = THEME_PURPLE;
            } else if (strcmp(value, "red") == 0) {
                config->theme = THEME_RED;
            } else {
                config->theme = THEME_GREEN;
            }
        } else if (strcmp(key, "sound_enabled") == 0) {
            config->sound_enabled = (strcmp(value, "true") == 0 ||
                                     strcmp(value, "1") == 0);
        } else if (strcmp(key, "sound_dir") == 0) {
            strncpy(config->sound_dir, value, sizeof(config->sound_dir) - 1);
        }
    }

    fclose(fp);
    return 0;
}

void config_print_usage(const char *prog_name)
{
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("\n");
    printf("A retro terminal-based Pomodoro timer.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -w MINS    Work duration in minutes (default: 25)\n");
    printf("  -s MINS    Short break duration in minutes (default: 5)\n");
    printf("  -l MINS    Long break duration in minutes (default: 60)\n");
    printf("  -c NUM     Cycles before long break (default: 4)\n");
    printf("  -t THEME   Color theme: green, amber, cyan, white, purple, red (default: green)\n");
    printf("  -d DIR     Sound files directory\n");
    printf("  -n         Disable sounds (use terminal bell)\n");
    printf("  -h         Show this help message\n");
    printf("\n");
    printf("Controls:\n");
    printf("  s          Start timer\n");
    printf("  p          Pause/unpause\n");
    printf("  r          Reset timer\n");
    printf("  SPACE      Acknowledge completion and advance\n");
    printf("  q          Quit\n");
    printf("\n");
    printf("Config file: ~/.pomodororc\n");
}

int config_load(AppConfig *config, int argc, char **argv)
{
    /* Start with defaults */
    *config = config_default();

    /* Try to load config file */
    char config_path[512];
    const char *home = getenv("HOME");
    if (home) {
        snprintf(config_path, sizeof(config_path), "%s/.pomodororc", home);
        parse_config_file(config, config_path);

        /* Also try XDG config location */
        snprintf(config_path, sizeof(config_path),
                 "%s/.config/pomodoro/config", home);
        parse_config_file(config, config_path);
    }

    /* Parse command-line arguments (override config file) */
    int opt;
    while ((opt = getopt(argc, argv, "w:s:l:c:t:d:nh")) != -1) {
        switch (opt) {
            case 'w':
                config->timer.work_duration = atoi(optarg);
                break;
            case 's':
                config->timer.short_break = atoi(optarg);
                break;
            case 'l':
                config->timer.long_break = atoi(optarg);
                break;
            case 'c':
                config->timer.cycles_before_long = atoi(optarg);
                break;
            case 't':
                if (strcmp(optarg, "green") == 0) {
                    config->theme = THEME_GREEN;
                } else if (strcmp(optarg, "amber") == 0) {
                    config->theme = THEME_AMBER;
                } else if (strcmp(optarg, "cyan") == 0) {
                    config->theme = THEME_CYAN;
                } else if (strcmp(optarg, "white") == 0) {
                    config->theme = THEME_WHITE;
                } else if (strcmp(optarg, "purple") == 0) {
                    config->theme = THEME_PURPLE;
                } else if (strcmp(optarg, "red") == 0) {
                    config->theme = THEME_RED;
                } else {
                    config->theme = THEME_GREEN;
                }
                break;
            case 'd':
                strncpy(config->sound_dir, optarg,
                        sizeof(config->sound_dir) - 1);
                break;
            case 'n':
                config->sound_enabled = false;
                break;
            case 'h':
                config_print_usage(argv[0]);
                exit(0);
            default:
                config_print_usage(argv[0]);
                exit(1);
        }
    }

    /* Validate configuration */
    if (config->timer.work_duration < 1) config->timer.work_duration = 1;
    if (config->timer.short_break < 1) config->timer.short_break = 1;
    if (config->timer.long_break < 1) config->timer.long_break = 1;
    if (config->timer.cycles_before_long < 1) config->timer.cycles_before_long = 1;

    return 0;
}

int config_save(const AppConfig *config, const char *path)
{
    FILE *fp = fopen(path, "w");
    if (!fp) {
        return -1;
    }

    /* Write header comment */
    fprintf(fp, "# Pomodoro Timer Configuration\n");
    fprintf(fp, "# This file is automatically generated by the pomodoro timer\n\n");

    /* Write timer settings */
    fprintf(fp, "# Timer durations (in minutes)\n");
    fprintf(fp, "work_duration=%d\n", config->timer.work_duration);
    fprintf(fp, "short_break=%d\n", config->timer.short_break);
    fprintf(fp, "long_break=%d\n", config->timer.long_break);
    fprintf(fp, "cycles_before_long=%d\n\n", config->timer.cycles_before_long);

    /* Write theme setting */
    fprintf(fp, "# Color theme: green, amber, cyan, white, purple, red\n");
    fprintf(fp, "theme=");
    switch (config->theme) {
        case THEME_GREEN:
            fprintf(fp, "green\n");
            break;
        case THEME_AMBER:
            fprintf(fp, "amber\n");
            break;
        case THEME_CYAN:
            fprintf(fp, "cyan\n");
            break;
        case THEME_WHITE:
            fprintf(fp, "white\n");
            break;
        case THEME_PURPLE:
            fprintf(fp, "purple\n");
            break;
        case THEME_RED:
            fprintf(fp, "red\n");
            break;
        default:
            fprintf(fp, "green\n");
            break;
    }
    fprintf(fp, "\n");

    /* Write sound settings */
    fprintf(fp, "# Sound settings\n");
    fprintf(fp, "sound_enabled=%s\n", config->sound_enabled ? "true" : "false");
    fprintf(fp, "sound_dir=%s\n", config->sound_dir);

    fclose(fp);
    return 0;
}
