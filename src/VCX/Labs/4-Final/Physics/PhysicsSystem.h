#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "Labs/4-Final/Physics/ConstraintSolver.h"
#include "Labs/4-Final/Physics/PhysicsTypes.h"

namespace VCX::Labs::Final {
    class AngryBirdsPhysics {
    public:
        std::vector<RigidBody> Bodies;

        glm::vec3 Gravity      = glm::vec3(0.f, -9.8f, 0.f);
        float     Restitution  = .28f;
        float     Friction     = .78f;
        float     LinearDamping = .99999f;
        float     AngularDamping = .99996f;
        int       FragmentsCreated = 0;

        SolverType CurrentSolver = SolverType::ConstraintBasedJacobi;

        int  AddBird(glm::vec3 const & anchor, BirdType birdType = BirdType::Normal, int birdSlot = -1);
        int  AddBox(BodyKind kind, glm::vec3 position, glm::vec3 halfSize, float density, glm::vec3 color, float toughness, bool breakable = true);
        void Step(float dt, std::vector<PinnedBody> const & pinnedBodies);
        void Clear();

        void SetSolverType(SolverType type) { CurrentSolver = type; }
        SolverType GetSolverType() const { return CurrentSolver; }

    private:
        void Integrate(float dt, std::vector<PinnedBody> const & pinnedBodies);
        void ResolveCollisions();

        ConstraintSolver m_ConstraintSolver;
    };
}
