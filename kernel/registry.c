/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include <string.h>
#include "serial.h"

#define MAX_REG_ENTRIES 128
#define REG_PATH "/registry/registry.conf"
#define REG_TMP_PATH "/registry/registry.tmp"

typedef struct {
    char key[64];
    char value[64];
    bool active;
} reg_entry_t;

static reg_entry_t registry[MAX_REG_ENTRIES];
static bool registry_loaded = false;

void registry_init(void) {
    memset(registry, 0, sizeof(registry));
    char buf[8192];
    vfs_mkdir("/registry");
    if (vfs_cat(REG_PATH, buf, sizeof(buf)) == 0) {
        char* line = buf;
        while (line && *line) {
            char* next_line = strchr(line, '\n');
            if (next_line) *next_line = '\0';

            char* sep = strchr(line, '=');
            if (sep) {
                *sep = '\0';
                registry_set(line, sep + 1);
            }

            if (next_line) line = next_line + 1;
            else line = NULL;
        }
        serial_printf("[REGISTRY] Loaded state from %s\n", REG_PATH);
    } else {
        /* Set defaults */
        registry_set("SESSION/CurrentUser", "Administrator");
        registry_set("USERS/Administrator/Role", "Administrator");
        registry_set("HKCU\\ControlPanel\\Desktop\\Wallpaper", "/system/wallpapers/pawel-czerwinski.jpg");
        registry_loaded = true;
        registry_flush();
    }
    registry_loaded = true;
}

int registry_set(const char* key, const char* value) {
    if (!key || !value) return -1;
    if (strlen(key) >= 64 || strlen(value) >= 64) return -1;

    for (int i = 0; i < MAX_REG_ENTRIES; i++) {
        if (registry[i].active && strcmp(registry[i].key, key) == 0) {
            strncpy(registry[i].value, value, 63);
            registry[i].value[63] = '\0';
            return 0;
        }
    }
    for (int i = 0; i < MAX_REG_ENTRIES; i++) {
        if (!registry[i].active) {
            strncpy(registry[i].key, key, 63);
            registry[i].key[63] = '\0';
            strncpy(registry[i].value, value, 63);
            registry[i].value[63] = '\0';
            registry[i].active = true;
            return 0;
        }
    }
    return -1;
}

const char* registry_get(const char* key) {
    for (int i = 0; i < MAX_REG_ENTRIES; i++) {
        if (registry[i].active && strcmp(registry[i].key, key) == 0) {
            return registry[i].value;
        }
    }
    return NULL;
}

void registry_flush(void) {
    if (!registry_loaded) return;

    char buf[8192];
    int off = 0;
    for (int i = 0; i < MAX_REG_ENTRIES; i++) {
        if (registry[i].active) {
            off += snprintf(buf + off, sizeof(buf) - off, "%s=%s\n", registry[i].key, registry[i].value);
        }
    }

    /* Industrial Atomic Write: Write to Temp -> Rename */
    vfs_mkdir("/registry");
    if (vfs_write(REG_TMP_PATH, buf) == 0) {
        vfs_rm(REG_PATH);
        vfs_rename(REG_TMP_PATH, REG_PATH);
        serial_printf("[REGISTRY] Flushed state atomically to %s\n", REG_PATH);
        extern void vfs_sync(void);
        vfs_sync();
    }
}
