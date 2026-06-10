#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "Labs/4-Final/Physics/PhysicsSystem.h"
#include "Labs/4-Final/Levels/LevelRegister.h"

namespace VCX::Labs::Final {
    struct AngryBirdsScene {
        glm::vec3 Anchor = glm::vec3(-5.5f, 1.95f, 0.f);

        std::vector<BirdType> Reset(AngryBirdsPhysics & physics, float breakThreshold, LevelRegister::LevelID level) const;
    };
}
