#include "Labs/4-Final/Physics/PhysicsSystem.h"

#include <algorithm>

#include <glm/ext.hpp>

#include "Labs/4-Final/Physics/BreakSolver.h"
#include "Labs/4-Final/Physics/ImpulseSolver.h"

namespace VCX::Labs::Final {
    namespace {
        constexpr float c_FragmentShrinkSpeed = .85f;
        constexpr float c_LifeTimeShrinkDuration = .5f;
    }

    void AngryBirdsPhysics::Clear() {
        Bodies.clear();
        FragmentsCreated = 0;
    }

    int AngryBirdsPhysics::AddBird(glm::vec3 const & anchor, BirdType birdType, int birdSlot) {
        RigidBody bird;
        bird.Kind = BodyKind::Bird;
        bird.Bird = birdType;
        bird.BirdSlot = birdSlot;
        bird.Mass = 1.4f;
        bird.InvMass = 1.f / bird.Mass;
        bird.Position = anchor;
        bird.Radius = BirdRadius;
        bird.HalfSize = glm::vec3(BirdRadius);
        if (birdType == BirdType::Speed) {
            bird.Color = glm::vec3(1.f, .82f, .08f);
        } else if (birdType == BirdType::Boomerang) {
            bird.Color = glm::vec3(.18f, .72f, .95f);
        } else {
            bird.Color = glm::vec3(.9f, .12f, .08f);
        }
        bird.Breakable = false;
        bird.Toughness = 100.f;
        bird.Age = 0.f;
        bird.LifeTime = -1.f;
        {
            float const I = 2.f / 5.f * bird.Mass * bird.Radius * bird.Radius;
            float const invI = I > 0.f ? 1.f / I : 0.f;
            bird.InvInertiaLocal = glm::mat3(invI);
        }
        Bodies.push_back(bird);
        return int(Bodies.size()) - 1;
    }

    int AngryBirdsPhysics::AddBox(BodyKind kind, glm::vec3 position, glm::vec3 halfSize, float density, glm::vec3 color, float toughness, bool breakable) {
        RigidBody body;
        body.Kind = kind;
        float const volume = 8.f * halfSize.x * halfSize.y * halfSize.z;
        body.Mass = density * volume;
        body.InvMass = body.Mass > 0.f ? 1.f / body.Mass : 0.f;
        body.IsStatic = body.Mass <= 0.f;
        body.Position = position;
        body.HalfSize = halfSize;
        body.Color = color;
        body.Alpha = kind == BodyKind::Glass ? .45f : 1.f;
        body.Toughness = toughness;
        body.Breakable = breakable;
        body.Age = 0.f;
        body.LifeTime = -1.f;
        if (body.Mass > 0.f) {
            float const Ixx = (1.f / 3.f) * body.Mass * (body.HalfSize.y * body.HalfSize.y + body.HalfSize.z * body.HalfSize.z);
            float const Iyy = (1.f / 3.f) * body.Mass * (body.HalfSize.x * body.HalfSize.x + body.HalfSize.z * body.HalfSize.z);
            float const Izz = (1.f / 3.f) * body.Mass * (body.HalfSize.x * body.HalfSize.x + body.HalfSize.y * body.HalfSize.y);
            glm::mat3 inv = glm::mat3(0.f);
            inv[0][0] = Ixx > 0.f ? 1.f / Ixx : 0.f;
            inv[1][1] = Iyy > 0.f ? 1.f / Iyy : 0.f;
            inv[2][2] = Izz > 0.f ? 1.f / Izz : 0.f;
            body.InvInertiaLocal = inv;
        } else {
            body.InvInertiaLocal = glm::mat3(0.f);
        }
        Bodies.push_back(body);
        return int(Bodies.size()) - 1;
    }

    void AngryBirdsPhysics::Step(float dt, std::vector<PinnedBody> const & pinnedBodies) {
        for (auto const & pinned : pinnedBodies) {
            if (pinned.Index < 0 || pinned.Index >= int(Bodies.size())) continue;
            auto & body = Bodies[pinned.Index];
            body.Position = pinned.Position;
            body.Velocity = glm::vec3(0.f);
            body.AngularVel = glm::vec3(0.f);
            body.Rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
        }

        Integrate(dt, pinnedBodies);
        ResolveCollisions();

        Bodies.erase(
            std::remove_if(Bodies.begin(), Bodies.end(), [](RigidBody const & body) {
                return !body.IsAlive || body.Position.y < -10.f || glm::length(body.Position) > 80.f;
            }),
            Bodies.end());
    }

    void AngryBirdsPhysics::Integrate(float dt, std::vector<PinnedBody> const & pinnedBodies) {
        for (int i = 0; i < int(Bodies.size()); ++i) {
            auto & body = Bodies[i];
            bool const isPinned = std::any_of(pinnedBodies.begin(), pinnedBodies.end(), [i](PinnedBody const & pinned) {
                return pinned.Index == i;
            });
            if (!body.IsAlive || body.IsStatic || isPinned) {
                continue;
            }

            body.Age += dt;

            if (body.Kind == BodyKind::Fragment) {
                body.Scale = std::max(body.Scale - c_FragmentShrinkSpeed * dt, 0.f);
                if (body.Scale <= 0.f) {
                    body.IsAlive = false;
                    continue;
                }
            }

            if (body.LifeTime > 0.f && body.Age >= body.LifeTime) {
                float const shrinkProgress = glm::clamp((body.Age - body.LifeTime) / c_LifeTimeShrinkDuration, 0.f, 1.f);
                body.Scale = 1.f - shrinkProgress;
                if (body.Scale <= 0.f) {
                    body.IsAlive = false;
                    continue;
                }
            }

            glm::vec3 const extraAcceleration = body.BoomerangActive && !body.BirdHasCollided ? body.BoomerangAcceleration : glm::vec3(0.f);
            body.Velocity += (Gravity + extraAcceleration) * dt;
            body.Position += body.Velocity * dt;
            body.Velocity *= LinearDamping;
            body.AngularVel *= AngularDamping;

            float const angularSpeed = glm::length(body.AngularVel);
            if (angularSpeed > 1e-5f) {
                glm::quat const dq = glm::angleAxis(angularSpeed * dt, body.AngularVel / angularSpeed);
                body.Rotation = glm::normalize(dq * body.Rotation);
            }
        }
    }

    void AngryBirdsPhysics::ResolveCollisions() {
        std::vector<Contact> impactContacts;
        if (CurrentSolver == SolverType::ConstraintBasedJacobi) {
            m_ConstraintSolver.ResolveCollisions(Bodies, Restitution, Friction, impactContacts);
        } else {
            ImpulseSolver::ResolveCollisions(Bodies, Restitution, Friction, impactContacts);
        }
        BreakSolver::TryBreakBodies(Bodies, impactContacts, FragmentsCreated);
    }
}
