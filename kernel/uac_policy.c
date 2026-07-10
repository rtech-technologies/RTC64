#include "pro_os.h"
#include <string.h>

static user_account_t accounts[10];
static int user_count = 0;
static char active_user[32] = "Guest";
static app_permit_t policies[MAX_TASKS];

void user_init(void) {
    user_count = 0;
    // Default system users
    user_add("Administrator", "admin123", ROLE_ADMIN);
    user_add("Standard", "user123", ROLE_STANDARD);
    user_add("Guest", "", ROLE_GUEST);

    strcpy(active_user, "Guest");
}

bool user_add(const char *username, const char *password, user_role_t role) {
    if (user_count >= 10) return false;
    // Ensure unique name
    for (int i = 0; i < user_count; i++) {
        if (strcmp(accounts[i].username, username) == 0) return false;
    }
    strncpy(accounts[user_count].username, username, 32);
    strncpy(accounts[user_count].password, password, 32);
    accounts[user_count].role = role;
    user_count++;
    return true;
}

bool user_authenticate(const char *username, const char *password) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(accounts[i].username, username) == 0) {
            if (accounts[i].role == ROLE_GUEST) return true;
            return strcmp(accounts[i].password, password) == 0;
        }
    }
    return false;
}

user_role_t user_get_role(const char *username) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(accounts[i].username, username) == 0) {
            return accounts[i].role;
        }
    }
    return ROLE_GUEST;
}

void user_get_active(char *username, user_role_t *role) {
    strcpy(username, active_user);
    *role = user_get_role(active_user);
}

void user_set_active(const char *username) {
    strncpy(active_user, username, 32);
}

bool user_delete(const char *username) {
    if (strcmp(username, "Administrator") == 0) return false; // Prevent Admin deletion
    for (int i = 0; i < user_count; i++) {
        if (strcmp(accounts[i].username, username) == 0) {
            for (int j = i; j < user_count - 1; j++) {
                accounts[j] = accounts[j + 1];
            }
            user_count--;
            if (strcmp(active_user, username) == 0) {
                strcpy(active_user, "Guest");
            }
            return true;
        }
    }
    return false;
}

int user_get_all(char usernames[][32], user_role_t roles[], int max_users) {
    int count = (user_count < max_users) ? user_count : max_users;
    for (int i = 0; i < count; i++) {
        strcpy(usernames[i], accounts[i].username);
        roles[i] = accounts[i].role;
    }
    return count;
}

bool user_change_password(const char *username, const char *new_password) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(accounts[i].username, username) == 0) {
            strncpy(accounts[i].password, new_password, 32);
            return true;
        }
    }
    return false;
}

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
