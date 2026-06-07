#pragma once

namespace VCX::Labs::Final {
    class FluidWorld;
    class RigidWorld;

    // 서브시스템 간 상호작용. 모든 결합 로직을 여기 한 곳에 모은다.
    namespace Coupling {
        // 유체 -> 강체: 부력/항력. 수면 높이장 기준으로 잠긴 비율을 계산한다.
        // skipIndex: 드래그 중인 발사체 등 적분에서 제외할 바디 인덱스.
        void ApplyBuoyancy(FluidWorld const & fluid, RigidWorld & rigid, float dt, int skipIndex);

        // 강체 -> 유체 (1): 강체가 차지하는 grid 셀을 동적 solid 로 마킹 + 경계 속도 부여.
        // 매 유체 스텝 전에 호출. 물이 강체에 막히고 밀린다.
        void MarkRigidSolids(RigidWorld const & rigid, FluidWorld & fluid);

        // 강체 -> 유체 (2): 강체 내부로 파고든 입자를 표면 밖으로 밀어내고 속도를 맞춘다.
        // 매 유체 스텝 후에 호출. 물보라/관통 방지.
        void PushParticlesOutOfRigid(RigidWorld const & rigid, FluidWorld & fluid);
    }
} // namespace VCX::Labs::Final
