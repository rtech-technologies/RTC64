#ifndef APP_UI_H
#define APP_UI_H
#include "pro_os.h"
typedef enum { STATE_LOGIN, STATE_INSTALLER, STATE_DESKTOP, STATE_LAB, STATE_CHELL } app_screen_t;
struct app_state {
    app_screen_t current_state;
    void* chell; void* lab; void* installer;
    int show_launcher; int show_terminal; int show_explorer; int show_settings; int show_uac;
    char password[32]; int perm_net; int perm_storage; int cpu_usage;
};
void ui_init_style(struct nk_context* ctx);
void ui_render(struct nk_context* ctx, struct app_state* app, int width, int height);
void chell_init(void* state); void chell_update(struct nk_context* ctx, void* state);
void lab_init(void* state); void lab_update(struct nk_context* ctx, void* state);
void installer_init(void* state); void installer_update(struct nk_context* ctx, void* state);
#endif
