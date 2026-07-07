/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include <stdint.h>
#include "pro_os.h"

#define PI 3.14159265358979323846
#define PI_2 1.57079632679489661923

double fabs(double x) { return x < 0 ? -x : x; }

double floor(double x) {
    if (x >= 0) return (double)((long long)x);
    long long i = (long long)x;
    if (x == (double)i) return x;
    return (double)(i - 1);
}

double ceil(double x) {
    if (x >= 0) {
        long long i = (long long)x;
        if (x == (double)i) return x;
        return (double)(i + 1);
    }
    return (double)((long long)x);
}

double sqrt(double x) {
    if (x <= 0) return 0;
    double res = x;
    for (int i = 0; i < 20; i++) res = 0.5 * (res + x / res);
    return res;
}

double log(double x) {
    if (x <= 0) return -1.0e300; /* Minimal sanity */
    double res = 0;
    double y = (x - 1) / (x + 1);
    double y2 = y * y;
    double term = y;
    for (int i = 0; i < 20; i++) {
        res += term / (2 * i + 1);
        term *= y2;
    }
    return 2 * res;
}

double exp(double x) {
    double res = 1.0;
    double term = 1.0;
    for (int i = 1; i <= 20; i++) {
        term *= x / (double)i;
        res += term;
    }
    return res;
}

double pow(double x, double y) {
    if (y == 0) return 1.0;
    if (x == 0) return 0;
    if (y == 1.0) return x;
    if (y == (double)((int)y)) {
        int iy = (int)y;
        double res = 1.0;
        double base = x;
        if (iy < 0) { base = 1.0 / base; iy = -iy; }
        while (iy > 0) {
            if (iy & 1) res *= base;
            base *= base;
            iy >>= 1;
        }
        return res;
    }
    if (x < 0) return 0;
    return exp(y * log(x));
}

double sin(double x) {
    while (x > PI) x -= 2.0 * PI;
    while (x < -PI) x += 2.0 * PI;
    double res = 0, term = x, x2 = x * x;
    for (int i = 1; i <= 19; i += 2) {
        res += term;
        term *= -x2 / ((double)(i + 1) * (double)(i + 2));
    }
    return res;
}

double cos(double x) {
    while (x > PI) x -= 2.0 * PI;
    while (x < -PI) x += 2.0 * PI;
    double res = 0, term = 1.0, x2 = x * x;
    for (int i = 0; i <= 18; i += 2) {
        res += term;
        term *= -x2 / ((double)(i + 1) * (double)(i + 2));
    }
    return res;
}

double atan(double x) {
    int neg = 0; if (x < 0) { neg = 1; x = -x; }
    int invert = 0; if (x > 1.0) { invert = 1; x = 1.0 / x; }
    double res = 0, term = x, x2 = x * x;
    for (int i = 1; i <= 39; i += 2) {
        res += term / (double)i;
        term *= -x2;
    }
    if (invert) res = PI_2 - res;
    if (neg) res = -res;
    return res;
}

double atan2(double y, double x) {
    if (x > 0) return atan(y / x);
    if (x < 0 && y >= 0) return atan(y / x) + PI;
    if (x < 0 && y < 0) return atan(y / x) - PI;
    if (x == 0 && y > 0) return PI_2;
    if (x == 0 && y < 0) return -PI_2;
    return 0;
}

double asin(double x) {
    if (x > 1.0 || x < -1.0) return 0;
    return atan2(x, sqrt(1.0 - x * x));
}

double acos(double x) {
    if (x > 1.0 || x < -1.0) return 0;
    return PI_2 - asin(x);
}

double fmod(double x, double y) {
    if (y == 0) return 0;
    return x - (double)((long long)(x / y)) * y;
}

float fabsf(float x) { return x < 0 ? -x : x; }
float sqrtf(float x) { return (float)sqrt((double)x); }
float sinf(float x) { return (float)sin((double)x); }
float cosf(float x) { return (float)cos((double)x); }
float tanf(float x) { return (float)sin((double)x) / (float)cos((double)x); }
float fmodf(float x, float y) { return (float)fmod((double)x, (double)y); }
float atan2f(float y, float x) { return (float)atan2((double)y, (double)x); }
float ceilf(float x) { return (float)((int)x + (x > 0 && x != (float)(int)x)); }
float floorf(float x) { return (float)((int)x - (x < 0 && x != (float)(int)x)); }
float roundf(float x) { return (float)((int)(x + (x >= 0 ? 0.5f : -0.5f))); }

long long strtoll(const char *nptr, char **endptr, int base) {
    (void)base;
    long long res = 0;
    int sign = 1;
    if (*nptr == '-') { sign = -1; nptr++; }
    while (*nptr >= '0' && *nptr <= '9') {
        res = res * 10 + (*nptr - '0');
        nptr++;
    }
    if (endptr) *endptr = (char*)nptr;
    return res * sign;
}

int sscanf(const char *str, const char *format, ...) {
    va_list args;
    va_start(args, format);
    /* Extreme minimal implementation for NanoSVG color hex: "#%2x%2x%2x" */
    int count = 0;
    if (format[0] == '#' && str[0] == '#') {
        str++;
        for (int i = 0; i < 3; i++) {
            unsigned int *val = va_arg(args, unsigned int*);
            char hex[3] = {str[0], str[1], '\0'};
            *val = (unsigned int)strtoll(hex, NULL, 16);
            str += 2;
            count++;
        }
    }
    va_end(args);
    return count;
}

float acosf(float x) {
    return (float)acos((double)x);
}

int isnan(double x) {
    return x != x;
}
