#include "Labs/4-Final/AngryBirdsPhysics.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <random>

#include <Eigen/Geometry>
#include <fcl/narrowphase/collision.h>
#include <fcl/narrowphase/contact.h>
#include <glm/ext.hpp>

namespace VCX::Labs::Final {
    namespace {
        constexpr float c_ContactSlop = .005f;
        constexpr float c_PositionCorrection = .72f;
        constexpr float c_RotationImpulseScale = 1.0f;
        constexpr float c_FragmentShrinkSpeed = .85f;
        constexpr float c_LifeTimeShrinkDuration = .5f;

        // Random number generator for fragment explosion effects
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

        glm::vec3 SupportExtent(glm::quat const & rotation, glm::vec3 const & halfSize, glm::vec3 const & axis) {
            glm::mat3 const r = glm::mat3_cast(rotation);
            return glm::vec3(
                std::abs(glm::dot(axis, r[0])) * halfSize.x,
                std::abs(glm::dot(axis, r[1])) * halfSize.y,
                std::abs(glm::dot(axis, r[2])) * halfSize.z);
        }

        float ProjectedRadius(glm::quat const & rotation, glm::vec3 const & halfSize, glm::vec3 const & axis) {
            glm::vec3 const extent = SupportExtent(rotation, halfSize, axis);
            return extent.x + extent.y + extent.z;
        }

        glm::vec3 ClosestPointOnOrientedBox(glm::vec3 const & point, RigidBody const & box) {
            glm::vec3 const local = glm::inverse(box.Rotation) * (point - box.Position);
            glm::vec3 const clamped = glm::clamp(local, -box.HalfSize, box.HalfSize);
            return box.Position + box.Rotation * clamped;
        }

        void AddAxis(std::vector<glm::vec3> & axes, glm::vec3 const & axis) {
            float const len = glm::length(axis);
            if (len > 1e-5f) {
                axes.push_back(axis / len);
            }
        }

        glm::vec3 ContactVelocity(RigidBody const & body, glm::vec3 const & point) {
            if (body.IsStatic) return glm::vec3(0.f);
            return body.Velocity + glm::cross(body.AngularVel, point - body.Position);
        }

        fcl::CollisionObjectf CreateCollisionObject(RigidBody const & body) {
            fcl::Transform3f transform = fcl::Transform3f::Identity();
            transform.translation() = Eigen::Vector3f(body.Position.x, body.Position.y, body.Position.z);
            transform.linear() = Eigen::Quaternionf(body.Rotation.w, body.Rotation.x, body.Rotation.y, body.Rotation.z).toRotationMatrix();

            if (body.Kind == BodyKind::Bird) {
                auto shape = std::make_shared<fcl::Spheref>(body.Radius * body.Scale);
                return fcl::CollisionObjectf(shape, transform);
            }

            glm::vec3 const scaledHalfSize = body.HalfSize * body.Scale;
            auto shape = std::make_shared<fcl::Boxf>(scaledHalfSize.x * 2.f, scaledHalfSize.y * 2.f, scaledHalfSize.z * 2.f);
            return fcl::CollisionObjectf(shape, transform);
        }

