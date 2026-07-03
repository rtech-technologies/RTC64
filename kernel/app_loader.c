/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include "app_loader.h"
#include "rsl.h"
#include "nuklear.h"
#include "serial.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/* Local workspace provides include/string.h which shadows the system header
 * and omits some helpers we rely on here (strtok, strstr). Declare them
 * explicitly to satisfy the freestanding build. */
/* Lightweight local helpers to avoid depending on libc in freestanding build. */
static char *app_tok_ptr = NULL;
static char *app_strtok(char *str, const char *delim) {
    if (str) app_tok_ptr = str;
    if (!app_tok_ptr) return NULL;
    char *start = app_tok_ptr;
    char *p = start;
    while (*p && !strchr(delim, *p)) p++;
    if (*p) {
        *p = '\0';
        app_tok_ptr = p + 1;
    } else {
        app_tok_ptr = NULL;
    }
    return start;
}

static const char *app_strstr(const char *haystack, const char *needle) {
    if (!*needle) return haystack;
    for (; *haystack; haystack++) {
        const char *h = haystack; const char *n = needle;
        while (*h && *n && *h == *n) { h++; n++; }
        if (!*n) return haystack;
    }
    return NULL;
}

#define APP_LOADER_MAX_SCRIPT    16384
#define APP_LOADER_MAX_LINES     256
#define APP_LOADER_MAX_BUTTONS   16
#define APP_LOADER_MAX_OUTPUT    8192
#define APP_LOADER_MAX_COMMANDS  256

typedef enum {
    CMD_TITLE,
    CMD_LABEL,
    CMD_BUTTON,
    CMD_PRINT,
    CMD_EXIT,
    CMD_BLOCK
} loader_cmd_type_t;

typedef struct {
    loader_cmd_type_t type;
    char text[128];
    char id[32];
    int child_start;
    int child_count;
} loader_cmd_t;

static struct {
    bool loaded;
    bool active;
    char path[128];
    char title[64];
    loader_cmd_t commands[APP_LOADER_MAX_COMMANDS];
    int command_count;
    char output[APP_LOADER_MAX_OUTPUT];
    int output_len;
    struct {
        char id[32];
        char label[64];
        bool pressed;
    } buttons[APP_LOADER_MAX_BUTTONS];
    int button_count;
} state;

static const char* trim_leading(const char* str) {
    while (*str == ' ' || *str == '\t') str++;
    return str;
}

static void trim_trailing(char* str) {
    int len = strlen(str);
    while (len > 0 && (str[len-1] == '\n' || str[len-1] == '\r' || str[len-1] == ' ' || str[len-1] == '\t')) {
        str[--len] = '\0';
    }
}

static bool parse_quoted(const char* src, char* dst, int dst_size) {
    const char* begin = src;
    while (*begin == ' ' || *begin == '\t') begin++;
    if (*begin == '"') {
        begin++;
        const char* end = strchr(begin, '"');
        if (!end) return false;
        int len = (int)(end - begin);
        if (len >= dst_size) len = dst_size - 1;
        memcpy(dst, begin, len);
        dst[len] = '\0';
        return true;
    }
    /* Unquoted fallback, take entire remainder */
    int len = 0;
    while (*begin && *begin != '\n' && *begin != '\r' && len < dst_size - 1) {
        dst[len++] = *begin++;
    }
    dst[len] = '\0';
    return len > 0;
}

static int find_block_end(int start) {
    int count = 0;
    for (int i = start + 1; i < state.command_count; i++) {
        if (state.commands[i].type == CMD_BLOCK) break;
        count++;
    }
    return count;
}

static void append_output(const char* text) {
    int space = APP_LOADER_MAX_OUTPUT - state.output_len - 2;
    if (space <= 0) return;
    int len = snprintf(state.output + state.output_len, space + 1, "%s\n", text);
    if (len > 0) state.output_len += len;
}

static int find_button(const char* id) {
    for (int i = 0; i < state.button_count; i++) {
        if (strcmp(state.buttons[i].id, id) == 0) return i;
    }
    return -1;
}

static void execute_block(int block_index) {
    loader_cmd_t* block = &state.commands[block_index];
    for (int i = 0; i < block->child_count; i++) {
        loader_cmd_t* cmd = &state.commands[block->child_start + i];
        if (cmd->type == CMD_PRINT) {
            append_output(cmd->text);
        } else if (cmd->type == CMD_EXIT) {
            state.active = false;
            break;
        }
    }
}

