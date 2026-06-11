#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Labs/4-Final/Physics/PhysicsTypes.h"

namespace VCX::Labs::Final {
    enum class SolverType {
        SequentialImpulse,
        ConstraintBasedJacobi
    };

    struct ContactConstraint {
        int BodyA = -1;
        int BodyB = -1;
        
        glm::vec3 Normal = glm::vec3(0.f, 1.f, 0.f);
        glm::vec3 Point = glm::vec3(0.f);
        float Penetration = 0.f;
        
        glm::vec3 rA = glm::vec3(0.f);
        glm::vec3 rB = glm::vec3(0.f);
        
        float EffectiveMassNormal = 0.f;
        float EffectiveMassTangent1 = 0.f;
        float EffectiveMassTangent2 = 0.f;
        
        float NormalImpulse = 0.f;
        float TangentImpulse1 = 0.f;
        float TangentImpulse2 = 0.f;
        
        float Friction = 0.f;
        float Restitution = 0.f;
        float Bias = 0.f;
        
        glm::vec3 Tangent1 = glm::vec3(0.f);
        glm::vec3 Tangent2 = glm::vec3(0.f);
    };

    struct VelocityState {
        glm::vec3 Velocity = glm::vec3(0.f);
        glm::vec3 AngularVelocity = glm::vec3(0.f);
    };

    class ConstraintSolver {
    public:
        void SolveConstraints(
            std::vector<RigidBody>& bodies,
            const std::vector<Contact>& contacts,
            float restitution,
            float friction,
            int iterations = 12
        );

        void ResolveCollisions(
            std::vector<RigidBody>& bodies,
            float restitution,
            float friction,
            std::vector<Contact>& impactContacts
        );

        void SetPositionCorrectionEnabled(bool enabled) { m_PositionCorrectionEnabled = enabled; }
        void SetWarmStartingEnabled(bool enabled) { m_WarmStartingEnabled = enabled; }

    private:
        void PrepareConstraints(
            const std::vector<RigidBody>& bodies,
            const std::vector<Contact>& contacts,
            float restitution,
            float friction
        );

        void ApplyPositionCorrection(std::vector<RigidBody>& bodies, const std::vector<Contact>& contacts);
        void SolveNormalConstraintsJacobi(const std::vector<RigidBody>& bodies);
        void SolveFrictionConstraintsJacobi(const std::vector<RigidBody>& bodies);
        void ApplyImpulses(std::vector<RigidBody>& bodies);

        void ComputeTangents(const glm::vec3& normal, glm::vec3& tangent1, glm::vec3& tangent2);
        float ComputeEffectiveMass(
            const RigidBody& body,
            const glm::vec3& r,
            const glm::vec3& dir,
            const glm::mat3& invInertia
        );

        std::vector<ContactConstraint> m_Constraints;
        std::vector<VelocityState> m_VelocityStates;
        std::vector<glm::mat3> m_InvInertias;
        std::vector<glm::vec3> m_DeltaVelocities;
        std::vector<glm::vec3> m_DeltaAngularVelocities;
        std::vector<glm::vec3> m_DeltaPositions;
        
        bool m_PositionCorrectionEnabled = true;
        bool m_WarmStartingEnabled = true;
        
        static constexpr int   c_OuterIterations = 10;
        static constexpr int   c_VelocityIterations = 6;
        static constexpr float c_ContactSlop = 0.005f;
        static constexpr float c_PositionCorrectionFactor = 0.45f;
        static constexpr float c_MaxPositionCorrection = 0.05f;
        static constexpr float c_PositionRelaxation = 0.5f;
        static constexpr float c_JacobiRelaxation = 0.45f;
        static constexpr float c_BaumgarteFactor = 0.18f;
        static constexpr float c_RestitutionVelocityThreshold = 2.0f;
        static constexpr float c_MaxJacobiRestitution = 0.12f;
    };

}
