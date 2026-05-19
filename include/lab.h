#ifndef LAB_H
#define LAB_H

#include "nuklear.h"

struct lab_state {
    int active;
    int current_tab;

    /* Storage Test */
    char test_status[128];

    /* Input Test */
    int last_x, last_y;
    uint32_t last_key;

    /* VFS Test */
    char file_content[512];
    char file_path[64];
};

void lab_init(struct lab_state *state);
void lab_ui_render(struct nk_context *ctx, struct lab_state *state);

#endif
