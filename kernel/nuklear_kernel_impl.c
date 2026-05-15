#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Core math implementations for Nuklear in freestanding mode */
double pow(double x, double y) { (void)x; (void)y; return 0; }
double sqrt(double x) { (void)x; return 0; }
double sin(double x) { (void)x; return 0; }
double cos(double x) { (void)x; return 0; }

/* Core memory implementations for Nuklear and CherryUSB */
void* memset(void* s, int c, size_t n) {
    unsigned char* p = s;
    while(n--) *p++ = (unsigned char)c;
    return s;
}

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    while(n--) *d++ = *s++;
    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char *p1 = s1, *p2 = s2;
    while(n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++; p2++;
    }
    return 0;
}

size_t strlen(const char* s) {
    size_t len = 0;
    while(*s++) len++;
    return len;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while((*d++ = *src++));
    return dest;
}

int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while(n--) {
        if(*s1 != *s2) return *(unsigned char*)s1 - *(unsigned char*)s2;
        if(*s1 == 0) break;
        s1++; s2++;
    }
    return 0;
}

char* strchr(const char* s, int c) {
    while(*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    if (c == 0) return (char*)s;
    return NULL;
}

long strtol(const char* nptr, char** endptr, int base) {
    (void)nptr; (void)endptr; (void)base;
    return 0;
}

#define NK_IMPLEMENTATION
#include "pro_os.h"

void* malloc(size_t size) { return tlsf_malloc(NULL, size); }
void free(void* ptr) { tlsf_free(NULL, ptr); }
void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function) { (void)assertion; (void)file; (void)line; (void)function; }
