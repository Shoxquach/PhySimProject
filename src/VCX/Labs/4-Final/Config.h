#pragma once

namespace VCX::Labs::Final {
    // 무대 전체 확대 배율. 길이(위치/크기/카메라)와 중력을 함께 곱해
    // 물리적으로 일관된 "확대된 동일 씬"을 만든다. 무대를 키우려면 이 값만 올리면 됨.
    constexpr float WorldScale = 1.5f;
} // namespace VCX::Labs::Final
