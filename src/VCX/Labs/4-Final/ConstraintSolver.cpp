#include "Labs/4-Final/ConstraintSolver.h"
#include "Labs/4-Final/AngryBirdsPhysics.h"

#include <algorithm>
#include <cmath>

namespace VCX::Labs::Final {

    namespace {
        glm::vec3 SafeNormalize(const glm::vec3& v, const glm::vec3& fallback = glm::vec3(0.f, 1.f, 0.f)) {
            float len = glm::length(v);
            return len > 1e-6f ? v / len : fallback;
        }

        glm::mat3 ComputeWorldInvInertia(const RigidBody& body) {
            if (body.IsStatic || body.InvMass <= 0.f) return glm::mat3(0.f);
            glm::mat3 R = glm::mat3_cast(body.Rotation);
            return R * body.InvInertiaLocal * glm::transpose(R);
        }

        glm::vec3 GetContactVelocity(const RigidBody& body, const glm::vec3& point) {
            if (body.IsStatic) return glm::vec3(0.f);
            return body.Velocity + glm::cross(body.AngularVel, point - body.Position);
        }
    }

    void ConstraintSolver::SolveConstraints(
        std::vector<RigidBody>& bodies,
        const std::vector<Contact>& contacts,
        float restitution,
        float friction,
        int iterations
    ) {
        if (contacts.empty()) return;

        if (m_PositionCorrectionEnabled) {
            ApplyPositionCorrection(bodies, contacts);
        }

        PrepareConstraints(bodies, contacts, restitution, friction);

        m_VelocityStates.resize(bodies.size());
        for (size_t i = 0; i < bodies.size(); ++i) {
            m_VelocityStates[i].Velocity = bodies[i].Velocity;
            m_VelocityStates[i].AngularVelocity = bodies[i].AngularVel;
        }

        for (int iter = 0; iter < iterations; ++iter) {
            SolveVelocityConstraints(bodies);
        }

        ApplyImpulses(bodies);
    }

    void ConstraintSolver::PrepareConstraints(
        const std::vector<RigidBody>& bodies,
        const std::vector<Contact>& contacts,
        float restitution,
        float friction
    ) {
        m_Constraints.clear();
        m_Constraints.reserve(contacts.size() * 4);
        m_InvInertias.resize(bodies.size());

        for (size_t i = 0; i < bodies.size(); ++i) {
            m_InvInertias[i] = ComputeWorldInvInertia(bodies[i]);
        }

        for (const auto& contact : contacts) {
            const RigidBody& bodyA = bodies[contact.A];
            RigidBody groundBody;
            groundBody.IsStatic = true;
            groundBody.InvMass = 0.f;
            const RigidBody& bodyB = (contact.B >= 0) ? bodies[contact.B] : groundBody;

            if (bodyA.IsStatic && bodyB.IsStatic) continue;

            const auto& points = contact.Points.empty() 
                ? std::vector<glm::vec3>{ contact.Point } 
                : contact.Points;

            for (const auto& point : points) {
                ContactConstraint constraint;
                constraint.BodyA = contact.A;
                constraint.BodyB = contact.B;
                constraint.Normal = contact.Normal;
                constraint.Point = point;
                constraint.Penetration = contact.Penetration;
                constraint.Friction = friction;
                constraint.Restitution = restitution;

                constraint.rA = point - bodyA.Position;
                if (!bodyB.IsStatic && contact.B >= 0) {
                    constraint.rB = point - bodyB.Position;
                } else {
                    constraint.rB = glm::vec3(0.f);
                }

                ComputeTangents(constraint.Normal, constraint.Tangent1, constraint.Tangent2);

                constraint.EffectiveMassNormal = ComputeEffectiveMass(
                    bodyA, constraint.rA, constraint.Normal, m_InvInertias[contact.A]
                );
                if (!bodyB.IsStatic) {
                    constraint.EffectiveMassNormal += ComputeEffectiveMass(
                        bodyB, constraint.rB, constraint.Normal, 
                        (contact.B >= 0) ? m_InvInertias[contact.B] : glm::mat3(0.f)
                    );
                }

                if (constraint.EffectiveMassNormal > 1e-6f) {
                    constraint.EffectiveMassNormal = 1.0f / constraint.EffectiveMassNormal;
                } else {
                    constraint.EffectiveMassNormal = 0.f;
                }

                constraint.EffectiveMassTangent1 = ComputeEffectiveMass(
                    bodyA, constraint.rA, constraint.Tangent1, m_InvInertias[contact.A]
                );
                if (!bodyB.IsStatic) {
                    constraint.EffectiveMassTangent1 += ComputeEffectiveMass(
                        bodyB, constraint.rB, constraint.Tangent1,
                        (contact.B >= 0) ? m_InvInertias[contact.B] : glm::mat3(0.f)
                    );
                }
                constraint.EffectiveMassTangent1 = (constraint.EffectiveMassTangent1 > 1e-6f) 
                    ? 1.0f / constraint.EffectiveMassTangent1 : 0.f;

                constraint.EffectiveMassTangent2 = ComputeEffectiveMass(
                    bodyA, constraint.rA, constraint.Tangent2, m_InvInertias[contact.A]
                );
                if (!bodyB.IsStatic) {
                    constraint.EffectiveMassTangent2 += ComputeEffectiveMass(
                        bodyB, constraint.rB, constraint.Tangent2,
                        (contact.B >= 0) ? m_InvInertias[contact.B] : glm::mat3(0.f)
                    );
                }
                constraint.EffectiveMassTangent2 = (constraint.EffectiveMassTangent2 > 1e-6f) 
                    ? 1.0f / constraint.EffectiveMassTangent2 : 0.f;

                glm::vec3 velA = GetContactVelocity(bodyA, point);
                glm::vec3 velB = GetContactVelocity(bodyB, point);
                float relVelNormal = glm::dot(velB - velA, constraint.Normal);

                if (relVelNormal < -1.0f) {
                    constraint.Bias = restitution * relVelNormal;
                } else {
                    constraint.Bias = 0.f;
                }

                m_Constraints.push_back(constraint);
            }
        }
    }

