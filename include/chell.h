#ifndef CHELL_H
#define CHELL_H

#include "nuklear.h"

struct chell_state {
    int active;
    char input[64];
    char output[2048];
};

void chell_init(struct chell_state *state);
void chell_ui_render(struct nk_context *ctx, struct chell_state *state);

#endif
