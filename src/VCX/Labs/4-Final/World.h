#pragma once

#include <optional>

#include <glm/glm.hpp>

#include "Labs/4-Final/Physics/RigidWorld.h"
#include "Labs/4-Final/Physics/FluidWorld.h"
#include "Labs/4-Final/Physics/Coupling.h"

namespace VCX::Labs::Final {
    // 子系统间相互作用的 on/off 开关.
    // 关卡在 Setup() 中只开启需要的耦合.
    struct CouplingFlags {
        bool Buoyancy  = false; // (M1) 流体 -> 刚体 浮力/阻力 (单向)
        bool FlowSolid = false; // (M2) 刚体 -> 流体 solid 边界标记 (双向)
        bool FluidSoft = false; // (M3) 流体 <-> FEM
    };

    // 持有所有物理子系统, 按固定顺序推进的调度器.
    //   (M3) std::optional<SoftWorld> Soft;
    class World {
    public:
        RigidWorld                Rigid;
        std::optional<FluidWorld> Fluid;   // 仅当关卡开启时创建 (不用则零开销)
        CouplingFlags             Couple;

        // 刚体一步 (+耦合). 为碰撞稳定性, 每帧多次(子步)调用.
        void Step(float dt, int draggedIndex, glm::vec3 const & draggedPosition) {
            if (Fluid && Couple.Buoyancy) {
                Coupling::ApplyBuoyancy(*Fluid, Rigid, dt, draggedIndex);
            }
            Rigid.Step(dt, draggedIndex, draggedPosition);
        }

        // 流体一步. 比刚体重, 故每帧只调用一次.
        void StepFluid(float dt) {
            if (!Fluid) return;
            if (Couple.FlowSolid) {
                Coupling::MarkRigidSolids(Rigid, *Fluid);        // 刚体 -> 流体 边界 (步进前)
            }
            Fluid->Step(dt);
            if (Couple.FlowSolid) {
                Coupling::PushParticlesOutOfRigid(Rigid, *Fluid); // 清理穿透 (步进后)
            }
        }

        // 切换关卡时重置所有子系统.
        void Reset() {
            Rigid.Clear();
            Fluid.reset();
            Couple = CouplingFlags {};
        }
    };
} // namespace VCX::Labs::Final
