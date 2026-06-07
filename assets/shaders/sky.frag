#version 410 core

layout(location = 0) in  float v_T;

layout(location = 0) out vec4 f_Color;

uniform vec3 u_Top;
uniform vec3 u_Bottom;

void main() {
    f_Color = vec4(mix(u_Bottom, u_Top, v_T), 1.0);
}
