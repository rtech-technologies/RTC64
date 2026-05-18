#include <pro_os.h>
#include <external/TLSF/tlsf.h>

#define KERNEL_HEAP_SIZE (64 * 1024 * 1024)
static uint8_t kernel_heap[KERNEL_HEAP_SIZE] __attribute__((aligned(8)));
static tlsf_t kernel_pool = NULL;

void libc_init(void) {
    if (!kernel_pool) {
        kernel_pool = tlsf_create_with_pool(kernel_heap, KERNEL_HEAP_SIZE);
    }
}

void* memset(void* s, int c, size_t n) {
    uint8_t* p = (uint8_t*)s;
    while(n--) *p++ = (uint8_t)c;
    return s;
}

void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    while(n--) *d++ = *s++;
    return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const uint8_t* p1 = (const uint8_t*)s1;
    const uint8_t* p2 = (const uint8_t*)s2;
    while(n--) {
        if (*p1 != *p2) return (int)*p1 - (int)*p2;
        p1++; p2++;
    }
    return 0;
}

size_t strlen(const char* s) {
    size_t len = 0;
    while(s[len]) len++;
    return len;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++; s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++; s2++; n--;
    }
    if (n == 0) return 0;
    return (unsigned char)*s1 - (unsigned char)*s2;
}

char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) dest[i] = src[i];
    for ( ; i < n; i++) dest[i] = '\0';
    return dest;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

char* strchr(const char* s, int c) {
    while(*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return NULL;
}

void* malloc(size_t size) {
    if (!kernel_pool) libc_init();
    return tlsf_malloc(kernel_pool, size);
}

void free(void* ptr) {
    if (kernel_pool && ptr) tlsf_free(kernel_pool, ptr);
}

void* realloc(void* ptr, size_t size) {
    if (!kernel_pool) libc_init();
    return tlsf_realloc(kernel_pool, ptr, size);
}

void* calloc(size_t nmemb, size_t size) {
    void* ptr = malloc(nmemb * size);
    if (ptr) memset(ptr, 0, nmemb * size);
    return ptr;
}

void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function) {
    (void)assertion; (void)file; (void)line; (void)function;
    extern void quartermaster_panic(const char* msg);
    quartermaster_panic("Assertion Failed");
}

int printf(const char* fmt, ...) {
    (void)fmt; return 0;
}
