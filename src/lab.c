/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "nuklear.h"
#include "os_api.h"
#include <string.h>
#include "app_ui.h"

void lab_init(void* s) {
    struct app_state* app = (struct app_state*)s;
    if (app) app->show_explorer = 1;
}

void lab_update(struct nk_context* ctx, void* s) {
    struct app_state* app = (struct app_state*)s;
    (void)app;
    if (nk_begin(ctx, "Sovereign Lab", nk_rect(150, 150, 400, 300), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE|NK_WINDOW_CLOSABLE)) {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, "System Diagnostics", NK_TEXT_LEFT);

        static char dev_list[1024];
        if (nk_button_label(ctx, "Refresh Hardware List")) {
            os_devmgr_list(dev_list, sizeof(dev_list));
        }

        nk_layout_row_dynamic(ctx, 150, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_MULTILINE | NK_EDIT_READ_ONLY, dev_list, sizeof(dev_list), nk_filter_default);
    }
    nk_end(ctx);
}
