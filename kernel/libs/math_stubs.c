#include <stdint.h>
#include <stddef.h>

double fabs(double x) { return x < 0 ? -x : x; }
double floor(double x) { if (x >= 0) return (double)((long long)x); else return (double)((long long)x - 1); }
double ceil(double x) { if (x <= 0) return (double)((long long)x); else return (double)((long long)x + 1); }

// Extremely simple sqrt approximation
double sqrt(double x) {
    if (x <= 0) return 0;
    double res = x;
    for (int i = 0; i < 10; i++) res = 0.5 * (res + x / res);
    return res;
}

double fmod(double x, double y) { return x - (int)(x/y) * y; }
double pow(double x, double y) { (void)y; return x; } // Placeholder
double cos(double x) { (void)x; return 1.0; } // Placeholder
double acos(double x) { (void)x; return 0.0; } // Placeholder
