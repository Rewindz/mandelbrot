#version 330 core
out vec2 v_pos;
void main(){
  vec2 positions [6] = vec2[](vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0), vec2(-1.0, 1.0), vec2(1.0, -1.0), vec2(1.0, 1.0));
  v_pos = positions[gl_VertexID];
  gl_Position = vec4(v_pos, 0.0, 1.0);
}
