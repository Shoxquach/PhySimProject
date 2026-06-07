#pragma once

#include "Labs/4-Final/Levels/ILevel.h"

namespace VCX::Labs::Final {
    // M1 关卡: 护城河(moat). FLIP 水池 + 浮力耦合(单向).
    // 木筏浮起(密度<水), 石头下沉(密度>水).
    class Level04_Moat : public ILevel {
    public:
        std::string_view Name() const override { return "4. Moat (Buoyancy)"; }
        void             Setup(World & world, float breakThreshold) override;
        GameState        Status(World const & world) const override;
    };
} // namespace VCX::Labs::Final
