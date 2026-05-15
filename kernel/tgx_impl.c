#include "external/tgx.h"
#include <string.h>

/* High-performance Software Blitter Implementation */

void tgx_clear(tgx_canvas_t *canvas, uint32_t color) {
    for (int i = 0; i < canvas->width * canvas->height; i++) {
        canvas->pixels[i] = color;
    }
}

void tgx_blit_rect(tgx_canvas_t *canvas, int x, int y, int w, int h, uint32_t color) {
    if (x < 0 || y < 0 || x + w > canvas->width || y + h > canvas->height) return;
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            canvas->pixels[(y + j) * (canvas->pitch / 4) + (x + i)] = color;
        }
    }
}

void tgx_draw_bevel(tgx_canvas_t *canvas, int x, int y, int w, int h, uint32_t light, uint32_t dark) {
    if (x < 0 || y < 0 || x + w > canvas->width || y + h > canvas->height) return;

    /* Draw top and left edges */
    for (int i = 0; i < w; i++) canvas->pixels[y * (canvas->pitch / 4) + (x + i)] = light;
    for (int j = 0; j < h; j++) canvas->pixels[(y + j) * (canvas->pitch / 4) + x] = light;

    /* Draw bottom and right edges */
    for (int i = 0; i < w; i++) canvas->pixels[(y + h - 1) * (canvas->pitch / 4) + (x + i)] = dark;
    for (int j = 0; j < h; j++) canvas->pixels[(y + j) * (canvas->pitch / 4) + (x + w - 1)] = dark;
}
