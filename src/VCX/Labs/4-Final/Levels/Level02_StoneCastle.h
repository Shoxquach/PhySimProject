#pragma once

#include "Labs/4-Final/Levels/ILevel.h"

namespace VCX::Labs::Final {
    // 纯刚体关卡: 石柱上玻璃顶的城堡.
    class Level02_StoneCastle : public ILevel {
    public:
        std::string_view Name() const override { return "2. Stone Castle"; }
        void             Setup(World & world, float breakThreshold) override;
        GameState        Status(World const & world) const override;
    };
} // namespace VCX::Labs::Final
