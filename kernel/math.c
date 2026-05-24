#include <stdint.h>

#define PI 3.14159265358979323846
#define PI_2 1.57079632679489661923

double fabs(double x) {
    return x < 0 ? -x : x;
}

double floor(double x) {
    if (x >= 0) {
        return (double)((long long)x);
    } else {
        long long i = (long long)x;
        if (x == (double)i) return x;
        return (double)(i - 1);
    }
}

double ceil(double x) {
    if (x >= 0) {
        long long i = (long long)x;
        if (x == (double)i) return x;
        return (double)(i + 1);
    } else {
        return (double)((long long)x);
    }
}

double sqrt(double x) {
    if (x < 0) return 0;
    if (x == 0) return 0;
    double res = x;
    for (int i = 0; i < 20; i++) {
        res = 0.5 * (res + x / res);
    }
    return res;
}

int abs(int x) {
    return x < 0 ? -x : x;
}

double log(double x) {
    if (x <= 0) return 0;
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
        term *= x / i;
        res += term;
    }
    return res;
}

double pow(double x, double y) {
    if (y == 0) return 1.0;
    if (x == 0) return 0;
    if (y == 1.0) return x;

    /* Integer power optimization */
    if (y == (double)((int)y)) {
        int iy = (int)y;
        double res = 1.0;
        double base = x;
        if (iy < 0) {
            base = 1.0 / base;
            iy = -iy;
        }
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

double ldexp(double x, int exp_val) {
    return x * pow(2.0, (double)exp_val);
}

double sin(double x) {
    while (x > PI) x -= 2.0 * PI;
    while (x < -PI) x += 2.0 * PI;

    double res = 0, term = x;
    double x2 = x * x;
    for (int i = 1; i <= 19; i += 2) {
        res += term;
        term *= -x2 / ((double)(i + 1) * (double)(i + 2));
    }
    return res;
}

double cos(double x) {
    while (x > PI) x -= 2.0 * PI;
    while (x < -PI) x += 2.0 * PI;

    double res = 0, term = 1.0;
    double x2 = x * x;
    for (int i = 0; i <= 18; i += 2) {
        res += term;
        term *= -x2 / ((double)(i + 1) * (double)(i + 2));
    }
    return res;
}
