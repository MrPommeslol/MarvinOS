#include <stdint.h>

#include "keyboard.h"
#include "io.h"

#define KBD_DATA   0x60
#define KBD_STATUS 0x64
#define SC_LSHIFT  0x2A
#define SC_RSHIFT  0x36

static int shift_pressed = 0; // 1 = Shift wird gerade gehalten
static int extended      = 0; // 1 = vorheriges Byte war 0xE0

// Ohne Shift
static const char keymap_normal[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4', [0x06] = '5',
    [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9', [0x0B] = '0',
    [0x0E] = '\b',                       // Rücktaste
    [0x0F] = '\t',                       // Tab
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r', [0x14] = 't',
    [0x15] = 'z', [0x16] = 'u', [0x17] = 'i', [0x18] = 'o', [0x19] = 'p',
    [0x1B] = '+',
    [0x1C] = '\n',                       // Enter
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f', [0x22] = 'g',
    [0x23] = 'h', [0x24] = 'j', [0x25] = 'k', [0x26] = 'l',
    [0x29] = '^',
    [0x2B] = '#',
    [0x2C] = 'y', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v', [0x30] = 'b',
    [0x31] = 'n', [0x32] = 'm',
    [0x33] = ',', [0x34] = '.', [0x35] = '-',
    [0x39] = ' ',
    [0x56] = '<',
};

// Mit Shift
static const char keymap_shift[128] = {
    [0x02] = '!', [0x03] = '"', [0x05] = '$', [0x06] = '%',
    [0x07] = '&', [0x08] = '/', [0x09] = '(', [0x0A] = ')', [0x0B] = '=',
    [0x0C] = '?',
    [0x0D] = '`',
    [0x0E] = '\b',
    [0x0F] = '\t',
    [0x10] = 'Q', [0x11] = 'W', [0x12] = 'E', [0x13] = 'R', [0x14] = 'T',
    [0x15] = 'Z', [0x16] = 'U', [0x17] = 'I', [0x18] = 'O', [0x19] = 'P',
    [0x1B] = '*',
    [0x1C] = '\n',
    [0x1E] = 'A', [0x1F] = 'S', [0x20] = 'D', [0x21] = 'F', [0x22] = 'G',
    [0x23] = 'H', [0x24] = 'J', [0x25] = 'K', [0x26] = 'L',
    [0x2B] = '\'',
    [0x2C] = 'Y', [0x2D] = 'X', [0x2E] = 'C', [0x2F] = 'V', [0x30] = 'B',
    [0x31] = 'N', [0x32] = 'M',
    [0x33] = ';', [0x34] = ':', [0x35] = '_',
    [0x39] = ' ',
    [0x56] = '>',
};

void keyboard_init(void)
{
    // Alte Bytes wegwerfen, die noch im Controller liegen
    while (inb(KBD_STATUS) & 0x01)
    {
        inb(KBD_DATA);
    }
    shift_pressed = 0;
    extended      = 0;
}

uint8_t keyboard_read_scancode(void)
{
    // Warten, bis der Controller ein Byte hat (Status-Bit 0)
    while ((inb(KBD_STATUS) & 0x01) == 0)
    {
    }
    return inb(KBD_DATA);
}

char keyboard_getchar(void)
{
    while (1)
    {
        uint8_t scancode = keyboard_read_scancode();

        // Sondertasten (Pfeile, Entf, rechte Strg ...) beginnen mit 0xE0
        if (scancode == 0xE0)
        {
            extended = 1;
            continue;
        }
        if (extended)
        {
            extended = 0;
            continue;   // Sondertasten ertmal ignorieren
        }

        int released = scancode & 0x80;   // Bit 7 = losgelassen?
        uint8_t key = scancode & 0x7F;   // reine Tastennummer

        // Shift merken
        if (key == SC_LSHIFT || key == SC_RSHIFT)
        {
            shift_pressed = !released;
            continue;
        }

        // Loslassen interessiert uns bei normalen Tasten nicht
        if (released)
        {
            continue;
        }

        char c = shift_pressed ? keymap_shift[key] : keymap_normal[key];
        if (c != 0)
        {
            return c;
        }
        // c == 0 -> Taste ohne Zeichen (z. B. Strg) -> weiter warten
    }
}