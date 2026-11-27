#include <stddef.h>
#include "fb.h"
#include "font8x8_basic.h"

static volatile uint32_t *fb_address = NULL;
static uint64_t fb_w = 0;
static uint64_t fb_h = 0;
static uint64_t fb_pixels_per_row = 0;

int fb_init(struct limine_framebuffer *framebuffer)
{
    if (framebuffer == NULL || framebuffer->bpp != 32)
    {
        return 1; // Fehler kein Framebuffer oder falsches Format des Framebuffers
    }

    fb_address = (volatile uint32_t *)framebuffer->address;
    fb_w = framebuffer->width;
    fb_h = framebuffer->height;
    fb_pixels_per_row = framebuffer->pitch / 4;
    return 0;
}

void fb_put_pixel(uint64_t x, uint64_t y, uint32_t color)
{
    if (fb_address == NULL || x >= fb_w || y >= fb_h)
    {
        return;   // Fehler Pixel ist außerhalb des Bildschirms
    }

    fb_address[y * fb_pixels_per_row + x] = color;
}

void fb_fill_rect(uint64_t x, uint64_t y, uint64_t w, uint64_t h, uint32_t color)
{
    for (uint64_t row = y; row < y + h; row++)
    {
        for (uint64_t col = x; col < x + w; col++)
        {
            fb_put_pixel(col, row, color);
        }
    }
}

void fb_draw_char(char c, uint64_t x, uint64_t y, uint32_t fg, uint32_t bg, uint64_t scale)
{
    unsigned char index = (unsigned char)c;
    if (index >= 128) // Zeichen in der Schrift nicht vorhanden
    {
        index = '?';
    }

    const unsigned char *sign = font8x8_basic[index];

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            int pixel_on = (sign[i] >> j) & 1;
            uint32_t color = pixel_on ? fg : bg;
            fb_fill_rect(x + j * scale, y + i * scale, scale, scale, color);
        }
    }
}

void fb_clear(uint32_t color)
{
    fb_fill_rect(0, 0, fb_w, fb_h, color);
}

uint64_t fb_width(void)  { return fb_w; }
uint64_t fb_height(void) { return fb_h; }



void fb_scroll_up(uint64_t pixels, uint32_t bg)
{
    if (fb_address == NULL) { return; }
    if (pixels >= fb_h)
    {
        fb_clear(bg);
        return;
    }

    for (uint64_t y = 0; y < fb_h - pixels; y++)
    {
        for (uint64_t x = 0; x < fb_w; x++)
        {
            fb_address[y * fb_pixels_per_row + x] =
                fb_address[(y + pixels) * fb_pixels_per_row + x];
        }
    }

    fb_fill_rect(0, fb_h - pixels, fb_w, pixels, bg);
}