#pragma once

namespace VCX::Labs::Final {
    class FluidWorld;
    class RigidWorld;

    // 子系统间相互作用. 所有耦合逻辑集中于此.
    namespace Coupling {
        // 流体 -> 刚体: 浮力/阻力. 依据水面高度场计算浸没比例.
        // skipIndex: 需从积分中排除的刚体索引(如拖拽中的发射体).
        void ApplyBuoyancy(FluidWorld const & fluid, RigidWorld & rigid, float dt, int skipIndex);

        // 刚体 -> 流体 (1): 把刚体占据的网格单元标记为动态 solid + 赋予边界速度.
        // 每个流体步之前调用. 水被刚体挡住并推开.
        void MarkRigidSolids(RigidWorld const & rigid, FluidWorld & fluid);

        // 刚体 -> 流体 (2): 把侵入刚体内部的粒子推到表面外并匹配速度.
        // 每个流体步之后调用. 防止水花飞溅/穿透.
        void PushParticlesOutOfRigid(RigidWorld const & rigid, FluidWorld & fluid);
    }
} // namespace VCX::Labs::Final
