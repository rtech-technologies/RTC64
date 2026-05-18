#include <pro_os.h>

#define HEAP_SIZE (16 * 1024 * 1024)
static uint8_t heap[HEAP_SIZE] __attribute__((aligned(8)));
static size_t heap_ptr = 0;

void* bump_alloc(size_t size) {
    size = (size + 7) & ~7; // 8-byte align

    if (heap_ptr + size > HEAP_SIZE * 0.9) {
        // Serial alert
        const char *alert = "\n[ALERT] Bump Allocator threshold (90%) reached!\n";
        while(*alert) {
            extern void vga_putc(char c);
            vga_putc(*alert++);
        }
        return NULL;
    }

    void* ptr = &heap[heap_ptr];
    heap_ptr += size;
    return ptr;
}

void bump_reset(void) {
    heap_ptr = 0;
}
