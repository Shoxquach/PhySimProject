#include "Labs/4-Final/Levels/MoatLevel.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void MoatLevel::Setup(World & world, float breakThreshold) const {
        float const t = breakThreshold;

        glm::vec3 const tankCenter(3.4f, 1.0f, 0.f);
        glm::vec3 const tankSize(6.2f, 2.0f, 3.2f);
        world.Fluid.emplace();
        world.Fluid->Density = 1.0f;
        world.Fluid->Init(16, tankCenter, tankSize, glm::vec3(0.94f, 0.5f, 0.94f));
        world.Couple.Buoyancy = true;

        LevelBuilder b(world.Rigid, t);

        for (float x : { 2.4f, 3.2f, 4.0f, 4.8f }) {
            b.AddWood(glm::vec3(x, 1.02f, 0.f), glm::vec3(.42f, .28f, .80f));
        }

        b.AddWood(glm::vec3(2.8f, 1.55f, 0.f), glm::vec3(.25f, .30f, .60f));
        b.AddWood(glm::vec3(4.4f, 1.55f, 0.f), glm::vec3(.25f, .30f, .60f));
        b.AddGlass(glm::vec3(3.6f, 1.95f, 0.f), glm::vec3(.55f, .22f, .60f), 0.7f);
        b.AddTarget(glm::vec3(3.6f, 2.35f, 0.f), glm::vec3(.30f, .30f, .30f), 0.4f);
        b.AddTarget(glm::vec3(2.4f, 1.55f, 0.f), glm::vec3(.27f, .27f, .27f), 0.45f);

        b.AddWood(glm::vec3(5.4f, 1.10f, .9f), glm::vec3(.30f, .30f, .30f));
        b.AddWood(glm::vec3(1.8f, 1.10f, -.9f), glm::vec3(.30f, .30f, .30f));
        b.AddStone(glm::vec3(5.6f, 1.6f, .6f), glm::vec3(.3f, .3f, .3f));
        b.AddStone(glm::vec3(1.4f, 1.6f, -.6f), glm::vec3(.3f, .3f, .3f));
    }
}
