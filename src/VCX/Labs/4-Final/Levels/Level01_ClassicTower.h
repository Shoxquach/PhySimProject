#pragma once

#include "Labs/4-Final/Levels/ILevel.h"

namespace VCX::Labs::Final {
    // 纯刚体关卡: 木/玻璃/石搭成的塔. 无流体/FEM.
    class Level01_ClassicTower : public ILevel {
    public:
        std::string_view Name() const override { return "1. Classic Tower"; }
        void             Setup(World & world, float breakThreshold) override;
        GameState        Status(World const & world) const override;
    };
} // namespace VCX::Labs::Final
