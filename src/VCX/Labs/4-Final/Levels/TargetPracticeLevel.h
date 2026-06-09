#pragma once

#include "Labs/4-Final/Levels/ILevel.h"

namespace VCX::Labs::Final {
    /// @brief 射击练习关卡
    struct TargetPracticeLevel : ILevel {
        void Setup(AngryBirdsPhysics & physics, float breakThreshold) const override;
        std::vector<BirdType> GetBirds() const override {
            return { BirdType::Normal, BirdType::Speed };
        }
    };
}
