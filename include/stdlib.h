/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people' property' OS license. */
#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

void* malloc(size_t size);
void free(void* ptr);
void* calloc(size_t nmemb, size_t size);
void* realloc(void* ptr, size_t size);
int abs(int n);
long strtol(const char* nptr, char** endptr, int base);
void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));

#endif
