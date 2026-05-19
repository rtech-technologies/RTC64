#ifndef INSTALLER_H
#define INSTALLER_H

#include "nuklear.h"

struct installer_state {
    int active;
    int step; /* 0: Drive selection, 1: Formatting, 2: Done */
    int selected_drive;
    int selected_partition;
    char status[128];
    int progress;
};

void installer_init(struct installer_state *state);
void installer_ui_render(struct nk_context *ctx, struct installer_state *state);

#endif
