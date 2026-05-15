#ifndef APP_UI_H
#define APP_UI_H

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

    /* Permits */
    int perm_net;
    int perm_storage;

    /* Metrics */
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
