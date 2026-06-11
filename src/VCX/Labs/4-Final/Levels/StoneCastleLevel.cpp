#include "Labs/4-Final/Levels/StoneCastleLevel.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void StoneCastleLevel::Setup(World & world, float breakThreshold) const {
        LevelBuilder builder(world.Rigid, breakThreshold);

        // Main keep
        builder.AddWood(glm::vec3(3.35f, .18f, 0.f), glm::vec3(1.3f, .18f, 1.05f));
        builder.AddStone(glm::vec3(2.45f, 1.28f, -.45f), glm::vec3(.32f, .92f, .3f));
        builder.AddStone(glm::vec3(2.45f, 1.28f, .45f), glm::vec3(.32f, .92f, .3f));
        builder.AddStone(glm::vec3(4.25f, 1.28f, -.45f), glm::vec3(.32f, .92f, .3f));
        builder.AddStone(glm::vec3(4.25f, 1.28f, .45f), glm::vec3(.32f, .92f, .3f));
        builder.AddGlass(glm::vec3(3.35f, 2.48f, 0.f), glm::vec3(1.35f, .28f, .95f));
        builder.AddStone(glm::vec3(3.35f, 3.18f, 0.f), glm::vec3(.48f, .42f, .48f));
        builder.AddStone(glm::vec3(2.5f, 3.08f, 0.f), glm::vec3(.28f, .32f, .42f));
        builder.AddStone(glm::vec3(4.2f, 3.08f, 0.f), glm::vec3(.28f, .32f, .42f));

        // Side towers
        builder.AddStone(glm::vec3(1.6f, .52f, 0.f), glm::vec3(.42f, .52f, .55f));
        builder.AddStone(glm::vec3(1.6f, 1.48f, 0.f), glm::vec3(.34f, .44f, .48f));
        builder.AddGlass(glm::vec3(1.6f, 2.1f, 0.f), glm::vec3(.38f, .18f, .62f));
        builder.AddStone(glm::vec3(1.6f, 2.56f, 0.f), glm::vec3(.38f, .28f, .38f));

        builder.AddStone(glm::vec3(5.1f, .52f, 0.f), glm::vec3(.42f, .52f, .55f));
        builder.AddStone(glm::vec3(5.1f, 1.48f, 0.f), glm::vec3(.34f, .44f, .48f));
        builder.AddGlass(glm::vec3(5.1f, 2.1f, 0.f), glm::vec3(.38f, .18f, .62f));
        builder.AddStone(glm::vec3(5.1f, 2.56f, 0.f), glm::vec3(.38f, .28f, .38f));

        // Extra front and rear cover
        builder.AddStone(glm::vec3(3.35f, .42f, -1.45f), glm::vec3(.75f, .42f, .22f));
        builder.AddStone(glm::vec3(3.35f, .42f, 1.45f), glm::vec3(.75f, .42f, .22f));
        builder.AddWood(glm::vec3(2.15f, .18f, -1.45f), glm::vec3(.45f, .18f, .28f));
        builder.AddWood(glm::vec3(4.55f, .18f, 1.45f), glm::vec3(.45f, .18f, .28f));

        // Targets hidden at different depths/heights
        builder.AddTarget(glm::vec3(3.35f, .61f, 0.f), glm::vec3(.25f, .25f, .25f));
        builder.AddTarget(glm::vec3(1.6f, 3.07f, 0.f), glm::vec3(.23f, .23f, .23f));
        builder.AddTarget(glm::vec3(5.1f, 3.07f, 0.f), glm::vec3(.23f, .23f, .23f));
        builder.AddTarget(glm::vec3(3.35f, 3.82f, 0.f), glm::vec3(.22f, .22f, .22f));
    }
}
