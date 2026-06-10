#include "Labs/4-Final/Physics/ImpulseSolver.h"

#include <algorithm>

#include <glm/ext.hpp>

#include "Labs/4-Final/Physics/ContactDetection.h"

namespace VCX::Labs::Final {
    namespace {
        constexpr float c_ContactSlop = .005f;
        constexpr float c_PositionCorrection = .72f;

        glm::vec3 ContactVelocity(RigidBody const & body, glm::vec3 const & point) {
            if (body.IsStatic) return glm::vec3(0.f);
            return body.Velocity + glm::cross(body.AngularVel, point - body.Position);
        }

        void StopBoomerangOnImpact(RigidBody & body) {
            if (body.Kind == BodyKind::Bird && body.BirdWasLaunched && (body.Position.x > 0.f || body.BoomerangActive)) {
                body.BirdHasCollided = true;
                body.BoomerangActive = false;
                body.BoomerangAcceleration = glm::vec3(0.f);
            }
        }
    }

    namespace ImpulseSolver {
    void ResolveCollisions(
        std::vector<RigidBody> & bodies,
        float restitution,
        float friction,
        std::vector<Contact> & impactContacts
    ) {
        std::vector<Contact> contacts;
        impactContacts.clear();

        for (int iteration = 0; iteration < 12; ++iteration) {
            ContactDetection::CollectContacts(bodies, contacts);

            for (auto & c : contacts) {
                auto & a = bodies[c.A];
                RigidBody groundBody;
                groundBody.IsStatic = true;
                groundBody.InvMass = 0.f;
                RigidBody & b = c.B >= 0 ? bodies[c.B] : groundBody;
                StopBoomerangOnImpact(a);
                if (c.B >= 0) {
                    StopBoomerangOnImpact(b);
                }
                if (a.IsStatic && b.IsStatic) continue;

                float const invMassSum = a.InvMass + b.InvMass;
                if (invMassSum <= 0.f) continue;
                float const correctionMag = std::max(c.Penetration - c_ContactSlop, 0.f) / invMassSum * c_PositionCorrection;
                if (!a.IsStatic) a.Position -= c.Normal * correctionMag * a.InvMass;
                if (!b.IsStatic) b.Position += c.Normal * correctionMag * b.InvMass;

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

                    float const impulseMag = -(1.f + restitution) * normalVel / effectiveMass;
                    glm::vec3 const impulse = impulseMag * c.Normal;

                    if (!a.IsStatic) {
                        a.Velocity -= impulse * a.InvMass;
                        a.AngularVel -= Ia_inv * glm::cross(rA, impulse);
                    }
                    if (!b.IsStatic) {
                        b.Velocity += impulse * b.InvMass;
                        b.AngularVel += Ib_inv * glm::cross(rB, impulse);
                    }

                    glm::vec3 tangent = relVel - normalVel * c.Normal;
                    if (glm::length(tangent) > 1e-5f) {
                        tangent = glm::normalize(tangent);
                        float const angAT = a.IsStatic ? 0.f : angularTerm(Ia_inv, rA, tangent);
                        float const angBT = b.IsStatic ? 0.f : angularTerm(Ib_inv, rB, tangent);
                        float const effectiveMassT = invMassSum + angAT + angBT;
                        if (effectiveMassT > 1e-6f) {
                            float const jt = -glm::dot(relVel, tangent) / effectiveMassT;
                            float const maxF = impulseMag * friction;
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

    }
    }
}
