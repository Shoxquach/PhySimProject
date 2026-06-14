#include "Labs/4-Final/Levels/SunkenTempleLevel.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void SunkenTempleLevel::Setup(World & world, float breakThreshold) const {
        float const t = breakThreshold;

        // Large temple tank: water surface at ~y=1.80
        glm::vec3 const tankCenter(3.5f, 1.5f, 0.f);
        glm::vec3 const tankSize(8.0f, 3.0f, 4.0f);
        world.Fluid.emplace();
        world.Fluid->Density = 1.0f;
        world.Fluid->Init(16, tankCenter, tankSize, glm::vec3(1.0f, 0.60f, 1.0f));
        world.Couple.Buoyancy = true;

        LevelBuilder b(world.Rigid, t);

        // ── Temple: four tall stone pillars rising from the tank floor ──
        // Pillar top = 0.90+0.90 = 1.80, just at water surface
        for (float px : { 2.5f, 4.5f }) {
            for (float pz : { -1.1f, 1.1f }) {
                world.Rigid.AddBox(BodyKind::Stone, glm::vec3(px, .90f, pz),
                    glm::vec3(.25f, .90f, .25f), 0.f, Materials::Stone, t * 3.f);
            }
        }

        // ── Stone architrave spanning the pillars (static, visibly seated on top) ──
        // Pillar top=1.80, beam bottom=1.78 gives a small overlap.
        world.Rigid.AddBox(BodyKind::Stone, glm::vec3(3.5f, 1.94f, -1.1f),
            glm::vec3(1.15f, .16f, .22f), 0.f, Materials::Stone, t * 3.f);
        world.Rigid.AddBox(BodyKind::Stone, glm::vec3(3.5f, 1.94f, 1.1f),
            glm::vec3(1.15f, .16f, .22f), 0.f, Materials::Stone, t * 3.f);

        // ── Wood platform across the architraves ──
        // Beam top=2.10, platform bottom=2.08 for stable contact.
        b.AddWood(glm::vec3(3.5f, 2.20f, 0.f), glm::vec3(1.24f, .12f, 1.14f));

        // ── Small shrine on the platform ──
        // Platform top=2.20+0.12=2.32, glass bottom=2.62-0.18=2.44 (no contact with platform!)
        // Need glass to rest ON platform. Platform top=2.32. Glass bottom should be ~2.30.
        // Glass: y=2.30+0.18=2.48. Top=2.66.
        b.AddGlass(glm::vec3(3.5f, 2.48f, 0.f), glm::vec3(.35f, .18f, .40f), 0.75f);
        // Target on shrine: glass top=2.66, target halfY=0.22 → y=2.84 (bottom=2.62, 0.04 overlap)
        b.AddTarget(glm::vec3(3.5f, 2.84f, 0.f), glm::vec3(.22f, .22f, .22f), 0.35f);

        // ── Two smaller side platforms with proper supports ──
        world.Rigid.AddBox(BodyKind::Stone, glm::vec3(1.6f, .65f, -.70f),
            glm::vec3(.20f, .65f, .20f), 0.f, Materials::Stone, t * 3.f);
        world.Rigid.AddBox(BodyKind::Stone, glm::vec3(1.6f, 1.28f, -.70f),
            glm::vec3(.34f, .08f, .34f), 0.f, Materials::Stone, t * 3.f);
        b.AddWood(glm::vec3(1.6f, 1.45f, -.70f), glm::vec3(.30f, .10f, .32f));
        b.AddTarget(glm::vec3(1.6f, 1.73f, -.70f), glm::vec3(.20f, .20f, .20f), 0.40f);

        world.Rigid.AddBox(BodyKind::Stone, glm::vec3(5.4f, .65f, .70f),
            glm::vec3(.20f, .65f, .20f), 0.f, Materials::Stone, t * 3.f);
        world.Rigid.AddBox(BodyKind::Stone, glm::vec3(5.4f, 1.28f, .70f),
            glm::vec3(.34f, .08f, .34f), 0.f, Materials::Stone, t * 3.f);
        b.AddWood(glm::vec3(5.4f, 1.45f, .70f), glm::vec3(.30f, .10f, .32f));
        b.AddTarget(glm::vec3(5.4f, 1.73f, .70f), glm::vec3(.20f, .20f, .20f), 0.40f);

        // ── Stable elevated targets ──
        // Water surface is around y=1.80; these pedestals keep targets above water at start.
        world.Rigid.AddBox(BodyKind::Stone, glm::vec3(2.0f, .92f, 1.20f),
            glm::vec3(.24f, .92f, .28f), 0.f, Materials::Stone, t * 3.f);
        world.Rigid.AddBox(BodyKind::Stone, glm::vec3(2.0f, 1.88f, 1.20f),
            glm::vec3(.38f, .08f, .38f), 0.f, Materials::Stone, t * 3.f);
        b.AddTarget(glm::vec3(2.0f, 2.14f, 1.20f), glm::vec3(.20f, .20f, .20f), 0.38f);

        world.Rigid.AddBox(BodyKind::Stone, glm::vec3(5.0f, .92f, -1.20f),
            glm::vec3(.28f, .92f, .30f), 0.f, Materials::Stone, t * 3.f);
        world.Rigid.AddBox(BodyKind::Stone, glm::vec3(5.0f, 1.88f, -1.20f),
            glm::vec3(.40f, .08f, .40f), 0.f, Materials::Stone, t * 3.f);
        b.AddTarget(glm::vec3(5.0f, 2.14f, -1.20f), glm::vec3(.20f, .20f, .20f), 0.38f);

        world.Rigid.AddBox(BodyKind::Stone, glm::vec3(4.2f, .94f, 0.f),
            glm::vec3(.30f, .94f, .30f), 0.f, Materials::Stone, t * 3.f);
        world.Rigid.AddBox(BodyKind::Stone, glm::vec3(4.2f, 1.92f, 0.f),
            glm::vec3(.44f, .08f, .44f), 0.f, Materials::Stone, t * 3.f);
        b.AddTarget(glm::vec3(4.2f, 2.25f, 0.f), glm::vec3(.25f, .25f, .25f), 0.50f);

        // Static rubble on the tank floor for visual depth.
        world.Rigid.AddBox(BodyKind::Stone, glm::vec3(2.8f, .18f, -.40f),
            glm::vec3(.22f, .18f, .18f), 0.f, Materials::Stone, t * 3.f);
        world.Rigid.AddBox(BodyKind::Stone, glm::vec3(4.2f, .14f, .60f),
            glm::vec3(.18f, .14f, .22f), 0.f, Materials::Stone, t * 3.f);
        world.Rigid.AddBox(BodyKind::Stone, glm::vec3(3.0f, .16f, -.85f),
            glm::vec3(.16f, .16f, .20f), 0.f, Materials::Stone, t * 3.f);
    }
}
