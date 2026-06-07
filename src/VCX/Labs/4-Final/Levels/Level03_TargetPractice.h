#pragma once

#include "Labs/4-Final/Levels/ILevel.h"

namespace VCX::Labs::Final {
    // 纯刚体关卡: 石座上的目标. 精准练习.
    class Level03_TargetPractice : public ILevel {
    public:
        std::string_view Name() const override { return "3. Target Practice"; }
        void             Setup(World & world, float breakThreshold) override;
        GameState        Status(World const & world) const override;
    };
} // namespace VCX::Labs::Final
