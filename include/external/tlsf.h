#ifndef TLSF_H
#define TLSF_H

#include <stddef.h>

/* Simplified TLSF interface for the "Pro" OS */
typedef void* tlsf_t;

tlsf_t tlsf_create_with_pool(void* mem, size_t bytes);
void* tlsf_malloc(tlsf_t tlsf, size_t size);
void tlsf_free(tlsf_t tlsf, void* ptr);
void* tlsf_realloc(tlsf_t tlsf, void* ptr, size_t size);

#endif
