#include <pro_os.h>
#include <ff.h>

typedef struct {
    char prefix[8];
    FATFS fs;
} vfs_route_t;

static vfs_route_t routes[4];
static int route_count = 0;

void vfs_init() {
    // Mount BOOT:/ using FatFs
}

// Prefix Router
FRESULT vfs_open(FIL* fp, const char* path, BYTE mode) {
    // Route "BOOT:/file.txt" -> FatFs handle for BOOT partition
    return f_open(fp, path, mode);
}
