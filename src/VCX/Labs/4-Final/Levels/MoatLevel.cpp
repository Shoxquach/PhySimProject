#include "Labs/4-Final/Levels/MoatLevel.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void MoatLevel::Setup(World & world, float breakThreshold) const {
        float const t = breakThreshold;

        // Tank: water surface ~y=0.55, bridge sits safely above
        glm::vec3 const tankCenter(3.4f, 0.75f, 0.f);
        glm::vec3 const tankSize(6.2f, 1.5f, 3.6f);
        world.Fluid.emplace();
        world.Fluid->Density = 1.0f;
        world.Fluid->Init(16, tankCenter, tankSize, glm::vec3(1.0f, 0.35f, 1.0f));
        world.Couple.Buoyancy = true;

        LevelBuilder b(world.Rigid, t);

        // Static stone pillars on tank floor, supporting the bridge
        for (float x : { 2.4f, 3.2f, 4.0f, 4.8f }) {
            for (float z : { -.95f, .95f }) {
                world.Rigid.AddBox(BodyKind::Stone, glm::vec3(x, .42f, z),
                    glm::vec3(.18f, .42f, .18f), 0.f, Materials::Stone, t * 3.f);
            }
        }

        // Bridge planks on pillars, above water (plank top = 1.05+0.26 = 1.31)
        for (float x : { 2.4f, 3.2f, 4.0f, 4.8f }) {
            b.AddWood(glm::vec3(x, 1.05f, 0.f), glm::vec3(.42f, .26f, 1.30f));
        }

        // Upper bridge: two wood supports on planks (wood top = 1.55+0.28 = 1.83)
        b.AddWood(glm::vec3(2.8f, 1.55f, 0.f), glm::vec3(.25f, .28f, .55f));
        b.AddWood(glm::vec3(4.4f, 1.55f, 0.f), glm::vec3(.25f, .28f, .55f));

        // Glass pane spanning the wood supports (glass bottom=1.92-0.20=1.72 < wood top=1.83 ✓)
        b.AddGlass(glm::vec3(3.6f, 1.92f, 0.f), glm::vec3(.55f, .20f, .80f), 0.7f);

        // Target on glass: glass top=2.12, target halfY=.28 → y=2.32 (bottom=2.04, 0.08 overlap)
        b.AddTarget(glm::vec3(3.6f, 2.32f, 0.f), glm::vec3(.28f, .28f, .28f), 0.38f);

        // Target on bridge plank: plank top=1.31, target halfY=.25 → y=1.50 (bottom=1.25, 0.06 overlap)
        b.AddTarget(glm::vec3(2.4f, 1.50f, 0.f), glm::vec3(.25f, .25f, .25f), 0.42f);

        // Far bank targets on ground-level wood bases — zero interpenetration
        // Wood top=0.70, target halfY=.22 → y=0.86 (bottom=0.64, 0.06 overlap)
        b.AddWood(glm::vec3(5.6f, .35f, .85f), glm::vec3(.35f, .35f, .35f));
        b.AddTarget(glm::vec3(5.6f, .86f, .85f), glm::vec3(.22f, .22f, .22f), 0.45f);

        b.AddWood(glm::vec3(1.4f, .35f, -.85f), glm::vec3(.35f, .35f, .35f));
        b.AddTarget(glm::vec3(1.4f, .86f, -.85f), glm::vec3(.22f, .22f, .22f), 0.45f);
    }
}
