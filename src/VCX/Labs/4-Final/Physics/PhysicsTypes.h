#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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
        WaterBalloon,
    };

    enum class GameState {
        Playing,
        Won,
        Lost,
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
        float     Alpha      = 1.f;
        float     Radius     = .5f;
        float     Toughness  = 8.f;
        int       Generation = 0;
        float     Age        = 0.f;
        float     LifeTime   = -1.f;
        float     Scale      = 1.f;
        glm::mat3 InvInertiaLocal = glm::mat3(0.f);
        float     LastImpact = 0.f;
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

    constexpr float GroundY = 0.f;
    constexpr float BirdRadius = .38f;
}
