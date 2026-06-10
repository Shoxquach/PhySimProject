#include "Labs/4-Final/Levels/DominoRunLevel.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void DominoRunLevel::Setup(AngryBirdsPhysics & physics, float breakThreshold) const {
        LevelBuilder builder(physics, breakThreshold);

        builder.AddStone(glm::vec3(-.46f, 1.f, 0.f), glm::vec3(1.f, 1.f, 1.f));
        builder.AddStone(glm::vec3(0.f, 2.2f, 0.f), glm::vec3(1.76f, .2f, 1.f));
        builder.AddStone(glm::vec3(-.46f, 2.8f, 0.f), glm::vec3(1.f, .4f, 1.f));
        builder.AddTarget(glm::vec3(1.1f, .28f, 0.f), glm::vec3(.28f, .28f, .28f));

        constexpr int   dominoCount = 9;
        constexpr float startX       = 2.65f;
        constexpr float spacing      = 1.78f;

        for (int i = 0; i < dominoCount; ++i) {
            float const x = startX + float(i) * spacing;
            builder.AddStone(glm::vec3(x, 1.05f, 0.f), glm::vec3(.12f, 1.05f, .48f));
        }

        float const targetX = startX + float(dominoCount) * spacing + .28f;
        builder.AddWood(glm::vec3(targetX, .14f, 0.f), glm::vec3(.38f, .14f, .42f));
        builder.AddTarget(glm::vec3(targetX, .52f, 0.f), glm::vec3(.28f, .28f, .28f));

        // Side decorations are offset in Z so they frame the run without blocking it.
        builder.AddGlass(glm::vec3(3.2f, .35f, -1.05f), glm::vec3(.28f, .35f, .18f));
        builder.AddGlass(glm::vec3(4.6f, .35f, 1.05f), glm::vec3(.28f, .35f, .18f));
        builder.AddStone(glm::vec3(5.8f, .22f, -1.05f), glm::vec3(.35f, .22f, .35f));
        builder.AddStone(glm::vec3(6.35f, .22f, 1.05f), glm::vec3(.35f, .22f, .35f));
    }
}
