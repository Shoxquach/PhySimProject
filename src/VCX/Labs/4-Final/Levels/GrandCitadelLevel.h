#pragma once

#include "Labs/4-Final/Levels/ILevel.h"

namespace VCX::Labs::Final {
    /// @brief 大型三重堡垒关卡：多塔结构、内外层防御、6 个隐藏目标
    struct GrandCitadelLevel : ILevel {
        void Setup(AngryBirdsPhysics & physics, float breakThreshold) const override;
        std::vector<BirdType> GetBirds() const override {
            return {
                BirdType::Normal,
                BirdType::Speed,
                BirdType::Normal,
                BirdType::Speed,
                BirdType::Boomerang,
                BirdType::Normal,
            };
        }
    };
}
