#ifndef TGX_H
#define TGX_H

#include <stdint.h>

/* Simplified TGX interface for high-performance 2D/3D blitting */
typedef struct {
    uint32_t *pixels;
    int width;
    int height;
    int pitch;
} tgx_canvas_t;

void tgx_clear(tgx_canvas_t *canvas, uint32_t color);
void tgx_blit_rect(tgx_canvas_t *canvas, int x, int y, int w, int h, uint32_t color);
void tgx_draw_bevel(tgx_canvas_t *canvas, int x, int y, int w, int h, uint32_t light, uint32_t dark);

#endif
