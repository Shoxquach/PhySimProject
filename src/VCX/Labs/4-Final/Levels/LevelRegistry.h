#pragma once

#include <memory>
#include <vector>

#include "Labs/4-Final/Levels/ILevel.h"

namespace VCX::Labs::Final {
    // 按顺序创建游戏中注册的所有关卡.
    // 新增关卡 = include 一行 + push_back 一行.
    std::vector<std::unique_ptr<ILevel>> CreateAllLevels();
} // namespace VCX::Labs::Final
