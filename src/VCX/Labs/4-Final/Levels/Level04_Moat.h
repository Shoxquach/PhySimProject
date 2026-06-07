#pragma once

#include "Labs/4-Final/Levels/ILevel.h"

namespace VCX::Labs::Final {
    // M1 레벨: 해자(moat). FLIP 물웅덩이 + 부력 커플링(one-way).
    // 나무 뗏목은 물 위에 뜨고(밀도<물), 돌은 가라앉는다(밀도>물).
    class Level04_Moat : public ILevel {
    public:
        std::string_view Name() const override { return "4. Moat (Buoyancy)"; }
        void             Setup(World & world, float breakThreshold) override;
        GameState        Status(World const & world) const override;
    };
} // namespace VCX::Labs::Final
