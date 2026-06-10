#include "Labs/4-Final/AngryBirdsScene.h"

namespace VCX::Labs::Final {
    std::vector<BirdType> AngryBirdsScene::Reset(World & world, float breakThreshold, LevelRegister::LevelID level) const {
        world.Reset();

        auto levelInstance = LevelRegister::GetInstance().GetLevel(level);
        if (levelInstance) {
            levelInstance->Setup(world, breakThreshold);
            return levelInstance->GetBirds();
        }

        return { BirdType::Normal };
    }
}
