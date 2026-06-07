#pragma once

#include <glm/glm.hpp>

#include "Labs/4-Final/Levels/ILevel.h"
#include "Labs/4-Final/World.h"

namespace VCX::Labs::Final {
    // 모든 레벨이 공유하는 재질 팔레트. (원래 각 씬 함수에 중복돼 있던 값)
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

    // 살아있는 Target 블록 개수.
    inline int CountAliveTargets(World const & world) {
        int n = 0;
        for (auto const & b : world.Rigid.Bodies) {
            if (b.IsAlive && b.Kind == BodyKind::Target) ++n;
        }
        return n;
    }

    // "모든 타깃을 파괴하면 승리" 라는 공통 승패 규칙.
    inline GameState TargetsClearedStatus(World const & world) {
        return CountAliveTargets(world) == 0 ? GameState::Won : GameState::Playing;
    }
} // namespace VCX::Labs::Final
