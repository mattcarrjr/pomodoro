#include "digits.h"
#include <stddef.h>

/* 3D beveled digits using block + box-drawing characters */

static const char *digit_0[] = {
    " ██████╗ ",
    "██╔═══██╗",
    "██║   ██║",
    "██║   ██║",
    "██║   ██║",
    "╚██████╔╝",
    " ╚═════╝ "
};

static const char *digit_1[] = {
    "   ██╗   ",
    " ████║   ",
    " ╚═██║   ",
    "   ██║   ",
    "   ██║   ",
    "   ██║   ",
    "   ╚═╝   "
};

static const char *digit_2[] = {
    " ██████╗ ",
    "╚════██╗ ",
    "    ██╔╝ ",
    " █████╔╝ ",
    "██╔════╝ ",
    "████████╗",
    "╚═══════╝"
};

static const char *digit_3[] = {
    " ██████╗ ",
    "╚════██╗ ",
    "    ██╔╝ ",
    " █████╔╝ ",
    "╚════██╗ ",
    " █████╔╝ ",
    " ╚════╝  "
};

static const char *digit_4[] = {
    "██╗  ██╗ ",
    "██║  ██║ ",
    "██║  ██║ ",
    "████████╗",
    "╚═══██╔═╝",
    "    ██║  ",
    "    ╚═╝  "
};

static const char *digit_5[] = {
    "████████╗",
    "██╔═════╝",
    "███████╗ ",
    "╚════██╗ ",
    "     ██║ ",
    "█████╔═╝ ",
    "╚════╝   "
};

static const char *digit_6[] = {
    " ██████╗ ",
    "██╔════╝ ",
    "██║      ",
    "███████╗ ",
    "██╔═══██╗",
    "╚██████╔╝",
    " ╚═════╝ "
};

static const char *digit_7[] = {
    "████████╗",
    "╚═════██║",
    "     ██╔╝",
    "    ██╔╝ ",
    "   ██╔╝  ",
    "   ██║   ",
    "   ╚═╝   "
};

static const char *digit_8[] = {
    " ██████╗ ",
    "██╔═══██╗",
    "╚██████╔╝",
    " ██████╗ ",
    "██╔═══██╗",
    "╚██████╔╝",
    " ╚═════╝ "
};

static const char *digit_9[] = {
    " ██████╗ ",
    "██╔═══██╗",
    "╚███████║",
    " ╚════██║",
    "     ██╔╝",
    " █████╔╝ ",
    " ╚════╝  "
};

static const char *colon[] = {
    "    ",
    " ██╗",
    " ╚═╝",
    "    ",
    " ██╗",
    " ╚═╝",
    "    "
};

static const char **digits[] = {
    digit_0, digit_1, digit_2, digit_3, digit_4,
    digit_5, digit_6, digit_7, digit_8, digit_9
};

const char **digit_get(int digit)
{
    if (digit < 0 || digit > 9) {
        return NULL;
    }
    return digits[digit];
}

const char **digit_get_colon(void)
{
    return colon;
}

int digit_width(void)
{
    return DIGIT_WIDTH;
}

int digit_height(void)
{
    return DIGIT_HEIGHT;
}

int colon_width(void)
{
    return COLON_WIDTH;
}
