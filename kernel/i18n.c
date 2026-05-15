#include "pro_os.h"
#include <string.h>

/* Direct translation engine */
const char* i18n_translate(const char *key) {
    if (strcmp(key, "welcome") == 0) return "welcome back user!";
    if (strcmp(key, "login") == 0) return "Login";
    return key;
}
