#include "Labs/4-Final/Levels/DamBreakLevel.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void DamBreakLevel::Setup(World & world, float breakThreshold) const {
        float const t = breakThreshold;

        glm::vec3 const tankCenter(2.0f, 1.3f, 0.f);
        glm::vec3 const tankSize(9.0f, 2.6f, 3.2f);
        world.Fluid.emplace();
        world.Fluid->Density = 1.0f;
        world.Fluid->Init(16, tankCenter, tankSize, glm::vec3(0.5f, 0.78f, 0.92f));
        world.Couple.Buoyancy  = true;
        world.Couple.FlowSolid = true;

        LevelBuilder b(world.Rigid, t);

        float const damTough = t * 1.0f;
        for (int col = 0; col < 2; ++col) {
            float const x = 2.0f + float(col) * 0.78f;
            for (int row = 0; row < 3; ++row) {
                float const y = 0.42f + float(row) * 0.84f;
                b.AddStone(glm::vec3(x, y, 0.f), glm::vec3(.40f, .42f, 1.55f), 1.0f);
            }
        }

        b.AddWood(glm::vec3(4.6f, 0.32f, .45f), glm::vec3(.28f, .32f, .28f));
        b.AddWood(glm::vec3(4.6f, 0.32f, -.45f), glm::vec3(.28f, .32f, .28f));
        b.AddGlass(glm::vec3(4.6f, 0.86f, 0.f), glm::vec3(.55f, .22f, .85f), 0.7f);
        b.AddTarget(glm::vec3(4.6f, 1.36f, 0.f), glm::vec3(.28f, .28f, .28f), 0.3f);

        b.AddStone(glm::vec3(5.9f, 0.30f, 0.f), glm::vec3(.50f, .30f, .60f));
        b.AddTarget(glm::vec3(5.9f, 0.88f, 0.f), glm::vec3(.28f, .28f, .28f), 0.3f);
        b.AddTarget(glm::vec3(3.9f, 0.26f, .8f), glm::vec3(.26f, .26f, .26f), 0.3f);

        b.AddWood(glm::vec3(-0.4f, 1.7f, .4f), glm::vec3(.35f, .30f, .40f));
        b.AddWood(glm::vec3(-0.9f, 1.7f, -.4f), glm::vec3(.35f, .30f, .40f));
    }
}
