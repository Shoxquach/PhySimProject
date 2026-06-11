#pragma once

#include <glm/glm.hpp>

#include "Labs/4-Final/Physics/PhysicsSystem.h"
#include "Labs/4-Final/World.h"

namespace VCX::Labs::Final {
    namespace Materials {
        inline glm::vec3 const Wood   = glm::vec3(0.62f, 0.38f, 0.16f);
        inline glm::vec3 const Glass  = glm::vec3(0.35f, 0.72f, 0.95f);
        inline glm::vec3 const Stone  = glm::vec3(0.55f, 0.55f, 0.58f);
        inline glm::vec3 const Target = glm::vec3(0.28f, 0.75f, 0.22f);
    }

    namespace Densities {
        constexpr float Wood   = 0.8f;
        constexpr float Glass  = 0.5f;
        constexpr float Stone  = 2.1f;
        constexpr float Target = 0.6f;
    }

    class LevelBuilder {
    public:
        LevelBuilder(AngryBirdsPhysics & physics, float breakThreshold)
            : _physics(physics), _breakThreshold(breakThreshold) {}

        int AddWood(glm::vec3 const & position, glm::vec3 const & halfSize, float toughnessMul = 1.0f) {
            return _physics.AddBox(BodyKind::Wood, position, halfSize, Densities::Wood, Materials::Wood, _breakThreshold * toughnessMul);
        }

        int AddGlass(glm::vec3 const & position, glm::vec3 const & halfSize, float toughnessMul = .6f) {
            return _physics.AddBox(BodyKind::Glass, position, halfSize, Densities::Glass, Materials::Glass, _breakThreshold * toughnessMul);
        }

        int AddStone(glm::vec3 const & position, glm::vec3 const & halfSize, float toughnessMul = 1.7f) {
            return _physics.AddBox(BodyKind::Stone, position, halfSize, Densities::Stone, Materials::Stone, _breakThreshold * toughnessMul);
        }

        int AddTarget(glm::vec3 const & position, glm::vec3 const & halfSize, float toughnessMul = .24f) {
            return _physics.AddBox(BodyKind::Target, position, halfSize, Densities::Target, Materials::Target, _breakThreshold * toughnessMul);
        }

    private:
        AngryBirdsPhysics & _physics;
        float               _breakThreshold;
    };

    inline int CountAliveTargets(World const & world) {
        int n = 0;
        for (auto const & b : world.Rigid.Bodies) {
            if (b.IsAlive && b.Kind == BodyKind::Target) ++n;
        }
        return n;
    }

    inline int CountAliveBreakables(World const & world) {
        int n = 0;
        for (auto const & b : world.Rigid.Bodies) {
            if (b.IsAlive && b.Breakable && b.Kind != BodyKind::Bird) ++n;
        }
        return n;
    }

    inline GameState TargetsClearedStatus(World const & world) {
        return CountAliveTargets(world) == 0 ? GameState::Won : GameState::Playing;
    }
}
