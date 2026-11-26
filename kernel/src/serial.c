#include <stdint.h>

#include "serial.h"
#include "io.h"

#define COM1 0x3F8

int serial_init(void) {
    outb(COM1 + 1, 0x00);  // 1. Interrupts aus (brauchen wir nicht)
    outb(COM1 + 3, 0x80);  // 2. "Geschwindigkeit einstellen"-Modus an
    outb(COM1 + 0, 0x03);  // 3. Geschwindigkeit: 38400 Baud (unteres Byte)
    outb(COM1 + 1, 0x00);  //    (oberes Byte)
    outb(COM1 + 3, 0x03);  // 4. Format: 8 Bits pro Zeichen, Modus wieder aus
    outb(COM1 + 2, 0xC7);  // 5. Puffer einschalten

    // --- Selbsttest ---
    outb(COM1 + 4, 0x1E);  // 6. Testmodus ("Loopback") an
    outb(COM1 + 0, 0xAE);  // 7. Testbyte senden
    if (inb(COM1 + 0) != 0xAE) {
        return 1;          // falsches Byte zurück -> Fehler
    }

    outb(COM1 + 4, 0x0F);  // 8. Testmodus aus, normaler Betrieb
    return 0;              // alles ok
}

void serial_putc(char c)
{
    if (c == '\n')
    {
        serial_putc('\r'); // Für \r\n
    }
    while ((inb(COM1 + 5) & 0x20) == 0)
    {
        // warten bis bereit
    }
    outb(COM1 + 0, (uint8_t)c);
}

void serial_puts(const char *s)
{
    while (*s != '\0')
    {
        serial_putc(*s);
        s++;
    }
}

void serial_putdec(uint64_t n)
{
    char buffer[20];
    int count = 0;

    if (n == 0)
    {
        serial_putc('0');
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
        serial_putc(buffer[count]);
    }
}

void serial_puthex(uint64_t n)
{
    const char *digits = "0123456789ABCDEF";
    serial_puts("0x");
    for (int i = 60; i >= 0; i -= 4)
    {
        uint8_t c = (n >> i) & 0xF;
        serial_putc(digits[c]);
    }
}