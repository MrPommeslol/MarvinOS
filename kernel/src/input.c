#include "input.h"
#include "keyboard.h"
#include "console.h"

size_t input_readline(char *buffer, size_t max)
{
    size_t len = 0;   // wie viele Zeichen schon im Puffer stehen

    while (1)
    {
        char c = keyboard_getchar();

        if (c == '\n')
        {
            // Enter: Eingabe fertig
            buffer[len] = '\0';
            console_putc('\n');
            return len;
        }

        if (c == '\b')
        {
            // Rücktaste: nur löschen, wenn wir selbst etwas getippt haben
            if (len > 0)
            {
                len--;
                console_putc('\b');
            }
            continue;
        }

        if (c == '\t')
        {
            continue;   // Tab ignorieren wir erstmal
        }

        // normales Zeichen: speichern und anzeigen, solange Platz ist
        if (len < max - 1)
        {
            buffer[len] = c;
            len++;
            console_putc(c);
        }
    }
}