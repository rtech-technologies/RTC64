#ifndef APP_UI_H
#define APP_UI_H

#include "nuklear_config.h"
#include "nuklear.h"

struct app_state {
    int current_state;
    char username[64];
    char password[64];
    int progress;
    int install_started;

    /* Desktop windows */
    int show_terminal;
    int show_explorer;
    int show_settings;
    int show_launcher;
    int show_uac;
    int show_calculator;
    int show_notepad;
    int show_taskmgr;

    /* Permits */
    int perm_net;
    int perm_storage;

    /* Metrics */
    int cpu_usage;

    /* Login user list dropdown / input */
    int show_user_select;
    char login_username[32];
    char login_password[32];
    int login_error;

    /* Installer fields */
    char inst_username[32];
    char inst_password[32];
    int inst_role_idx; // 0 = Admin, 1 = Standard, 2 = Guest

    /* Theme Personalization */
    int active_theme; // 0 = Midnight blue, 1 = Classic grey, 2 = Emerald green

    /* Notepad Buffer */
    char notepad_path[128];
    char notepad_buf[4096];

    /* Calculator Buffer */
    char calc_input[64];
    char calc_history[128];

    /* Terminal input/output */
    char cmd_input[128];
    char term_output[4096];
    int term_lines_count;
    char cmd_history[16][128];
    int cmd_history_count;
    int cmd_history_idx;
};

enum {
    STATE_LOGIN,
    STATE_INSTALLER,
    STATE_DESKTOP
};

void ui_render(struct nk_context *ctx, struct app_state *app, int window_width, int window_height);
void ui_init_style(struct nk_context *ctx);
void ui_apply_theme(struct nk_context *ctx, int theme);

#endif
