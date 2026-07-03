/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#ifndef APP_LOADER_H
#define APP_LOADER_H

#include "app_ui.h"

int app_spawn_script(const char* script_path);
void app_loader_update(struct nk_context *ctx, struct app_state *app);

#endif /* APP_LOADER_H */
