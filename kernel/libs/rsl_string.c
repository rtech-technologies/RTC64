#include <rsl.h>
#define STB_SPRINTF_IMPLEMENTATION
#include <external/stb_sprintf.h>

extern void* arc_alloc(size_t size);

void* str_create(const char* cstr) {
    size_t len = 0;
    const char* p = cstr;
    while(*p++) len++;

    char* s = arc_alloc(len + 1);
    char* d = s;
    p = cstr;
    while((*d++ = *p++));
    return s;
}

int str_match(void* s1, void* s2) {
    char* c1 = (char*)s1;
    char* c2 = (char*)s2;
    while(*c1 && (*c1 == *c2)) {
        c1++; c2++;
    }
    return (*c1 == *c2);
}

int str_is_empty(void* s) {
    return s == NULL || *(char*)s == '\0';
}

void rsl_printf(const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    stbsp_vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    print(buf);
}
