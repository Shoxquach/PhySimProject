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

    // 发射体种类. M0 只用 Bird, M2/M3 扩展.
    enum class ShotType {
        Bird,
        WaterBalloon,
        Jelly,
    };

    // 一个关卡 = 此接口的一个实现 = 一个文件.
    // 只负责"开启哪些子系统 / 布置什么 / 胜负条件是什么".
    // 发射体(小鸟)的生成与渲染由 Case 负责, 关卡只处理环境/目标.
    class ILevel {
    public:
        virtual ~ILevel() = default;

        // 用于 UI 下拉框与标签的名称.
        virtual std::string_view Name() const = 0;

        // 弹弓(发射基准点)位置. 应用舞台缩放.
        virtual glm::vec3 Anchor() const { return glm::vec3(-5.5f, 1.35f, 0.f) * WorldScale; }

        // 开启子系统 + 布置初始物体. 调用前 world 已被 Reset().
        // breakThreshold 为难度旋钮, 是各方块 toughness 的基准值.
        virtual void Setup(World & world, float breakThreshold) = 0;

        // 关卡专属逻辑 (水位上升, 时间限制等). 每帧调用.
        virtual void Tick(World & world, float dt) {}

        // 当前 胜/负/进行中 状态.
        virtual GameState Status(World const & world) const = 0;

        // 本关可用的发射体列表.
        virtual std::vector<ShotType> Shots() const { return { ShotType::Bird }; }
    };
} // namespace VCX::Labs::Final