static bool parse_script(const char* script) {
    memset(&state, 0, sizeof(state));
    strncpy(state.title, "Script App", sizeof(state.title) - 1);

    char buffer[APP_LOADER_MAX_SCRIPT];
    strncpy(buffer, script, sizeof(buffer) - 1);
    buffer[sizeof(buffer)-1] = '\0';

    char* line = app_strtok(buffer, "\n");
    int current_block = -1;

    while (line && state.command_count < APP_LOADER_MAX_COMMANDS) {
        trim_trailing(line);
        const char* raw = trim_leading(line);
        if (*raw == '\0' || *raw == '#') {
                line = app_strtok(NULL, "\n");
            continue;
        }

        int indent = 0;
        const char* p = line;
        while (*p == ' ' || *p == '\t') { indent++; p++; }
        raw = trim_leading(line);

        if (indent == 0) {
            current_block = -1;
            if (strncmp(raw, "title ", 6) == 0) {
                parse_quoted(raw + 6, state.title, sizeof(state.title));
            } else if (strncmp(raw, "print ", 6) == 0) {
                loader_cmd_t* cmd = &state.commands[state.command_count++];
                cmd->type = CMD_LABEL;
                parse_quoted(raw + 6, cmd->text, sizeof(cmd->text));
            } else if (strncmp(raw, "button ", 7) == 0) {
                if (state.button_count >= APP_LOADER_MAX_BUTTONS) {
                    line = app_strtok(NULL, "\n");
                    continue;
                }
                char label[64] = "";
                char id[32] = "";
                const char* ptr = raw + 7;
                parse_quoted(ptr, label, sizeof(label));
                const char* as_str = app_strstr(ptr, " as ");
                if (as_str) {
                    parse_quoted(as_str + 4, id, sizeof(id));
                    if (!id[0]) {
                        const char* token = as_str + 4;
                        while (*token == ' ' || *token == '\t') token++;
                        strncpy(id, token, sizeof(id) - 1);
                        id[sizeof(id)-1] = '\0';
                    }
                }
                if (!id[0]) {
                    int len = (int)strlen(label);
                    int pos = 0;
                    for (int i = 0; i < len && pos + 1 < (int)sizeof(id); i++) {
                        char c = label[i];
                        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_')) {
                            id[pos++] = c;
                        } else if (c == ' ' && pos > 0) {
                            id[pos++] = '_';
                        }
                    }
                    id[pos] = '\0';
                }
                loader_cmd_t* button = &state.commands[state.command_count++];
                button->type = CMD_BUTTON;
                strncpy(button->text, label, sizeof(button->text) - 1);
                strncpy(button->id, id, sizeof(button->id) - 1);
                strncpy(state.buttons[state.button_count].id, id, sizeof(state.buttons[state.button_count].id) - 1);
                strncpy(state.buttons[state.button_count].label, label, sizeof(state.buttons[state.button_count].label) - 1);
                state.buttons[state.button_count].pressed = false;
                state.button_count++;
            } else if (strncmp(raw, "when ", 5) == 0) {
                const char* colon = strchr(raw, ':');
                char trigger[32] = "";
                if (colon) {
                    int len = (int)(colon - (raw + 5));
                    if (len >= (int)sizeof(trigger)) len = sizeof(trigger)-1;
                    memcpy(trigger, raw + 5, len);
                    trigger[len] = '\0';
                    trim_trailing(trigger);
                } else {
                    parse_quoted(raw + 5, trigger, sizeof(trigger));
                }
                loader_cmd_t* block = &state.commands[state.command_count++];
                block->type = CMD_BLOCK;
                strncpy(block->id, trigger, sizeof(block->id) - 1);
                block->child_start = state.command_count;
                block->child_count = 0;
                current_block = state.command_count - 1;
            } else if (strcmp(raw, "exit") == 0) {
                loader_cmd_t* cmd = &state.commands[state.command_count++];
                cmd->type = CMD_EXIT;
            }
        } else if (current_block >= 0) {
            loader_cmd_t* parent = &state.commands[current_block];
            loader_cmd_t* child = &state.commands[state.command_count++];
            if (strncmp(raw, "print ", 6) == 0) {
                child->type = CMD_PRINT;
                parse_quoted(raw + 6, child->text, sizeof(child->text));
            } else if (strcmp(raw, "exit") == 0) {
                child->type = CMD_EXIT;
            } else {
                child->type = CMD_LABEL;
                parse_quoted(raw, child->text, sizeof(child->text));
            }
            parent->child_count++;
        }

        line = app_strtok(NULL, "\n");
    }

    return true;
}

