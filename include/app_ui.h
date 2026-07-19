#ifndef APP_UI_H
#define APP_UI_H

#include "nuklear_config.h"
#include "nuklear.h"

struct app_state {
    int current_state;

    /* Installer fields */
    char new_username[64];
    char new_password[64];
    char admin_password[64];

    /* Login fields */
    char login_username[64];
    char login_password[64];

    /* Active Session fields */
    char current_user[64];
    int is_admin;

    /* Desktop windows toggles */
    int show_terminal;
    int show_explorer;
    int show_settings;
    int show_launcher;
    int show_uac;
    int show_notepad;

    /* User Account Control (UAC) */
    char uac_action_desc[128];
    char uac_admin_pass_typed[64];
    int uac_authorized;
    void (*uac_on_success)(void);

    /* Personalization settings */
    int wallpaper_theme; // 0: Charcoal, 1: Sovereign Blue, 2: Industrial Plum, 3: Pitch Black
    int show_analog_clock;
    int show_calendar;

    /* Terminal state */
    char term_cmd[128];
    char term_history[4096];

    /* Explorer state */
    char explorer_path[128];

    /* Notepad state */
    char notepad_path[128];
    char notepad_text[2048];

    /* User Management in Settings */
    char add_username[64];
    char add_password[64];
    int add_is_admin;

    /* Performance / System metrics */
    int cpu_usage;
};

enum {
    STATE_LOGIN,
    STATE_INSTALLER,
    STATE_DESKTOP
};

void ui_render(struct nk_context *ctx, struct app_state *app, int window_width, int window_height);
void ui_init_style(struct nk_context *ctx);

#endif
