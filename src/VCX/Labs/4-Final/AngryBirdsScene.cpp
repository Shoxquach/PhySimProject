#include "Labs/4-Final/AngryBirdsScene.h"

namespace VCX::Labs::Final {
    std::vector<BirdType> AngryBirdsScene::Reset(AngryBirdsPhysics & physics, float breakThreshold, LevelRegister::LevelID level) const {
        physics.Clear();

        auto levelInstance = LevelRegister::GetInstance().GetLevel(level);
        if (levelInstance) {
            levelInstance->Setup(physics, breakThreshold);
            return levelInstance->GetBirds();
        }

        return { BirdType::Normal };
    }
}
