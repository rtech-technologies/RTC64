#include <stdint.h>
#include <stddef.h>
#include "font_8x8.h"

void draw_pixel(uint32_t* fb, uint32_t pitch, uint32_t x, uint32_t y, uint32_t color) {
    fb[y * (pitch / 4) + x] = color;
}

void draw_rect(uint32_t* fb, uint32_t pitch, int x, int y, int w, int h, uint32_t color) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            draw_pixel(fb, pitch, j, i, color);
        }
    }
}

void draw_rect_outline(uint32_t* fb, uint32_t pitch, int x, int y, int w, int h, uint32_t color) {
    for (int j = x; j < x + w; j++) {
        draw_pixel(fb, pitch, j, y, color);
        draw_pixel(fb, pitch, j, y + h - 1, color);
    }
    for (int i = y; i < y + h; i++) {
        draw_pixel(fb, pitch, x, i, color);
        draw_pixel(fb, pitch, x + w - 1, i, color);
    }
}

void draw_char(uint32_t* fb, uint32_t pitch, int x, int y, char c, uint32_t color) {
    if (c < 32 || c > 126) return;
    const uint8_t* glyph = font_8x8_data[c - 32];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (glyph[i] & (1 << (7 - j))) {
                draw_pixel(fb, pitch, x + j, y + i, color);
            }
        }
    }
}

void draw_text(uint32_t* fb, uint32_t pitch, int x, int y, const char* text, int len, uint32_t color) {
    for (int i = 0; i < len; i++) {
        draw_char(fb, pitch, x, y, text[i], color);
        x += 8;
    }
}
