#pragma once

#include <memory>
#include <vector>

#include "Labs/4-Final/Levels/ILevel.h"

namespace VCX::Labs::Final {
    // 게임에 등록된 모든 레벨을 순서대로 생성한다.
    // 새 레벨 추가 = 헤더 include 한 줄 + push_back 한 줄.
    std::vector<std::unique_ptr<ILevel>> CreateAllLevels();
} // namespace VCX::Labs::Final
