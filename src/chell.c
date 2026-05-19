#include "chell.h"
#include "pro_os.h"
#include <string.h>
#include <stdio.h>

/*
 * Chell - Sovereign OS Shell App
 * This proves modular application development for RTC64.
 */

void chell_init(struct chell_state *state) {
    memset(state, 0, sizeof(struct chell_state));
    strcpy(state->output, "Chell Kernel Shell v0.1 (FatFs Ready)\nType 'help' for commands.\n");
}

static void chell_append(struct chell_state *state, const char* text) {
    size_t cur_len = strlen(state->output);
    size_t add_len = strlen(text);
    if (cur_len + add_len < sizeof(state->output) - 1) {
        memcpy(state->output + cur_len, text, add_len);
        state->output[cur_len + add_len] = 0;
    }
}

static void chell_execute(struct chell_state *state) {
    if (strcmp(state->input, "ls") == 0) {
        char ls_buf[1024];
        if (vfs_ls("0:", ls_buf, sizeof(ls_buf)) == 0) {
            chell_append(state, ls_buf);
        } else {
            chell_append(state, "Error: Could not list directory (Drive 0).\n");
        }
    } else if (strcmp(state->input, "help") == 0) {
        chell_append(state, "Available commands: ls, help, clear, whoami\n");
    } else if (strcmp(state->input, "clear") == 0) {
        state->output[0] = 0;
    } else if (strcmp(state->input, "whoami") == 0) {
        chell_append(state, "Sovereign User (Ring 0)\n");
    } else if (state->input[0] != 0) {
        char buf[128];
        snprintf(buf, sizeof(buf), "Unknown command: %s\n", state->input);
        chell_append(state, buf);
    }
    state->input[0] = 0;
}

void chell_ui_render(struct nk_context *ctx, struct chell_state *state) {
    if (!state->active) return;

    if (nk_begin(ctx, "chell", nk_rect(120, 120, 500, 400),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE))
    {
        nk_layout_row_dynamic(ctx, 280, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_SELECTABLE|NK_EDIT_MULTILINE|NK_EDIT_READ_ONLY,
                                       state->output, sizeof(state->output), nk_filter_default);

        nk_layout_row_dynamic(ctx, 30, 2);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, state->input, sizeof(state->input), nk_filter_default);
        if (nk_button_label(ctx, "EXEC")) {
            chell_execute(state);
        }
    }
    if (nk_window_is_closed(ctx, "chell")) state->active = 0;
    nk_end(ctx);
}
