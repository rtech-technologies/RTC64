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
    char username[32];
    char password[32];
    int installed;
    int show_launcher;
    int show_terminal;
    int show_explorer;
    int show_settings;
    int show_app_studio;
    int show_script_app;
    int show_lab;
    int show_uac;
    int cpu_usage;
    int perm_net;
    int perm_storage;
    int show_crash_reports;
    char last_crash_path[128];

    /* File Explorer state */
    char explorer_path[256];

    /* Task Manager state */
    int show_task_manager;

    /* Notepad state */
    int show_notepad;
    char notepad_buffer[4096];
    char notepad_file[128];
};

void ui_init_style(struct nk_context *ctx);
void ui_render(struct nk_context *ctx, struct app_state *app, int window_width, int window_height);

void ui_icon_init(void);
struct nk_image ui_icon_load_svg(const char* name, const char* path, int w, int h);

/* App modules */
void chell_init(void* s);
void chell_update(struct nk_context* ctx, void* s);
void lab_init(void* s);
void lab_update(struct nk_context* ctx, void* s);
void installer_init(void* s);
void installer_update(struct nk_context* ctx, void* s);
void studio_init(void* s);
void studio_update(struct nk_context* ctx, void* s);

#endif
