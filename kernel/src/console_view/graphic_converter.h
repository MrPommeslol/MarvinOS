
#ifndef _GRAPHIC_CONVERTER

#define _GRAPHIC_CONVERTER

#include <cstdint>
using namespace std;

#define MAX_LINE_NUMBERS_IN_TEXT_BUFFER 128

#define RGB_MASK_BIT_SIZE 8

#define RGB_OFFSET_RED 24
#define RGB_OFFSET_GREEN 16
#define RGB_OFFSET_BLUE 8

typedef struct _position_2D {
    int x;
    int y;
} position_2D;

typedef struct _display_text_buffer {
    char *lines[MAX_LINE_NUMBERS_IN_TEXT_BUFFER];
} display_text_buffer;

typedef struct _display_color {
    uint32_t r;
    uint32_t g;
    uint32_t b;
} display_color;

position_2D *get_position_2D (int x, int y) {
    position_2D *position = new position_2D();
    position->x = x;
    position->y = y;

    return position;
}

uint32_t get_translated_color (display_color color) {
    uint32_t red_channel    = color.r << RGB_OFFSET_RED;
    uint32_t green_channel  = color.g << RGB_OFFSET_GREEN;
    uint32_t blue_channel   = color.b << RGB_OFFSET_BLUE;

    return red_channel | green_channel | blue_channel;
}

class graphic_converter{
    private:
        const int PIXEL_WIDTH_OF_CHAR = 5;
        const int PIXEL_HEIGHT_OF_CHAR = 12;

        const int BORDER_WIDTH = 20;

        const display_color TEXT_COLOR = {255,255,255};

        int SCREEN_WIDTH_IN_CHARS;
        int SCREEN_HEIGHT_IN_CHARS;

        display_text_buffer *display_text;

        volatile uint32_t *framebuffer_img;
        limine_framebuffer *framebuffer;

        bool is_valid_line_number (int line_number) {
            return line_number < MAX_LINE_NUMBERS_IN_TEXT_BUFFER;
        }

        bool is_pixel_on_char_painted (char character, position_2D *position_in_character) {
            return true;
        }

        char *get_line_from_text_buffer (int line_index) {
            return display_text->lines[line_index];
        }

        position_2D *get_pixel_from_character (position_2D *char_position, position_2D *position_in_char) {
            return get_position_2D(
                BORDER_WIDTH + char_position->x * PIXEL_WIDTH_OF_CHAR + position_in_char->x,
                BORDER_WIDTH + char_position->y * PIXEL_HEIGHT_OF_CHAR+ position_in_char->y
            );
        }

        void update_framebuffer_img () {
            framebuffer_img = static_cast<volatile std::uint32_t *>(framebuffer->address);
        }

        void paint_pixel (position_2D *pixel, display_color color) {
            framebuffer_img[pixel->x + pixel->y * framebuffer->width] = get_translated_color(color);
        }

        void print_character (char character, position_2D *char_position) {
            for (int x = 0; x < PIXEL_WIDTH_OF_CHAR; x++) {
                for (int y = 0; y < PIXEL_HEIGHT_OF_CHAR; y++) {
                    position_2D *position_in_character = get_position_2D(x, y);
                    position_2D *pixel = get_pixel_from_character(char_position, position_in_character);
                    if (is_pixel_on_char_painted(character, position_in_character)) {
                        paint_pixel (pixel, TEXT_COLOR);
                    }
                    delete  position_in_character;
                    delete  pixel;
                }
            }
        }

        void print_line_from_text_buffer (int line_number) {
            char *line = get_line_from_text_buffer(line_number);
            for (int char_index = 0; line[char_index] != '\0'; char_index++) {
                position_2D *position_of_char = get_position_2D(char_index, line_number);
                print_character(line[char_index], position_of_char);
                delete  position_of_char;
            }
        }

        void print_text () {
            for (int line_number = 0; line_number < SCREEN_HEIGHT_IN_CHARS; line_number++) {
                if (is_valid_line_number(line_number)) print_line_from_text_buffer(line_number);
            }
        }

    public:
        graphic_converter (display_text_buffer *text_ptr) {
            display_text = text_ptr;
            framebuffer = 0;
        }

        void update_framebuffer (limine_framebuffer *v_framebuffer) {
            framebuffer = v_framebuffer;
            update_framebuffer_img();

            SCREEN_WIDTH_IN_CHARS = framebuffer->width / PIXEL_WIDTH_OF_CHAR;
            SCREEN_HEIGHT_IN_CHARS = framebuffer->height / PIXEL_HEIGHT_OF_CHAR;
        }

        void paint_graphic () {
            update_framebuffer_img();
            print_text();
        }
};

#endif
