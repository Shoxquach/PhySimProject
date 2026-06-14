#include "Labs/4-Final/Levels/OverhangFortLevel.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void OverhangFortLevel::Setup(World & world, float breakThreshold) const {
        float const t = breakThreshold;
        LevelBuilder b(world.Rigid, t);

        // ── MAIN KEEP (x≈1.5): 4-pillar solid base ──
        // Row 0 — stone base. Top=0.32+0.32=0.64
        for (float px : { 1.2f, 1.8f }) {
            for (float pz : { -.60f, .60f }) {
                b.AddStone(glm::vec3(px, .32f, pz), glm::vec3(.22f, .32f, .22f), 1.8f);
            }
        }
        // Row 1 — stone pillars. Bottom=0.95-0.33=0.62 (< row-0 top=0.64, overlap 0.02)
        for (float px : { 1.2f, 1.8f }) {
            for (float pz : { -.60f, .60f }) {
                b.AddStone(glm::vec3(px, .95f, pz), glm::vec3(.20f, .33f, .20f), 1.6f);
            }
        }
        // Wood platform on top of row-1 pillars. Pillar top=1.28, plank bottom=1.42-0.12=1.30 (overlap 0.02)
        b.AddWood(glm::vec3(1.5f, 1.42f, 0.f), glm::vec3(.90f, .12f, .85f));
        // Target on main keep: platform top=1.54, target bottom=1.68-0.18=1.50 (overlap 0.04)
        b.AddTarget(glm::vec3(1.5f, 1.68f, 0.f), glm::vec3(.18f, .18f, .18f), 0.42f);

        // ── OVERHANG WING (x≈4.5): only 2 pillars — deliberately fragile ──
        // Row 0 — stone base. Top=0.64
        b.AddStone(glm::vec3(4.2f, .32f, -.35f), glm::vec3(.22f, .32f, .22f), 1.8f);
        b.AddStone(glm::vec3(4.8f, .32f, .35f), glm::vec3(.22f, .32f, .22f), 1.8f);
        // Row 1 — stone pillars. Top=0.95+0.33=1.28
        b.AddStone(glm::vec3(4.2f, .95f, -.35f), glm::vec3(.20f, .33f, .20f), 1.6f);
        b.AddStone(glm::vec3(4.8f, .95f, .35f), glm::vec3(.20f, .33f, .20f), 1.6f);
        // Wood platform. Top=1.42+0.12=1.54
        b.AddWood(glm::vec3(4.5f, 1.42f, 0.f), glm::vec3(.70f, .12f, .65f));
        // Target on overhang
        b.AddTarget(glm::vec3(4.5f, 1.68f, 0.f), glm::vec3(.18f, .18f, .18f), 0.42f);

        // ── GLASS BRIDGE connecting the two sections — THE WEAK POINT ──
        // Main platform right edge: 1.5+0.90=2.40. Overhang left edge: 4.5-0.70=3.80.
        // Glass: center(3.1, 1.66, 0), halfX=0.75 → x:2.35-3.85. Overlaps both platforms.
        // Platform top=1.54. Glass bottom=1.66-0.14=1.52. Overlap 0.02.
        b.AddGlass(glm::vec3(3.1f, 1.66f, 0.f), glm::vec3(.75f, .14f, .40f), 0.6f);
        // Target on glass bridge: glass top=1.80, target bottom=1.93-0.16=1.77 (overlap 0.03)
        b.AddTarget(glm::vec3(3.1f, 1.93f, 0.f), glm::vec3(.16f, .16f, .16f), 0.36f);

        // ── Side targets on the far edges ──
        // Left of main keep
        b.AddWood(glm::vec3(0.3f, .35f, -.70f), glm::vec3(.28f, .35f, .28f));
        b.AddTarget(glm::vec3(0.3f, .88f, -.70f), glm::vec3(.16f, .16f, .16f), 0.40f);
        // Right of overhang
        b.AddWood(glm::vec3(5.8f, .35f, .70f), glm::vec3(.28f, .35f, .28f));
        b.AddTarget(glm::vec3(5.8f, .88f, .70f), glm::vec3(.16f, .16f, .16f), 0.40f);

        // ── Ground-level target behind low cover ──
        b.AddWood(glm::vec3(3.1f, .16f, -.95f), glm::vec3(.45f, .16f, .16f));
        b.AddTarget(glm::vec3(3.1f, .48f, -.95f), glm::vec3(.18f, .18f, .18f), 0.45f);

        // ── Decorative scattered rubble ──
        b.AddStone(glm::vec3(1.0f, .10f, .85f), glm::vec3(.16f, .10f, .16f), 2.5f);
        b.AddStone(glm::vec3(5.0f, .12f, -.85f), glm::vec3(.18f, .12f, .14f), 2.5f);
        b.AddWood(glm::vec3(3.1f, .14f, .75f), glm::vec3(.22f, .14f, .18f));
    }
}
