#pragma once

namespace VCX::Labs::Final {
    class FluidWorld;
    class AngryBirdsPhysics;

    namespace Coupling {
        void ApplyBuoyancy(FluidWorld const & fluid, AngryBirdsPhysics & rigid, float dt, int skipIndex);
        void MarkRigidSolids(AngryBirdsPhysics const & rigid, FluidWorld & fluid);
        void PushParticlesOutOfRigid(AngryBirdsPhysics const & rigid, FluidWorld & fluid);
    }
}
