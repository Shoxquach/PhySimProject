#include "Labs/4-Final/AngryBirdsScene.h"

namespace VCX::Labs::Final {
    int AngryBirdsScene::Reset(AngryBirdsPhysics & physics, float breakThreshold, Level level) const {
        physics.Clear();
        int const birdIndex = physics.AddBird(Anchor);
        switch (level) {
            case Level::ClassicTower:    AddTower(physics, breakThreshold); break;
            case Level::StoneCastle:     AddCastle(physics, breakThreshold); break;
            case Level::TargetPractice:  AddTargetPractice(physics, breakThreshold); break;
        }
        return birdIndex;
    }

    void AngryBirdsScene::AddCastle(AngryBirdsPhysics & physics, float breakThreshold) const {
        glm::vec3 const wood(.62f, .38f, .16f);
        glm::vec3 const glass(.35f, .72f, .95f);
        glm::vec3 const stone(.55f, .55f, .58f);
        glm::vec3 const target(.28f, .75f, .22f);

        float const densityWood = .8f;
        float const densityGlass = .5f;
        float const densityStone = 2.1f;
        float const densityTarget = .6f;

        physics.AddBox(BodyKind::Wood, glm::vec3(3.35f, .25f, 0.f), glm::vec3(1.4f, .25f, .8f), densityWood, wood, breakThreshold);
        physics.AddBox(BodyKind::Stone, glm::vec3(2.6f, 1.45f, 0.f), glm::vec3(.35f, .95f, .35f), densityStone, stone, breakThreshold * 1.5f);
        physics.AddBox(BodyKind::Stone, glm::vec3(4.1f, 1.45f, 0.f), glm::vec3(.35f, .95f, .35f), densityStone, stone, breakThreshold * 1.5f);
        physics.AddBox(BodyKind::Glass, glm::vec3(3.35f, 2.45f, 0.f), glm::vec3(1.2f, .45f, 1.1f), densityGlass, glass, breakThreshold * .72f);
        physics.AddBox(BodyKind::Stone, glm::vec3(3.35f, 3.15f, 0.f), glm::vec3(.45f, .45f, .45f), densityStone, stone, breakThreshold * 1.7f);
        physics.AddBox(BodyKind::Target, glm::vec3(3.35f, .75f, 0.f), glm::vec3(.25f, .25f, .25f), densityTarget, target, breakThreshold * .36f);
    }

    void AngryBirdsScene::AddTargetPractice(AngryBirdsPhysics & physics, float breakThreshold) const {
        glm::vec3 const wood(.62f, .38f, .16f);
        glm::vec3 const glass(.35f, .72f, .95f);
        glm::vec3 const stone(.55f, .55f, .58f);
        glm::vec3 const target(.28f, .75f, .22f);

        float const densityWood = .8f;
        float const densityGlass = .5f;
        float const densityStone = 2.1f;
        float const densityTarget = .6f;

        for (int i = 0; i < 3; ++i) {
            float const x = 3.0f + float(i) * 1.1f;
            physics.AddBox(BodyKind::Stone, glm::vec3(x, .22f, 0.f), glm::vec3(.45f, .22f, .45f), densityStone, stone, breakThreshold * 1.7f);
            physics.AddBox(BodyKind::Target, glm::vec3(x, .72f, 0.f), glm::vec3(.28f, .28f, .28f), densityTarget, target, breakThreshold * .3f);
        }
    }

    void AngryBirdsScene::AddTower(AngryBirdsPhysics & physics, float breakThreshold) const {
        glm::vec3 const wood(.62f, .38f, .16f);
        glm::vec3 const glass(.35f, .72f, .95f);
        glm::vec3 const stone(.55f, .55f, .58f);
        glm::vec3 const target(.28f, .75f, .22f);

        float const densityWood = .8f;
        float const densityGlass = .5f;
        float const densityStone = 2.1f;
        float const densityTarget = .6f;

        for (int i = 0; i < 3; ++i) {
            float const x = 2.8f + float(i) * 1.15f;
            physics.AddBox(BodyKind::Wood, glm::vec3(x, .75f, -.8f), glm::vec3(.28f, .75f, .22f), densityWood, wood, breakThreshold);
            physics.AddBox(BodyKind::Wood, glm::vec3(x, .75f, .8f), glm::vec3(.28f, .75f, .22f), densityWood, wood, breakThreshold);
        }
        physics.AddBox(BodyKind::Glass, glm::vec3(3.95f, 2.15f, 0.f), glm::vec3(1.86f, .65f, 1.2f), densityGlass, glass, breakThreshold * .72f);
        physics.AddBox(BodyKind::Stone, glm::vec3(3.35f, .22f, 0.f), glm::vec3(.55f, .22f, .55f), densityStone, stone, breakThreshold * 1.7f);
        physics.AddBox(BodyKind::Stone, glm::vec3(4.7f, .22f, 0.f), glm::vec3(.55f, .22f, .55f), densityStone, stone, breakThreshold * 1.7f);
        physics.AddBox(BodyKind::Stone, glm::vec3(3.5f, 3.32f, 0.f), glm::vec3(.45f, .52f, .45f), densityStone, stone, breakThreshold * 1.7f);
        physics.AddBox(BodyKind::Stone, glm::vec3(2.05f, .75f, 0.f), glm::vec3(.45f, .75f, .7f), densityStone, stone, breakThreshold * 1.7f);

        physics.AddBox(BodyKind::Target, glm::vec3(3.75f, .72f, 0.f), glm::vec3(.28f, .28f, .28f), densityTarget, glm::vec3(.18f, .82f, .24f), breakThreshold * .36f);
    }
}