        bool QueryFclContact(RigidBody const & a, RigidBody const & b, Contact & contact) {
            auto objA = CreateCollisionObject(a);
            auto objB = CreateCollisionObject(b);

            fcl::CollisionRequestf request;
            request.enable_contact = true;
            request.num_max_contacts = 16;
            fcl::CollisionResultf result;
            fcl::collide(&objA, &objB, request, result);
            if (!result.isCollision()) return false;

            glm::vec3 normal(0.f);
            glm::vec3 position(0.f);
            float penetration = 0.f;
            contact.Points.clear();
            contact.Points.reserve(result.numContacts());
            for (int i = 0; i < result.numContacts(); ++i) {
                auto const & c = result.getContact(i);
                glm::vec3 const point = glm::vec3(c.pos[0], c.pos[1], c.pos[2]);
                normal += glm::vec3(c.normal[0], c.normal[1], c.normal[2]);
                position += point;
                penetration += c.penetration_depth;
                contact.Points.push_back(point);
            }
            normal = SafeNormalize(normal);
            if (glm::dot(normal, b.Position - a.Position) < 0.f) {
                normal = -normal;
            }
            contact.Normal = normal;
            contact.Point = position / float(result.numContacts());
            contact.Penetration = penetration / float(result.numContacts());
            return true;
        }
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
        // compute inverse inertia for a solid sphere: I = 2/5 m r^2
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
        body.Toughness = toughness;
        body.Breakable = breakable;
        body.Age = 0.f;
        body.LifeTime = -1.f;
        // compute inverse inertia for box (half-sizes provided)
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
        if (CurrentSolver == SolverType::ConstraintBasedJacobi) {
            ResolveCollisionsConstraintBased();
            return;
        }

        std::vector<Contact> contacts;
        std::vector<Contact> impactContacts;

        for (int iteration = 0; iteration < 12; ++iteration) {
            contacts.clear();
            for (int i = 0; i < int(Bodies.size()); ++i) {
                if (!Bodies[i].IsAlive) continue;

                Contact ground;
                if (GroundContact(Bodies[i], i, ground)) {
                    contacts.push_back(ground);
                }
                for (int j = i + 1; j < int(Bodies.size()); ++j) {
                    Contact contact;
                    if (FindContact(i, j, contact)) {
                        contacts.push_back(contact);
                    }
                }
            }

            for (auto & c : contacts) {
                auto & a = Bodies[c.A];
                RigidBody groundBody;
                groundBody.IsStatic = true;
                groundBody.InvMass = 0.f;
                RigidBody & b = c.B >= 0 ? Bodies[c.B] : groundBody;
                auto stopBoomerangOnImpact = [](RigidBody & body) {
                    if (body.Kind == BodyKind::Bird && body.BirdWasLaunched && (body.Position.x > 0.f || body.BoomerangActive)) {
                        body.BirdHasCollided = true;
                        body.BoomerangActive = false;
                        body.BoomerangAcceleration = glm::vec3(0.f);
                    }
                };
                stopBoomerangOnImpact(a);
                if (c.B >= 0) {
                    stopBoomerangOnImpact(b);
                }
                if (a.IsStatic && b.IsStatic) continue;

                // positional correction (translation only)
                float const invMassSum = a.InvMass + b.InvMass;
                if (invMassSum <= 0.f) continue;
                float const correctionMag = std::max(c.Penetration - c_ContactSlop, 0.f) / invMassSum * c_PositionCorrection;
                if (!a.IsStatic) a.Position -= c.Normal * correctionMag * a.InvMass;
                if (!b.IsStatic) b.Position += c.Normal * correctionMag * b.InvMass;

                // compute world-space inverse inertia
                glm::mat3 const Ra = glm::mat3_cast(a.Rotation);
                glm::mat3 const Rb = glm::mat3_cast(b.Rotation);
                glm::mat3 const Ia_inv = Ra * a.InvInertiaLocal * glm::transpose(Ra);
                glm::mat3 const Ib_inv = Rb * b.InvInertiaLocal * glm::transpose(Rb);

                auto angularTerm = [&](glm::mat3 const & Iinv, glm::vec3 const & r, glm::vec3 const & n) {
                    glm::vec3 const rcrossn = glm::cross(r, n);
                    glm::vec3 const tmp = Iinv * rcrossn;
                    return glm::dot(n, glm::cross(tmp, r));
                };

                auto const & points = c.Points.empty() ? std::vector<glm::vec3> { c.Point } : c.Points;
                for (auto const & point : points) {
                    glm::vec3 const relVel = ContactVelocity(b, point) - ContactVelocity(a, point);
                    float const normalVel = glm::dot(relVel, c.Normal);
                    c.Impact = std::max(c.Impact, std::max(-normalVel, 0.f));
                    if (normalVel >= 0.f) continue;

                    glm::vec3 const rA = point - a.Position;
                    glm::vec3 const rB = point - b.Position;

                    float const angA = a.IsStatic ? 0.f : angularTerm(Ia_inv, rA, c.Normal);
                    float const angB = b.IsStatic ? 0.f : angularTerm(Ib_inv, rB, c.Normal);

                    float const effectiveMass = invMassSum + angA + angB;
                    if (effectiveMass <= 1e-6f) continue;

                    float const impulseMag = -(1.f + Restitution) * normalVel / effectiveMass;
                    glm::vec3 const impulse = impulseMag * c.Normal;

                    if (!a.IsStatic) {
                        a.Velocity -= impulse * a.InvMass;
                        a.AngularVel -= Ia_inv * glm::cross(rA, impulse);
                    }
                    if (!b.IsStatic) {
                        b.Velocity += impulse * b.InvMass;
                        b.AngularVel += Ib_inv * glm::cross(rB, impulse);
                    }

                    // friction (tangential) impulse using same pattern with tangent direction
                    glm::vec3 tangent = relVel - normalVel * c.Normal;
                    if (glm::length(tangent) > 1e-5f) {
                        tangent = glm::normalize(tangent);
                        float const angAT = a.IsStatic ? 0.f : angularTerm(Ia_inv, rA, tangent);
                        float const angBT = b.IsStatic ? 0.f : angularTerm(Ib_inv, rB, tangent);
                        float const effectiveMassT = invMassSum + angAT + angBT;
                        if (effectiveMassT > 1e-6f) {
                            float const jt = -glm::dot(relVel, tangent) / effectiveMassT;
                            float const maxF = impulseMag * Friction;
                            float const jtClamped = glm::clamp(jt, -maxF, maxF);
                            glm::vec3 const frictionImpulse = jtClamped * tangent;
                            if (!a.IsStatic) {
                                a.Velocity -= frictionImpulse * a.InvMass;
                                a.AngularVel -= Ia_inv * glm::cross(rA, frictionImpulse);
                            }
                            if (!b.IsStatic) {
                                b.Velocity += frictionImpulse * b.InvMass;
                                b.AngularVel += Ib_inv * glm::cross(rB, frictionImpulse);
                            }
                        }
                    }
                }
                if (c.Impact > 0.f) {
                    impactContacts.push_back(c);
                }
            }
        }

