/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "nuklear.h"
#include "rsl.h"
#include <stdio.h>
#include <string.h>
#include "app_ui.h"
#include "app_loader.h"

void chell_init(void* s) {
    struct app_state* app = (struct app_state*)s;
    if (app) app->show_terminal = 1;
}

void chell_update(struct nk_context* ctx, void* s) {
    struct app_state* app = (struct app_state*)s;
    (void)app;
    if (nk_begin(ctx, "Sovereign Executive Shell", nk_rect(50, 50, 600, 450), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE|NK_WINDOW_CLOSABLE)) {
        static char output[4096] = "Welcome to the Sovereign Executive Environment.\nType 'help' for commands.\n";
        static char cmd[128];
        static int cmd_len = 0;

        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "Rtech Standard Userland v3.0", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 250, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_MULTILINE | NK_EDIT_READ_ONLY, output, sizeof(output), nk_filter_default);

        nk_layout_row_template_begin(ctx, 30);
        nk_layout_row_template_push_dynamic(ctx);
        nk_layout_row_template_push_static(ctx, 80);
        nk_layout_row_template_end(ctx);

        nk_edit_string(ctx, NK_EDIT_FIELD, cmd, &cmd_len, 128, nk_filter_default);
        if (nk_button_label(ctx, "Execute") || (nk_input_is_key_pressed(&ctx->input, NK_KEY_ENTER))) {
            cmd[cmd_len] = '\0';
            if (strncmp(cmd, "ls ", 3) == 0) {
                rsl_ls(cmd + 3, output, sizeof(output));
            } else if (strncmp(cmd, "cat ", 4) == 0) {
                rsl_cat(cmd + 4, output, sizeof(output));
            } else if (strcmp(cmd, "mounts") == 0) {
                rsl_mounts(output, sizeof(output));
            } else if (strcmp(cmd, "hw") == 0 || strcmp(cmd, "pci") == 0) {
                rsl_hw_list(output, sizeof(output));
            } else if (strcmp(cmd, "uptime") == 0) {
                rsl_printf("System Uptime: %llu ms\n", rsl_uptime());
                strcpy(output, "Uptime printed to diagnostic log.");
            } else if (strcmp(cmd, "help") == 0) {
                strcpy(output, "Userland Commands: ls <path>, cat <path>, mounts, hw, uptime, help, clear");
            } else if (strcmp(cmd, "clear") == 0) {
                memset(output, 0, sizeof(output));
            } else if (strncmp(cmd, "run ", 4) == 0) {
                char* path = cmd + 4;
                if (app_spawn_script(path) < 0) {
                    snprintf(output, sizeof(output), "Failed to run app: %s\n", path);
                } else {
                    snprintf(output, sizeof(output), "Launched script: %s\n", path);
                }
            } else if (strlen(cmd) > 0) {
                rsl_printf("Userland Attempt: %s\n", cmd);
                strcpy(output, "Command dispatched to executive log.");
            }
            cmd_len = 0;
            /* Clear focus from input to allow typing the next command */
            nk_edit_unfocus(ctx);
        }
    }
    nk_end(ctx);
}
