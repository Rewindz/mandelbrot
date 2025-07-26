#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gtk/gtk.h>
#include <epoxy/gl.h>

#include "colours.h"

#define MANDEL_GPU_MODE


#define MAX_ITERATIONS 1000
#define PAN_SPEED 0.1
#define ZOOM_FACTOR 0.9

int width = 400;
int height = 400;

double center_x = -0.5;
double center_y = 0.0;
double scale = 4.0;

double start_x = 0;
double start_y = 0;
int dragging = 0;

static GLuint program;
static GLuint vao;


 //dvec2 uv = v_pos * u_resolution * u_scale + u_center;

 // https://www.khronos.org/opengl/wiki/Shader_Compilation

static GLuint compile_shader(GLenum type, const char *src)
{
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if(!success){
    char log[512];
    glGetShaderInfoLog(shader, 512, NULL, log);
    g_printerr("Shader compile error: %s\n", log);
  }
  
  return shader;
}

static char *shader_src_from_file(const char *path){
  FILE *src_file = fopen(path, "r");
  if(!src_file){
    fprintf(stderr, "Could not open file: %s!\n", path);
    return NULL;
  }

  if(fseek(src_file, 0, SEEK_END)){
    fprintf(stderr, "Could not seek into file: %s!\n", path);
    return NULL;
  }
  
  long src_size = ftell(src_file);
  rewind(src_file);

  char *src = calloc(src_size + 2, sizeof(char));

  if(!src){
    fprintf(stderr, "Could not allocate %zd bytes for file: %s!\n", src_size + 2, path);
    return NULL;
  }

  int read_bytes = fread(src, sizeof(char), src_size, src_file);
  printf("Read %zd bytes from file: %s\n", read_bytes, path);
  return src;
  
}

static void realize(GtkGLArea *area, gpointer user_data)
{
  gtk_gl_area_make_current(area);
  char *vertex_shader_src = shader_src_from_file("shaders/vertex.glsl");
  char *fragment_shader_src = shader_src_from_file("shaders/frag.glsl");
  if(!vertex_shader_src || !fragment_shader_src){
    fprintf(stderr, "Failed to load a shader source from disk!\n");
    abort(); // Great practice! /s
  }
  GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader_src);
  GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_src);
  free(vertex_shader_src);
  free(fragment_shader_src);
  
  program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);

  glDeleteShader(vs);
  glDeleteShader(fs);

  glGenVertexArrays(1, &vao);

}

static void render(GtkGLArea *area, GdkGLContext *context, gpointer user_data)
{
  gtk_gl_area_make_current(area);

  int w = gtk_widget_get_allocated_width(GTK_WIDGET(area));
  int h = gtk_widget_get_allocated_height(GTK_WIDGET(area));
  int max_iterations = MAX_ITERATIONS + (int)(log10(1.0 / scale) * 50);
  printf("Max iterations: %i\n", max_iterations);

  glViewport(0, 0, w, h);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(program);
  glBindVertexArray(vao);

  glUniform2f(glGetUniformLocation(program, "u_center"), center_x, center_y);
  glUniform1f(glGetUniformLocation(program, "u_scale"), scale / width);
  glUniform2f(glGetUniformLocation(program, "u_resolution"), (float)width, (float)height);
  glUniform1i(glGetUniformLocation(program, "u_iters"), max_iterations);

  glDrawArrays(GL_TRIANGLES, 0, 6);

}


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


int mandelbrot_escape_iterations(Complex c, int max_iterations, double *smooth_t)
{
  Complex z = {0, 0};

  int iterations = 0;
  for(; iterations < max_iterations; ++iterations){
    z = complex_square(z);
    z.real += c.real;
    z.imag += c.imag;

    if(z.real * z.real + z.imag * z.imag > 4.0)
      break;
  }

  if(iterations == max_iterations){
    *smooth_t = 0.0;
    return 1;
  }

  double mag = sqrt(z.real * z.real + z.imag * z.imag);
  *smooth_t = iterations + 1 - log(log(mag)) / log(2.0);
  return 0;

}

static gboolean scroll_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  GdkEventScroll *scroll_event = (GdkEventScroll *)event;
  double zoom = (scroll_event->direction == GDK_SCROLL_UP) ? 0.9 : 1.1;
  scale *= zoom;

  gtk_widget_queue_draw(widget); //trigger a redraw

  return TRUE;

}

