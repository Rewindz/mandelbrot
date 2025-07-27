#version 430
in vec2 v_pos;
out vec4 fragColor;

uniform vec2 u_center;
uniform float u_scale;
uniform vec2 u_resolution;
uniform int u_iters;


int mandelbrot(vec2 c){
  vec2 z = vec2(0.0);
  int i;
  for(i=0; i < u_iters; ++i){
    if(dot(z, z) > 4.0) break;
    z = vec2(z.x * z.x - z.y * z.y + c.x, 2.0 * z.x * z.y + c.y);
  }
  return i;
}


void main(){
  vec2 uv = v_pos * u_resolution * u_scale + u_center;
  int m = mandelbrot(uv);
  float t = float(m) / float(u_iters);
  vec3 color = vec3(t, t, t);
  fragColor = vec4(color, 1.0);
}
