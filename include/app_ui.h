/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#ifndef APP_UI_H
#define APP_UI_H

#include "nuklear.h"

typedef enum {
    STATE_LOGIN,
    STATE_INSTALLER,
    STATE_DESKTOP
} app_state_t;

struct app_state {
    app_state_t current_state;
    char password[32];
    int show_launcher;
    int show_terminal;
    int show_explorer;
    int show_settings;
    int show_uac;
    int cpu_usage;
    int perm_net;
    int perm_storage;
};

void ui_init_style(struct nk_context *ctx);
void ui_render(struct nk_context *ctx, struct app_state *app, int window_width, int window_height);

/* App modules */
void chell_init(void* s);
void chell_update(struct nk_context* ctx, void* s);
void lab_init(void* s);
void lab_update(struct nk_context* ctx, void* s);
void installer_init(void* s);
void installer_update(struct nk_context* ctx, void* s);

#endif
