// Frame Buffer (fb)

#pragma once
#include <stdint.h>
#include <limine.h>

int fb_init(struct limine_framebuffer *framebuffer);
void fb_put_pixel(uint64_t x, uint64_t y, uint32_t color);
void fb_fill_rect(uint64_t x, uint64_t y, uint64_t w, uint64_t h, uint32_t color);
void fb_draw_char(char c, uint64_t x, uint64_t y, uint32_t fg, uint32_t bg, uint64_t scale);
void fb_clear(uint32_t color);
uint64_t fb_width(void);
uint64_t fb_height(void);

void fb_scroll_up(uint64_t pixels, uint32_t bg);