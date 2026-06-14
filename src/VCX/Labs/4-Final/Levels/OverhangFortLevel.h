#pragma once

#include "Labs/4-Final/Levels/ILevel.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    struct OverhangFortLevel : ILevel {
        std::string_view Name() const override { return "Overhang Fort"; }
        void Setup(World & world, float breakThreshold) const override;
        std::vector<BirdType> GetBirds() const override {
            return { BirdType::Normal, BirdType::Speed, BirdType::Normal,
                     BirdType::Boomerang, BirdType::Normal };
        }
        GameState Status(World const & world) const override { return TargetsClearedStatus(world); }
    };
}
