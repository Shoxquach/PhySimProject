#pragma once

#include "Labs/4-Final/Levels/ILevel.h"

namespace VCX::Labs::Final {
    // M2 레벨: 댐 붕괴. 돌블록 댐이 물을 막고 있다 (two-way solid).
    // 댐을 부수면 갇혀 있던 물이 쏟아져 오른쪽 타깃을 덮친다.
    class Level05_DamBreak : public ILevel {
    public:
        std::string_view      Name() const override { return "5. Dam Break (Two-way)"; }
        void                  Setup(World & world, float breakThreshold) override;
        GameState             Status(World const & world) const override;
        std::vector<ShotType> Shots() const override { return { ShotType::Bird, ShotType::WaterBalloon }; }
    };
} // namespace VCX::Labs::Final
