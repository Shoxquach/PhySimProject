#pragma once

#include <optional>
#include <vector>

#include <glm/glm.hpp>

#include "Labs/4-Final/Physics/PhysicsSystem.h"
#include "Labs/4-Final/Physics/FluidWorld.h"
#include "Labs/4-Final/Physics/Coupling.h"

namespace VCX::Labs::Final {
    struct CouplingFlags {
        bool Buoyancy  = false;
        bool FlowSolid = false;
    };

    class World {
    public:
        AngryBirdsPhysics           Rigid;
        std::optional<FluidWorld>   Fluid;
        CouplingFlags               Couple;

        void StepRigid(float dt, std::vector<PinnedBody> const & pinnedBodies, int buoyancySkipIndex = -1) {
            if (Fluid && Couple.Buoyancy) {
                Coupling::ApplyBuoyancy(*Fluid, Rigid, dt, buoyancySkipIndex);
            }
            Rigid.Step(dt, pinnedBodies);
        }

        void StepFluid(float dt) {
            if (!Fluid) return;
            if (Couple.FlowSolid) {
                Coupling::MarkRigidSolids(Rigid, *Fluid);
                Coupling::PushParticlesOutOfRigid(Rigid, *Fluid);
            }
            Fluid->Step(dt);
            if (Couple.FlowSolid) {
                Coupling::PushParticlesOutOfRigid(Rigid, *Fluid);
            }
        }

        void Reset() {
            Rigid.Clear();
            Fluid.reset();
            Couple = CouplingFlags {};
        }
    };
}
