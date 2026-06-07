/* Modified by Sovereign: Enhanced TGX Graphics Engine with optimized primitives */
#include "external/tgx.h"
#include "pro_os.h"

void tgx_clear(tgx_canvas_t *canvas, uint32_t color) {
    if (!canvas || !canvas->pixels) return;
    size_t size = (size_t)canvas->width * (size_t)canvas->height;
    memset(canvas->pixels, (int)color, size * 4); /* Uses SSE optimized memset */
}

void tgx_blit_rect(tgx_canvas_t *canvas, int x, int y, int w, int h, uint32_t color) {
    if (!canvas || !canvas->pixels) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > canvas->width) w = canvas->width - x;
    if (y + h > canvas->height) h = canvas->height - y;
    if (w <= 0 || h <= 0) return;

    for (int i = 0; i < h; i++) {
        uint32_t *line = canvas->pixels + (y + i) * (canvas->pitch / 4) + x;
        for (int j = 0; j < w; j++) {
            line[j] = color;
        }
    }
}

void tgx_draw_bevel(tgx_canvas_t *canvas, int x, int y, int w, int h, uint32_t light, uint32_t dark) {
    tgx_blit_rect(canvas, x, y, w, 1, light);
    tgx_blit_rect(canvas, x, y, 1, h, light);
    tgx_blit_rect(canvas, x + w - 1, y, 1, h, dark);
    tgx_blit_rect(canvas, x, y + h - 1, w, 1, dark);
}

/* POWER: Meaty Optimized Line Drawing (Bresenham's) */
void tgx_draw_line(tgx_canvas_t *canvas, int x0, int y1_0, int x1, int y1_1, uint32_t color) {
    int dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1_1 - y1_0), sy = y1_0 < y1_1 ? 1 : -1;
    int err = dx + dy, e2;

    for (;;) {
        tgx_blit_rect(canvas, x0, y1_0, 1, 1, color);
        if (x0 == x1 && y1_0 == y1_1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y1_0 += sy; }
    }
}
