#ifndef DIGITS_H
#define DIGITS_H

#define DIGIT_HEIGHT 7
#define DIGIT_WIDTH  9
#define COLON_WIDTH  4

/* Get the ASCII art for a digit (0-9) */
const char **digit_get(int digit);

/* Get the ASCII art for a colon separator */
const char **digit_get_colon(void);

/* Width of each digit in characters */
int digit_width(void);

/* Height of each digit in characters */
int digit_height(void);

/* Width of colon in characters */
int colon_width(void);

#endif /* DIGITS_H */
