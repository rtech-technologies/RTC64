/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#ifndef MATH_H
#define MATH_H
#define PI 3.14159265358979323846
#define PI_2 1.57079632679489661923

double fabs(double x);
double floor(double x);
double ceil(double x);
double sqrt(double x);
double log(double x);
double exp(double x);
double pow(double x, double y);
double sin(double x);
double cos(double x);
double atan(double x);
double atan2(double y, double x);
double asin(double x);
double acos(double x);
float  acosf(float x);
int    isnan(double x);
double fmod(double x, double y);

float  fabsf(float x);
float  sqrtf(float x);
float  sinf(float x);
float  cosf(float x);
float  tanf(float x);
float  fmodf(float x, float y);
float  atan2f(float y, float x);
float  ceilf(float x);
float  floorf(float x);
float  roundf(float x);

#endif
