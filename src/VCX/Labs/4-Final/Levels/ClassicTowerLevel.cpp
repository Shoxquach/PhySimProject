#include "Labs/4-Final/Levels/ClassicTowerLevel.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void ClassicTowerLevel::Setup(AngryBirdsPhysics & physics, float breakThreshold) const {
        LevelBuilder builder(physics, breakThreshold);

        for (int i = 0; i < 3; ++i) {
            float const x = 2.8f + float(i) * 1.15f;
            builder.AddWood(glm::vec3(x, .75f, -.8f), glm::vec3(.28f, .75f, .22f));
            builder.AddWood(glm::vec3(x, .75f, .8f), glm::vec3(.28f, .75f, .22f));
        }
        builder.AddGlass(glm::vec3(3.95f, 2.15f, 0.f), glm::vec3(1.86f, .65f, 1.2f));
        builder.AddStone(glm::vec3(3.35f, .22f, 0.f), glm::vec3(.55f, .22f, .55f));
        builder.AddStone(glm::vec3(4.7f, .22f, 0.f), glm::vec3(.55f, .22f, .55f));
        builder.AddStone(glm::vec3(3.5f, 3.32f, 0.f), glm::vec3(.45f, .52f, .45f));
        builder.AddStone(glm::vec3(2.05f, .75f, 0.f), glm::vec3(.45f, .75f, .7f));

        builder.AddTarget(glm::vec3(3.75f, .72f, 0.f), glm::vec3(.28f, .28f, .28f));
    }
}
