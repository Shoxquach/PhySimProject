#pragma once

#include <glm/glm.hpp>

#include "Labs/4-Final/AngryBirdsPhysics.h"

namespace VCX::Labs::Final {
    struct AngryBirdsScene {
        enum class Level {
            ClassicTower,
            StoneCastle,
            TargetPractice,
        };

        glm::vec3 Anchor = glm::vec3(-5.5f, 1.35f, 0.f);

        int Reset(AngryBirdsPhysics & physics, float breakThreshold, Level level) const;
        void AddTower(AngryBirdsPhysics & physics, float breakThreshold) const;
        void AddCastle(AngryBirdsPhysics & physics, float breakThreshold) const;
        void AddTargetPractice(AngryBirdsPhysics & physics, float breakThreshold) const;
    };
}
