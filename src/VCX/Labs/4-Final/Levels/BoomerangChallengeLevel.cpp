#include "Labs/4-Final/Levels/BoomerangChallengeLevel.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void BoomerangChallengeLevel::Setup(AngryBirdsPhysics & physics, float breakThreshold) const {
        LevelBuilder builder(physics, breakThreshold);

        builder.AddStone(glm::vec3(3.7f, 1.3f, 0.f), glm::vec3(.68f, 1.3f, 1.1f), 2.2f);
        builder.AddTarget(glm::vec3(4.66f, .34f, 0.f), glm::vec3(.28f, .28f, .28f), .32f);

        builder.AddWood(glm::vec3(4.66f, .08f, 0.f), glm::vec3(.42f, .08f, .45f));
        builder.AddGlass(glm::vec3(2.75f, .35f, -.95f), glm::vec3(.22f, .35f, .18f));
        builder.AddGlass(glm::vec3(2.75f, .35f, .95f), glm::vec3(.22f, .35f, .18f));
    }
}
