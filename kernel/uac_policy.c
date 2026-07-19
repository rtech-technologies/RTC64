#include "pro_os.h"
#include <string.h>

/* On-disk User Account Database & Security Policy Engine */

static app_permit_t policies[MAX_TASKS];

bool uac_check_permit(int app_id, const char *action) {
    if (app_id < 0 || app_id >= MAX_TASKS) return false;
    if (strcmp(action, "network") == 0) return policies[app_id].can_network;
    if (strcmp(action, "storage") == 0) return policies[app_id].can_storage;
    return false;
}

void uac_request_permit(int app_id, const char *action) {
    (void)app_id; (void)action;
}

void uac_set_permit(int app_id, bool net, bool storage) {
    if (app_id >= 0 && app_id < MAX_TASKS) {
        policies[app_id].can_network = net;
        policies[app_id].can_storage = storage;
    }
}

/* User Registration on AHCI/SATA FAT Filesystem */

int db_user_exists(const char *username) {
    char buf[1024];
    int len = vfs_read("/System/Config/passwd", buf, sizeof(buf) - 1);
    if (len < 0) return 0; //passwd doesn't exist
    buf[len] = '\0';

    char *line = buf;
    while (line && *line) {
        char *next_line = strchr(line, '\n');
        if (next_line) {
            *next_line = '\0';
            next_line++;
        }

        char user[64] = {0};
        char *colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            strcpy(user, line);
            if (strcmp(user, username) == 0) {
                return 1; // Found
            }
        }
        line = next_line;
    }
    return 0;
}

int db_register_user(const char *username, const char *password, int is_admin) {
    if (!username || !username[0] || !password || !password[0]) return -1;
    if (db_user_exists(username)) return -1; // Duplicate user

    char passwd_buf[1024] = {0};
    int len = vfs_read("/System/Config/passwd", passwd_buf, sizeof(passwd_buf) - 1);
    if (len < 0) len = 0;
    passwd_buf[len] = '\0';

    // Append new user entry: "username:password:is_admin\n"
    char new_entry[256];
    snprintf(new_entry, sizeof(new_entry), "%s:%s:%d\n", username, password, is_admin ? 1 : 0);

    if (len + strlen(new_entry) < sizeof(passwd_buf)) {
        strcat(passwd_buf, new_entry);
        vfs_write("/System/Config/passwd", passwd_buf, strlen(passwd_buf));

        // Auto-provision personal desktop folder
        char desktop_path[128];
        snprintf(desktop_path, sizeof(desktop_path), "/Users/%s", username);
        vfs_mkdir(desktop_path);

        snprintf(desktop_path, sizeof(desktop_path), "/Users/%s/Desktop", username);
        vfs_mkdir(desktop_path);

        // Put a welcome note on user's desktop
        char welcome_path[256];
        snprintf(welcome_path, sizeof(welcome_path), "/Users/%s/Desktop/Welcome.txt", username);
        char note[256];
        snprintf(note, sizeof(note), "Welcome to your personal workstation space, %s!\nThis folder is persistent on your hard drive.\n", username);
        vfs_write(welcome_path, note, strlen(note));

        return 0;
    }
    return -1;
}

int db_verify_user(const char *username, const char *password, int *out_is_admin) {
    char buf[1024];
    int len = vfs_read("/System/Config/passwd", buf, sizeof(buf) - 1);
    if (len < 0) return -1;
    buf[len] = '\0';

    char *line = buf;
    while (line && *line) {
        char *next_line = strchr(line, '\n');
        if (next_line) {
            *next_line = '\0';
            next_line++;
        }

        char user[64] = {0};
        char pass[64] = {0};
        int admin = 0;

        char *colon1 = strchr(line, ':');
        if (colon1) {
            *colon1 = '\0';
            strcpy(user, line);

            char *colon2 = strchr(colon1 + 1, ':');
            if (colon2) {
                *colon2 = '\0';
                strcpy(pass, colon1 + 1);
                admin = (colon2[1] == '1') ? 1 : 0;

                if (strcmp(user, username) == 0 && strcmp(pass, password) == 0) {
                    if (out_is_admin) *out_is_admin = admin;
                    return 0; // Match
                }
            }
        }
        line = next_line;
    }
    return -1; // Unauthorized
}
