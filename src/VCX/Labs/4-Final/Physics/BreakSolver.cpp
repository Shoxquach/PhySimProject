#include "Labs/4-Final/Physics/BreakSolver.h"

#include <algorithm>
#include <cmath>
#include <random>

#include <glm/ext.hpp>

namespace VCX::Labs::Final::BreakSolver {
    namespace {
        std::mt19937& GetRandomEngine() {
            static std::mt19937 engine(std::random_device{}());
            return engine;
        }

        float RandomFloat(float min, float max) {
            std::uniform_real_distribution<float> dist(min, max);
            return dist(GetRandomEngine());
        }

        glm::vec3 RandomUnitVector() {
            float phi = RandomFloat(0.f, 2.f * 3.14159265359f);
            float cosTheta = RandomFloat(-1.f, 1.f);
            float sinTheta = std::sqrt(1.f - cosTheta * cosTheta);
            return glm::vec3(
                sinTheta * std::cos(phi),
                sinTheta * std::sin(phi),
                cosTheta
            );
        }

        glm::vec3 SafeNormalize(glm::vec3 const & v, glm::vec3 const & fallback = glm::vec3(0.f, 1.f, 0.f)) {
            float const len = glm::length(v);
            return len > 1e-6f ? v / len : fallback;
        }

        void BreakBody(std::vector<RigidBody> & bodies, int index, glm::vec3 const & impulseDir, float impact, int & fragmentsCreated) {
            RigidBody const source = bodies[index];
            if (source.Generation >= 1 || source.Kind == BodyKind::Fragment) {
                bodies[index].IsAlive = false;
                return;
            }

            bodies[index].IsAlive = false;
            glm::vec3 const h = source.HalfSize * .52f;
            glm::vec3 const dir = SafeNormalize(impulseDir);

            for (int x = -1; x <= 1; x += 2) {
                for (int y = -1; y <= 1; y += 2) {
                    for (int z = -1; z <= 1; z += 2) {
                        RigidBody frag = source;
                        frag.Kind = BodyKind::Fragment;
                        frag.Generation = source.Generation + 1;
                        frag.HalfSize = h * glm::vec3(.72f);
                        frag.Position = source.Position + source.Rotation * (glm::vec3(x * h.x, y * h.y, z * h.z) * .55f);
                        frag.Mass = std::max(source.Mass / 8.f, .08f);
                        frag.InvMass = 1.f / frag.Mass;
                        {
                            float const Ixx = (1.f / 3.f) * frag.Mass * (frag.HalfSize.y * frag.HalfSize.y + frag.HalfSize.z * frag.HalfSize.z);
                            float const Iyy = (1.f / 3.f) * frag.Mass * (frag.HalfSize.x * frag.HalfSize.x + frag.HalfSize.z * frag.HalfSize.z);
                            float const Izz = (1.f / 3.f) * frag.Mass * (frag.HalfSize.x * frag.HalfSize.x + frag.HalfSize.y * frag.HalfSize.y);
                            glm::mat3 inv = glm::mat3(0.f);
                            inv[0][0] = Ixx > 0.f ? 1.f / Ixx : 0.f;
                            inv[1][1] = Iyy > 0.f ? 1.f / Iyy : 0.f;
                            inv[2][2] = Izz > 0.f ? 1.f / Izz : 0.f;
                            frag.InvInertiaLocal = inv;
                        }
                        frag.Color = source.Color * glm::vec3(.95f + .04f * x, .95f + .04f * y, .95f + .04f * z);
                        frag.Alpha = glm::clamp(source.Alpha + RandomFloat(-0.04f, 0.04f), 0.25f, 1.f);
                        frag.Toughness = 100.f;
                        frag.Breakable = false;

                        glm::vec3 const fragOffset = glm::vec3(x * h.x, y * h.y, z * h.z) * .55f;
                        glm::vec3 const fragWorldOffset = source.Rotation * fragOffset;
                        glm::vec3 const randomDir = RandomUnitVector();
                        float const randomBurstSpeed = RandomFloat(2.0f, 6.0f) + impact * RandomFloat(0.3f, 0.8f);
                        glm::vec3 baseVelocity = source.Velocity;
                        glm::vec3 impactVelocity = dir * (impact * RandomFloat(1.0f, 2.0f) + RandomFloat(0.5f, 2.0f));
                        glm::vec3 burstVelocity = randomDir * randomBurstSpeed;
                        glm::vec3 outwardDir = SafeNormalize(fragWorldOffset, randomDir);
                        glm::vec3 outwardVelocity = outwardDir * RandomFloat(1.5f, 4.0f);

                        frag.Velocity = baseVelocity + impactVelocity + burstVelocity + outwardVelocity;
                        frag.AngularVel = source.AngularVel + RandomUnitVector() * RandomFloat(2.0f, 10.0f);

                        frag.Age = 0.f;
                        frag.LifeTime = -1.f;
                        frag.Scale = 1.f;
                        bodies.push_back(frag);
                        ++fragmentsCreated;
                    }
                }
            }
        }
    }

    void TryBreakBodies(std::vector<RigidBody> & bodies, std::vector<Contact> const & contacts, int & fragmentsCreated) {
        struct BreakRequest {
            int       Index = -1;
            glm::vec3 Normal = glm::vec3(0.f, 1.f, 0.f);
            float     Impact = 0.f;
        };

        std::vector<BreakRequest> breaks;
        for (auto const & c : contacts) {
            auto const & a = bodies[c.A];
            RigidBody const * b = c.B >= 0 ? &bodies[c.B] : nullptr;
            float const impact = c.Impact;

            if (a.Breakable && impact > a.Toughness) {
                breaks.push_back({ c.A, -c.Normal, impact });
            }
            if (b && b->Breakable && impact > b->Toughness) {
                breaks.push_back({ c.B, c.Normal, impact });
            }
        }

        std::sort(breaks.begin(), breaks.end(), [](auto const & lhs, auto const & rhs) {
            if (lhs.Index != rhs.Index) return lhs.Index < rhs.Index;
            return lhs.Impact > rhs.Impact;
        });
        breaks.erase(std::unique(breaks.begin(), breaks.end(), [](auto const & lhs, auto const & rhs) { return lhs.Index == rhs.Index; }), breaks.end());

        for (auto const & request : breaks) {
            int const index = request.Index;
            if (index >= 0 && index < int(bodies.size()) && bodies[index].IsAlive) {
                BreakBody(bodies, index, request.Normal, request.Impact, fragmentsCreated);
            }
        }
    }
}
