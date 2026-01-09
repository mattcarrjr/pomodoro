#define _POSIX_C_SOURCE 200809L
#include "sound.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

/* Platform-specific sound command */
#ifdef __APPLE__
    #define SOUND_CMD "afplay"
#else
    /* Linux - try paplay first, fall back to aplay */
    #define SOUND_CMD "paplay"
    #define SOUND_CMD_ALT "aplay"
#endif

static const char *sound_files[] = {
    "sines/sines.ogg",
    "sines/sines.ogg",
    "sines/sines.ogg"
};

static bool file_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

int sound_init(SoundContext *ctx, const char *sound_dir)
{
    ctx->enabled = true;

    if (sound_dir && strlen(sound_dir) > 0) {
        strncpy(ctx->sound_dir, sound_dir, sizeof(ctx->sound_dir) - 1);
        ctx->sound_dir[sizeof(ctx->sound_dir) - 1] = '\0';
    } else {
        /* Default to current directory's sounds folder */
        strncpy(ctx->sound_dir, "./sounds", sizeof(ctx->sound_dir) - 1);
    }

    return 0;
}

void sound_play(SoundContext *ctx, SoundType type)
{
    if (!ctx->enabled) {
        sound_bell();
        return;
    }

    char path[512];
    snprintf(path, sizeof(path), "%s/%s", ctx->sound_dir, sound_files[type]);

    if (!file_exists(path)) {
        /* Sound file not found, use terminal bell */
        sound_bell();
        return;
    }

    /* Fork and play sound in background */
    pid_t pid = fork();
    if (pid == 0) {
        /* Child process */
        /* Redirect stdout/stderr to /dev/null */
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

#ifdef __APPLE__
        /* Try ffplay first (supports OGG), then mpv, then afplay */
        execlp("ffplay", "ffplay", "-nodisp", "-autoexit", "-loglevel", "quiet", path, NULL);
        execlp("mpv", "mpv", "--no-video", "--really-quiet", path, NULL);
        execlp(SOUND_CMD, SOUND_CMD, path, NULL);
#else
        /* Try paplay first */
        execlp(SOUND_CMD, SOUND_CMD, path, NULL);
        /* If that fails, try aplay */
        execlp(SOUND_CMD_ALT, SOUND_CMD_ALT, path, NULL);
#endif
        /* If exec fails, exit child */
        _exit(1);
    }
    /* Parent continues immediately (non-blocking) */
}

void sound_bell(void)
{
    /* Terminal bell character */
    putchar('\a');
    fflush(stdout);
}

void sound_set_enabled(SoundContext *ctx, bool enabled)
{
    ctx->enabled = enabled;
}
