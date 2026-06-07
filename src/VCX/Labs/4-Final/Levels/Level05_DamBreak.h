#pragma once

#include "Labs/4-Final/Levels/ILevel.h"

namespace VCX::Labs::Final {
    // M2 关卡: 溃坝. 石块水坝挡着水 (two-way solid).
    // 击垮水坝后, 蓄积的水倾泻而下冲击右侧目标.
    class Level05_DamBreak : public ILevel {
    public:
        std::string_view      Name() const override { return "5. Dam Break (Two-way)"; }
        void                  Setup(World & world, float breakThreshold) override;
        GameState             Status(World const & world) const override;
        std::vector<ShotType> Shots() const override { return { ShotType::Bird, ShotType::WaterBalloon }; }
    };
} // namespace VCX::Labs::Final
