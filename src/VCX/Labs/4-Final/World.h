#pragma once

#include <optional>

#include <glm/glm.hpp>

#include "Labs/4-Final/Physics/RigidWorld.h"
#include "Labs/4-Final/Physics/FluidWorld.h"
#include "Labs/4-Final/Physics/Coupling.h"

namespace VCX::Labs::Final {
    // 서브시스템 간 상호작용 on/off 플래그.
    // 레벨이 Setup()에서 필요한 커플링만 켠다.
    struct CouplingFlags {
        bool Buoyancy  = false; // (M1) 유체 -> 강체 부력/항력 (one-way)
        bool FlowSolid = false; // (M2) 강체 -> 유체 solid 경계 마킹 (two-way)
        bool FluidSoft = false; // (M3) 유체 <-> FEM
    };

    // 모든 물리 서브시스템을 들고, 정해진 순서로 전진시키는 조율자.
    //   (M3) std::optional<SoftWorld> Soft;
    class World {
    public:
        RigidWorld                Rigid;
        std::optional<FluidWorld> Fluid;   // 레벨이 켤 때만 생성 (안 쓰면 비용 0)
        CouplingFlags             Couple;

        // 강체 한 스텝 (+커플링). 충돌 안정성을 위해 프레임당 여러 번(substep) 호출된다.
        void Step(float dt, int draggedIndex, glm::vec3 const & draggedPosition) {
            if (Fluid && Couple.Buoyancy) {
                Coupling::ApplyBuoyancy(*Fluid, Rigid, dt, draggedIndex);
            }
            Rigid.Step(dt, draggedIndex, draggedPosition);
        }

        // 유체 한 스텝. 강체보다 무거우므로 프레임당 한 번만 호출한다.
        void StepFluid(float dt) {
            if (!Fluid) return;
            if (Couple.FlowSolid) {
                Coupling::MarkRigidSolids(Rigid, *Fluid);        // 강체 -> 유체 경계 (스텝 전)
            }
            Fluid->Step(dt);
            if (Couple.FlowSolid) {
                Coupling::PushParticlesOutOfRigid(Rigid, *Fluid); // 관통 정리 (스텝 후)
            }
        }

        // 레벨 전환 시 모든 서브시스템 초기화.
        void Reset() {
            Rigid.Clear();
            Fluid.reset();
            Couple = CouplingFlags {};
        }
    };
} // namespace VCX::Labs::Final
