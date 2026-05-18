#include <pro_os.h>

typedef struct {
    uint64_t ref_count;
} arc_header_t;

extern void* bump_alloc(size_t size);

void* arc_alloc(size_t size) {
    arc_header_t* header = bump_alloc(size + sizeof(arc_header_t));
    if (!header) return NULL;
    header->ref_count = 1;
    return (void*)(header + 1);
}

void retain(void* ptr) {
    if (!ptr) return;
    arc_header_t* header = (arc_header_t*)ptr - 1;
    header->ref_count++;
}

void release(void* ptr) {
    if (!ptr) return;
    arc_header_t* header = (arc_header_t*)ptr - 1;
    if (header->ref_count > 0) {
        header->ref_count--;
    }
    // In a real system, ref_count == 0 would trigger free.
    // Here we use bump_alloc, so objects persist until bump_reset.
}
