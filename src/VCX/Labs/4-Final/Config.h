#pragma once

namespace VCX::Labs::Final {
    // 舞台整体缩放系数. 长度(位置/大小/相机)与重力一起相乘,
    // 得到物理一致的"放大后的同一场景". 想放大舞台只需调大此值.
    constexpr float WorldScale = 1.5f;
} // namespace VCX::Labs::Final
