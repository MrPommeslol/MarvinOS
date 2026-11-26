#pragma once

int serial_init(void);
void serial_putc(char c);
void serial_puts(const char *s);
void serial_putdec(uint64_t n);
void serial_puthex(uint64_t n);