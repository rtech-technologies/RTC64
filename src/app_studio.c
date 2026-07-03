/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "nuklear.h"
#include "app_ui.h"
#include "app_loader.h"
#include "rsl.h"
#include <string.h>
#include <stdio.h>

void studio_init(void* s) {
    struct app_state* app = (struct app_state*)s;
    if (app) app->show_app_studio = 1;
}

void studio_update(struct nk_context* ctx, void* s) {
    struct app_state* app = (struct app_state*)s;
    (void)app;
    if (nk_begin(ctx, "App Studio", nk_rect(220, 140, 520, 460),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE))
    {
        static char app_name[64] = "new_app";
        static char app_path[128] = "/mnt/disk0/scripts/new_app.rsl";
        static char output[2048] = "title \"Hello, Sovereign\"\nprint(\"This is an RSL script app.\")\nbutton(\"Close\", \"close\")\nlabel close\nprint(\"Goodbye.\")\nexit()\n";
        static char status[128] = "";

        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "Rapid app creation for RTC64 scripts", NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 28, 1);
        nk_label(ctx, "App name:", NK_TEXT_LEFT);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app_name, sizeof(app_name), nk_filter_default);

        nk_layout_row_dynamic(ctx, 28, 1);
        nk_label(ctx, "Save path:", NK_TEXT_LEFT);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app_path, sizeof(app_path), nk_filter_default);

        nk_layout_row_dynamic(ctx, 28, 1);
        nk_label(ctx, status, NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 30, 3);
        if (nk_button_label(ctx, "Save Script")) {
            if (rsl_write(app_path, output) == 0) {
                snprintf(status, sizeof(status), "Saved %s", app_path);
            } else {
                snprintf(status, sizeof(status), "Failed to save %s", app_path);
            }
        }
        if (nk_button_label(ctx, "Run Script")) {
            if (app_spawn_script(app_path) == 0) {
                snprintf(status, sizeof(status), "Running %s", app_path);
            } else {
                snprintf(status, sizeof(status), "Failed to open %s", app_path);
            }
        }
        if (nk_button_label(ctx, "Reset")) {
            strncpy(output, "title \"Hello, Sovereign\"\nprint(\"This is an RSL script app.\")\nbutton(\"Close\", \"close\")\nlabel close\nprint(\"Goodbye.\")\nexit()\n", sizeof(output));
            status[0] = '\0';
        }

        nk_layout_row_dynamic(ctx, 250, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_MULTILINE, output, sizeof(output), nk_filter_default);
    }
    if (nk_window_is_closed(ctx, "App Studio") && app) {
        app->show_app_studio = 0;
    }
    nk_end(ctx);
}
