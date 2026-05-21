#ifndef NUKLEAR_CONFIG_H
#define NUKLEAR_CONFIG_H

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_ASSERT(x)

#ifdef KERNEL_MODE
    /* Math overrides for freestanding kernel */
    extern double sqrt(double x);
    extern double pow(double x, double y);
    extern double sin(double x);
    extern double cos(double x);
    extern double fabs(double x);
    extern double floor(double x);
    extern double ceil(double x);

    #define STBTT_sqrt(x)      sqrt(x)
    #define STBTT_pow(x,y)     pow(x,y)
    #define STBTT_cos(x)       cos(x)
    #define STBTT_sin(x)       sin(x)
    #define STBTT_fabs(x)      fabs(x)
    #define STBTT_ifloor(x)    ((int)floor(x))
    #define STBTT_iceil(x)     ((int)ceil(x))

    #define NK_SIN(x) sin(x)
    #define NK_COS(x) cos(x)
    #define NK_SQRT(x) sqrt(x)
    #define NK_INV_SQRT(x) (1.0/sqrt(x))
#endif

#endif
