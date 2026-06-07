#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Labs/4-Final/Config.h"

namespace VCX::Labs::Final {
    enum class BodyKind {
        Bird,
        Wood,
        Glass,
        Target,
        Stone,
        Fragment,
        Ground,
        WaterBalloon,
    };

    struct RigidBody {
        BodyKind  Kind       = BodyKind::Wood;
        bool      IsStatic   = false;
        bool      IsAlive    = true;
        bool      Breakable  = true;
        float     Mass       = 1.f;
        float     InvMass    = 1.f;
        glm::vec3 Position   = glm::vec3(0.f);
        glm::vec3 Velocity   = glm::vec3(0.f);
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
        float     LastImpact = 0.f;   // 이번 스텝 최대 충격 (물풍선 터짐 판정용)
    };

    struct Contact {
        int       A           = -1;
        int       B           = -1;
        glm::vec3 Normal      = glm::vec3(0.f, 1.f, 0.f);
        float     Penetration = 0.f;
        glm::vec3 Point       = glm::vec3(0.f);
        float     Impact      = 0.f;
    };

    class RigidWorld {
    public:
        std::vector<RigidBody> Bodies;

        glm::vec3 Gravity      = glm::vec3(0.f, -12.5f * WorldScale, 0.f);
        float     Restitution  = .28f;
        float     Friction     = .78f;
        float     LinearDamping = .9999f;
        float     AngularDamping = .9996f;
        int       FragmentsCreated = 0;

        int  AddBird(glm::vec3 const & anchor);
        int  AddWaterBalloon(glm::vec3 const & anchor);
        int  AddBox(BodyKind kind, glm::vec3 position, glm::vec3 halfSize, float density, glm::vec3 color, float toughness, bool breakable = true);
        void Step(float dt, int draggedIndex, glm::vec3 const & draggedPosition);
        void Clear();

    private:
        void Integrate(float dt, int draggedIndex);
        void ResolveCollisions();
        void TryBreakBodies(std::vector<Contact> const & contacts);
        void BreakBody(int index, glm::vec3 const & impulseDir, float impact);

        bool FindContact(int a, int b, Contact & contact) const;
        bool SphereBoxContact(RigidBody const & sphere, RigidBody const & box, int sphereIndex, int boxIndex, Contact & contact) const;
        bool BoxBoxContact(RigidBody const & a, RigidBody const & b, int aIndex, int bIndex, Contact & contact) const;
        bool GroundContact(RigidBody const & body, int index, Contact & contact) const;
    };

    constexpr float GroundY = 0.f;
    constexpr float BirdRadius = .38f * WorldScale;
}
