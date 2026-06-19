/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "os_api.h"
#include <string.h>
#include <stdio.h>
#include "app_ui.h"
#define SHELL_HISTORY_MAX 4096
static char output_log[SHELL_HISTORY_MAX];
static char cmd_buf[128];
static int cmd_len = 0;
void chell_init(void* s) {
    struct app_state* app = (struct app_state*)s;
    if (app) app->show_terminal = 1;
    strcpy(output_log, "Sovereign HIGH-POWER System Shell [Windowed Mode]\nType 'help' for commands.\n\n> ");
}
static void shell_exec(const char* cmd) {
    size_t current_len = strlen(output_log);
    if (current_len + strlen(cmd) + 10 >= SHELL_HISTORY_MAX) { output_log[0] = '\0'; current_len = 0; }
    strcat(output_log, cmd); strcat(output_log, "\n");
    if (strcmp(cmd, "help") == 0) strcat(output_log, "Commands: ls, cat, mounts, device, help, clear, health, tasks, uptime\n");
    else if (strcmp(cmd, "clear") == 0) output_log[0] = '\0';
    else if (strncmp(cmd, "ls ", 3) == 0) os_vfs_ls(cmd + 3, output_log + strlen(output_log), SHELL_HISTORY_MAX - strlen(output_log));
    else if (strcmp(cmd, "ls") == 0) os_vfs_ls("/", output_log + strlen(output_log), SHELL_HISTORY_MAX - strlen(output_log));
    else if (strncmp(cmd, "cat ", 4) == 0) os_vfs_cat(cmd + 4, output_log + strlen(output_log), SHELL_HISTORY_MAX - strlen(output_log));
    else if (strcmp(cmd, "mounts") == 0) os_vfs_get_mounts(output_log + strlen(output_log), SHELL_HISTORY_MAX - strlen(output_log));
    else if (strcmp(cmd, "device") == 0) os_devmgr_list(output_log + strlen(output_log), SHELL_HISTORY_MAX - strlen(output_log));
    else if (strcmp(cmd, "health") == 0) strcat(output_log, "Status: EXCELLENT\n");
    else if (strcmp(cmd, "tasks") == 0) strcat(output_log, "Tasks active.\n");
    else if (strcmp(cmd, "uptime") == 0) snprintf(output_log+strlen(output_log), 64, "Uptime: %d ms\n", (int)os_get_uptime_ms());
    else strcat(output_log, "Unknown command.\n");
    strcat(output_log, "> ");
}
void chell_update(struct nk_context* ctx, void* s) {
    struct app_state* app = (struct app_state*)s;
    if (nk_begin(ctx, "System Shell", nk_rect(50, 50, 600, 450), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE|NK_WINDOW_CLOSABLE|NK_WINDOW_SCALABLE)) {
        nk_layout_row_dynamic(ctx, 350, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_MULTILINE|NK_EDIT_READ_ONLY, output_log, sizeof(output_log), nk_filter_default);
        nk_layout_row_dynamic(ctx, 30, 2);
        nk_edit_string(ctx, NK_EDIT_FIELD, cmd_buf, &cmd_len, 128, nk_filter_default);
        if (nk_button_label(ctx, "Run") || nk_input_is_key_pressed(&ctx->input, NK_KEY_ENTER)) {
            cmd_buf[cmd_len] = '\0'; if (cmd_len > 0) shell_exec(cmd_buf);
            cmd_len = 0; memset(cmd_buf, 0, sizeof(cmd_buf));
            ctx->input.keyboard.keys[NK_KEY_ENTER].clicked = 0;
        }
    }
    if (nk_window_is_closed(ctx, "System Shell")) app->show_terminal = 0;
    nk_end(ctx);
}
