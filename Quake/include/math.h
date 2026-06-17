#ifndef _MATH_H_
#define _MATH_H_

#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923

double sin(double x);
double cos(double x);
double tan(double x);
double asin(double x);
double acos(double x);
double atan(double x);
double atan2(double y, double x);

double sinh(double x);
double cosh(double x);
double tanh(double x);

double exp(double x);
double log(double x);
double log10(double x);
double pow(double x, double y);
double sqrt(double x);
double cbrt(double x);

double floor(double x);
double ceil(double x);
double fabs(double x);
double fmod(double x, double y);
double frexp(double x, int *exp);
double ldexp(double x, int exp);
double modf(double x, double *ipart);

#endif
