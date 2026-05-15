#ifndef WOLFIP_H
#define WOLFIP_H

/* Simplified WolfIP interface for the "Bridge" */
typedef struct {
    uint32_t ip;
    uint32_t netmask;
    uint32_t gateway;
} wolfip_config_t;

int wolfip_init(wolfip_config_t *config);
void wolfip_poll(void);

#endif
