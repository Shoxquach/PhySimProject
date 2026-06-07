#pragma once

#include "Labs/4-Final/Levels/ILevel.h"

namespace VCX::Labs::Final {
    // 순수 강체 레벨: 돌받침 위 타깃 3개. 정확도 연습.
    class Level03_TargetPractice : public ILevel {
    public:
        std::string_view Name() const override { return "3. Target Practice"; }
        void             Setup(World & world, float breakThreshold) override;
        GameState        Status(World const & world) const override;
    };
} // namespace VCX::Labs::Final
