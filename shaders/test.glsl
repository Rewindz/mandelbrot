#version 430
layout(local_size_x = 16, local_size_y = 16) in;

layout(rgba8, binding = 0) writeonly uniform image2D img;
uniform ivec2 u_resolution;

void main() {
    ivec2 px = ivec2(gl_GlobalInvocationID.xy);
    if (px.x >= u_resolution.x || px.y >= u_resolution.y) return;

    vec2 uv = vec2(px) / vec2(u_resolution);
    imageStore(img, px, vec4(uv.x, uv.y, 0.5, 1.0)); // gradient debug
}
