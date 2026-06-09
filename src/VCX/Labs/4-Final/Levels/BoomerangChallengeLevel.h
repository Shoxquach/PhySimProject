#pragma once

#include "Labs/4-Final/Levels/ILevel.h"

namespace VCX::Labs::Final {
    /// @brief 回旋鸟挑战关卡
    struct BoomerangChallengeLevel : ILevel {
        void Setup(AngryBirdsPhysics & physics, float breakThreshold) const override;
        std::vector<BirdType> GetBirds() const override {
            return { BirdType::Boomerang };
        }
    };
}
