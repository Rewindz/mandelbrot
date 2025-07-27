#version 430
// Fragment shader
in vec2 v_tex;
out vec4 fragColor;
uniform sampler2D tex;

void main() {
    fragColor = texture(tex, v_tex);
}
