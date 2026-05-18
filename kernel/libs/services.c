#include <pro_os.h>

static service_func_t services[16];
static int service_count = 0;

void register_service(service_func_t init_func) {
    if (service_count < 16) {
        services[service_count++] = init_func;
    }
}

void dispatch_event(kernel_event_t event) {
    for (int i = 0; i < service_count; i++) {
        if (services[i]) {
            services[i](event);
        }
    }
}
