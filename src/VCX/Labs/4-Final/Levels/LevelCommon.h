#pragma once

#include <glm/glm.hpp>

#include "Labs/4-Final/Levels/ILevel.h"
#include "Labs/4-Final/World.h"

namespace VCX::Labs::Final {
    // 所有关卡共享的材质调色板. (原本在各场景函数中重复)
    namespace Mat {
        inline glm::vec3 const Wood   { .62f, .38f, .16f };
        inline glm::vec3 const Glass  { .35f, .72f, .95f };
        inline glm::vec3 const Stone  { .55f, .55f, .58f };
        inline glm::vec3 const Target { .28f, .75f, .22f };

        constexpr float DensWood   = .8f;
        constexpr float DensGlass  = .5f;
        constexpr float DensStone  = 2.1f;
        constexpr float DensTarget = .6f;
    }

    // 存活的 Target 方块数量.
    inline int CountAliveTargets(World const & world) {
        int n = 0;
        for (auto const & b : world.Rigid.Bodies) {
            if (b.IsAlive && b.Kind == BodyKind::Target) ++n;
        }
        return n;
    }

    // "摧毁所有目标即胜利"的通用胜负规则.
    inline GameState TargetsClearedStatus(World const & world) {
        return CountAliveTargets(world) == 0 ? GameState::Won : GameState::Playing;
    }
} // namespace VCX::Labs::Final
