#pragma once

#include "Labs/4-Final/Levels/ILevel.h"

namespace VCX::Labs::Final {
    /// @brief 多米诺骨牌关卡
    struct DominoRunLevel : ILevel {
        void Setup(AngryBirdsPhysics & physics, float breakThreshold) const override;
        std::vector<BirdType> GetBirds() const override {
            return { BirdType::Normal, BirdType::Normal, BirdType::Normal };
        }
    };
}
