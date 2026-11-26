#include "console.h"

#include "fb.h"

#define FONT_SIZE 8
#define PREFIX_LEN 2 // "> "

static uint64_t scale    = 1;
static uint64_t char_px  = 8;   // Größe einer Zelle in Pixeln (FONT_SIZE * scale)
static uint64_t margin   = 0;   // Rand in Pixeln
static uint64_t cols     = 0;   // Zeichen pro Zeile
static uint64_t rows     = 0;   // Zeilen auf dem Bildschirm

static uint64_t cursor_col = 0;
static uint64_t cursor_row = 0;
static uint64_t wrap_depth = 0; // wie oft ist der aktuelle Abschnitt umgebrochen

static uint32_t fg_color = 0xFFFFFF;
static uint32_t bg_color = 0x000000;

// ---------- Hilfsfunktionen ----------

static void draw_cell(char c, uint64_t col, uint64_t row)
{
    fb_draw_char(c, margin + col * char_px, margin + row * char_px, fg_color, bg_color, scale);
}

static void draw_cursor(uint32_t color)
{
    uint64_t thickness = 2 * scale;
    uint64_t x = margin + cursor_col * char_px;
    uint64_t y = margin + cursor_row * char_px + char_px - thickness;
    fb_fill_rect(x, y, char_px, thickness, color);
}

static void cursor_show(void) { draw_cursor(fg_color); }
static void cursor_hide(void) { draw_cursor(bg_color); }

static void draw_prefix(int wrapped)
{
    draw_cell(wrapped ? ' ' : '>', 0, cursor_row);
    draw_cell(' ', 1, cursor_row);
    cursor_col = PREFIX_LEN;
}

static void new_line(int wrapped)
{
    cursor_row++;

    if (cursor_row >= rows)
    {
        fb_scroll_up(char_px, bg_color);
        fb_fill_rect(0, 0, fb_width(), margin, bg_color);                                // oberer Rand
        fb_fill_rect(0, margin + (rows - 1) * char_px, fb_width(), char_px, bg_color);   // letzte Zeile
        cursor_row = rows - 1;
    }

    if (wrapped)
    {
        wrap_depth++;      // gleicher Abschnitt, nur umgebrochen
    }
    else
    {
        wrap_depth = 0;    // echter neuer Abschnitt
    }

    draw_prefix(wrapped);
}

static void backspace(void)
{
    if (cursor_col > PREFIX_LEN)
    {
        // normaler Fall: einfach eins nach links
        cursor_col--;
    }
    else if (wrap_depth > 0 && cursor_row > 0)
    {
        // Anfang einer umgebrochenen Zeile: zurück ans Ende der Zeile darüber
        cursor_row--;
        wrap_depth--;
        cursor_col = cols - 1;
    }
    else
    {
        return;   // Anfang des Abschnitts erreicht, "> " wird nicht gelöscht
    }

    draw_cell(' ', cursor_col, cursor_row);   // Feld leer malen
}

// ---------- Öffentliche Funktionen ----------

void console_init(uint32_t fg, uint32_t bg, uint64_t s, uint64_t m)
{
    fg_color = fg;
    bg_color = bg;
    scale    = s;
    char_px  = FONT_SIZE * scale;
    margin   = m;

    if (2 * margin >= fb_width() || 2 * margin >= fb_height())
    {
        margin = 0;
    }

    cols = (fb_width()  - 2 * margin) / char_px;
    rows = (fb_height() - 2 * margin) / char_px;

    console_clear();
}

void console_clear(void)
{
    fb_clear(bg_color);
    cursor_row = 0;
    wrap_depth = 0;
    draw_prefix(0);
    cursor_show();
}

void console_putc(char c)
{
    cursor_hide();

    if (c == '\n')
    {
        new_line(0);
    }
    else if (c == '\b')
    {
        backspace();
    }
    else
    {
        draw_cell(c, cursor_col, cursor_row);
        cursor_col++;
        if (cursor_col >= cols)
        {
            new_line(1);   // Zeile voll -> umbrechen
        }
    }

    cursor_show();
}

void console_puts(const char *s)
{
    while (*s != '\0')
    {
        console_putc(*s);
        s++;
    }
}