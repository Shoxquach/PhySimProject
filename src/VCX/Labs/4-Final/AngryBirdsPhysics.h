#pragma once

#include <utility>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Labs/4-Final/ConstraintSolver.h"

namespace VCX::Labs::Final {
    enum class BodyKind {
        Bird,
        Wood,
        Glass,
        Target,
        Stone,
        Fragment,
        Ground,
    };

    enum class BirdType {
        Normal,
        Speed,
        Boomerang,
    };

    struct RigidBody {
        BodyKind  Kind       = BodyKind::Wood;
        BirdType  Bird       = BirdType::Normal;
        int       BirdSlot   = -1;
        bool      BirdWasLaunched = false;
        bool      BirdHasCollided = false;
        bool      BoomerangActive = false;
        bool      IsStatic   = false;
        bool      IsAlive    = true;
        bool      Breakable  = true;
        float     Mass       = 1.f;
        float     InvMass    = 1.f;
        glm::vec3 Position   = glm::vec3(0.f);
        glm::vec3 Velocity   = glm::vec3(0.f);
        glm::vec3 BoomerangAcceleration = glm::vec3(0.f);
        glm::quat Rotation   = glm::quat(1.f, 0.f, 0.f, 0.f);
        glm::vec3 AngularVel = glm::vec3(0.f);
        glm::vec3 HalfSize   = glm::vec3(.5f);
        glm::vec3 Color      = glm::vec3(1.f);
        float     Radius     = .5f;
        float     Toughness  = 8.f;
        int       Generation = 0;
        float     Age        = 0.f;
        float     LifeTime   = -1.f;
        float     Scale      = 1.f;
        glm::mat3 InvInertiaLocal = glm::mat3(0.f);
    };

    struct Contact {
        int       A           = -1;
        int       B           = -1;
        glm::vec3 Normal      = glm::vec3(0.f, 1.f, 0.f);
        float     Penetration = 0.f;
        glm::vec3 Point       = glm::vec3(0.f);
        std::vector<glm::vec3> Points;
        float     Impact      = 0.f;
    };

    struct PinnedBody {
        int       Index    = -1;
        glm::vec3 Position = glm::vec3(0.f);
    };

    class AngryBirdsPhysics {
    public:
        std::vector<RigidBody> Bodies;

        glm::vec3 Gravity      = glm::vec3(0.f, -9.8f, 0.f);
        float     Restitution  = .28f;
        float     Friction     = .78f;
        float     LinearDamping = .9999f;
        float     AngularDamping = .9996f;
        int       FragmentsCreated = 0;

        SolverType CurrentSolver = SolverType::SequentialImpulse;

        int  AddBird(glm::vec3 const & anchor, BirdType birdType = BirdType::Normal, int birdSlot = -1);
        int  AddBox(BodyKind kind, glm::vec3 position, glm::vec3 halfSize, float density, glm::vec3 color, float toughness, bool breakable = true);
        void Step(float dt, std::vector<PinnedBody> const & pinnedBodies);
        void Clear();

        void SetSolverType(SolverType type) { CurrentSolver = type; }
        SolverType GetSolverType() const { return CurrentSolver; }

    private:
        void Integrate(float dt, std::vector<PinnedBody> const & pinnedBodies);
        void ResolveCollisions();
        void ResolveCollisionsConstraintBased();
        void TryBreakBodies(std::vector<Contact> const & contacts);
        void BreakBody(int index, glm::vec3 const & impulseDir, float impact);

        bool FindContact(int a, int b, Contact & contact) const;
        bool SphereBoxContact(RigidBody const & sphere, RigidBody const & box, int sphereIndex, int boxIndex, Contact & contact) const;
        bool BoxBoxContact(RigidBody const & a, RigidBody const & b, int aIndex, int bIndex, Contact & contact) const;
        bool GroundContact(RigidBody const & body, int index, Contact & contact) const;

        ConstraintSolver m_ConstraintSolver;
    };

    constexpr float GroundY = 0.f;
    constexpr float BirdRadius = .38f;
}