    void ConstraintSolver::ApplyPositionCorrection(std::vector<RigidBody>& bodies, const std::vector<Contact>& contacts) {
        for (const auto& contact : contacts) {
            RigidBody& bodyA = bodies[contact.A];
            RigidBody groundBody;
            groundBody.IsStatic = true;
            groundBody.InvMass = 0.f;
            RigidBody& bodyB = contact.B >= 0 ? bodies[contact.B] : groundBody;

            if (bodyA.IsStatic && bodyB.IsStatic) continue;

            float const invMassSum = bodyA.InvMass + bodyB.InvMass;
            if (invMassSum <= 0.f) continue;

            float const correctionMag = std::max(contact.Penetration - c_ContactSlop, 0.f) / invMassSum * c_PositionCorrectionFactor;
            glm::vec3 const correction = contact.Normal * correctionMag;

            if (!bodyA.IsStatic) bodyA.Position -= correction * bodyA.InvMass;
            if (!bodyB.IsStatic) bodyB.Position += correction * bodyB.InvMass;
        }
    }

    void ConstraintSolver::SolveVelocityConstraints(const std::vector<RigidBody>& bodies) {
        std::vector<glm::vec3> deltaVelocities(bodies.size(), glm::vec3(0.f));
        std::vector<glm::vec3> deltaAngularVelocities(bodies.size(), glm::vec3(0.f));

        for (auto& constraint : m_Constraints) {
            const RigidBody& bodyA = bodies[constraint.BodyA];
            RigidBody groundBody;
            groundBody.IsStatic = true;
            const RigidBody& bodyB = (constraint.BodyB >= 0) ? bodies[constraint.BodyB] : groundBody;

            VelocityState& stateA = m_VelocityStates[constraint.BodyA];
            VelocityState stateB = (constraint.BodyB >= 0) 
                ? m_VelocityStates[constraint.BodyB] 
                : VelocityState{};

            glm::vec3 velA = stateA.Velocity + glm::cross(stateA.AngularVelocity, constraint.rA);
            glm::vec3 velB = stateB.Velocity + glm::cross(stateB.AngularVelocity, constraint.rB);
            glm::vec3 relativeVel = velB - velA;

            float vn = glm::dot(relativeVel, constraint.Normal);
            float lambda = -(vn + constraint.Bias) * constraint.EffectiveMassNormal;

            float oldImpulse = constraint.NormalImpulse;
            constraint.NormalImpulse = std::max(oldImpulse + lambda, 0.f);
            lambda = constraint.NormalImpulse - oldImpulse;

            glm::vec3 impulse = lambda * constraint.Normal;

            if (!bodyA.IsStatic) {
                deltaVelocities[constraint.BodyA] -= impulse * bodyA.InvMass;
                deltaAngularVelocities[constraint.BodyA] -= m_InvInertias[constraint.BodyA] * glm::cross(constraint.rA, impulse);
            }
            if (!bodyB.IsStatic && constraint.BodyB >= 0) {
                deltaVelocities[constraint.BodyB] += impulse * bodyB.InvMass;
                deltaAngularVelocities[constraint.BodyB] += m_InvInertias[constraint.BodyB] * glm::cross(constraint.rB, impulse);
            }

            velA = stateA.Velocity + glm::cross(stateA.AngularVelocity, constraint.rA);
            velB = stateB.Velocity + glm::cross(stateB.AngularVelocity, constraint.rB);
            relativeVel = velB - velA;

            float vt1 = glm::dot(relativeVel, constraint.Tangent1);
            float lambdaT1 = -vt1 * constraint.EffectiveMassTangent1;

            float maxFriction = constraint.Friction * constraint.NormalImpulse;
            float oldTangentImpulse1 = constraint.TangentImpulse1;
            constraint.TangentImpulse1 = glm::clamp(oldTangentImpulse1 + lambdaT1, -maxFriction, maxFriction);
            lambdaT1 = constraint.TangentImpulse1 - oldTangentImpulse1;

            glm::vec3 tangentImpulse1 = lambdaT1 * constraint.Tangent1;

            if (!bodyA.IsStatic) {
                deltaVelocities[constraint.BodyA] -= tangentImpulse1 * bodyA.InvMass;
                deltaAngularVelocities[constraint.BodyA] -= m_InvInertias[constraint.BodyA] * glm::cross(constraint.rA, tangentImpulse1);
            }
            if (!bodyB.IsStatic && constraint.BodyB >= 0) {
                deltaVelocities[constraint.BodyB] += tangentImpulse1 * bodyB.InvMass;
                deltaAngularVelocities[constraint.BodyB] += m_InvInertias[constraint.BodyB] * glm::cross(constraint.rB, tangentImpulse1);
            }

            velA = stateA.Velocity + glm::cross(stateA.AngularVelocity, constraint.rA);
            velB = stateB.Velocity + glm::cross(stateB.AngularVelocity, constraint.rB);
            relativeVel = velB - velA;

            float vt2 = glm::dot(relativeVel, constraint.Tangent2);
            float lambdaT2 = -vt2 * constraint.EffectiveMassTangent2;

            float oldTangentImpulse2 = constraint.TangentImpulse2;
            constraint.TangentImpulse2 = glm::clamp(oldTangentImpulse2 + lambdaT2, -maxFriction, maxFriction);
            lambdaT2 = constraint.TangentImpulse2 - oldTangentImpulse2;

            glm::vec3 tangentImpulse2 = lambdaT2 * constraint.Tangent2;

            if (!bodyA.IsStatic) {
                deltaVelocities[constraint.BodyA] -= tangentImpulse2 * bodyA.InvMass;
                deltaAngularVelocities[constraint.BodyA] -= m_InvInertias[constraint.BodyA] * glm::cross(constraint.rA, tangentImpulse2);
            }
            if (!bodyB.IsStatic && constraint.BodyB >= 0) {
                deltaVelocities[constraint.BodyB] += tangentImpulse2 * bodyB.InvMass;
                deltaAngularVelocities[constraint.BodyB] += m_InvInertias[constraint.BodyB] * glm::cross(constraint.rB, tangentImpulse2);
            }
        }

        for (size_t i = 0; i < m_VelocityStates.size(); ++i) {
            m_VelocityStates[i].Velocity += deltaVelocities[i];
            m_VelocityStates[i].AngularVelocity += deltaAngularVelocities[i];
        }
    }

