#include <stdint.h>

double fabs(double x) {
    return x < 0 ? -x : x;
}

double sqrt(double x) {
    if (x < 0) return 0;
    double res = x;
    for (int i = 0; i < 10; i++) {
        if (res <= 0) break;
        res = 0.5 * (res + x / res);
    }
    return res;
}

double pow(double x, double y) {
    /* Very basic integer power for what Nuklear might need */
    if (y == 0) return 1.0;
    if (y < 0) return 1.0 / pow(x, -y);
    double res = 1.0;
    int iy = (int)y;
    for (int i = 0; i < iy; i++) {
        res *= x;
    }
    return res;
}

double sin(double x) {
    /* Taylor series approximation */
    double res = 0, term = x;
    for (int i = 1; i <= 9; i += 2) {
        res += term;
        term *= -x * x / ((i + 1) * (i + 2));
    }
    return res;
}

double cos(double x) {
    /* Taylor series approximation */
    double res = 0, term = 1;
    for (int i = 0; i <= 8; i += 2) {
        res += term;
        term *= -x * x / ((i + 1) * (i + 2));
    }
    return res;
}
