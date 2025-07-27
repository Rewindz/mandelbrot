#version 430
// Use 430 for better compatibility (or use 400 + ARB extension for doubles)

layout (local_size_x = 16, local_size_y = 16) in;

layout (rgba8, binding = 0) writeonly uniform image2D img;

uniform dvec2 u_center;
uniform double u_scale;
uniform ivec2 u_resolution;
uniform int u_iters;

int mandelbrot(dvec2 c) {
    dvec2 z = dvec2(0.0);
    int i;
    for (i = 0; i < u_iters; ++i) {
        if (dot(z, z) > 4.0) break;
        z = dvec2(z.x*z.x - z.y*z.y + c.x, 2.0*z.x*z.y + c.y);
    }
    return i;
}

void main() {
  ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
  if (pixel.x >= u_resolution.x || pixel.y >= u_resolution.y) return;

  // Normalize to [-1, 1] range
  dvec2 pos = (dvec2(pixel) - dvec2(u_resolution) * 0.5) * u_scale + u_center;
  int m = mandelbrot(pos);

  float t = float(m) / float(u_iters);
  vec4 color = vec4(t, t, t, 1.0);
  imageStore(img, pixel, color);
}