static gboolean keypress_event(GtkWidget *widget, GdkEventKey *key_event, gpointer user_data)
{
  switch(key_event->keyval){
  case GDK_KEY_Up:
  case GDK_KEY_w:
    center_y -= PAN_SPEED * scale;
    break;
  case GDK_KEY_Down:
  case GDK_KEY_s:
    center_y += PAN_SPEED * scale;
    break;
  case GDK_KEY_Left:
  case GDK_KEY_a:
    center_x -= PAN_SPEED * scale;
    break;
  case GDK_KEY_Right:
  case GDK_KEY_d:
    center_x += PAN_SPEED * scale;
    break;
  case GDK_KEY_f:
    scale *= ZOOM_FACTOR;
    break;
  case GDK_KEY_r:
    scale /= ZOOM_FACTOR; 
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

static gboolean mouse_press_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  GdkEventButton *button_event = (GdkEventButton *)event;
  if(button_event->button == 1) {
    start_x = button_event->x;
    start_y = button_event->y;
    dragging = TRUE;
  }

  return TRUE;
}

static gboolean mouse_release_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  GdkEventButton *button_event = (GdkEventButton *)event;
  if(button_event->button == 1) dragging = FALSE;
  return TRUE;
}

static gboolean mouse_motion_event(GtkWidget *widget, GdkEventMotion *motion_event, gpointer user_data)
{
  if(dragging){
    double dx = motion_event->x - start_x;
    double dy = motion_event->y - start_y;

    center_x -= dx * scale / width;
#ifdef MANDEL_GPU_MODE
    center_y += dy * scale / height;
#else
    center_y -= dy * scale / height;
#endif //MANDLE_GPU_MODE

    start_x = motion_event->x;
    start_y = motion_event->y;

    gtk_widget_queue_draw(widget);
    
  }

  return TRUE;
  
}


static gboolean draw_handler(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
  width = gtk_widget_get_allocated_width(widget);
  height = gtk_widget_get_allocated_height(widget);
  
  int max_iterations = MAX_ITERATIONS + (int)(log10(1.0 / scale) * 50);

  for (int px = 0; px < width; px++){
    for(int py = 0; py < height; py++){
      double x0 = center_x + (px - width / 2.0) * scale / width;
      double y0 = center_y + (py - height / 2.0) * scale / height;

      Complex c = {x0, y0};

      double smooth_t;
      int iterations = mandelbrot_escape_iterations(c, max_iterations, &smooth_t);
      double t = smooth_t / max_iterations;
      
      if(iterations == max_iterations)
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
      else{
        
	double r, g, b;
	simple_rgb(t, &r, &g, &b);
	//waves_rgb(t, &r, &g, &b);
	//rainbow_rgb(t, &r, &g, &b);
	cairo_set_source_rgb(cr, r, b, g);
      }
      
      
      cairo_rectangle(cr, px, py, 1, 1);
      cairo_fill(cr);
    }
  }

  return FALSE;

  
}


static void activate(GtkApplication *app, gpointer user_data)
{
  GtkWidget *window;
  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Mandelbrot Set");
  gtk_window_set_default_size(GTK_WINDOW(window), width, height);
  

#ifdef MANDEL_GPU_MODE
  printf("GPU MODE\n");
  GtkWidget *gl_area = gtk_gl_area_new();
  gtk_container_add(GTK_CONTAINER(window), gl_area);
  
  gtk_widget_add_events(gl_area, GDK_SCROLL_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_KEY_PRESS_MASK);
  gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);

  g_signal_connect(window, "key-press-event", G_CALLBACK(keypress_event), NULL);
  g_signal_connect(gl_area, "realize", G_CALLBACK(realize), NULL);
  g_signal_connect(gl_area, "render", G_CALLBACK(render), NULL);
  g_signal_connect(gl_area, "button-press-event", G_CALLBACK(mouse_press_event), NULL);
  g_signal_connect(gl_area, "button-release-event", G_CALLBACK(mouse_release_event), NULL);
  g_signal_connect(gl_area, "motion-notify-event", G_CALLBACK(mouse_motion_event), NULL);
  g_signal_connect(gl_area, "scroll-event", G_CALLBACK(scroll_event), NULL);
#endif //MANDEL_GPU_MODE

#ifndef MANDEL_GPU_MODE
  printf("CPU MODE\n");
  gtk_widget_set_app_paintable(window, TRUE);
  gtk_widget_add_events(window, GDK_SCROLL_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK | GDK_KEY_PRESS_MASK);
  g_signal_connect(window, "draw", G_CALLBACK(draw_handler), NULL);
  g_signal_connect(window, "button-press-event", G_CALLBACK(mouse_press_event), NULL);
  g_signal_connect(window, "button-release-event", G_CALLBACK(mouse_release_event), NULL);
  g_signal_connect(window, "motion-notify-event", G_CALLBACK(mouse_motion_event), NULL);
  g_signal_connect(window, "scroll-event", G_CALLBACK(scroll_event), NULL);
  g_signal_connect(window, "key-press-event", G_CALLBACK(keypress_event), NULL);
#endif //!MANDLE_GPU_MODE
  
  gtk_widget_show_all(window);
  
#ifdef MANDEL_GPU_MODE
  gtk_widget_grab_focus(gl_area);

  const GLubyte *renderer = glGetString(GL_RENDERER);
  const GLubyte *vendor = glGetString(GL_VENDOR);
  printf("GL Renderer: %s\nGL Vendor: %s\n", renderer, vendor);
#endif //MANDLE_GPU_MODE
  
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
