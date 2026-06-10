#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "Labs/4-Final/Levels/ILevel.h"
#include "Labs/4-Final/Levels/BoomerangChallengeLevel.h"
#include "Labs/4-Final/Levels/ClassicTowerLevel.h"
#include "Labs/4-Final/Levels/DominoRunLevel.h"
#include "Labs/4-Final/Levels/DamBreakLevel.h"
#include "Labs/4-Final/Levels/GrandCitadelLevel.h"
#include "Labs/4-Final/Levels/MoatLevel.h"
#include "Labs/4-Final/Levels/StoneCastleLevel.h"
#include "Labs/4-Final/Levels/TargetPracticeLevel.h"

namespace VCX::Labs::Final {
    /// @brief 关卡注册管理器
    class LevelRegister {
    public:
        enum class LevelID {
            ClassicTower,
            StoneCastle,
            TargetPractice,
            DominoRun,
            BoomerangChallenge,
            GrandCitadel,
            Moat,
            DamBreak,
        };

        /// @brief 获取注册管理器单例
        static LevelRegister & GetInstance() {
            static LevelRegister instance;
            return instance;
        }

        /// @brief 根据关卡ID获取关卡实例
        /// @param levelID 关卡ID
        /// @return 关卡接口指针
        std::shared_ptr<ILevel> GetLevel(LevelID levelID) const {
            auto it = _levelMap.find(static_cast<int>(levelID));
            if (it != _levelMap.end()) {
                return it->second;
            }
            return nullptr;
        }

        /// @brief 获取关卡总数
        size_t GetLevelCount() const {
            return _levelMap.size();
        }

    private:
        LevelRegister() {
            // 注册所有关卡
            _levelMap[static_cast<int>(LevelID::ClassicTower)] = std::make_shared<ClassicTowerLevel>();
            _levelMap[static_cast<int>(LevelID::StoneCastle)] = std::make_shared<StoneCastleLevel>();
            _levelMap[static_cast<int>(LevelID::TargetPractice)] = std::make_shared<TargetPracticeLevel>();
            _levelMap[static_cast<int>(LevelID::DominoRun)] = std::make_shared<DominoRunLevel>();
            _levelMap[static_cast<int>(LevelID::BoomerangChallenge)] = std::make_shared<BoomerangChallengeLevel>();
            _levelMap[static_cast<int>(LevelID::GrandCitadel)] = std::make_shared<GrandCitadelLevel>();
            _levelMap[static_cast<int>(LevelID::Moat)] = std::make_shared<MoatLevel>();
            _levelMap[static_cast<int>(LevelID::DamBreak)] = std::make_shared<DamBreakLevel>();
        }

        ~LevelRegister() = default;

        LevelRegister(const LevelRegister &) = delete;
        LevelRegister & operator=(const LevelRegister &) = delete;

        std::unordered_map<int, std::shared_ptr<ILevel>> _levelMap;
    };
}
