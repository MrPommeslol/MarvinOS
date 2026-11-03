
#ifndef _GRAPHIC_CONVERTER

#define _GRAPHIC_CONVERTER

#include <string>
using namespace std;

#define MAX_LINE_NUMBERS_IN_TEXT_BUFFER 128

typedef struct _position_2D {
    int x;
    int y;
} position_2D;

typedef struct _display_text_buffer {
    string lines[MAX_LINE_NUMBERS_IN_TEXT_BUFFER];
    int current_line;
} display_text_buffer;

class Graphic_converter{
    private:
        const int PIXEL_WIDTH_OF_CHAR = 5;
        const int PIXEL_HEIGHT_OF_CHAR = 12;

        int SCREEN_WIDTH_IN_CHARS;
        int SCREEN_HEIGHT_IN_CHARS;

        display_text_buffer *display_text;
        limine_framebuffer *framebuffer;
    public:
        Graphic_converter (display_text_buffer *text_ptr, limine_framebuffer *fb) {
            display_text = text_ptr;
            framebuffer = fb;

            SCREEN_WIDTH_IN_CHARS = fb->width / PIXEL_WIDTH_OF_CHAR;
            SCREEN_HEIGHT_IN_CHARS = fb->height / PIXEL_HEIGHT_OF_CHAR;
        }

        position_2D *get_char_position_on_screen_from_pixel (position_2D *pixel) {
            int char_pos_x = pixel->x / PIXEL_WIDTH_OF_CHAR;
            int char_pos_y = pixel->y / PIXEL_HEIGHT_OF_CHAR;

            position_2D *char_position = new position_2D();
            char_position->x = char_pos_x;
            char_position->y = char_pos_y;
            return char_position;
        }

        bool is_valid_line_number (int line_number) {
            return line_number < MAX_LINE_NUMBERS_IN_TEXT_BUFFER;
        }

        string *get_line_from_display_text (int display_line) {
            int line_index_in_display_text = display_text->current_line + display_line;
            if (!is_valid_line_number(line_index_in_display_text)) {
                return NULL;
            }
            return &(display_text->lines[line_index_in_display_text]);
        }

        char get_char_at_position_from_line (string *line, int position) {
            bool is_position_in_line = position < line->length();
            if (is_position_in_line) {
                return (*line)[position];
            }
            return ' ';
        }

        char find_char_on_char_position (position_2D *char_position_on_screen) {
            string *line_with_char_position = get_line_from_display_text(char_position_on_screen->y);
            return get_char_at_position_from_line(line_with_char_position, char_position_on_screen->x);
        }

        void print_pixel (position_2D *pixel) {
            position_2D *char_position_on_screen = get_char_position_on_screen_from_pixel (pixel);
            char char_on_position = find_char_on_char_position(char_position_on_screen);
            // search for pixel in char
            // print pixel
        }

        void print_text () {
            volatile uint32_t *fb_ptr = static_cast<volatile uint32_t *>(framebuffer->address);
            for (size_t y = 0; y < framebuffer->height; y++) {
                for (size_t x = 0; x < framebuffer->width; x++) {
                    position_2D *pixel = new position_2D();
                    pixel->x = x;
                    pixel->y = y;
                    print_pixel(pixel);
                }
            }
        }

        void print_graphic () {
            print_text();
        }
};

#endif
