#pragma once

#include <string_view>
#include <vector>

#include "Labs/4-Final/World.h"

namespace VCX::Labs::Final {
    struct ILevel {
        virtual ~ILevel() = default;

        virtual std::string_view Name() const = 0;
        virtual void Setup(World & world, float breakThreshold) const = 0;
        virtual std::vector<BirdType> GetBirds() const = 0;
        virtual float GetMaxPull() const { return 2.2f; }
        virtual void Tick(World & world, float dt) const { (void)world; (void)dt; }
        virtual GameState Status(World const & world) const = 0;
    };
}
