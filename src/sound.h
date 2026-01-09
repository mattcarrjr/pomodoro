#ifndef SOUND_H
#define SOUND_H

#include <stdbool.h>

/* Sound types */
typedef enum {
    SOUND_WORK_COMPLETE,
    SOUND_BREAK_COMPLETE,
    SOUND_LONG_BREAK_COMPLETE
} SoundType;

/* Sound context */
typedef struct {
    bool enabled;
    char sound_dir[256];
} SoundContext;

/* Initialize sound system */
int sound_init(SoundContext *ctx, const char *sound_dir);

/* Play a sound (non-blocking) */
void sound_play(SoundContext *ctx, SoundType type);

/* Play terminal bell as fallback */
void sound_bell(void);

/* Enable/disable sounds */
void sound_set_enabled(SoundContext *ctx, bool enabled);

#endif /* SOUND_H */
