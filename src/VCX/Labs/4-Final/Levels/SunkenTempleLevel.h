#pragma once

#include "Labs/4-Final/Levels/ILevel.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    struct SunkenTempleLevel : ILevel {
        std::string_view Name() const override { return "Sunken Temple"; }
        void Setup(World & world, float breakThreshold) const override;
        std::vector<BirdType> GetBirds() const override {
            return { BirdType::Normal, BirdType::Normal, BirdType::Speed,
                     BirdType::WaterBalloon, BirdType::WaterBalloon };
        }
        GameState Status(World const & world) const override { return TargetsClearedStatus(world); }
    };
}
