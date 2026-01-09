#ifndef CONFIG_H
#define CONFIG_H

#include "timer.h"
#include "ui.h"
#include <stdbool.h>

/* Full application configuration */
typedef struct AppConfig {
    TimerConfig timer;
    ColorTheme theme;
    bool sound_enabled;
    char sound_dir[256];
} AppConfig;

/* Load configuration from file and command-line args */
int config_load(AppConfig *config, int argc, char **argv);

/* Save configuration to file */
int config_save(const AppConfig *config, const char *path);

/* Get default configuration */
AppConfig config_default(void);

/* Print usage/help */
void config_print_usage(const char *prog_name);

#endif /* CONFIG_H */
