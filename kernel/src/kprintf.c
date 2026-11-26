#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include "kprintf.h"
#include "serial.h"
#include "console.h"

typedef void (*putc_func)(char c);

// ---------- Hilfsfunktionen ----------
static void out_str(putc_func out, const char *s)
{
    while (*s != '\0')
    {
        out(*s);
        s++;
    }
}

static void out_dec(putc_func out, uint64_t n)
{
    char buffer[20];
    int count = 0;

    if (n == 0)
    {
        out('0');
        return;
    }

    while (n > 0)
    {
        buffer[count] = '0' + (n % 10);
        count++;
        n = n / 10;
    }

    while (count > 0)
    {
        count--;
        out(buffer[count]);
    }
}

static void out_hex(putc_func out, uint64_t n)
{
    const char *digits = "0123456789ABCDEF";

    out_str(out, "0x");
    for (int shift = 60; shift >= 0; shift -= 4)
    {
        out(digits[(n >> shift) & 0xF]);
    }
}

// ---------- Gemeinsamer Kern ----------
static void format_print(putc_func out, const char *format, va_list args)
{
    for (int i = 0; format[i] != '\0'; i++)
    {
        if (format[i] != '%')
        {
            out(format[i]);
            continue;
        }

        i++;
        switch (format[i])
        {
            case 'd':
            {
                int64_t value = va_arg(args, int);
                if (value < 0)
                {
                    out('-');
                    value = -value;
                }
                out_dec(out, (uint64_t)value);
                break;
            }

            case 'u':
                out_dec(out, va_arg(args, unsigned int));
                break;

            case 'x':
                out_hex(out, va_arg(args, unsigned int));
                break;

            case 'p':
                out_hex(out, (uintptr_t)va_arg(args, void *));
                break;

            case 'c':
                out((char)va_arg(args, int));
                break;

            case 's':
            {
                const char *text = va_arg(args, const char *);
                if (text == NULL)
                {
                    text = "(null)";
                }
                out_str(out, text);
                break;
            }

            case '%':
                out('%');
                break;

            case '\0':
                return;

            default:
                out('%');
                out(format[i]);
                break;
        }
    }
}

// ---------- Öffentliche Funktionen ----------
void kprintf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    format_print(serial_putc, format, args);
    va_end(args);
}

void tprintf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    format_print(console_putc, format, args);
    va_end(args);
}