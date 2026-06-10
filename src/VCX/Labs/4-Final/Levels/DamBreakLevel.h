#pragma once

#include "Labs/4-Final/Levels/ILevel.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    struct DamBreakLevel : ILevel {
        std::string_view Name() const override { return "Dam Break (Two-way)"; }
        void Setup(World & world, float breakThreshold) const override;
        std::vector<BirdType> GetBirds() const override {
            return { BirdType::Normal, BirdType::WaterBalloon, BirdType::Speed, BirdType::WaterBalloon };
        }
        GameState Status(World const & world) const override { return TargetsClearedStatus(world); }
    };
}
