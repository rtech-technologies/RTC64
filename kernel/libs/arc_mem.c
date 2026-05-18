#include <pro_os.h>

typedef struct {
    uint64_t ref_count;
} arc_header_t;

void* arc_alloc(size_t size) {
    arc_header_t* header = malloc(size + sizeof(arc_header_t));
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
        if (header->ref_count == 0) free(header);
    }
}
