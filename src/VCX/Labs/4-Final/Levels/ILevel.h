#pragma once

#include <string_view>
#include <vector>

#include <glm/glm.hpp>

#include "Labs/4-Final/World.h"

namespace VCX::Labs::Final {
    enum class GameState {
        Playing,
        Won,
        Lost,
    };

    // 발사체 종류. M0 에서는 Bird 만 사용. M2/M3 에서 확장.
    enum class ShotType {
        Bird,
        WaterBalloon,
        Jelly,
    };

    // 레벨 한 개 = 이 인터페이스의 한 구현체 = 한 파일.
    // "어떤 서브시스템을 켜고 / 무엇을 배치하고 / 승패 조건이 무엇인지" 만 책임진다.
    // 발사체(새) 생성과 렌더링은 Case 가 담당하므로 레벨은 환경/타깃만 다룬다.
    class ILevel {
    public:
        virtual ~ILevel() = default;

        // UI 콤보박스 및 라벨에 쓰일 이름.
        virtual std::string_view Name() const = 0;

        // 새총(발사 기준점) 위치. 무대 배율 적용.
        virtual glm::vec3 Anchor() const { return glm::vec3(-5.5f, 1.35f, 0.f) * WorldScale; }

        // 서브시스템 켜기 + 초기 오브젝트 배치. world 는 호출 전에 Reset() 된 상태.
        // breakThreshold 는 난이도 노브로, 각 블록 toughness 의 기준값이 된다.
        virtual void Setup(World & world, float breakThreshold) = 0;

        // 레벨 고유 로직 (수위 상승, 시간 제한 등). 매 프레임 호출.
        virtual void Tick(World & world, float dt) {}

        // 현재 승/패/진행 상태.
        virtual GameState Status(World const & world) const = 0;

        // 이 레벨에서 사용 가능한 발사체 목록.
        virtual std::vector<ShotType> Shots() const { return { ShotType::Bird }; }
    };
} // namespace VCX::Labs::Final
