#pragma once

#include <vector>

#include "Labs/4-Final/AngryBirdsPhysics.h"

namespace VCX::Labs::Final {
    /// @brief 关卡基接口
    struct ILevel {
        virtual ~ILevel() = default;
        
        /// @brief 初始化关卡
        /// @param physics 物理引擎实例
        /// @param breakThreshold 破坏阈值
        virtual void Setup(AngryBirdsPhysics & physics, float breakThreshold) const = 0;

        /// @brief 获取本关卡可用的小鸟序列
        virtual std::vector<BirdType> GetBirds() const = 0;
    };
}
