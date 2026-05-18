#include <pro_os.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <external/stb_truetype.h>

extern struct limine_framebuffer *fb;
static stbtt_fontinfo font_info;
static uint8_t* font_buffer = NULL;

void typography_init(void* ttf_data) {
    font_buffer = (uint8_t*)ttf_data;
    stbtt_InitFont(&font_info, font_buffer, 0);
}

void typography_draw_char(char c, int x, int y, uint32_t color) {
    if (!fb || !fb->address || !font_buffer) return;

    int width, height, xoff, yoff;
    float scale = stbtt_ScaleForPixelHeight(&font_info, 16.0);
    uint8_t* bitmap = stbtt_GetCodepointBitmap(&font_info, 0, scale, c, &width, &height, &xoff, &yoff);

    if (bitmap) {
        uint32_t* screen = (uint32_t*)fb->address;
        for (int py = 0; py < height; py++) {
            for (int px = 0; py < width; px++) {
                int sx = x + xoff + px;
                int sy = y + yoff + py;
                if (sx >= 0 && sx < (int)fb->width && sy >= 0 && sy < (int)fb->height) {
                    uint8_t alpha = bitmap[py * width + px];
                    if (alpha > 0) {
                         // Simple alpha blending placeholder
                         screen[sy * (fb->pitch/4) + sx] = color;
                    }
                }
            }
        }
        stbtt_FreeBitmap(bitmap, NULL);
    }
}
