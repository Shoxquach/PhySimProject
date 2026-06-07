#include "Labs/4-Final/Levels/LevelRegistry.h"

#include "Labs/4-Final/Levels/Level01_ClassicTower.h"
#include "Labs/4-Final/Levels/Level02_StoneCastle.h"
#include "Labs/4-Final/Levels/Level03_TargetPractice.h"
#include "Labs/4-Final/Levels/Level04_Moat.h"
#include "Labs/4-Final/Levels/Level05_DamBreak.h"

namespace VCX::Labs::Final {
    std::vector<std::unique_ptr<ILevel>> CreateAllLevels() {
        std::vector<std::unique_ptr<ILevel>> levels;
        levels.push_back(std::make_unique<Level01_ClassicTower>());
        levels.push_back(std::make_unique<Level02_StoneCastle>());
        levels.push_back(std::make_unique<Level03_TargetPractice>());
        levels.push_back(std::make_unique<Level04_Moat>());
        levels.push_back(std::make_unique<Level05_DamBreak>());
        return levels;
    }
} // namespace VCX::Labs::Final
