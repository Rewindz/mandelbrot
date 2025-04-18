#include "colours.h"


void simple_rgb(double t_factor, double *r, double *g, double *b)
{
  *r = 9*(1-t_factor)*t_factor*t_factor*t_factor;
  *g = 15*(1-t_factor)*(1-t_factor)*t_factor*t_factor;
  *b = 8.5*(1-t_factor)*(1-t_factor)*(1-t_factor)*t_factor;
}

void waves_rgb(double t_factor, double *r, double *g, double *b)
{
  *r = 0.5 + 0.5 * sin(t_factor * 6.2832);
  *g = 0.5 + 0.5 * cos(t_factor * 6.2832);
  *b = 0.5 + 0.5 * sin(t_factor * 3.1416);
}

void rainbow_rgb(double t_factor, double *r, double *g, double *b)
{
  double hue = t_factor * 0.7;
  hsv_to_rgb(hue, 1.0, 1.0, r, g, b);
}



void hsv_to_rgb(double h, double s, double v, double *r, double *g, double *b)
{
  int i = (int)(h*6);
  double f = h * 6 - i;
  double p = v * (1 - s);
  double q = v * (1 - f * s);
  double t = v * (1 - (1 - f) * s);

  switch(i % 6){
  case 0: *r = v; *g = t; *b = p; break;
  case 1: *r = q; *g = v; *b = p; break;
  case 2: *r = p; *g = v; *b = t; break;
  case 3: *r = p; *g = q; *b = v; break;
  case 4: *r = t; *g = p; *b = v; break;
  case 5: *r = v; *g = p; *b = q; break;
  }

}

