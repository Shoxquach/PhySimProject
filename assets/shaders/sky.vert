#version 410 core

layout(location = 0) in  vec3 a_Position;   // fullscreen quad in NDC

layout(location = 0) out float v_T;         // 0 = bottom, 1 = top

void main() {
    v_T = a_Position.y * 0.5 + 0.5;
    gl_Position = vec4(a_Position.xy, 1.0, 1.0);
}
