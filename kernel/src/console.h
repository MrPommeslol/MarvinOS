#pragma once
#include <stdint.h>

void console_init(uint32_t fg, uint32_t bg, uint64_t scale, uint64_t margin);
void console_clear(void);
void console_putc(char c);
void console_puts(const char *s);