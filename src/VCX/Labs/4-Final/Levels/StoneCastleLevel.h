#pragma once

#include "Labs/4-Final/Levels/ILevel.h"

namespace VCX::Labs::Final {
    /// @brief 石堡关卡
    struct StoneCastleLevel : ILevel {
        void Setup(AngryBirdsPhysics & physics, float breakThreshold) const override;
        std::vector<BirdType> GetBirds() const override {
            return { BirdType::Normal, BirdType::Speed, BirdType::Speed, BirdType::Normal, BirdType::Boomerang };
        }
    };
}
