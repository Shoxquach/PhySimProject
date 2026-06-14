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

        // ── Stone architrave spanning the pillars (above water) ──
        // Pillar top=1.80, stone halfY=0.14 → bottom=1.98-0.14=1.84 > 1.80 (sits on pillars)
        b.AddStone(glm::vec3(3.5f, 1.98f, -1.1f), glm::vec3(1.15f, .14f, .22f), 1.8f);
        b.AddStone(glm::vec3(3.5f, 1.98f, 1.1f), glm::vec3(1.15f, .14f, .22f), 1.8f);

        // ── Wood platform across the architraves ──
        // Architrave top=1.98+0.14=2.12, wood bottom=2.20-0.12=2.08 (0.04 overlap)
        b.AddWood(glm::vec3(3.5f, 2.20f, 0.f), glm::vec3(1.20f, .12f, 1.10f));

        // ── Small shrine on the platform ──
        // Platform top=2.20+0.12=2.32, glass bottom=2.62-0.18=2.44 (no contact with platform!)
        // Need glass to rest ON platform. Platform top=2.32. Glass bottom should be ~2.30.
        // Glass: y=2.30+0.18=2.48. Top=2.66.
        b.AddGlass(glm::vec3(3.5f, 2.48f, 0.f), glm::vec3(.35f, .18f, .40f), 0.75f);
        // Target on shrine: glass top=2.66, target halfY=0.22 → y=2.84 (bottom=2.62, 0.04 overlap)
        b.AddTarget(glm::vec3(3.5f, 2.84f, 0.f), glm::vec3(.22f, .22f, .22f), 0.35f);

        // ── Two smaller platform pillars at the sides ──
        // Left side pillar + target
        world.Rigid.AddBox(BodyKind::Stone, glm::vec3(1.6f, .65f, -.70f),
            glm::vec3(.20f, .65f, .20f), 0.f, Materials::Stone, t * 3.f);
        b.AddWood(glm::vec3(1.6f, 1.40f, -.70f), glm::vec3(.30f, .10f, .32f));
        b.AddTarget(glm::vec3(1.6f, 1.70f, -.70f), glm::vec3(.20f, .20f, .20f), 0.40f);

        // Right side pillar + target
        world.Rigid.AddBox(BodyKind::Stone, glm::vec3(5.4f, .65f, .70f),
            glm::vec3(.20f, .65f, .20f), 0.f, Materials::Stone, t * 3.f);
        b.AddWood(glm::vec3(5.4f, 1.40f, .70f), glm::vec3(.30f, .10f, .32f));
        b.AddTarget(glm::vec3(5.4f, 1.70f, .70f), glm::vec3(.20f, .20f, .20f), 0.40f);

        // ── Floating debris on the water surface ──
        // Water surface y=1.80. Wood density 0.8 → 80% submerged. Wood height=0.36. Center below surface: 0.36*(0.8-0.5)=0.108
        float const floatY = 1.80f - 0.108f;  // y=1.692
        b.AddWood(glm::vec3(2.0f, floatY, 1.20f), glm::vec3(.30f, .18f, .35f));
        b.AddWood(glm::vec3(5.0f, floatY, -1.20f), glm::vec3(.35f, .18f, .40f));

        // ── Floating target on debris ──
        // Target density 0.6 → 60% submerged. Height=0.40. Center below surface: 0.40*(0.6-0.5)=0.04
        b.AddTarget(glm::vec3(3.1f, 1.80f - 0.04f, 1.30f), glm::vec3(.20f, .20f, .20f), 0.38f);

        // ── Underwater target: rests on tank floor ──
        // On the floor (y=0), halfY=0.25 → top at 0.50 (deep underwater at surface=1.80)
        b.AddTarget(glm::vec3(4.2f, .25f, 0.f), glm::vec3(.25f, .25f, .25f), 0.50f);

        // ── Stone rubble scattered on the tank floor ──
        b.AddStone(glm::vec3(2.8f, .18f, -.40f), glm::vec3(.22f, .18f, .18f), 2.0f);
        b.AddStone(glm::vec3(4.2f, .14f, .60f), glm::vec3(.18f, .14f, .22f), 2.0f);
        b.AddStone(glm::vec3(3.0f, .16f, -.85f), glm::vec3(.16f, .16f, .20f), 2.0f);
    }
}
