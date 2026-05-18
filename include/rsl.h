#ifndef RSL_H
#define RSL_H

#include <pro_os.h>
#include <stdarg.h>

// String interface
void* str_create(const char* cstr);
int str_match(void* s1, void* s2);
int str_is_empty(void* s);
void rsl_printf(const char* fmt, ...);

// Console interface
void set_color(uint32_t fg, uint32_t bg);
void print(const char* msg);
char* input(const char* prompt);

#endif
