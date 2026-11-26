#pragma once

void kprintf(const char *format, ...) __attribute__((format(printf, 1, 2)));
void tprintf(const char *format, ...) __attribute__((format(printf, 1, 2)));