        TryBreakBodies(impactContacts);
    }

    void AngryBirdsPhysics::TryBreakBodies(std::vector<Contact> const & contacts) {
        struct BreakRequest {
            int       Index = -1;
            glm::vec3 Normal = glm::vec3(0.f, 1.f, 0.f);
            float     Impact = 0.f;
        };
        std::vector<BreakRequest> breaks;
        for (auto const & c : contacts) {
            auto const & a = Bodies[c.A];
            RigidBody const * b = c.B >= 0 ? &Bodies[c.B] : nullptr;
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
            if (index >= 0 && index < int(Bodies.size()) && Bodies[index].IsAlive) {
                BreakBody(index, request.Normal, request.Impact);
            }
        }
    }

    void AngryBirdsPhysics::BreakBody(int index, glm::vec3 const & impulseDir, float impact) {
        RigidBody const source = Bodies[index];
        if (source.Generation >= 1 || source.Kind == BodyKind::Fragment) {
            Bodies[index].IsAlive = false;
            return;
        }

        Bodies[index].IsAlive = false;
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
                    // compute inverse inertia for fragment (approx box)
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
                    frag.Toughness = 100.f;
                    frag.Breakable = false;

                    // Calculate fragment position relative to center for explosion direction
                    glm::vec3 const fragOffset = glm::vec3(x * h.x, y * h.y, z * h.z) * .55f;
                    glm::vec3 const fragWorldOffset = source.Rotation * fragOffset;

                    // Random explosion effect: fragments burst in random directions
                    glm::vec3 const randomDir = RandomUnitVector();
                    float const randomBurstSpeed = RandomFloat(2.0f, 6.0f) + impact * RandomFloat(0.3f, 0.8f);

                    // Base velocity from source
                    glm::vec3 baseVelocity = source.Velocity;

                    // Impact-driven velocity (along impulse direction with randomness)
                    glm::vec3 impactVelocity = dir * (impact * RandomFloat(1.0f, 2.0f) + RandomFloat(0.5f, 2.0f));

                    // Random burst velocity (explosion effect)
                    glm::vec3 burstVelocity = randomDir * randomBurstSpeed;

                    // Outward velocity from center (positional explosion)
                    glm::vec3 outwardDir = SafeNormalize(fragWorldOffset, randomDir);
                    glm::vec3 outwardVelocity = outwardDir * RandomFloat(1.5f, 4.0f);

                    // Combine all velocity components
                    frag.Velocity = baseVelocity + impactVelocity + burstVelocity + outwardVelocity;

                    // Random angular velocity for tumbling effect
                    frag.AngularVel = source.AngularVel + RandomUnitVector() * RandomFloat(2.0f, 10.0f);

                    frag.Age = 0.f;
                    frag.LifeTime = -1.f;
                    frag.Scale = 1.f;
                    Bodies.push_back(frag);
                    FragmentsCreated++;
                }
            }
        }
    }

    bool AngryBirdsPhysics::FindContact(int a, int b, Contact & contact) const {
        auto const & ba = Bodies[a];
        auto const & bb = Bodies[b];
        if (!ba.IsAlive || !bb.IsAlive || (ba.IsStatic && bb.IsStatic)) return false;
        if (ba.Kind == BodyKind::Fragment || bb.Kind == BodyKind::Fragment) return false;
        if (ba.Kind == BodyKind::Bird && bb.Kind == BodyKind::Bird) {
            if (!ba.BirdWasLaunched && !bb.BirdWasLaunched) return false;
            return SphereBoxContact(ba, bb, a, b, contact);
        }

        if (ba.Kind == BodyKind::Bird && bb.Kind != BodyKind::Bird) {
            return SphereBoxContact(ba, bb, a, b, contact);
        }
        if (bb.Kind == BodyKind::Bird && ba.Kind != BodyKind::Bird) {
            // bb is the sphere and ba is the box: swap arguments and indices
            return SphereBoxContact(bb, ba, b, a, contact);
        }
        return BoxBoxContact(ba, bb, a, b, contact);
    }

    bool AngryBirdsPhysics::SphereBoxContact(RigidBody const & sphere, RigidBody const & box, int sphereIndex, int boxIndex, Contact & contact) const {
        if (!QueryFclContact(sphere, box, contact)) return false;
        contact.A = sphereIndex;
        contact.B = boxIndex;
        return true;
    }

    bool AngryBirdsPhysics::BoxBoxContact(RigidBody const & a, RigidBody const & b, int aIndex, int bIndex, Contact & contact) const {
        if (!QueryFclContact(a, b, contact)) return false;
        contact.A = aIndex;
        contact.B = bIndex;
        return true;
    }

    bool AngryBirdsPhysics::GroundContact(RigidBody const & body, int index, Contact & contact) const {
        if (body.IsStatic || !body.IsAlive) return false;

        float bottom = body.Position.y - body.Radius * body.Scale;
        glm::vec3 contactPoint = glm::vec3(body.Position.x, GroundY, body.Position.z);
        contact.Points.clear();
        contact.Points.push_back(contactPoint);
        if (body.Kind != BodyKind::Bird) {
            bottom = std::numeric_limits<float>::max();
            glm::vec3 sumPoint(0.f);
            int count = 0;
            contact.Points.clear();
            std::array<glm::vec3, 8> const signs = {
                glm::vec3(-1,  1,  1), glm::vec3( 1,  1,  1), glm::vec3( 1,  1, -1), glm::vec3(-1,  1, -1),
                glm::vec3(-1, -1,  1), glm::vec3( 1, -1,  1), glm::vec3( 1, -1, -1), glm::vec3(-1, -1, -1),
            };
            glm::vec3 const scaledHalfSize = body.HalfSize * body.Scale;
            for (auto const & sign : signs) {
                glm::vec3 const vertex = body.Position + body.Rotation * (sign * scaledHalfSize);
                if (vertex.y < bottom - 1e-5f) {
                    bottom = vertex.y;
                    sumPoint = vertex;
                    count = 1;
                    contact.Points.clear();
                    contact.Points.push_back(vertex);
                } else if (vertex.y <= bottom + 1e-5f) {
                    sumPoint += vertex;
                    ++count;
                    contact.Points.push_back(vertex);
                }
            }
            if (count > 0) {
                contactPoint = sumPoint / float(count);
            }
        }
        if (bottom >= GroundY) return false;

        contact.A = index;
        contact.B = -1;
        contact.Normal = glm::vec3(0.f, -1.f, 0.f);
        contact.Penetration = GroundY - bottom;
        contact.Point = contactPoint;
        if (contact.Points.empty()) {
            contact.Points.push_back(contactPoint);
        }
        return true;
    }

    void AngryBirdsPhysics::ResolveCollisionsConstraintBased() {
        std::vector<Contact> contacts;
        std::vector<Contact> impactContacts;

        for (int i = 0; i < int(Bodies.size()); ++i) {
            if (!Bodies[i].IsAlive) continue;

            Contact ground;
            if (GroundContact(Bodies[i], i, ground)) {
                contacts.push_back(ground);
            }
            for (int j = i + 1; j < int(Bodies.size()); ++j) {
                Contact contact;
                if (FindContact(i, j, contact)) {
                    contacts.push_back(contact);
                }
            }
        }

        for (auto & c : contacts) {
            auto & a = Bodies[c.A];
            RigidBody groundBody;
            groundBody.IsStatic = true;
            groundBody.InvMass = 0.f;
            RigidBody & b = c.B >= 0 ? Bodies[c.B] : groundBody;

            auto stopBoomerangOnImpact = [](RigidBody & body) {
                if (body.Kind == BodyKind::Bird && body.BirdWasLaunched && (body.Position.x > 0.f || body.BoomerangActive)) {
                    body.BirdHasCollided = true;
                    body.BoomerangActive = false;
                    body.BoomerangAcceleration = glm::vec3(0.f);
                }
            };
            stopBoomerangOnImpact(a);
            if (c.B >= 0) {
                stopBoomerangOnImpact(b);
            }

            if (a.IsStatic && b.IsStatic) continue;

            glm::mat3 const Ra = glm::mat3_cast(a.Rotation);
            glm::mat3 const Rb = glm::mat3_cast(b.Rotation);
            glm::mat3 const Ia_inv = Ra * a.InvInertiaLocal * glm::transpose(Ra);
            glm::mat3 const Ib_inv = Rb * b.InvInertiaLocal * glm::transpose(Rb);

            auto contactVelocity = [&](RigidBody const & body, glm::vec3 const & point) {
                if (body.IsStatic) return glm::vec3(0.f);
                return body.Velocity + glm::cross(body.AngularVel, point - body.Position);
            };

            const auto& points = c.Points.empty() ? std::vector<glm::vec3>{ c.Point } : c.Points;
            for (auto const & point : points) {
                glm::vec3 const relVel = contactVelocity(b, point) - contactVelocity(a, point);
                float const normalVel = glm::dot(relVel, c.Normal);
                c.Impact = std::max(c.Impact, std::max(-normalVel, 0.f));
            }

            if (c.Impact > 0.f) {
                impactContacts.push_back(c);
            }
        }

        m_ConstraintSolver.SolveConstraints(Bodies, contacts, Restitution, Friction, 12);

        TryBreakBodies(impactContacts);
    }
}
