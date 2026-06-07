#pragma once

#include "Labs/4-Final/Levels/ILevel.h"

namespace VCX::Labs::Final {
    // 순수 강체 레벨: 나무/유리/돌로 쌓은 탑. 유체/FEM 없음.
    class Level01_ClassicTower : public ILevel {
    public:
        std::string_view Name() const override { return "1. Classic Tower"; }
        void             Setup(World & world, float breakThreshold) override;
        GameState        Status(World const & world) const override;
    };
} // namespace VCX::Labs::Final
