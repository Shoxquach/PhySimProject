#include "Labs/4-Final/AngryBirdsPhysics.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

#include <Eigen/Geometry>
#include <fcl/narrowphase/collision.h>
#include <fcl/narrowphase/contact.h>
#include <glm/ext.hpp>

namespace VCX::Labs::Final {
    namespace {
        constexpr float c_ContactSlop = .005f;
        constexpr float c_PositionCorrection = .72f;
        constexpr float c_RotationImpulseScale = 1.0f;

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
                auto shape = std::make_shared<fcl::Spheref>(body.Radius);
                return fcl::CollisionObjectf(shape, transform);
            }

            auto shape = std::make_shared<fcl::Boxf>(body.HalfSize.x * 2.f, body.HalfSize.y * 2.f, body.HalfSize.z * 2.f);
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
            for (int i = 0; i < result.numContacts(); ++i) {
                auto const & c = result.getContact(i);
                normal += glm::vec3(c.normal[0], c.normal[1], c.normal[2]);
                position += glm::vec3(c.pos[0], c.pos[1], c.pos[2]);
                penetration += c.penetration_depth;
            }
            normal = glm::normalize(normal);
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

    int AngryBirdsPhysics::AddBird(glm::vec3 const & anchor) {
        RigidBody bird;
        bird.Kind = BodyKind::Bird;
        bird.Mass = 1.4f;
        bird.InvMass = 1.f / bird.Mass;
        bird.Position = anchor;
        bird.Radius = BirdRadius;
        bird.HalfSize = glm::vec3(BirdRadius);
        bird.Color = glm::vec3(.9f, .12f, .08f);
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

    void AngryBirdsPhysics::Step(float dt, int draggedIndex, glm::vec3 const & draggedPosition) {
        if (draggedIndex >= 0 && draggedIndex < int(Bodies.size())) {
            auto & body = Bodies[draggedIndex];
            body.Position = draggedPosition;
            body.Velocity = glm::vec3(0.f);
            body.AngularVel = glm::vec3(0.f);
            body.Rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
        }

        Integrate(dt, draggedIndex);
        ResolveCollisions();

        Bodies.erase(
            std::remove_if(Bodies.begin(), Bodies.end(), [](RigidBody const & body) {
                return !body.IsAlive || (body.LifeTime > 0.f && body.Age >= body.LifeTime)
                    || body.Position.y < -10.f || glm::length(body.Position) > 80.f;
            }),
            Bodies.end());
    }

    void AngryBirdsPhysics::Integrate(float dt, int draggedIndex) {
        for (int i = 0; i < int(Bodies.size()); ++i) {
            auto & body = Bodies[i];
            if (!body.IsAlive || body.IsStatic || i == draggedIndex) {
                continue;
            }

            body.Age += dt;
            
            // Calculate scale for disappearing animation
            if (body.LifeTime > 0.f && body.Age >= body.LifeTime * 0.95f) {
                float disappearProgress = (body.Age - body.LifeTime * 0.95f) / (body.LifeTime * .05f);
                disappearProgress = glm::clamp(disappearProgress, 0.f, 1.f);
                body.Scale = 1.f - disappearProgress * disappearProgress;
            }
            
            body.Velocity += Gravity * dt;
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
                if (a.IsStatic && b.IsStatic) continue;

                // positional correction (translation only)
                float const invMassSum = a.InvMass + b.InvMass;
                if (invMassSum <= 0.f) continue;
                float const correctionMag = std::max(c.Penetration - c_ContactSlop, 0.f) / invMassSum * c_PositionCorrection;
                if (!a.IsStatic) a.Position -= c.Normal * correctionMag * a.InvMass;
                if (!b.IsStatic) b.Position += c.Normal * correctionMag * b.InvMass;

                glm::vec3 const relVel = ContactVelocity(b, c.Point) - ContactVelocity(a, c.Point);
                float const normalVel = glm::dot(relVel, c.Normal);
                c.Impact = std::max(c.Impact, std::max(-normalVel, 0.f));
                if (c.Impact > 0.f) {
                    impactContacts.push_back(c);
                }
                if (normalVel >= 0.f) continue;

                // compute world-space inverse inertia
                glm::mat3 const Ra = glm::mat3_cast(a.Rotation);
                glm::mat3 const Rb = glm::mat3_cast(b.Rotation);
                glm::mat3 const Ia_inv = Ra * a.InvInertiaLocal * glm::transpose(Ra);
                glm::mat3 const Ib_inv = Rb * b.InvInertiaLocal * glm::transpose(Rb);

                glm::vec3 const rA = c.Point - a.Position;
                glm::vec3 const rB = c.Point - b.Position;

                auto angularTerm = [&](glm::mat3 const & Iinv, glm::vec3 const & r, glm::vec3 const & n) {
                    glm::vec3 const rcrossn = glm::cross(r, n);
                    glm::vec3 const tmp = Iinv * rcrossn;
                    return glm::dot(n, glm::cross(tmp, r));
                };

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
                    frag.Velocity = source.Velocity + dir * (impact * 1.4f + 1.5f) + source.Rotation * (glm::vec3(x, y, z) * .8f);
                    frag.AngularVel = source.AngularVel + glm::vec3(z, x, y) * 5.f;
                    frag.Age = 0.f;
                    frag.LifeTime = 4.5f;
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
        if (body.IsStatic || !body.IsAlive || body.Kind == BodyKind::Fragment) return false;

        float bottom = body.Position.y - body.Radius;
        glm::vec3 contactPoint = glm::vec3(body.Position.x, GroundY, body.Position.z);
        if (body.Kind != BodyKind::Bird) {
            bottom = std::numeric_limits<float>::max();
            glm::vec3 sumPoint(0.f);
            int count = 0;
            std::array<glm::vec3, 8> const signs = {
                glm::vec3(-1,  1,  1), glm::vec3( 1,  1,  1), glm::vec3( 1,  1, -1), glm::vec3(-1,  1, -1),
                glm::vec3(-1, -1,  1), glm::vec3( 1, -1,  1), glm::vec3( 1, -1, -1), glm::vec3(-1, -1, -1),
            };
            for (auto const & sign : signs) {
                glm::vec3 const vertex = body.Position + body.Rotation * (sign * body.HalfSize);
                if (vertex.y < bottom - 1e-5f) {
                    bottom = vertex.y;
                    sumPoint = vertex;
                    count = 1;
                } else if (vertex.y <= bottom + 1e-5f) {
                    sumPoint += vertex;
                    ++count;
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
        return true;
    }
}
