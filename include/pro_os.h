#ifndef PRO_OS_H
#define PRO_OS_H

#include "limine.h"
#include "nuklear.h"
#include "external/stb_image.h"
#include "external/stb_truetype.h"
#include "external/tlsf.h"
#include "external/tgx.h"
#include "wolfssl/wolfip.h"
#include "hal.h"

/* Scheduler / Task Manager */
#define MAX_TASKS 5
typedef enum { TASK_IDLE, TASK_RUNNING, TASK_SQUEEZED } task_state_t;

typedef struct {
    int id;
    const char *name;
    task_state_t state;
    void (*entry)(void);
} task_t;

void scheduler_init(void);
void scheduler_add_task(const char *name, void (*entry)(void));
void scheduler_run(void);

/* VFS */
void vfs_init(void);
const char* vfs_resolve(const char *path);

/* Security / UAC */
typedef struct {
    bool can_network;
    bool can_storage;
    bool can_input;
} app_permit_t;

bool uac_check_permit(int app_id, const char *action);
void uac_request_permit(int app_id, const char *action);

/* I18n */
const char* i18n_translate(const char *key);

#endif