    void ConstraintSolver::ApplyImpulses(std::vector<RigidBody>& bodies) {
        for (size_t i = 0; i < bodies.size(); ++i) {
            if (!bodies[i].IsStatic) {
                bodies[i].Velocity = m_VelocityStates[i].Velocity;
                bodies[i].AngularVel = m_VelocityStates[i].AngularVelocity;
            }
        }
    }

    void ConstraintSolver::ComputeTangents(const glm::vec3& normal, glm::vec3& tangent1, glm::vec3& tangent2) {
        if (std::abs(normal.x) < 0.707f) {
            tangent1 = glm::normalize(glm::cross(glm::vec3(1.f, 0.f, 0.f), normal));
        } else {
            tangent1 = glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), normal));
        }
        tangent2 = glm::cross(normal, tangent1);
    }

    float ConstraintSolver::ComputeEffectiveMass(
        const RigidBody& body,
        const glm::vec3& r,
        const glm::vec3& dir,
        const glm::mat3& invInertia
    ) {
        if (body.IsStatic || body.InvMass <= 0.f) return 0.f;

        float massTerm = body.InvMass;

        glm::vec3 rcrossn = glm::cross(r, dir);
        glm::vec3 invIcross = invInertia * rcrossn;
        float inertiaTerm = glm::dot(rcrossn, invIcross);

        return massTerm + inertiaTerm;
    }

}
