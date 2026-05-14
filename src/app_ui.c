#include <string.h>
#include <stdio.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"
#include "app_ui.h"
#include "services.h"

void ui_render(struct nk_context *ctx, struct app_state *app, int window_width, int window_height)
{
    if (app->current_state == STATE_LOGIN) {
        if (nk_begin(ctx, "Login", nk_rect(window_width/2 - 150, window_height/2 - 100, 300, 200),
            NK_WINDOW_BORDER|NK_WINDOW_NO_SCROLLBAR))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, "R-TECH SYSTEM LOGIN", NK_TEXT_CENTERED);
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->username, sizeof(app->username), nk_filter_default);
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app->password, sizeof(app->password), nk_filter_default);
            nk_layout_row_dynamic(ctx, 30, 1);
            if (nk_button_label(ctx, "Login")) app->current_state = STATE_INSTALLER;
        }
        nk_end(ctx);
    } else if (app->current_state == STATE_INSTALLER) {
        if (nk_begin(ctx, "Installer", nk_rect(window_width/2 - 200, window_height/2 - 150, 400, 300),
            NK_WINDOW_BORDER|NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label(ctx, "Welcome to R-TECH Installer", NK_TEXT_LEFT);
            nk_layout_row_dynamic(ctx, 150, 1);
            nk_label_wrap(ctx, "Installing system features...");
            if (app->install_started) {
                nk_layout_row_dynamic(ctx, 30, 1);
                nk_size prog = (nk_size)app->progress;
                nk_progress(ctx, &prog, 100, NK_MODIFIABLE);
                if (app->progress < 100) app->progress++;
                else if (nk_button_label(ctx, "Finish")) app->current_state = STATE_MAIN;
            } else if (nk_button_label(ctx, "Start")) app->install_started = 1;
        }
        nk_end(ctx);
    } else {
        if (nk_begin(ctx, "R-TECH Console", nk_rect(10, 10, window_width-20, window_height-20),
            NK_WINDOW_BORDER|NK_WINDOW_TITLE))
        {
            nk_layout_row_dynamic(ctx, 30, 2);
            if (g_services.usb) nk_label(ctx, "USB: Active", NK_TEXT_LEFT);
            else nk_label(ctx, "USB: Missing", NK_TEXT_LEFT);

            nk_layout_row_static(ctx, 100, 150, 4);

            /* STORAGE SERVICE */
            if (!g_services.storage) nk_widget_disable_begin(ctx);
            if (nk_button_label(ctx, "Storage")) {
                if (g_services.storage && g_services.storage->do_work) g_services.storage->do_work();
            }
            if (!g_services.storage) nk_widget_disable_end(ctx);

            /* CLOCK SERVICE */
            if (!g_services.clock) nk_widget_disable_begin(ctx);
            if (nk_button_label(ctx, "Clock")) {
                if (g_services.clock && g_services.clock->do_work) g_services.clock->do_work();
            }
            if (!g_services.clock) nk_widget_disable_end(ctx);

            /* NETWORK SERVICE */
            if (!g_services.network) nk_widget_disable_begin(ctx);
            if (nk_button_label(ctx, "Network")) {
                if (g_services.network && g_services.network->do_work) g_services.network->do_work();
            }
            if (!g_services.network) nk_widget_disable_end(ctx);
        }
        nk_end(ctx);
    }
}
