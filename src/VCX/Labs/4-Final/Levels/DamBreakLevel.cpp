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

        // Dam wall: 2 cols × 3 rows of stone, each row overlaps the one below by ~0.05
        // Row 0 top=0.44+0.44=0.88, Row 1 bottom=1.31-0.43=0.88, etc.
        for (int col = 0; col < 2; ++col) {
            float const x = 2.0f + float(col) * 0.78f;
            b.AddStone(glm::vec3(x, 0.44f, 0.f), glm::vec3(.40f, .44f, 1.55f), 1.0f);
            b.AddStone(glm::vec3(x, 1.31f, 0.f), glm::vec3(.40f, .43f, 1.55f), 1.0f);
            b.AddStone(glm::vec3(x, 2.15f, 0.f), glm::vec3(.40f, .43f, 1.55f), 1.0f);
        }

        // Downstream structure: wood bases → glass → target, with proper overlap
        // Wood top=0.32+0.33=0.65, Glass bottom=0.84-0.22=0.62 (0.03 overlap)
        b.AddWood(glm::vec3(4.6f, 0.32f, .45f), glm::vec3(.28f, .33f, .28f));
        b.AddWood(glm::vec3(4.6f, 0.32f, -.45f), glm::vec3(.28f, .33f, .28f));
        // Glass top=0.84+0.22=1.06, Target bottom=1.32-0.28=1.04 (0.02 overlap)
        b.AddGlass(glm::vec3(4.6f, 0.84f, 0.f), glm::vec3(.55f, .22f, .85f), 0.7f);
        b.AddTarget(glm::vec3(4.6f, 1.32f, 0.f), glm::vec3(.28f, .28f, .28f), 0.3f);

        // Right-side: stone base → target
        // Stone top=0.32+0.32=0.64, Target bottom=0.90-0.28=0.62 (0.02 overlap)
        b.AddStone(glm::vec3(5.9f, 0.32f, 0.f), glm::vec3(.50f, .32f, .60f));
        b.AddTarget(glm::vec3(5.9f, 0.90f, 0.f), glm::vec3(.28f, .28f, .28f), 0.3f);

        // Floating target downstream — target density 0.6, 60% submerged, center y=1.97
        b.AddTarget(glm::vec3(3.9f, 1.97f, .8f), glm::vec3(.26f, .28f, .26f), 0.3f);

        // Floating wood debris near slingshot — wood density 0.8 < water 1.0, 80% submerged at equilibrium
        // Water surface ~y=2.03, wood halfY=0.30, center 0.18 below surface → y=1.85
        b.AddWood(glm::vec3(-0.4f, 1.85f, .4f), glm::vec3(.35f, .30f, .40f));
        b.AddWood(glm::vec3(-0.9f, 1.85f, -.4f), glm::vec3(.35f, .30f, .40f));
    }
}
