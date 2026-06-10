#include "Labs/4-Final/Levels/GrandCitadelLevel.h"
#include "Labs/4-Final/Levels/LevelCommon.h"

namespace VCX::Labs::Final {
    void GrandCitadelLevel::Setup(World & world, float breakThreshold) const {
        LevelBuilder b(world.Rigid, breakThreshold);

        // Box center positions; bottom = y - halfSize.y. Ground at y = 0.
        // Each stacked block: centerY = supportTop + halfSize.y.

        // ── Ground foundation (7) ─────────────────────────────────────────────
        b.AddStone(glm::vec3(3.5f, .28f, 0.f), glm::vec3(.88f, .28f, .92f));
        b.AddWood(glm::vec3(1.25f, .18f, -.78f), glm::vec3(.44f, .18f, .36f));
        b.AddWood(glm::vec3(1.25f, .18f, .78f), glm::vec3(.44f, .18f, .36f));
        b.AddStone(glm::vec3(2.15f, .24f, 0.f), glm::vec3(.38f, .24f, .44f));
        b.AddWood(glm::vec3(5.75f, .18f, -.78f), glm::vec3(.44f, .18f, .36f));
        b.AddWood(glm::vec3(5.75f, .18f, .78f), glm::vec3(.44f, .18f, .36f));
        b.AddStone(glm::vec3(4.85f, .24f, 0.f), glm::vec3(.38f, .24f, .44f));

        // ── Outer curtain walls on ground (4) ───────────────────────────────
        b.AddStone(glm::vec3(2.15f, .24f, -1.38f), glm::vec3(.58f, .24f, .2f));
        b.AddStone(glm::vec3(4.85f, .24f, 1.38f), glm::vec3(.58f, .24f, .2f));
        b.AddWood(glm::vec3(3.5f, .16f, -1.38f), glm::vec3(.82f, .16f, .2f));
        b.AddWood(glm::vec3(3.5f, .16f, 1.38f), glm::vec3(.82f, .16f, .2f));

        // ── Left bastion columns (6), wing top = 0.36 ───────────────────────
        float const leftX = 1.25f;
        b.AddStone(glm::vec3(leftX, .72f, -.78f), glm::vec3(.36f, .36f, .32f));
        b.AddStone(glm::vec3(leftX, 1.44f, -.78f), glm::vec3(.32f, .36f, .28f));
        b.AddWood(glm::vec3(leftX, 1.9f, -.78f), glm::vec3(.38f, .1f, .34f));
        b.AddStone(glm::vec3(leftX, 2.2f, -.78f), glm::vec3(.3f, .3f, .28f));
        b.AddStone(glm::vec3(leftX, .72f, .78f), glm::vec3(.36f, .36f, .32f));
        b.AddGlass(glm::vec3(leftX, 1.44f, .78f), glm::vec3(.28f, .32f, .26f));

        // ── Right bastion columns (6) ─────────────────────────────────────────
        float const rightX = 5.75f;
        b.AddStone(glm::vec3(rightX, .72f, -.78f), glm::vec3(.36f, .36f, .32f));
        b.AddStone(glm::vec3(rightX, 1.44f, -.78f), glm::vec3(.32f, .36f, .28f));
        b.AddWood(glm::vec3(rightX, 1.9f, -.78f), glm::vec3(.38f, .1f, .34f));
        b.AddStone(glm::vec3(rightX, 2.2f, -.78f), glm::vec3(.3f, .3f, .28f));
        b.AddStone(glm::vec3(rightX, .72f, .78f), glm::vec3(.36f, .36f, .32f));
        b.AddGlass(glm::vec3(rightX, 1.44f, .78f), glm::vec3(.28f, .32f, .26f));

        // ── Cross bridges on bastion L2 tops (y = 1.8) ────────────────────────
        b.AddWood(glm::vec3(2.1f, 1.88f, -.78f), glm::vec3(.52f, .08f, .2f));
        b.AddWood(glm::vec3(2.1f, 1.88f, .78f), glm::vec3(.52f, .08f, .2f));
        b.AddWood(glm::vec3(4.9f, 1.88f, -.78f), glm::vec3(.52f, .08f, .2f));
        b.AddWood(glm::vec3(4.9f, 1.88f, .78f), glm::vec3(.52f, .08f, .2f));

        // ── Central keep on base top 0.56 (6) ─────────────────────────────────
        b.AddStone(glm::vec3(3.5f, .96f, 0.f), glm::vec3(.72f, .4f, .78f));
        b.AddStone(glm::vec3(3.5f, 1.76f, 0.f), glm::vec3(.66f, .4f, .72f));
        b.AddGlass(glm::vec3(3.5f, 2.38f, 0.f), glm::vec3(.88f, .22f, .74f));
        b.AddStone(glm::vec3(3.5f, 2.92f, 0.f), glm::vec3(.58f, .32f, .62f));
        b.AddWood(glm::vec3(3.5f, 3.36f, 0.f), glm::vec3(.62f, .12f, .58f));
        b.AddStone(glm::vec3(3.5f, 3.74f, 0.f), glm::vec3(.48f, .26f, .48f));

        // ── Inner buttresses on plinth tops 0.48 (4) ──────────────────────────
        b.AddWood(glm::vec3(2.15f, .84f, 0.f), glm::vec3(.34f, .36f, .4f));
        b.AddStone(glm::vec3(2.15f, 1.44f, 0.f), glm::vec3(.3f, .24f, .36f));
        b.AddWood(glm::vec3(4.85f, .84f, 0.f), glm::vec3(.34f, .36f, .4f));
        b.AddStone(glm::vec3(4.85f, 1.44f, 0.f), glm::vec3(.3f, .24f, .36f));

        // ── Side spurs on base top 0.56, clear of keep footprint (3) ──────────
        b.AddWood(glm::vec3(2.55f, .7f, -.62f), glm::vec3(.14f, .14f, .18f));
        b.AddWood(glm::vec3(4.45f, .7f, .62f), glm::vec3(.14f, .14f, .18f));
        b.AddStone(glm::vec3(3.5f, .16f, -1.05f), glm::vec3(.38f, .16f, .22f));

        // 40 structural blocks total

        // ── Targets (6), each bottom flush on support top ─────────────────────
        b.AddTarget(glm::vec3(3.5f, .54f, -1.05f), glm::vec3(.22f, .22f, .22f));     // on front spur top 0.32
        b.AddTarget(glm::vec3(1.25f, 2.7f, -.78f), glm::vec3(.2f, .2f, .2f));        // on left cap top 2.5
        b.AddTarget(glm::vec3(5.75f, 2.7f, .78f), glm::vec3(.2f, .2f, .2f));         // on right cap top 2.5
        b.AddTarget(glm::vec3(3.5f, 4.2f, 0.f), glm::vec3(.2f, .2f, .2f));           // on spire top 4.0
        b.AddTarget(glm::vec3(2.15f, 1.88f, 0.f), glm::vec3(.2f, .2f, .2f));         // on left buttress top 1.68
        b.AddTarget(glm::vec3(4.85f, 1.88f, 0.f), glm::vec3(.2f, .2f, .2f));         // on right buttress top 1.68
    }
}
