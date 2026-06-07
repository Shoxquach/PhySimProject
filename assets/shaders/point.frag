#version 410 core

layout(location = 0) in  vec3 v_Color;

layout(location = 0) out vec4 f_Color;

void main() {
    // 把方形点变成圆形水滴: 离中心距离 > 1 就丢弃.
    vec2 d = gl_PointCoord * 2.0 - 1.0;
    float r2 = dot(d, d);
    if (r2 > 1.0) discard;

    // 边缘稍微变暗增加立体感.
    float shade = 0.65 + 0.35 * (1.0 - r2);
    f_Color = vec4(v_Color * shade, 1.0);
}
