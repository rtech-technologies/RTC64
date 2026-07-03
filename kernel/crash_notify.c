/* Simple storage for the most recent crash report path so UI can display it */
#include "crash_notify.h"
#include <string.h>

static char last_path[128];

void set_last_crash_path(const char* path) {
    if (!path) { last_path[0] = '\0'; return; }
    strncpy(last_path, path, sizeof(last_path)-1);
    last_path[sizeof(last_path)-1] = '\0';
}

const char* get_last_crash_path(void) {
    return last_path[0] ? last_path : NULL;
}
