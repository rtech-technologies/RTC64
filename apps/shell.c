#include "user_rsl.h"

int main(void) {
    rsl_printf("Hello from Userland Shell!\n");
    char buf[1024];
    if (rsl_ls("/", buf, sizeof(buf)) == 0) {
        rsl_printf("Root Directory:\n");
        rsl_printf(buf);
    }
    return 0;
}
