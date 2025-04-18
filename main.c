#include <stdio.h>
#include <math.h>

#include <gtk/gtk.h>

#include "colours.h"


#define MAX_ITERATIONS 100
#define PAN_SPEED 0.1
#define ZOOM_FACTOR 0.9

int width = 800;
int height = 600;

double center_x = -0.5;
double center_y = 0.0;
double scale = 4.0;

double start_x = 0;
double start_y = 0;
int dragging = 0;


typedef struct
{
  double real;
  double imag;
}Complex;

Complex complex_square(Complex z)
{
  Complex result;
  result.real = z.real * z.real - z.imag * z.imag;
  result.imag = 2 * z.real * z.imag;
  return result;
}

int is_in_mandelbrot(Complex c, int max_iterations)
{
  Complex z = {0, 0};
  for (int i = 0; i < max_iterations;++i)
    {
      z = complex_square(z);
      z.real += c.real;
      z.imag += c.imag;

      if(z.real * z.real + z.imag * z.imag > 4)
	{
	  return 0;
	}
    }
  return 1;

}


int mandelbrot_escape_iterations(Complex c, int max_iterations)
{
  Complex z = {0, 0};

  for(int i = 0; i < max_iterations; ++i){
    z = complex_square(z);
    z.real += c.real;
    z.imag += c.imag;

    if(z.real * z.real + z.imag * z.imag > 4.0){
      return i;
    }
  }
  return max_iterations;

}

static gboolean scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
  if(event->direction == GDK_SCROLL_UP){
    scale *= ZOOM_FACTOR;
  } else if(event->direction == GDK_SCROLL_DOWN) {
    scale /= ZOOM_FACTOR;
  }

  gtk_widget_queue_draw(widget); //trigger a redraw

  return TRUE;

}

static gboolean key_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
  switch(event->keyval){
  case GDK_KEY_Up:
    center_y -= PAN_SPEED * scale;
    break;
  case GDK_KEY_Down:
    center_y += PAN_SPEED * scale;
    break;
  case GDK_KEY_Left:
    center_x -= PAN_SPEED * scale;
    break;
  case GDK_KEY_Right:
    center_x += PAN_SPEED * scale;
    break;
  case GDK_KEY_0:
    center_y = 0;
    center_x = 0;
    scale = 4.0;
    break;
  default:
    return FALSE;
  }
  gtk_widget_queue_draw(widget);
  return TRUE;
}

static gboolean mouse_press_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
  if(event->button == 1) {
    start_x = event->x;
    start_y = event->y;
    dragging = TRUE;
  }

  return TRUE;
}

static gboolean mouse_release_event(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
  if(event->button == 1) dragging = FALSE;
  return TRUE;
}

static gboolean mouse_motion_event(GtkWidget *widget, GdkEventMotion *event, gpointer user_data)
{
  if(dragging){
    double dx = event->x - start_x;
    double dy = event->y - start_y;

    center_x -= dx * scale / width;
    center_y -= dy * scale / height;

    start_x = event->x;
    start_y = event->y;

    gtk_widget_queue_draw(widget);
    
  }

  return TRUE;
  
}


static gboolean draw_handler(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
  //gtk_window_get_size(gtk_widget_get_window(widget), &width, &height);

  int max_iterations = MAX_ITERATIONS + (int)(log10(1.0 / scale) * 50);

  for (int px = 0; px < width; px++){
    for(int py = 0; py < height; py++){
      double x0 = center_x + (px - width / 2.0) * scale / width;
      double y0 = center_y + (py - height / 2.0) * scale / height;

      Complex c = {x0, y0};

      int iterations = mandelbrot_escape_iterations(c, max_iterations);
      if(iterations == max_iterations)
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
      else{
        double t = (double)iterations / max_iterations;
	double r, g, b;
	//simple_rgb(t, &r, &g, &b);
	//waves_rgb(t, &r, &g, &b);
	rainbow_rgb(t, &r, &g, &b);
	cairo_set_source_rgb(cr, r, b, g);
      }
      
      
      cairo_rectangle(cr, px, py, 1, 1);
      cairo_fill(cr);
    }
  }

  
  
  //cairo_rectangle(cr, x, y, 1, 1);
  //cairo_fill(cr);

  return FALSE;

  
}


static void activate(GtkApplication *app, gpointer user_data)
{
  GtkWidget *window;
  // GtkWidget *label;

  window = gtk_application_window_new(app);
  //label = gtk_label_new("Hello There GNOME!!");
  //gtk_container_add(GTK_CONTAINER(window), label);
  gtk_window_set_title(GTK_WINDOW(window), "Welcome to GNOME");
  gtk_window_set_default_size(GTK_WINDOW(window), width, height);

  gtk_widget_set_app_paintable(window, TRUE);

  gtk_widget_add_events(window, GDK_SCROLL_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);

  g_signal_connect(window, "button-press-event", G_CALLBACK(mouse_press_event), NULL);
  g_signal_connect(window, "button-release-event", G_CALLBACK(mouse_release_event), NULL);
  g_signal_connect(window, "motion-notify-event", G_CALLBACK(mouse_motion_event), NULL);
  
  g_signal_connect(window, "scroll-event", G_CALLBACK(scroll_event), NULL);

  g_signal_connect(window, "key-press_event", G_CALLBACK(key_event), NULL);
  
  g_signal_connect(window, "draw", G_CALLBACK(draw_handler), NULL);
  
  gtk_widget_show_all(window);
  
}


int main(int argc, char **argv)
{
  GtkApplication *app;
  int status;

#if GLIB_CHECK_VERSION(2, 74, 0)
  app = gtk_application_new(NULL, G_APPLICATION_DEFAULT_FLAGS);
#else
  app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);
#endif
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  
  return status;
}