static void app_entry_stub(void* arg) {
    void (*entry)() = (void (*)(void))arg;
    entry();
    rsl_exit(0);
}

int app_spawn_binary(const char* path) {
    if (!path) return -1;
    /* MEATY: Load flat binary to a fixed high memory address for demo.
     * In a real OS we'd use a per-process page table and ELF loader. */
    void* load_addr = pmm_alloc_blocks(16); // 64KB
    if (!load_addr) return -1;
    void* virt_addr = (void*)((uint64_t)load_addr + hhdm_offset);

    int bytes = rsl_read(path, virt_addr, 16 * 4096);
    if (bytes <= 0) {
        /* free... */
        return -1;
    }

    serial_printf("[LOADER] Loaded binary %s (%d bytes) at %p\n", path, bytes, virt_addr);

    return scheduler_spawn(path, app_entry_stub, virt_addr);
}

int app_spawn_script(const char* script_path) {
    if (!script_path) return -1;

    /* If it ends in .bin, try binary spawn */
    if (app_strstr(script_path, ".bin")) {
        return app_spawn_binary(script_path);
    }

    char buffer[APP_LOADER_MAX_SCRIPT];
    int bytes = rsl_read(script_path, buffer, sizeof(buffer) - 1);
    if (bytes < 0) return -1;
    if (bytes >= (int)sizeof(buffer)) bytes = sizeof(buffer) - 1;
    buffer[bytes] = '\0';
    if (!parse_script(buffer)) return -1;
    strncpy(state.path, script_path, sizeof(state.path) - 1);
    state.loaded = true;
    state.active = true;
    state.output_len = 0;
    return 0;
}

void app_loader_update(struct nk_context *ctx, struct app_state *app) {
    if (!state.loaded) {
        if (nk_begin(ctx, "Script App", nk_rect(280, 180, 420, 240), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_TITLE)) {
            nk_layout_row_dynamic(ctx, 24, 1);
            nk_label(ctx, "No script loaded.", NK_TEXT_CENTERED);
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_label_wrap(ctx, "Use the App Studio to create an RSL-style app script and then run it from the desktop.");
            if (nk_button_label(ctx, "Close")) {
                app->show_script_app = 0;
            }
        }
        nk_end(ctx);
        return;
    }

    if (!state.active) {
        app->show_script_app = 0;
        return;
    }

    if (nk_begin(ctx, state.title, nk_rect(260, 140, 520, 420), NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|NK_WINDOW_CLOSABLE|NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, state.path, NK_TEXT_LEFT);

        if (state.button_count > 0) {
            nk_layout_row_dynamic(ctx, 36, 2);
            for (int i = 0; i < state.button_count; i++) {
                if (nk_button_label(ctx, state.buttons[i].label)) {
                    state.buttons[i].pressed = true;
                }
            }
        }

        for (int i = 0; i < state.command_count; i++) {
            loader_cmd_t* cmd = &state.commands[i];
            if (cmd->type == CMD_LABEL) {
                nk_layout_row_dynamic(ctx, 22, 1);
                nk_label(ctx, cmd->text, NK_TEXT_LEFT);
            }
        }

        for (int i = 0; i < state.command_count; i++) {
            loader_cmd_t* cmd = &state.commands[i];
            if (cmd->type == CMD_BLOCK) {
                int btn = find_button(cmd->id);
                if (btn >= 0 && state.buttons[btn].pressed) {
                    execute_block(i);
                    state.buttons[btn].pressed = false;
                }
            }
        }

        nk_layout_row_dynamic(ctx, 120, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_MULTILINE|NK_EDIT_READ_ONLY, state.output, sizeof(state.output), nk_filter_default);
    }
    if (nk_window_is_closed(ctx, state.title)) {
        app->show_script_app = 0;
    }
    nk_end(ctx);
}
