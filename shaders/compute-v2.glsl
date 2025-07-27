#version 430
layout(local_size_x = 16, local_size_y = 16) in;

layout(rgba8, binding = 0) writeonly uniform image2D img;

uniform dvec2 u_center;
uniform double u_scale;
uniform ivec2 u_resolution;
uniform int u_iters;

vec3 hsl_to_rgb(float h, float s, float l) {
    float c = (1.0 - abs(2.0 * l - 1.0)) * s;
    float x = c * (1.0 - abs(mod(h * 6.0, 2.0) - 1.0));
    float m = l - 0.5 * c;
    vec3 rgb;
    if      (h < 1.0/6.0) rgb = vec3(c, x, 0.0);
    else if (h < 2.0/6.0) rgb = vec3(x, c, 0.0);
    else if (h < 3.0/6.0) rgb = vec3(0.0, c, x);
    else if (h < 4.0/6.0) rgb = vec3(0.0, x, c);
    else if (h < 5.0/6.0) rgb = vec3(x, 0.0, c);
    else                  rgb = vec3(c, 0.0, x);
    return rgb + vec3(m);
}

int mandelbrot(dvec2 c, out float t_out) {
    dvec2 z = dvec2(0.0);
    int i;
    for (i = 0; i < u_iters; ++i) {
        if (dot(z, z) > 4.0) break;
        z = dvec2(z.x*z.x - z.y*z.y + c.x, 2.0*z.x*z.y + c.y);
    }

    if (i == u_iters) {
        t_out = 0.0;
        return 0;
    }

    double mag = length(z);
	float log_mag = log(float(mag));       // convert to float before log
	float log_log_mag = log(log_mag);      // safe
	t_out = float(i + 1) - log_log_mag / log(2.0);
    return i;
}

void main() {
    ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
    if (pixel.x >= u_resolution.x || pixel.y >= u_resolution.y) return;

    dvec2 pos = (dvec2(pixel) - dvec2(u_resolution) * 0.5) * u_scale + u_center;

    float t_normalized;
    int iter = mandelbrot(pos, t_normalized);
    float t = t_normalized / float(u_iters);

    vec3 color = vec3(0.0);
    if (iter < u_iters) {
        float hue = mod(t * 0.9 + 0.1, 1.0); // clean HSL
        color = hsl_to_rgb(hue, 1.0, 0.5);
    }

    imageStore(img, pixel, vec4(color, 1.0));
}
