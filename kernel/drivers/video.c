#include <stdint.h>
#include <stddef.h>

void draw_pixel(uint32_t* fb, uint32_t width, uint32_t x, uint32_t y, uint32_t color) {
    fb[y * width + x] = color;
}
