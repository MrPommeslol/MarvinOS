#pragma once
#include <stdint.h>

void keyboard_init(void);
uint8_t keyboard_read_scancode(void);
char keyboard_getchar(void);