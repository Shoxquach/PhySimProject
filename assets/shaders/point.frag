#version 410 core

layout(location = 0) in  vec3 v_Color;

layout(location = 0) out vec4 f_Color;

void main() {
    // 사각 포인트를 원형 물방울로: 중심에서 거리 > 1 이면 버린다.
    vec2 d = gl_PointCoord * 2.0 - 1.0;
    float r2 = dot(d, d);
    if (r2 > 1.0) discard;

    // 가장자리를 살짝 어둡게 해서 입체감.
    float shade = 0.65 + 0.35 * (1.0 - r2);
    f_Color = vec4(v_Color * shade, 1.0);
}
