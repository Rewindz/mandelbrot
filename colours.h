#ifndef COLOURS_H_
#define COLOURS_H_
#include <math.h>

void hsv_to_rgb(double h, double s, double v, double *r, double *g, double *b);

void simple_rgb(double t_factor, double *r, double *g, double *b);
void waves_rgb(double t_factor, double *r, double *g, double *b);
void rainbow_rgb(double t_factor, double *r, double *g, double *b);

#endif //COLOURS_H_
